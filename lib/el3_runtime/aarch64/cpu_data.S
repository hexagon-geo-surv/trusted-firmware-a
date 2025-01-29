/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPU_DATA_S
#define CPU_DATA_S

#include <asm_macros.S>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/per_cpu/per_cpu.S>

.globl	_cpu_data_by_index
.globl	get_this_cpu_data
.globl	get_for_cpu_data

/* -----------------------------------------------------------------
 * cpu_data_t *_cpu_data_by_index(uint32_t cpu_index)
 *
 * Return the cpu_data structure for the CPU with given linear index
 *
 * This can be called without a valid stack.
 * clobbers: x0, x1
 * -----------------------------------------------------------------
 */
func _cpu_data_by_index
	mov_imm	x1, CPU_DATA_SIZE
	mul	x0, x0, x1
	adrp	x1, percpu_data
	add	x1, x1, :lo12:percpu_data
	add	x0, x0, x1
	ret
endfunc _cpu_data_by_index

/* -----------------------------------------------------------------
 * cpu_data_t * get_this_cpu_data(void)
 *
 * Return the cpu_data structure for the calling CPU
 *
 * This can be called without a valid stack.
 * clobbers: x0 - x2
 * -----------------------------------------------------------------
 */
func get_this_cpu_data
	this_cpu_ptr percpu_data
	ret
endfunc get_this_cpu_data

/* -----------------------------------------------------------------
 * cpu_data_t * get_this_cpu_data(uint32_t cpu_index)
 *
 * Return the cpu_data structure for the CPU with given linear index
 *
 * This can be called without a valid stack.
 * clobbers: x0 - x3
 * -----------------------------------------------------------------
 */
func get_for_cpu_data
	for_cpu_ptr percpu_data x0
	ret
endfunc get_for_cpu_data

#endif /* CPU_DATA_S */
