/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PER_CPU_IMPL_H
#define PER_CPU_IMPL_H

#include <plat/common/platform.h>
#include <common/bl_common.h>
#include <lib/per_cpu/per_cpu_defs.h>

/* section where per-cpu objects using per-cpu framework would reside */
#define PER_CPU_SECTION_NAME	".per_cpu"

/* per cpu section size for a single CPU/PE */
#define PER_CPU_UNIT_SIZE	(PER_CPU_END_UNIT_CLA - PER_CPU_START_UNIT)
#define PER_CPU_OFFSET(x)	(x - PER_CPU_START)

/*******************************************************************************
 * per-cpu definer and accessor interfaces
 ******************************************************************************/

/* Declare a per-CPU object */
#define DECLARE_PER_CPU(TYPE, NAME)				\
	extern TYPE NAME

/* Define a per-CPU object */
#define DEFINE_PER_CPU(TYPE, NAME)				\
	TYPE NAME						\
	__section(PER_CPU_SECTION_NAME)

/* Get a pointer to a per-CPU object for a given CPU */
#define FOR_CPU_PTR(NAME, CPU) __extension__			\
	((__typeof__(&NAME))					\
	(plat_get_node_base(CPU) +				\
	PER_CPU_OFFSET((uintptr_t)&NAME) +			\
	((CPU % PER_CPU_NODE_CORE_COUNT) * PER_CPU_UNIT_SIZE)))

/* Get a per-CPU object value for a given CPU */
#define FOR_CPU(NAME, CPU) __extension__			\
	(*(__typeof__(&NAME))					\
	(plat_get_node_base(CPU) +				\
	PER_CPU_OFFSET((uintptr_t)&NAME) +			\
	(CPU % PER_CPU_NODE_CORE_COUNT) * PER_CPU_UNIT_SIZE))

/* Get a pointer to a per-CPU object for the current CPU */
#define THIS_CPU_PTR(NAME) __extension__			\
	({							\
		((__typeof__(&NAME))				\
		(read_tpidr_el3() +				\
		PER_CPU_OFFSET((uintptr_t)&NAME)));		\
	})

/* Get a per-CPU object for the current CPU (lvalue-safe) */
#define GET_THIS_CPU(NAME) __extension__			\
	(*(__typeof__(&NAME))					\
	(read_tpidr_el3() +					\
	PER_CPU_OFFSET((uintptr_t)&NAME)))

/* Put a per-CPU object for the current CPU (returns rvalue) */
#define PUT_THIS_CPU(NAME) __extension__			\
	({							\
		(*(__typeof__(&NAME))				\
		(read_tpidr_el3() +				\
		PER_CPU_OFFSET((uintptr_t)&NAME)));		\
	})

/*******************************************************************************
 * Functions
 ******************************************************************************/

uintptr_t plat_get_node_base(int cpu);

#endif /* PER_CPU_IMPL_H */
