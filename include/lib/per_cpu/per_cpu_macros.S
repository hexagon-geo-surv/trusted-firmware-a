/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PER_CPU_MACROS_S
#define PER_CPU_MACROS_S

#include <arch.h>
#include <asm_macros.S>
#include <lib/per_cpu/per_cpu_defs.h>


/* -----------------------------------------------------------------
 *
 * cpu_data_t * base_this_cpu_ptr(void)
 *
 * Return the base per-cpu object pointer for the calling CPU
 *
 * This can be called without a valid stack.
 * Clobbers: None
 * -----------------------------------------------------------------
 */
.macro base_this_cpu_ptr _reg
	mrs \_reg, tpidr_el3
.endm

/* -----------------------------------------------------------------
 * cpu_data_t * base_for_cpu_ptr(uint32_t cpu_index)
 *
 * Return the base per-cpu object pointer for a given cpu index
 *
 * This can be called without a valid stack.
 * clobbers: x0 - x3
 * -----------------------------------------------------------------
 */
.macro base_for_cpu_ptr _label, _cpu
	for_cpu_ptr \_label, \_cpu
.endm

/* -----------------------------------------------------------------
 *	Expects object name to be passed in _label.
 *      Returns objects address in x0.
 *	Clobbers: x0 - x2
 * -----------------------------------------------------------------
 */
.macro this_cpu_ptr _label
	mrs	x0, tpidr_el3
	per_cpu_object_addr x0, \_label
.endm

/* -------------------------------------------------------------------
 *	Expects object name to be passed in _label.
 *      Expects cpu num to be passed in _cpu.
 *      Returns objects address in x0.
 *	Clobbers: x0 - x3
 * -------------------------------------------------------------------
 */
.macro for_cpu_ptr _label, _cpu
	per_cpu_pe_addr \_cpu
	per_cpu_object_addr x0, \_label
.endm

/* -----------------------------------------------------------------
 *	Expects cpu num to be passed in _cpu.
 *	returns cpu specfic per cpu section's offset address in x0.
 *	Clobbers: x0 - x3
 * -----------------------------------------------------------------
 */
.macro  per_cpu_pe_addr _cpu
	mov	x3, \_cpu
#if NUMA_AWARE_PER_CPU
	/* Get base address of per_cpu section */
	bl	plat_get_node_base
	mov	x1, x0
	mov	x0, #PER_CPU_NODE_CORE_COUNT
	udiv	x2, x3, x0
	/* x3 = cpu % PER_CPU_NODE_CORE_COUNT */
	msub	x3, x2, x0, x3
#else
	adrp	x1, __PER_CPU_START__
	add	x1, x1, :lo12:__PER_CPU_START__
#endif /* NUMA_AWARE_PER_CPU */

	/* Compute size of per_cpu section */
	ldr x2, = __PER_CPU_UNIT_SECTION_SIZE__

	/* Find current cpu's offset */
	madd	x0, x3, x2, x1
.endm

/* -------------------------------------------------------------------
 *	Expects pe's per-cpu region address to be passed in _base.
 *      Expects object name to be passed in _label.
 *      Returns objects absolute address in x0.
 *	Clobbers: x0 - x2
 * -------------------------------------------------------------------
 */
.macro per_cpu_object_addr _base, _label
	mov	x0, \_base
	adrp	x1, \_label
	add	x1, x1, :lo12:\_label
	adrp	x2, __PER_CPU_START__
	add	x2, x2, :lo12:__PER_CPU_START__
	sub	x1, x1, x2
	/* x0 (absolute address) = \_base + relative_add */
	add	x0, \_base, x1
.endm

#endif
