/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PER_CPU_DEFS_H
#define PER_CPU_DEFS_H

#include <platform_def.h>

/*
 * The NUMA aware per-cpu framework uses the term NODE to denote a group of CPUs
 * that have uniform proximity to a given memory node. Without NUMA awareness,
 * all CPUs on the system fall within the same NODE. With NUMA awareness, it is
 * the platforms responsibility to define how CPUs are grouped on a per NODE
 * basis.
 */
#if NUMA_AWARE_PER_CPU
/*
 * In NUMA aware per-cpu framework, PLATFORM_NODE_CORE_COUNT must be defined as
 * the maximum core count among all nodes. For example, if nodes have 10, 12,
 * 18, and 12 cores, then PLATFORM_NODE_CORE_COUNT should be set to 18. This
 * ensures that the linker allocates sufficient space in the .per_cpu section
 * for PER-CPU data across all nodes, preventing under-allocation and alignment
 * issues.
 *
 * Proper PLATFORM_NODE_CORE_COUNT definition guarantees that the .per_cpu
 * section in the linker configuration is correctly sized for the entire system,
 * ensuring optimal performance in NUMA environments.
 */
#define PER_CPU_NODE_CORE_COUNT	(PLATFORM_NODE_CORE_COUNT)
#else
/*
 * If the platform wishes to not enable NUMA awareness for per-cpu objects,
 * PLATFORM_CORE_COUNT would be picked up as the number of cores available in
 * the platform. This would mean that all per-cpu objects are placed at the
 * same memory where .per_cpu section would reside.
 */
#define PER_CPU_NODE_CORE_COUNT	(PLATFORM_CORE_COUNT)
#endif

#endif
