/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <lib/cassert.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/per_cpu/per_cpu.h>

/* The per_cpu_ptr_cache_t space allocation */
DEFINE_PER_CPU(cpu_data_t, percpu_data);
