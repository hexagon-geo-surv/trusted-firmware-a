/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RMMD_RMM_LFA_H
#define RMMD_RMM_LFA_H

#include <services/lfa_activation_handler.h>

struct lfa_activator_fns* get_rmm_activator(void);

#endif /* RMMD_RMM_LFA_H */
