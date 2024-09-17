/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/tbbr/tbbr_img_def.h>
#include <services/bl31_lfa.h>
#include <services/lfa_svc.h>
#include <services/rmmd_rmm_lfa.h>
#include <smccc_helpers.h>

#include <plat/common/platform.h>

/*
 * TODO: This is to be in-sync with platform chosen the number of
 * firmware to be live activated
 **/
#define MAX_LFA_COMPONENT_COUNT		20

uint32_t lfa_component_count = 0U;
lfa_component_t lfa_components[MAX_LFA_COMPONENT_COUNT];
struct lfa_activation current_activation;

void lfa_reset_activation()
{
	memset(&current_activation, 0, sizeof(current_activation));
	current_activation.component_id = U(0xFFFFFFFF);
	current_activation.prime_status = PRIME_NONE;
}

static void lfa_register_component(uint32_t image_id, const uuid_t *uuid,
				   struct lfa_activator_fns *activator)
{
	assert(lfa_component_count <
		sizeof(lfa_components)/sizeof(lfa_components[0]));

	lfa_components[lfa_component_count].image_id = image_id;
	lfa_components[lfa_component_count].uuid = *uuid;
	lfa_components[lfa_component_count].activator = activator;
	lfa_components[lfa_component_count].activation_pending = false;
	lfa_component_count += 1;
}

static void lfa_initialize_components()
{
	struct lfa_activator_fns *activator;
	const uuid_t *uuid;
	uint32_t image_id;

	/*
	 * Retrieve the number of images, their UUIDs, and image IDs from the
	 * platform, and register these components for live activation.
	 **/
	for (uint32_t index = 0U;; index++) {
		image_id = plat_get_lfa_image_info(index, &uuid);

		if (image_id == UINT32_MAX) {
			/* break if there are no further images */
			break;
		}

		switch (image_id) {
			case BL31_IMAGE_ID:
				activator = get_bl31_activator();
				break;
#if ENABLE_RME
			case RMM_IMAGE_ID:
				activator = get_rmm_activator();
				break;
#endif /* ENABLE_RME */
			default:
				activator = NULL;
				break;
		}

		/* Register the image that the platform claims */
		lfa_register_component(image_id, uuid, activator);
	}
}

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
		if (x1 == 0) {
			if (lfa_component_count == 0U) {
				lfa_initialize_components();
				lfa_reset_activation();
			}
			SMC_RET3(handle, LFA_SUCCESS, lfa_component_count, 0);
		} else {
			SMC_RET1(handle, LFA_INVALID_ARGUMENTS);
		}
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
