/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>
#include <lib/psci/psci_lib.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <services/lfa_holding_pen.h>

#include <platform_def.h>

/* These symbols are needed so the relocatable code can be loaded. */
IMPORT_SYM(uintptr_t, __LFA_RELOCATABLE_CODE_START__,	LFA_RELOCATABLE_CODE_START);
IMPORT_SYM(uintptr_t, __LFA_RELOCATABLE_CODE_END__,	LFA_RELOCATABLE_CODE_END);
IMPORT_SYM(uintptr_t, __LFA_RELOCATABLE_LMA__,		LFA_RELOCATABLE_LMA);
IMPORT_SYM(uintptr_t, __LFA_RELOCATABLE_DATA_START__,	LFA_RELOCATABLE_DATA_START);
IMPORT_SYM(uintptr_t, __LFA_RELOCATABLE_DATA_END__,	LFA_RELOCATABLE_DATA_END);

static spinlock_t holding_lock;
static spinlock_t activation_lock;
static volatile uint32_t activation_count;
static enum lfa_retc activation_status;

/**
 * lfa_holding_start - Called by each active CPU to coordinate live activation.
 *
 * Note that only CPUs that are active at the time of activation will
 * participate in CPU rendezvous.
 *
 * This function is invoked by each CPU participating in the LFA Activate
 * process. It increments the shared activation count under `activation_lock`
 * to track how many CPUs have entered the activation phase.
 *
 * The first CPU to enter acquires the `holding_lock`, which ensures
 * serialization during the wait and activation phases. This lock is
 * released only after the last CPU completes the activation.
 *
 * The function returns `true` only for the last CPU to enter, allowing it
 * to proceed with performing the live firmware activation. All other CPUs
 * receive `false` and will wait in `lfa_holding_wait()` until activation
 * is complete.
 *
 * In the case of a BL31 live activation, this function can also initialize the
 * relocatable holding pen.
 *
 * @return `true` for the last CPU, `false` for all others.
 */
bool lfa_holding_start(bool relocate) {
	unsigned int no_of_cpus = psci_num_cpus_running_on_safe(plat_my_core_pos());

	/*
	 * This lock ensures only one CPU uses the activation_count
	 * variable at a time.
	 */
	spin_lock(&activation_lock);

	/* First CPU to arrive locks the holding pen. */
	if (activation_count == 0U) {
		spin_lock(&holding_lock);
	}

	activation_count += 1U;

	/* Release the lock once count is updated. */
	spin_unlock(&activation_lock);

	/* If we are not using a relocated holding pen, then we're done. */
	if (!relocate) {
		return (activation_count == no_of_cpus);
	}

	/* Last CPU to reach this point handles the activation. */
	if (activation_count == no_of_cpus) {
		VERBOSE("Core %d performing activation.\n", plat_my_core_pos());

		/*
		 * Only one core will load the relocatable code module and
		 * acquire the relocatable lock, then release the rest of the
		 * cores which will then wait for us to release the relocatable
		 * lock once the activation is complete.
		 */
		 lfa_load_relocatable();
		 lfa_r_holding_lock(&lfa_r_holding_lock_var);
		 spin_unlock(&holding_lock);
		 return true;

	} else {
		/* Wait until we are released by the primary core. */
		spin_lock(&holding_lock);
		spin_unlock(&holding_lock);
		return false;
	}
}

/**
 * lfa_holding_wait - CPUs wait until activation is completed by the last CPU.
 *
 * All CPUs are serialized using `holding_lock`, which is initially acquired
 * by the first CPU in `lfa_holding_start()` and only released by the last
 * CPU through `lfa_holding_release()`. This ensures that no two CPUs enter
 * the critical section at the same time during the wait phase. Once the
 * last CPU completes activation, each CPU decrements the activation count
 * and returns the final activation status,  which was set by the last CPU
 * to complete the activation process.
 *
 * @return Activation status set by the last CPU.
 */
enum lfa_retc lfa_holding_wait(void)
{
	spin_lock(&holding_lock);
	activation_count -= 1U;
	spin_unlock(&holding_lock);
	return activation_status;
}

/**
 * lfa_holding_release - Called by the last CPU to complete activation.
 *
 * This function is used by the last participating CPU after it completes
 * live firmware activation. It updates the shared activation status and
 * resets the activation count. Finally, it releases the `holding_lock` to
 * allow other CPUs that were waiting in `lfa_holding_wait()` to proceed.
 *
 * @param status Activation status to be shared with other CPUs.
 */
void lfa_holding_release(enum lfa_retc status)
{
	activation_count = 0U;
	activation_status = status;
	spin_unlock(&holding_lock);
}

void lfa_load_relocatable(void)
{
	/*
	 * The relocatable code area is outside of the normal BL31 image, so
	 * before we can use it we have to make it writeable then copy the
	 * relocatable code into it. Then once the copy is complete we must
	 * mark it read only and executable again.
	 */
	xlat_change_mem_attributes(LFA_RELOCATABLE_CODE_START,
				   (LFA_RELOCATABLE_DATA_START -LFA_RELOCATABLE_CODE_START),
				   MT_MEMORY | MT_RW | MT_EXECUTE_NEVER | EL3_PAS);

	memcpy((void *)LFA_RELOCATABLE_CODE_START, (void *)LFA_RELOCATABLE_LMA,
	       (uintptr_t)(LFA_RELOCATABLE_DATA_START - LFA_RELOCATABLE_CODE_START));

	xlat_change_mem_attributes(LFA_RELOCATABLE_CODE_START,
				   (LFA_RELOCATABLE_DATA_START - LFA_RELOCATABLE_CODE_START),
				   MT_MEMORY | MT_RO | MT_EXECUTE | EL3_PAS);

	/*
	 * The relocatable data section contains unknown information at this
	 * point so zero it out before we use it.
	 *
	 * TODO do we actually need to do this?
	 */
	memset((void *)LFA_RELOCATABLE_DATA_START, 0,
	       (LFA_RELOCATABLE_DATA_END - LFA_RELOCATABLE_DATA_START));
}
