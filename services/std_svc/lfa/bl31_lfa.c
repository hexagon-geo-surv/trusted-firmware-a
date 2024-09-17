/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <services/bl31_lfa.h>
#include <services/lfa_svc.h>

static int32_t lfa_bl31_prime(struct lfa_activation *activation)
{
        return LFA_WRONG_STATE;
}

static int32_t lfa_bl31_activate(struct lfa_activation *activation,
				 uint64_t ep_address,
				 uint64_t context_id)
{
	return LFA_WRONG_STATE;
}

static struct lfa_activator_fns bl31_activator = {
        .prime = lfa_bl31_prime,
        .activate = lfa_bl31_activate,
};

struct lfa_activator_fns* get_bl31_activator(void)
{
        return &bl31_activator;
}
