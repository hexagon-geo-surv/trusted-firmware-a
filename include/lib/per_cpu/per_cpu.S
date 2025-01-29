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

.globl plat_get_node_base

/* -----------------------------------------------------------------
 * Gets the base address of .per_cpu section. When NUMA awareness is
 * enabled, it is the platforms responsibility to implement
 * plat_per_cpu_section_base.
 *
 * args - none
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
endfunc plat_get_node_base

/* -----------------------------------------------------------------
 *	Expects cpu num to be passed in _cpu.
 *	returns cpu specfic per cpu section's offset address in x0.
 *	Clobbers: x0 - x3
 * -----------------------------------------------------------------
 */
.macro  per_cpu_pe_addr _cpu
#if NUMA_AWARE_PER_CPU
	/* Get base address of per_cpu section */
	mov	x3, \_cpu
	bl	plat_get_node_base
	mov	x1, x0
	mov	x0, #PER_CPU_NODE_CORE_COUNT
	udiv	x2, x3, x0
	/* x3 = cpu % PER_CPU_NODE_CORE_COUNT */
	msub	x3, x2, x0, x3
#else
	adrp	x1, __PER_CPU_START__
	add	x1, x1, :lo12:__PER_CPU_START__
	mov	x3, \_cpu
#endif /* NUMA_AWARE_PER_CPU */

	/* Compute size of per_cpu section */
	ldr x2, = __PER_CPU_UNIT_SECTION_SIZE__

	/* Find current cpu's offset */
	madd	x0, x3, x2, x1
.endm

/* -----------------------------------------------------------------
 *	Expects object name to be passed in _label. Returns objects
 *	address in x0.
 *	Clobbers: x0 - x2
 * -----------------------------------------------------------------
 */
.macro this_cpu_ptr _label
	mrs	x0, tpidr_el3
	per_cpu_object_addr x0, \_label
.endm

/* -------------------------------------------------------------------
 *	Expects object name to be passed in _label. Expects cpu num to be
 *	passed in _cpu. Returns objects address in x0.
 *	Clobbers: x0 - x3
 * -------------------------------------------------------------------
 */
.macro for_cpu_ptr _label, _cpu
	per_cpu_pe_addr \_cpu
	per_cpu_object_addr x0, \_label
.endm

/* -------------------------------------------------------------------
 *	Expects pe's address to be passed in _base. Expects
 *	object name to be passed in _label. Returns objects
 *	absolute address in x0.
 *	Clobbers: x0 - x2
 * -------------------------------------------------------------------
 */
.macro per_cpu_object_addr _base, _label
	adrp	x1, \_label
	add	x1, x1, :lo12:\_label
	adrp	x2, __PER_CPU_START__
	add	x2, x2, :lo12:__PER_CPU_START__
	sub	x1, x1, x2
	/* x0 (absolute address) = \_base + relative_add */
	add	x0, \_base, x1
.endm

#endif /* PER_CPU_S*/
