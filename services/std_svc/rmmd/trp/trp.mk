#
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

RMM_SOURCES		+=	services/std_svc/rmmd/trp/trp_entry.S \
				services/std_svc/rmmd/trp/trp_main.c  \
				services/std_svc/rmmd/trp/trp_helpers.c

RMM_DEFAULT_LINKER_SCRIPT_SOURCE := services/std_svc/rmmd/trp/linker.ld.S

ifeq ($($(ARCH)-ld-id),gnu-gcc)
        RMM_LDFLAGS	+=	-Wl,--sort-section=alignment
else ifneq ($(filter llvm-lld gnu-ld,$($(ARCH)-ld-id)),)
        RMM_LDFLAGS	+=	--sort-section=alignment
endif

# Include the platform-specific TRP Makefile
# If no platform-specific TRP Makefile exists, it means TRP is not supported
# on this platform.
TRP_PLAT_MAKEFILE := $(wildcard ${PLAT_DIR}/trp/trp-${PLAT}.mk)
ifeq (,${TRP_PLAT_MAKEFILE})
  $(error TRP is not supported on platform ${PLAT})
else
  include ${TRP_PLAT_MAKEFILE}
endif
