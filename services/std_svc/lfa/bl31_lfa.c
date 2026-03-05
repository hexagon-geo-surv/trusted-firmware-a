/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/build_message.h>
#include <common/debug.h>
#include <errno.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <services/bl31_lfa.h>
#include <services/lfa_holding_pen.h>
#include <services/lfa_svc.h>

#if SEPARATE_CODE_AND_RODATA
static const __attribute__((unused)) uintptr_t CODE_START = (uintptr_t)__TEXT_START__;
static const __attribute__((unused)) uintptr_t CODE_END = (uintptr_t)__RODATA_END__;
#else
static const __attribute__((unused)) uintptr_t CODE_START = (uintptr_t)__CODE_START__;
static const __attribute__((unused)) uintptr_t CODE_END = (uintptr_t)__CODE_END__;
#endif

/* Static function prototypes. */
static int lfa_bl31_check(uintptr_t base_va, size_t size,uint64_t **entry);
static int lfa_r_bl31_copy(uintptr_t va, uint64_t *entry, void *src, size_t size, bool reset);

int32_t lfa_bl31_prime(struct lfa_component_status *activation)
{
	// TODO: Check that activation->image_size is not too large.
	return LFA_SUCCESS;
}

/* Function that actually performs the activation of the firmware image */
static int lfa_bl31_image_activation(struct lfa_component_status *activation,
	uint64_t ep_address, uint64_t context_id)
{
	VERBOSE("Starting BL31 Image Activation (core %d)\n", plat_my_core_pos());
	VERBOSE("  Address: 0x%lX\n", activation->image_address);
	VERBOSE("  Size:    %ld bytes\n", activation->image_size);
	VERBOSE("  Reset:   %s\n", activation->reset ? "yes" : "no");

#if !ENABLE_RUNTIME_INSTRUMENTATION
	/*
	 * Used to avoid console corruption. Not used if measuring performance
	 * since printouts are removed. Without it, N1SDP may not finish the LFA
	 * process. Multiple reboots may be required to get the proper
	 * measurements/process
	 */
	console_flush();
#endif

	uint64_t *entry = NULL;
	if (lfa_bl31_check(CODE_START, CODE_END - CODE_START, &entry)) {
		lfa_r_holding_unlock(&lfa_r_holding_lock_var);
		return LFA_CRITICAL_ERROR;
	}

	/* Prepare for warm reset and setup NS entrypoint for primary cores */
	if (activation->reset) {
		if (prepare_warm_reset(ep_address, context_id,
				(uint64_t)&lfa_r_holding_lock_var) != LFA_SUCCESS) {
			lfa_r_holding_unlock(&lfa_r_holding_lock_var);
			return LFA_CRITICAL_ERROR;
		}
	}

	/*
	 * Reset is initiated inside lfa_r_bl31_copy if required and will not
	 * return to this point if enabled
	 */
	lfa_r_bl31_copy(CODE_START, entry, (void *)activation->image_address, activation->image_size, activation->reset);

	return LFA_SUCCESS;
}

int32_t lfa_bl31_activate(struct lfa_component_status *activation,
	uint64_t ep_address, uint64_t context_id)
{
	uint32_t core_pos = plat_my_core_pos();
	lfa_r_ep_addresses[core_pos] = ep_address;
	lfa_r_context_ids[core_pos] = context_id;

	int ret = LFA_SUCCESS;

	/* Case when Firmware requires CPU Rendezvous for LFA Activation */
	if(activation->cpu_rendezvous) {
		/*
		 * Only one core will return true from this function and it
		 * controls the activation process.
		 */
		if(lfa_holding_start(true)) {
			VERBOSE("BL31 LFA: CPU Leader (%lu bytes)\n", activation->image_size);
			VERBOSE("BL31 LFA: CPU Leader Core %d /w core_id: %lu (%lu bytes)\n", plat_my_core_pos(), read_mpidr_el1(), activation->image_size);

			/* Get image information from the platform. */
			ret = plat_lfa_get_image_info(activation->component_id, &activation->image_address, &activation->image_size);
			if (ret) {
				ERROR("BL31 LFA: Could not get image info!\n");
				return ret;
			}

			/*
			 * Perform image activation. Will not return if reset
			 * requested.
			 */
			ret = lfa_bl31_image_activation(activation, ep_address, context_id);

			/* Unlock holding pen for all other cores */
			lfa_r_holding_unlock(&lfa_r_holding_lock_var);

			if (ret) {
				ERROR("BL31 LFA: ailed image activation with error: %d\n", ret);
				return ret;
			} else {
				INFO("BL31 LFA: Successful image activation!\n");
			}
		} else {
			if (activation->reset) {
				/*
				 * Prepare for warm reset and setup NS
				 * entrypoint for secondary cores
				 */
				if (prepare_warm_reset(ep_address, context_id,
						(uint64_t)&lfa_r_holding_lock_var) != LFA_SUCCESS) {
					return LFA_CRITICAL_ERROR;
				}

				/*
				 * Wait until activation of the image is
				 * complete and warm reset occurs on main core
				 * then reset.
				 */
				lfa_r_holding_wait_warm_reset(&lfa_r_holding_lock_var);
			} else {
				/*
				 * Wait until main core is completed with
				 * activation
				 */
				lfa_r_holding_wait(&lfa_r_holding_lock_var);
			}
		}
	}

	/*
	* No CPU rendezvous required. Perform LFA Activate with single core
	* NOTE: Assumption that only one core will call activate with
	* CPU_RENDEZVOUS=0
	*/
	else {
		/*
		* Loading relocatable module normally happens in the holding
		* pen. Since there is no holding pen without CPU rendezvous we
		* are doing it here instead.
		*/
		lfa_load_relocatable();

		/* Perform actual image activation */
		ret = lfa_bl31_image_activation(activation, ep_address, context_id);
		if(ret) {
			ERROR("Failed image activation with error: %d\n", ret);
			return ret;
		}
	}

	NOTICE("BL31 Live Activation Without Reset %u\n", plat_my_core_pos());
	NOTICE("  Version : %s\n", build_version_string);
	NOTICE("  %s\n", build_message);

	return ret;
}

