#
# Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/libfdt/libfdt.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_INCLUDES		:=	-Iplat/faraday/a600/include

PLAT_BL_COMMON_SOURCES	:=	drivers/ti/uart/aarch64/16550_console.S	\
				plat/faraday/a600/a600_common.c			\
				${XLAT_TABLES_LIB_SRCS}

BL2_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				common/desc_image_load.c		\
				drivers/arm/tzc/tzc400.c	\
				drivers/io/io_fip.c			\
				drivers/io/io_dummy.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				drivers/delay_timer/delay_timer.c	\
				drivers/delay_timer/generic_delay_timer.c \
				drivers/io/io_block.c			\
				plat/common/aarch64/platform_mp_stack.S	\
				plat/faraday/a600/aarch64/plat_helpers.S	\
				plat/faraday/a600/aarch64/a600_bl2_mem_params_desc.c \
				plat/faraday/a600/a600_bl2_setup.c		\
				plat/faraday/a600/a600_image_load.c		\
				plat/faraday/a600/a600_io_storage.c

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				plat/common/plat_gicv2.c		\
				plat/common/plat_psci_common.c		\
				plat/faraday/a600/aarch64/plat_helpers.S	\
				plat/faraday/a600/a600_bl31_setup.c		\
				plat/faraday/a600/a600_pm.c			\
				plat/faraday/a600/a600_topology.c		\
				${LIBFDT_SRCS}

# Tune compiler for Cortex-A53
ifeq ($(notdir $(CC)),armclang)
    TF_CFLAGS_aarch64	+=	-mcpu=cortex-a53
else ifneq ($(findstring clang,$(notdir $(CC))),)
    TF_CFLAGS_aarch64	+=	-mcpu=cortex-a53
else
    TF_CFLAGS_aarch64	+=	-mtune=cortex-a53
endif

# Platform Makefile target
# ------------------------
NEED_BL32			:= no
NEED_BL33			:= no
#A600_BL1_PAD_BIN	:=	${BUILD_PLAT}/bl1_pad.bin
#A600_ARMSTUB8_BIN	:=	${BUILD_PLAT}/armstub8.bin

# Add new default target when compiling this platform
#all: armstub

# This target concatenates BL1 and the FIP so that the base addresses match the
# ones defined in the memory map
armstub: bl1 fip
#	@echo "  CAT     $@"
#	${Q}cp ${BUILD_PLAT}/bl1.bin ${A600_BL1_PAD_BIN}
#	${Q}truncate --size=131072 ${A600_BL1_PAD_BIN}
#	${Q}cat ${A600_BL1_PAD_BIN} ${BUILD_PLAT}/fip.bin > ${A600_ARMSTUB8_BIN}
#	@${ECHO_BLANK_LINE}
#	@echo "Built $@ successfully"
#	@${ECHO_BLANK_LINE}

# Build config flags
# ------------------

# Enable all errata workarounds for Cortex-A53
ERRATA_A53_826319		:= 1
ERRATA_A53_835769		:= 1
ERRATA_A53_836870		:= 1
ERRATA_A53_843419		:= 1
ERRATA_A53_855873		:= 1

WORKAROUND_CVE_2017_5715	:= 0

# Disable stack protector by default
ENABLE_STACK_PROTECTOR	 	:= 0

BL2_AT_EL3				:= 1

# Reset to BL31 isn't supported
RESET_TO_BL31			:= 0

# Have different sections for code and rodata
SEPARATE_CODE_AND_RODATA	:= 1

# System coherency is managed in hardware
HW_ASSISTED_COHERENCY	:= 0

# Use Coherent memory
USE_COHERENT_MEM		:= 0

# This platform is single-cluster and does not require coherency setup.
WARMBOOT_ENABLE_DCACHE_EARLY	:= 0

# Platform build flags
# --------------------

# BL33 images are in AArch64 by default
A600_BL33_IN_AARCH32		:= 0

