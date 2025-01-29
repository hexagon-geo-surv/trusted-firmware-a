/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PER_CPU_S
#define PER_CPU_S

#include <arch.h>
#include <asm_macros.S>
#include <lib/per_cpu/per_cpu_defs.h>
#include <lib/per_cpu/per_cpu_macros.S>

.globl per_cpu_init
.globl plat_get_node_base

/* -----------------------------------------------------------------
 * Gets the base address of .per_cpu section. When NUMA awareness is
 * enabled, it is the platforms responsibility to implement
 * plat_per_cpu_section_base.
 *
 * This function would be called by asm and C routines. If NUMA awareness is
 * enabled, care must be taken to preserve the clobber list.
 *
 * args - cpu in x0
 * ret  - per cpu section address in x0.
 * -----------------------------------------------------------------
 */
func plat_get_node_base
#if !NUMA_AWARE_PER_CPU
	adrp	x0, __PER_CPU_START__
	add	x0, x0, :lo12:__PER_CPU_START__
#else
	b	plat_per_cpu_section_base
#endif
	ret
endfunc plat_get_node_base

/* -----------------------------------------------------------------
 *	Populates tpidr_el3  with cpu specfic per cpu section's
 *	offset address.
 *	Clobbers: x0 - x3
 * -----------------------------------------------------------------
 */
func per_cpu_init
	mov	x10, x30
	bl	plat_my_core_pos
	per_cpu_pe_addr x0
	/* Update tpidr_el3 with offset */
	msr	tpidr_el3, x0
	mov	x30, x10
	ret
endfunc per_cpu_init

#endif /* PER_CPU_S*/
