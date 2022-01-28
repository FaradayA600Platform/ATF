#
# Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

SP_MIN_WITH_SECURE_FIQ	:=	1

BL32_SOURCES		+=	plat/common/aarch32/platform_mp_stack.S		\
				plat/leo/l1/leo_setup.c		\
				plat/leo/l1/leo_pm.c			\
				plat/leo/l1/leo_topology.c
# Generic GIC v2
BL32_SOURCES		+=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				plat/common/plat_gicv2.c		\
				plat/leo/l1/leo_gic.c

# stm32mp1 specific services
#BL32_SOURCES		+=	plat/leo/l1/services/bsec_svc.c		\
#				plat/leo/l1/services/leo_svc_setup.c