# Assume that BL33 isn't the Linux kernel by default
A600_DIRECT_LINUX_BOOT		:= 0

# UART to use at runtime. -1 means the runtime UART is disabled.
# Any other value means the default UART will be used.
A600_RUNTIME_UART		:= -1

# BL32 location
A600_BL32_RAM_LOCATION	:= tdram
ifeq (${A600_BL32_RAM_LOCATION}, tsram)
  A600_BL32_RAM_LOCATION_ID = SEC_SRAM_ID
else ifeq (${A600_BL32_RAM_LOCATION}, tdram)
  A600_BL32_RAM_LOCATION_ID = SEC_DRAM_ID
else
  $(error "Unsupported A600_BL32_RAM_LOCATION value")
endif

# Process platform flags
# ----------------------

$(eval $(call add_define,A600_BL32_RAM_LOCATION_ID))
$(eval $(call add_define,A600_BL33_IN_AARCH32))
$(eval $(call add_define,A600_DIRECT_LINUX_BOOT))
ifdef A600_PRELOADED_DTB_BASE
$(eval $(call add_define,A600_PRELOADED_DTB_BASE))
endif
$(eval $(call add_define,A600_RUNTIME_UART))

# Verify build config
# -------------------
#
ifneq (${A600_DIRECT_LINUX_BOOT}, 0)
  ifndef A600_PRELOADED_DTB_BASE
    $(error Error: A600_PRELOADED_DTB_BASE needed if A600_DIRECT_LINUX_BOOT=1)
  endif
endif

ifneq (${RESET_TO_BL31}, 0)
  $(error Error: a600 needs RESET_TO_BL31=0)
endif

ifeq (${ARCH},aarch32)
  $(error Error: AArch32 not supported on a600)
endif

ifneq ($(ENABLE_STACK_PROTECTOR), 0)
PLAT_BL_COMMON_SOURCES	+=	plat/faraday/a600/a600_rng.c			\
				plat/faraday/a600/a600_stack_protector.c
endif

ifeq (${SPD},opteed)
BL2_SOURCES	+=							\
		lib/optee/optee_utils.c
endif

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2))
endif

ifneq (${TRUSTED_BOARD_BOOT},0)

    include drivers/auth/mbedtls/mbedtls_crypto.mk
    include drivers/auth/mbedtls/mbedtls_x509.mk

    AUTH_SOURCES	:=	drivers/auth/auth_mod.c			\
				drivers/auth/crypto_mod.c		\
				drivers/auth/img_parser_mod.c		\
				drivers/auth/tbbr/tbbr_cot.c

    BL1_SOURCES		+=	${AUTH_SOURCES}				\
				bl1/tbbr/tbbr_img_desc.c		\
				plat/common/tbbr/plat_tbbr.c		\
				plat/faraday/a600/a600_trusted_boot.c	     	\
				plat/faraday/a600/a600_rotpk.S

    BL2_SOURCES		+=	${AUTH_SOURCES}				\
				plat/common/tbbr/plat_tbbr.c		\
				plat/faraday/a600/a600_trusted_boot.c	     	\
				plat/faraday/a600/a600_rotpk.S

    ROT_KEY             = $(BUILD_PLAT)/rot_key.pem
    ROTPK_HASH          = $(BUILD_PLAT)/rotpk_sha256.bin

    $(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))

    $(BUILD_PLAT)/bl1/a600_rotpk.o: $(ROTPK_HASH)
    $(BUILD_PLAT)/bl2/a600_rotpk.o: $(ROTPK_HASH)

    certificates: $(ROT_KEY)

    $(ROT_KEY):
	@echo "  OPENSSL $@"
	$(Q)openssl genrsa 2048 > $@ 2>/dev/null

    $(ROTPK_HASH): $(ROT_KEY)
	@echo "  OPENSSL $@"
	$(Q)openssl rsa -in $< -pubout -outform DER 2>/dev/null |\
	openssl dgst -sha256 -binary > $@ 2>/dev/null
endif
