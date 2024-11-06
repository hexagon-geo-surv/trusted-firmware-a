/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <platform_def.h>
#include <services/lfa_holding_pen.h>
#include <services/lfa_svc.h>

static spinlock_t holding_lock;
static spinlock_t activation_lock;
uint32_t activation_count = 0;
uint32_t activation_status = LFA_SUCCESS;

/**
 * This function is called by each CPU during the LFA Activate process to
 * increment the activation count while holding the activation lock.
 *
 * The first CPU acquires the holding lock, which is released once the
 * firmware activation is completed by the last CPU.
 *
 * This function returns `false` for all CPUs except the last one, for
 * which it returns `true`, allowing it to proceed with the live activation
 * of the firmware.
 */
bool lfa_holding_start()
{
	spin_lock(&activation_lock);

	if (activation_count == 0) {
		/* First CPU locks holding lock */
		spin_lock(&holding_lock);
	}

	activation_count += 1;
	if (activation_count < PLATFORM_CORE_COUNT) {
		VERBOSE("Hold, %d CPU left\n", PLATFORM_CORE_COUNT - activation_count);
		spin_unlock(&activation_lock);
		return false;
	} else {
		spin_unlock(&activation_lock);
		return true;
	}
}

/**
 * CPUs, wait for the lock until activation is completed by the last CPU.
 * Once done, decrement the activation count and return the activation
 * status from the last CPU.
 */
int lfa_holding_wait()
{
	spin_lock(&holding_lock);
	activation_count -= 1;
	spin_unlock(&holding_lock);
	return activation_status;
}

/**
 * Used by the last CPU to update its activation status.
 * Once live activation is complete on that CPU, it releases
 * the holding lock for other CPUs.
 */
void lfa_holding_release(int status)
{
	activation_status = status;
	spin_unlock(&holding_lock);
}
