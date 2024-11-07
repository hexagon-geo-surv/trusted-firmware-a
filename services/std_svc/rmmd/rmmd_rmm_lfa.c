/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/el3_runtime/context_mgmt.h>
#include <services/lfa_svc.h>
#include <services/lfa_holding_pen.h>
#include <services/rmmd_rmm_lfa.h>
#include <services/rmmd_svc.h>

#include <plat/common/platform.h>

static int32_t lfa_rmm_prime(struct lfa_activation *activation)
{
	/**
	 * LFA is assumed to be preloaded and authenticated so proceed
	 * without PRIME
	 */
	return LFA_SUCCESS;
}

static int32_t lfa_rmm_activate(struct lfa_activation *activation,
				uint64_t ep_address, uint64_t context_id)
{
	int ret = LFA_SUCCESS;

	if (lfa_holding_start()) {
		/* Leader CPU */
		INFO("LFA_ACTIVATE: Last CPU proceed with activation\n");

		cm_el2_sysregs_context_save(NON_SECURE);
		ret = rmmd_primary_warm_reset();
		cm_el2_sysregs_context_restore(NON_SECURE);

		cm_set_next_eret_context(NON_SECURE);

		if (ret == LFA_SUCCESS) {
			VERBOSE("Successful activation of component %d\n", activation->component_id);
		} else {
			ERROR("Failed to activate component %d\n", activation->component_id);
			ret = LFA_BUSY;
		}

		lfa_holding_release(ret);

		// TODO: avoid race condition trying to prime next firmware
		// before secondary activations are finished.
		lfa_reset_activation();
	} else {
		ret = lfa_holding_wait();

		if (ret == LFA_SUCCESS) {
			cm_el2_sysregs_context_save(NON_SECURE);
			ret = rmmd_secondary_warm_reset();
			cm_el2_sysregs_context_restore(NON_SECURE);

			cm_set_next_eret_context(NON_SECURE);
		}
	}

	return ret;
}

static struct lfa_activator_fns rmm_activator = {
	.prime = lfa_rmm_prime,
	.activate = lfa_rmm_activate,
	.may_reset_cpu = 0,
	.cpu_rendezvous_required = 1,
};

struct lfa_activator_fns* get_rmm_activator(void)
{
	return &rmm_activator;
}
