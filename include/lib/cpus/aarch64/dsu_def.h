/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DSU_DEF_H
#define DSU_DEF_H

#include <lib/utils_def.h>

/********************************************************************
 * DSU Cluster Configuration registers definitions
 ********************************************************************/
#define CLUSTERCFR_EL1		S3_0_C15_C3_0

#define CLUSTERCFR_ACP_SHIFT	U(11)

/********************************************************************
 * DSU Cluster Main Revision ID registers definitions
 ********************************************************************/
#define CLUSTERIDR_EL1		S3_0_C15_C3_1

#define CLUSTERIDR_REV_SHIFT	U(0)
#define CLUSTERIDR_REV_BITS	U(4)
#define CLUSTERIDR_VAR_SHIFT	U(4)
#define CLUSTERIDR_VAR_BITS	U(4)

/********************************************************************
 * DSU Cluster Auxiliary Control registers definitions
 ********************************************************************/
#define CLUSTERACTLR_EL1	S3_0_C15_C3_3

#define CLUSTERACTLR_EL1_DISABLE_CLOCK_GATING	(ULL(1) << 15)
#define CLUSTERACTLR_EL1_DISABLE_SCLK_GATING	(ULL(3) << 15)

/********************************************************************
 * Masks applied for DSU errata workarounds
 ********************************************************************/
#define DSU_ERRATA_936184_MASK	(U(0x3) << 15)

#define CPUCFR_EL1		S3_0_C15_C0_0
/* SCU bit of CPU Configuration Register, EL1 */
#define SCU_SHIFT		U(2)

#ifndef __ASSEMBLER__
DEFINE_RENAME_SYSREG_RW_FUNCS(clusterpwrctlr_el1, CLUSTERPWRCTLR_EL1);

/* ---------------------------------------------
 * controls power features of the cluster
 * 1. Cache portion power not request
 * 2. Disable the retention circuit
 * ---------------------------------------------
 */
static inline void dsu_pwr_dwn(void)
{
	write_clusterpwrctlr_el1(0);
	isb();
}
#endif
#endif /* DSU_DEF_H */
