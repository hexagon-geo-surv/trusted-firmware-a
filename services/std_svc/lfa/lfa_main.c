/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <services/lfa_svc.h>
#include <smccc_helpers.h>

uint64_t lfa_smc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			 u_register_t x3, u_register_t x4, void *cookie,
			 void *handle, u_register_t flags)
{
	switch (smc_fid) {
	case LFA_SVC_VERSION:
		SMC_RET1(handle, LFA_VERSION);
		break;

	case LFA_SVC_FEATURES:
		SMC_RET1(handle, is_lfa_fid(x1) ? 0 : LFA_NOT_SUPPORTED);
		break;

	case LFA_SVC_GET_INFO:
		break;

	case LFA_SVC_GET_INVENTORY:
		break;

	case LFA_SVC_PRIME:
		break;

	case LFA_SVC_ACTIVATE:
		break;

	case LFA_SVC_CANCEL:
		break;

	default:
		WARN("Unimplemented LFA Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
		break; /* unreachable */

	}

	SMC_RET1(handle, SMC_UNK);

	return 0;
}
