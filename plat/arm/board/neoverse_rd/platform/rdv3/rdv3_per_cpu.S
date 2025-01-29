/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RDV3_PER_CPU_S
#define RDV3_PER_CPU_S

#include <asm_macros.S>
#include <lib/per_cpu/per_cpu_defs.h>

.globl plat_zero_init_per_cpu_section
.globl plat_per_cpu_section_base

/* ----------------------------------------------------------------------------
 * Zero-initialization all nodes .per_cpu section
 * Clobbers : x0 - x3, x10, x30
 * ----------------------------------------------------------------------------
*/
func plat_zero_init_per_cpu_section
	mov	x10, x30
	/* Zeromem requires x1 to be populated with the size */
	ldr	x1,= __PER_CPU_SIZE__

	/* Load the base address of array */
	adrp	x2, per_cpu_remote_section_base
	add	x2, x2, :lo12:per_cpu_remote_section_base

	/* Start with 1st node */
	mov	x3, #1
1:
	/* Compare with number of nodes */
	cmp	x3, #NRD_CHIP_COUNT
	bge	2f

	/* Calculate the address of base[x0] */
	/* Assuming uintptr_t is 8 bytes */
	add	x2, x2, x3, lsl #3
	ldr	x0, [x2]
	bl	zeromem
	add	x3, x3, #1
	b	1b

2:
	mov	x30, x10
	ret
endfunc plat_zero_init_per_cpu_section

/* ----------------------------------------------------------------------------
 * Calculate per cpu section base for particular cpu.
 * Clobbers : x0 - x1
 * return per cpu section base address in x0
 * ----------------------------------------------------------------------------
*/
func plat_per_cpu_section_base
	mov	x1, #PLATFORM_NODE_CORE_COUNT
	/* x0 = cpu_idx(x0) / (PLATFORM_NODE_CORE_COUNT) */
	udiv	x0, x0, x1

	/* Load the base address of array */
	adrp	x1, per_cpu_remote_section_base
	add	x1, x1, :lo12:per_cpu_remote_section_base

	/* Calculate the address of base[x0] */
	/* Assuming uintptr_t is 8 bytes */
	add	x1, x1, x0, lsl #3
	/* Loading x0 with particular per cpu section base address in x0 */
	ldr	x0, [x1]
	ret
endfunc plat_per_cpu_section_base

#endif /* RDV3_PER_CPU_S*/
