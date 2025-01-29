/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PER_CPU_H
#define PER_CPU_H

#if defined (IMAGE_BL31)
#include <lib/per_cpu/per_cpu_impl.h>
#else
#include <lib/per_cpu/per_cpu_stubs.h>
#endif /* IMAGE_BL31 */

#endif /* PER_CPU_H */
