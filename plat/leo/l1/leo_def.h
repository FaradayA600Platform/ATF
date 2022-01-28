/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LEO1_DEF_H
#define LEO1_DEF_H

#include <common/tbbr/tbbr_img_def.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

#ifndef __ASSEMBLY__
#include <boot_api.h>
#include <leo_dt.h>
#include <leo_shres_helpers.h>
#include <leo_private.h>
#endif

/*******************************************************************************
 * LEO1 memory map related constants
 ******************************************************************************/

#define LEO_SYSRAM_BASE	(0x80000000)
#define LEO_SYSRAM_SIZE		(0x01000000)

/* DDR configuration */
#define LEO_DDR_BASE		(0x80000000)
//#define LEO_SYSRAM_BASE LEO_DDR_BASE
#define LEO_DDR_MAX_SIZE	(0xf0000000)
#define LEO_DDR_S_SIZE		(0x02000000)

#define LEO_DDR_SHMEM_SIZE	(0x00200000)	/* 2 MB */
#define LEO_DDR_NS_START	LEO_DDR_BASE+LEO_DDR_S_SIZE

/* Section used inside TF binaries */
#define LEO_PARAM_LOAD_SIZE		(0x00002400)	/* 9 Ko for param */
/* 256 Octets reserved for header */
#define LEO_HEADER_SIZE		(0x00000100)

#define LEO_BINARY_BASE		(LEO_SYSRAM_BASE +		\
					 LEO_PARAM_LOAD_SIZE +	\
					 LEO_HEADER_SIZE)

#define LEO_BINARY_SIZE		(LEO_SYSRAM_SIZE -		\
					 (LEO_PARAM_LOAD_SIZE +	\
					  LEO_HEADER_SIZE))

/* BL2 */
#define LEO_BL2_BASE		(LEO_BL32_BASE - \
					 LEO_BL2_SIZE)
#define LEO_BL2_SIZE		(0x00017000)	/* 92 Ko for BL2 */


/* BL32 */
#define LEO_BL32_BASE		(LEO_SYSRAM_BASE + LEO_SYSRAM_SIZE - LEO_BL32_SIZE)
#define LEO_BL32_SIZE		(0x200000)
#define LEO_OPTEE_BASE		LEO_BL32_BASE
#define LEO_OPTEE_SIZE		(0x200000)

/* BL33 */
#define LEO_BL33_BASE		0 //not used

/* BL2 and BL32/sp_min require 5 tables */
#define MAX_XLAT_TABLES			5

/*
 * MAX_MMAP_REGIONS is usually:
 * BL leo1_mmap size + mmap regions in *_plat_arch_setup
 */
#if defined(IMAGE_BL2)
  #define MAX_MMAP_REGIONS		11
#endif
#if defined(IMAGE_BL32)
  #define MAX_MMAP_REGIONS		6
#endif





/*******************************************************************************
 * LEO1 device/io map related constants (used for MMU)
 ******************************************************************************/
#define LEO_DEVICE1_BASE		(0x40000000)
#define LEO_DEVICE1_SIZE		(0x40000000)

#define LEO_DEVICE2_BASE		(0xC0000000)
#define LEO_DEVICE2_SIZE		(0x40000000)

/*******************************************************************************
 * LEO1 UART
 ******************************************************************************/
#define UART0_BASE			(0x54E00000)
#define LEO_UART_BAUDRATE		(115200)

/* For UART crash console */
#define LEO_DEBUG_USART_BASE	UART0_BASE
/* UART4 on HSI@64MHz, TX on GPIOG11 Alternate 6 */
#define LEO_DEBUG_USART_CLK_FRQ	58300000

/*******************************************************************************
 * LEO1 TZC (TZ400)
 ******************************************************************************/
#define LEO1_TZC_BASE		(0x28600000)

#define LEO1_TZC_A7_ID		(0)
#define LEO1_TZC_M4_ID		(1)
#define LEO1_TZC_LCD_ID		(3)
#define LEO1_TZC_GPU_ID		(4)
#define LEO1_TZC_MDMA_ID		(5)
#define LEO1_TZC_DMA_ID		(6)
#define LEO1_TZC_USB_HOST_ID	(7)
#define LEO1_TZC_USB_OTG_ID		(8)
#define LEO1_TZC_SDMMC_ID		(9)
#define LEO1_TZC_ETH_ID		(10)
#define LEO1_TZC_DAP_ID		(15)

#define LEO1_FILTER_BIT_ALL		(3)
#endif /* LEO1_DEF_H */
