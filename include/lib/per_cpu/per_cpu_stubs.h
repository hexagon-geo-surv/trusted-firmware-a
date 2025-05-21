/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PER_CPU_STUBS_H
#define PER_CPU_STUBS_H

#include <plat/common/platform.h>

/*******************************************************************************
 * per-cpu definer and accessor interfaces
 ******************************************************************************/

/* Declare a per-CPU object */
#define DECLARE_PER_CPU(TYPE, NAME)				\
	extern TYPE NAME[PLATFORM_CORE_COUNT]

/* Define a per-CPU object */
#define DEFINE_PER_CPU(TYPE, NAME)				\
	TYPE NAME[PLATFORM_CORE_COUNT]

/* Declare the first per-cpu object in the entire per-cpu region */
#define DECLARE_PER_CPU_BASE(TYPE, NAME)			\
	DECLARE_PER_CPU(TYPE, NAME)

/* Define the first per-cpu object in the entire per-cpu region */
#define DEFINE_PER_CPU_BASE(TYPE, NAME)				\
	DEFINE_PER_CPU(TYPE, NAME)

/* Get a pointer to a per-CPU object for a given CPU */
#define FOR_CPU_PTR(NAME, CPU)					\
	(&(NAME)[CPU])

/* Get a pointer to the base per-cpu object for a given CPU */
#define BASE_FOR_CPU_PTR(NAME, CPU)				\
	FOR_CPU_PTR(NAME, CPU)

/* Get a per-CPU object value for a given CPU */
#define FOR_CPU(NAME, CPU)					\
	((NAME)[CPU])

/* Get a pointer to a per-CPU object for the current CPU */
#define THIS_CPU_PTR(NAME)					\
	({							\
		uintptr_t core_pos = plat_my_core_pos();	\
		&(NAME)[core_pos];				\
	})

/* Get a pointer to the base per-cpu object for the current CPU */
#define BASE_THIS_CPU_PTR(NAME)					\
	THIS_CPU_PTR(NAME)

/* Get a per-CPU object for the current CPU (lvalue-safe) */
#define GET_THIS_CPU(NAME)					\
	(*({							\
		uintptr_t __core_pos = plat_my_core_pos();	\
		&(NAME)[__core_pos];				\
	}))

/* Put a per-CPU object for the current CPU (returns rvalue) */
#define PUT_THIS_CPU(NAME)					\
	({							\
	uintptr_t core_pos = plat_my_core_pos();		\
	(NAME)[core_pos];					\
	})

#endif /* PER_CPU_STUBS_H */
