/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>

#include <common/tbbr/tbbr_img_def.h>
#include <services/bl31_lfa.h>
#include <services/lfa_svc.h>
#include <services/rmmd_rmm_lfa.h>
#include <smccc_helpers.h>

#include <plat/common/platform.h>

/*
 * TODO: This is to be in-sync with platform chosen the number of
 * firmware to be live activated
 **/
#define MAX_LFA_COMPONENT_COUNT		20

uint32_t lfa_component_count = 0U;
lfa_component_t lfa_components[MAX_LFA_COMPONENT_COUNT];
struct lfa_activation current_activation;

void lfa_reset_activation()
{
	memset(&current_activation, 0, sizeof(current_activation));
	current_activation.component_id = U(0xFFFFFFFF);
	current_activation.prime_status = PRIME_NONE;
}

static void lfa_register_component(uint32_t image_id, const uuid_t *uuid,
				   struct lfa_activator_fns *activator)
{
	assert(lfa_component_count <
		sizeof(lfa_components)/sizeof(lfa_components[0]));

	lfa_components[lfa_component_count].image_id = image_id;
	lfa_components[lfa_component_count].uuid = *uuid;
	lfa_components[lfa_component_count].activator = activator;
	lfa_components[lfa_component_count].activation_pending = false;
	lfa_component_count += 1;
}

static int lfa_cancel(uint32_t component_id)
{
	int ret = LFA_SUCCESS;

	if (lfa_component_count == 0U) {
		return LFA_WRONG_STATE;
	}

	/* Check if component ID is in range. */
	if (component_id >= lfa_component_count) {
		return LFA_INVALID_ARGUMENTS;
	}

	/* TODO: add proper termination prime and activate phases */
	lfa_reset_activation();

	return ret;
}

static void lfa_initialize_components()
{
	struct lfa_activator_fns *activator;
	const uuid_t *uuid;
	uint32_t image_id;

	/*
	 * Retrieve the number of images, their UUIDs, and image IDs from the
	 * platform, and register these components for live activation.
	 **/
	for (uint32_t index = 0U;; index++) {
		image_id = plat_get_lfa_image_info(index, &uuid);

		if (image_id == UINT32_MAX) {
			/* break if there are no further images */
			break;
		}

		switch (image_id) {
			case BL31_IMAGE_ID:
				activator = get_bl31_activator();
				break;
#if ENABLE_RME
			case RMM_IMAGE_ID:
				activator = get_rmm_activator();
				break;
#endif /* ENABLE_RME */
			default:
				activator = NULL;
				break;
		}

		/* Register the image that the platform claims */
		lfa_register_component(image_id, uuid, activator);
	}
}

static void lfa_set_fw_active_pend(uint32_t fw_seq_id)
{
	/* grab the UUID of the component */
	uint32_t image_id = lfa_components[fw_seq_id].image_id;

	/*
	 * check the presence of update of the given image_id in the
	 * platform specific way.
	 * */
	bool fw_update_avail = is_plat_fw_update_avail(image_id);

	lfa_components[fw_seq_id].activation_pending = fw_update_avail;
}

static int lfa_activate(uint32_t component_id, uint64_t ep_address,
			uint64_t context_id, uint64_t __unused *flags)
{
	int ret = LFA_SUCCESS;
	struct lfa_activator_fns *activator;

	if (lfa_component_count == 0U) {
		return LFA_COMPONENT_WRONG_STATE;
	}

	/* Check if fw_seq_id is in range. */
	if (component_id >= lfa_component_count) {
		return LFA_INVALID_ARGUMENTS;
	}

	if (lfa_components[component_id].activator == NULL) {
		return LFA_NOT_SUPPORTED;
	}

	if (!lfa_components[component_id].activation_pending) {
		return LFA_COMPONENT_WRONG_STATE;
	}

	if (current_activation.component_id != component_id) {
		return LFA_INVALID_ARGUMENTS;
	}

	if (current_activation.prime_status != PRIME_COMPLETE) {
		return LFA_COMPONENT_WRONG_STATE;
	}

	activator = lfa_components[component_id].activator;
	if (activator->activate != NULL) {
                /* Pass skip_cpu_rendezvous (flag[0]) only if flag[0]==1 & CPU_RENDEZVOUS is not required */
                if(*flags & 0x1) {
                        if(!activator->cpu_rendezvous_required) {
                                INFO("Skipping rendezvous requested by caller.\n");
                                current_activation.cpu_rendezvous = 0;
                        }
                        /* Return error if caller tries to skip rendezvous when it is required */
                        else {
                                ERROR("Cannot skip CPU Rendezvous when it is required.\n");
                                return LFA_INVALID_ARGUMENTS;
                        }
                }

		ret = activator->activate(&current_activation, ep_address,
					  context_id);
	}

	return ret;
}