struct lfa_component_ops bl31_activator = {
	.prime = lfa_bl31_prime,
	.activate = lfa_bl31_activate,
	.may_reset_cpu = true,
	.cpu_rendezvous_required = true,
};

struct lfa_component_ops *get_bl31_activator(void) {
	return &bl31_activator;
}

uint64_t *lfa_get_xlat_table_entry(uintptr_t virtual_addr, const xlat_ctx_t *ctx)
{
	uint64_t *table = ctx->base_table;
	unsigned int entries = ctx->base_table_entries;
	unsigned int start_level = GET_XLAT_TABLE_LEVEL_BASE((unsigned long long)ctx->va_max_address + 1ULL);

	for (unsigned int level = start_level; level <= XLAT_TABLE_LEVEL_MAX; level++) {
		uint64_t idx, desc, desc_type;

		idx = XLAT_TABLE_IDX(virtual_addr, level);
		if (idx >= entries) {
			return NULL;
		}

		desc = table[idx];
		desc_type = desc & DESC_MASK;

		if (desc_type == INVALID_DESC) {
			return NULL;
		}

		if (level == XLAT_TABLE_LEVEL_MAX) {
			/*
			 * Only page descriptors allowed at the final lookup
			 * level.
			 */
			assert(desc_type == PAGE_DESC);
			return &table[idx];
		}

		if (desc_type == BLOCK_DESC) {
			return &table[idx];
		}

		assert(desc_type == TABLE_DESC);
		table = (uint64_t *)(uintptr_t)(desc & TABLE_ADDR_MASK);
		entries = XLAT_TABLE_ENTRIES;
	}

	return NULL;
}

int lfa_bl31_check(uintptr_t base_va, size_t size, uint64_t **entry)
{
	xlat_ctx_t *ctx = get_xlat_tables();
	assert(ctx != NULL);
	assert(ctx->initialized);

	if (!IS_PAGE_ALIGNED(base_va)) {
		WARN("%s: Address 0x%lx is not aligned on a page boundary.\n", __func__, base_va);
		return -EINVAL;
	}

	if (size == 0U) {
		WARN("%s: Size is 0.\n", __func__);
		return -EINVAL;
	}

	if ((size % PAGE_SIZE) != 0U) {
		WARN("%s: Size 0x%zx is not a multiple of a page size.\n", __func__, size);
		return -EINVAL;
	}

	size_t pages_count = size / PAGE_SIZE;

	VERBOSE("BL31 update will impact %zu pages starting from address 0x%lx...\n",
		pages_count, base_va);

	*entry = lfa_get_xlat_table_entry(base_va, ctx);

	uint64_t *last = lfa_get_xlat_table_entry(base_va + pages_count * PAGE_SIZE, ctx);

	/* Check that all descriptors are in the same page. */
	if ((((uint64_t)*entry) >> 12) != ((((uint64_t)last) >> 12))) {
		ERROR("All page descriptors are not part of the same page.\n");
		return -EINVAL;
	}

	return 0;
}

__attribute__((noinline, section(".lfa_relocatable_code")))
int lfa_r_bl31_copy(uintptr_t base_va, uint64_t *entry, void *src, size_t size, bool reset)
{
	uint64_t *current = entry;
	uintptr_t va = base_va;

	/* Remove the RO attribute from each page descriptor. */
	for (size_t remaining = size; remaining > PAGE_SIZE;
		remaining -= PAGE_SIZE, va += PAGE_SIZE, current++) {
		uint64_t desc = *current;
		*current = desc & ~LOWER_ATTRS(AP_RO);
		dsbishst();
		tlbivae3is(TLBI_ADDR(va));
	}
	dsbish();
	isb();

	/* Copy over the new BL31 image. */
	for (uint64_t i = 0; i < size; i++) {
		((uint8_t *)base_va)[i] = ((uint8_t *)src)[i];
	}

	/* Add the RO attribute back to each page descriptor. */
	current = entry;
	va = base_va;
	for (size_t remaining = size; remaining > PAGE_SIZE;
		remaining -= PAGE_SIZE, va += PAGE_SIZE, current++) {
		uint64_t desc = *current;
		*current = desc | LOWER_ATTRS(AP_RO);
		dsbishst();
		tlbivae3is(TLBI_ADDR(va));
	}
	dsbish();
	isb();

	/* If reset is requested, spin in WFI loop. */
	while(reset) {
		wfi();
	}

	return 0;
}
