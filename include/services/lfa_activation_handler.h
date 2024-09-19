/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LFA_ACTIVATION_HANDLER_H
#define LFA_ACTIVATION_HANDLER_H

#include <stdbool.h>
#include <stdint.h>

#define PRIME_NONE		U(0)

struct lfa_activation {
	uint32_t component_id;
	int prime_status;
};

typedef int32_t (*component_prime_fn)(struct lfa_activation *activation);
typedef int32_t (*component_activate_fn)(struct lfa_activation *activation,
					 uint64_t ep_address,
					 uint64_t context_id);
struct lfa_activator_fns {
	component_prime_fn prime;
	component_activate_fn activate;
	bool may_reset_cpu;
	bool cpu_rendezvous_required;
};

#endif /* LFA_ACTIVATION_HANDLER_H */
