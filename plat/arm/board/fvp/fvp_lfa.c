/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <errno.h>
#include <plat/common/platform.h>
#include <services/bl31_lfa.h>
#include <services/lfa_svc.h>
#include <services/rmmd_rmm_lfa.h>
#include <tools_share/firmware_image_package.h>

#include <plat/arm/common/plat_arm_lfa_components.h>

/* Keep this array consistent with enum fvp_lfa_component_id_t */
static plat_lfa_component_info_t fvp_lfa_components[LFA_MAX_DEFINED_COMPONENTS] = {
	[LFA_BL31_COMPONENT] = {LFA_BL31_COMPONENT, UUID_EL3_RUNTIME_FIRMWARE_BL31,
				NULL, false},
#if BL32_BASE
	[LFA_BL32_COMPONENT] = {LFA_BL32_COMPONENT, UUID_SECURE_PAYLOAD_BL32,
				NULL, false},
#endif /* BL32_BASE */
	[LFA_BL33_COMPONENT] = {LFA_BL33_COMPONENT, UUID_NON_TRUSTED_FIRMWARE_BL33,
				NULL, false},
#if ENABLE_RME
	[LFA_RMM_COMPONENT]  = {LFA_RMM_COMPONENT, UUID_REALM_MONITOR_MGMT_FIRMWARE,
				NULL, false},
#endif /* ENABLE_RME */
};

uint32_t plat_lfa_get_components(plat_lfa_component_info_t **components)
{
	if (components == NULL) {
		return -EINVAL;
	}

	fvp_lfa_components[LFA_BL31_COMPONENT].activator = get_bl31_activator();

#if ENABLE_RME
	fvp_lfa_components[LFA_RMM_COMPONENT].activator = get_rmm_activator();
#endif /* ENABLE_RME */

	*components = fvp_lfa_components;
	return LFA_MAX_DEFINED_COMPONENTS;
}

bool is_plat_lfa_activation_pending(uint32_t lfa_component_id)
{
#if ENABLE_RME
	if (lfa_component_id == LFA_RMM_COMPONENT) {
		return true;
	}
#endif /* ENABLE_RME */

	if (lfa_component_id == LFA_BL31_COMPONENT) {
		return true;
	}

	return false;
}

int plat_lfa_cancel(uint32_t lfa_component_id)
{
	/* placeholder function to do cancel LFA of given component */
	return 0;
}

int plat_lfa_load_auth_image(uint32_t lfa_component_id)
{
	return 0;
}


int plat_lfa_get_image_info(uint32_t lfa_component_id, uintptr_t *image_address, size_t *image_size) {
	/*
	 * In AEM FVP, we don't want to bloat the code by adding
	 * loading and authentication mechanism, so here we assumed
	 * that the components are pre-loaded and authenticated already.
	 *
	 * The image is pre-loaded at 0xFB000000, with the first 8 bytes
	 * containing the image size encoded as a uint64_t. The image itself
	 * starts at 0xFB000008.
	 */
	 if (lfa_component_id == LFA_BL31_COMPONENT) {
		/* Update this to the current image size. */
		*image_size = LFA_FVP_BL31_IMAGE_SIZE;
		*image_address = (uintptr_t)(PLAT_LFA_STORE_BASE);
		VERBOSE("FVP BL31 LFA image at 0x%lX with size %ld bytes\n", (uint64_t)*image_address, *image_size);
		return 0;
	}

	return -EPERM;
}

int plat_lfa_notify_activate(uint32_t lfa_component_id)
{
	return 0;
}

uint64_t plat_lfa_mailbox_base(void) {
	return PLAT_ARM_TRUSTED_MAILBOX_BASE;
}