static int lfa_prime(uint32_t component_id, uint64_t *flags)
{
	int ret = LFA_SUCCESS;
	struct lfa_activator_fns *activator;

	if (lfa_component_count == 0U) {
		return LFA_WRONG_STATE;
	}

	/* Check if fw_seq_id is in range. */
	if (component_id >= lfa_component_count) {
		return LFA_INVALID_ARGUMENTS;
	}

	if (lfa_components[component_id].activator == NULL) {
		return LFA_NOT_SUPPORTED;
	}

	if (!lfa_components[component_id].activation_pending) {
		return LFA_WRONG_STATE;
	}

	if (current_activation.prime_status == PRIME_NONE) {
		current_activation.component_id = component_id;
		current_activation.prime_status = PRIME_STARTED;
	} else if (current_activation.prime_status == PRIME_STARTED) {
		if (current_activation.component_id != component_id) {
			return LFA_WRONG_STATE;
		}
	} else if (current_activation.prime_status == PRIME_COMPLETE) {
		return LFA_WRONG_STATE;
	}

	activator = lfa_components[component_id].activator;
	if (activator->prime != NULL) {
		ret = activator->prime(&current_activation);
		if (ret != 0) {
			return ret;
		}
	}

	ret = plat_lfa_load_auth_image(lfa_components[component_id].image_id);
	if (ret != 0) {
		if (ret == -EAUTH) {
			return LFA_AUTH_ERROR;
		} else if (ret == -ENOMEM) {
			return LFA_NO_MEMORY;
		} else {
			return LFA_DEVICE_ERROR;
		}
	}

	current_activation.prime_status = PRIME_COMPLETE;

	/* TODO: split this into multiple PRIME calls */
	*flags = 0;

	return ret;
}

uint64_t lfa_smc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			 u_register_t x3, u_register_t x4, void *cookie,
			 void *handle, u_register_t flags)
{
	uint64_t retx1, retx2;
	uint64_t lfa_flags = 0U;
	uint8_t *uuid_p;
	uint32_t ret;

	switch (smc_fid) {
	case LFA_SVC_VERSION:
		SMC_RET1(handle, LFA_VERSION);
		break;

	case LFA_SVC_FEATURES:
		SMC_RET1(handle, is_lfa_fid(x1) ? 0 : LFA_NOT_SUPPORTED);
		break;

	case LFA_SVC_GET_INFO:
		if (x1 == 0) {
			if (lfa_component_count == 0U) {
				lfa_initialize_components();
				lfa_reset_activation();
			}
			SMC_RET3(handle, LFA_SUCCESS, lfa_component_count, 0);
		} else {
			SMC_RET1(handle, LFA_INVALID_ARGUMENTS);
		}
		break;

	case LFA_SVC_GET_INVENTORY:
		if (lfa_component_count == 0U) {
			SMC_RET1(handle, LFA_WRONG_STATE);
		}

		/*
		 * Check if fw_seq_id is in range. LFA_GET_INFO must be called first to scan
		 * platform firmware and create a valid number of firmware components.
		 */
		if (x1 >= lfa_component_count) {
			SMC_RET1(handle, LFA_INVALID_ARGUMENTS);
		}

		/*
		 * grab the UUID of asked fw_seq_id and set the return UUID
		 * variables
		 */
		uuid_p = (uint8_t *)&lfa_components[x1].uuid;
		memcpy(&retx1, uuid_p, sizeof(uint64_t));
		memcpy(&retx2, uuid_p + sizeof(uint64_t), sizeof(uint64_t));

		/*
		 * check the given fw_seq_id's update available
		 * and accordingly set the active_pending flag
		 * */
		lfa_set_fw_active_pend((uint32_t)x1);

		if (lfa_components[x1].activator != NULL) {
			INFO("Component %lu supports live activation:\n", x1);
			INFO("  Activation pending: %s\n",
				lfa_components[x1].activation_pending ?
				"true" : "false");
		} else {
			INFO("Component %ld does not support live activation:\n", x1);
		}

		INFO("  x1 = 0x%016lx, x2 = 0x%016lx, \n", retx1, retx2);

		SMC_RET4(handle, LFA_SUCCESS, retx1, retx2,
			 (uint64_t)((lfa_components[x1].activator == NULL) ? 0 : 1) |
			 (uint64_t)(lfa_components[x1].activation_pending) << 1 |
			 (uint64_t)(lfa_components[x1].activator->may_reset_cpu) << 2 |
			 (uint64_t)!(lfa_components[x1].activator->cpu_rendezvous_required) << 3);

		break;

	case LFA_SVC_PRIME:
		ret = lfa_prime(x1, &lfa_flags);
		if (ret != LFA_SUCCESS) {
			SMC_RET1(handle, ret);
		} else {
			SMC_RET2(handle, ret, lfa_flags);
		}

		break;

	case LFA_SVC_ACTIVATE:
		ret = lfa_activate(x1, x2, x3, &lfa_flags);
		SMC_RET1(handle, ret);

		break;

	case LFA_SVC_CANCEL:
		ret = lfa_cancel(x1);

		SMC_RET1(handle, ret);
		break;

	default:
		WARN("Unimplemented LFA Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
		break; /* unreachable */

	}

	SMC_RET1(handle, SMC_UNK);

	return 0;
}
