#
# Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ARM_CORTEX_A7		:=	yes
ARM_WITH_NEON		:=	yes
BL2_AT_EL3		:=	1
USE_COHERENT_MEM	:=	0

LEO_TF_VERSION	?=	0

# Not needed for Cortex-A7
WORKAROUND_CVE_2017_5715:=	0

# Number of TF-A copies in the device
LEO_TF_A_COPIES		:=	2
$(eval $(call add_define,LEO_TF_A_COPIES))
ifeq ($(AARCH32_SP),optee)
PLAT_PARTITION_MAX_ENTRIES	:=	$(shell echo $$(($(LEO_TF_A_COPIES) + 4)))
else
PLAT_PARTITION_MAX_ENTRIES	:=	$(shell echo $$(($(LEO_TF_A_COPIES) + 1)))
endif
$(eval $(call add_define,PLAT_PARTITION_MAX_ENTRIES))

PLAT_INCLUDES		:=	-Iplat/leo/common/include/
PLAT_INCLUDES		+=	-Iplat/leo/l1/include/

# Device tree
DTB_FILE_NAME		?=	faraday-leo.dtb
FDT_SOURCES		:=	$(addprefix fdts/, $(patsubst %.dtb,%.dts,$(DTB_FILE_NAME)))
DTC_FLAGS		+=	-Wno-unit_address_vs_reg

include lib/libfdt/libfdt.mk

PLAT_BL_COMMON_SOURCES	:=	

PLAT_BL_COMMON_SOURCES	+=	drivers/leo/uart/aarch32/16550_console.S

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

PLAT_BL_COMMON_SOURCES	+=	lib/cpus/aarch32/cortex_a7.S

PLAT_BL_COMMON_SOURCES	+=	drivers/arm/tzc/tzc400.c				\
				drivers/delay_timer/delay_timer.c			\
				drivers/delay_timer/generic_delay_timer.c		\
				plat/leo/common/l1_dt.c				\
				plat/leo/l1/l1_helper.S			\
				plat/leo/l1/l1_security.c			\
				plat/leo/l1/plat_image_load.c	\
				plat/leo/l1/bl2_plat_setup.c	\
				plat/leo/l1/l1_private.c	\
				drivers/io/io_storage.c					\
				drivers/io/io_dummy.c					\
				plat/leo/common/bl2_io_storage.c				\

BL2_SOURCES		+=	

BL2_SOURCES		+=	common/desc_image_load.c				\
					plat/leo/l1/plat_bl2_mem_params_desc.c		\

BL2_SOURCES		+=	
BL2_SOURCES		+=	

ifeq ($(AARCH32_SP),optee)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif

# Macros and rules to build TF binary
LEO_TF_ELF_LDFLAGS	:=	--hash-style=gnu --as-needed
LEO_DT_BASENAME	:=	$(DTB_FILE_NAME:.dtb=)
LEO_TF_LEO		:=	${BUILD_PLAT}/tf-a-${LEO_DT_BASENAME}.LEO
LEO_TF_BINARY		:=	$(LEO_TF_LEO:.LEO=.bin)
LEO_TF_MAPFILE	:=	$(LEO_TF_LEO:.LEO=.map)
LEO_TF_LINKERFILE	:=	$(LEO_TF_LEO:.LEO=.ld)
LEO_TF_ELF		:=	$(LEO_TF_LEO:.LEO=.elf)
LEO_TF_DTBFILE	:=      ${BUILD_PLAT}/fdts/${DTB_FILE_NAME}
LEO_TF_OBJS		:=	${BUILD_PLAT}/l1.o

.PHONY:			${LEO_TF_LEO}
.SUFFIXES:

all: check_dtc_version ${LEO_TF_LEO} 

ifeq ($(AARCH32_SP),sp_min)
# BL32 is built only if using SP_MIN
BL32_DEP		:= bl32
BL32_PATH		:= -DBL32_BIN_PATH=\"${BUILD_PLAT}/bl32.bin\"
endif

distclean realclean clean: clean

check_dtc_version:
	$(eval DTC_V = $(shell $(DTC) -v | awk '{print $$NF}'))
	$(eval DTC_VERSION = $(shell printf "%d" $(shell echo ${DTC_V} | cut -d- -f1 | sed "s/\./0/g")))
	@if [ ${DTC_VERSION} -lt 10404 ]; then \
		echo "dtc version too old (${DTC_V}), you need at least version 1.4.4"; \
		false; \
	fi


${LEO_TF_OBJS}:	plat/leo/l1/l1.S bl2 ${BL32_DEP} ${LEO_TF_DTBFILE}
			@echo "  AS      $<"
			${Q}${AS} ${ASFLAGS} ${TF_CFLAGS} \
				${BL32_PATH} \
				-DBL2_BIN_PATH=\"${BUILD_PLAT}/bl2.bin\" \
				-DDTB_BIN_PATH=\"${LEO_TF_DTBFILE}\" \
				-c plat/leo/l1/l1.S -o $@

${LEO_TF_LINKERFILE}:	plat/leo/l1/l1.ld.S ${BUILD_PLAT}
			@echo "  LDS     $<"
			${Q}${AS} ${ASFLAGS} ${TF_CFLAGS} -P -E $< -o $@

${LEO_TF_ELF}:	${LEO_TF_OBJS} ${LEO_TF_LINKERFILE}
			@echo "  LDS     $<"
			${Q}${LD} -o $@ ${LEO_TF_ELF_LDFLAGS} -Map=${LEO_TF_MAPFILE} --script ${LEO_TF_LINKERFILE} ${LEO_TF_OBJS}

${LEO_TF_BINARY}:	${LEO_TF_ELF}
			${Q}${OC} -O binary ${LEO_TF_ELF} $@
			@echo
			@echo "Built $@ successfully"
			@echo

${LEO_TF_LEO}:	${LEO_TF_BINARY}
			@echo
			@echo "Generated $@"
			$(eval LOADADDR =  $(shell cat ${LEO_TF_MAPFILE} | grep RAM | awk '{print $$2}'))
			$(eval ENTRY =  $(shell cat ${LEO_TF_MAPFILE} | grep "__BL2_IMAGE_START" | awk '{print $$1}'))
			@echo
      @echo "LOADADDR = ${LOADADDR}"
      @echo "Entry = ${Entry}"
