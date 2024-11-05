/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/io/io_storage.h>
#include <tools_share/firmware_image_package.h>

typedef struct plat_img_info {
	uint32_t img_id;
	uuid_t uuid;
} plat_img_info_t;

static const plat_img_info_t fvp_imgs[] = {
	{BL31_IMAGE_ID, UUID_EL3_RUNTIME_FIRMWARE_BL31},
#if ENABLE_RME
	{RMM_IMAGE_ID, UUID_REALM_MONITOR_MGMT_FIRMWARE},
#endif
};

uint32_t plat_get_lfa_image_info(int index, const uuid_t **uuid)
{
	if (index >= sizeof(fvp_imgs)/sizeof(fvp_imgs[0])) {
		return UINT32_MAX;
	}

	*uuid = &fvp_imgs[index].uuid;

	return fvp_imgs[index].img_id;
}

bool is_plat_fw_update_avail(uint32_t image_id)
{
	/* As RMM_IMAGE is preloaded */
	if (image_id == RMM_IMAGE_ID) {
		return true;
	}

	return false;
}
