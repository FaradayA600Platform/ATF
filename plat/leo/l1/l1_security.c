/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/arm/tzc400.h>
//#include <dt-bindings/clock/leo-clks.h>
#include <lib/mmio.h>
//#define ORIGINAL

#define TZC_REGION_NSEC_ALL_ACCESS_RDWR \
	TZC_REGION_ACCESS_RDWR(LEO1_TZC_A7_ID) | \
	TZC_REGION_ACCESS_RDWR(LEO1_TZC_GPU_ID) | \
	TZC_REGION_ACCESS_RDWR(LEO1_TZC_LCD_ID) | \
	TZC_REGION_ACCESS_RDWR(LEO1_TZC_MDMA_ID) | \
	TZC_REGION_ACCESS_RDWR(LEO1_TZC_M4_ID) | \
	TZC_REGION_ACCESS_RDWR(LEO1_TZC_DMA_ID) | \
	TZC_REGION_ACCESS_RDWR(LEO1_TZC_USB_HOST_ID) | \
	TZC_REGION_ACCESS_RDWR(LEO1_TZC_USB_OTG_ID) | \
	TZC_REGION_ACCESS_RDWR(LEO1_TZC_SDMMC_ID) | \
	TZC_REGION_ACCESS_RDWR(LEO1_TZC_ETH_ID) | \
	TZC_REGION_ACCESS_RDWR(LEO1_TZC_DAP_ID)

/*******************************************************************************
 * Initialize the TrustZone Controller. Configure Region 0 with Secure RW access
 * and allow Non-Secure masters full access.
 ******************************************************************************/
static void init_tzc400(void)
{
	unsigned long long region_base, region_top;

	tzc400_init(LEO1_TZC_BASE);

//	tzc400_disable_filters(); kay 20200311 for 0302 vdk version , we can't set this 

	/*
	 * Region 1 set to cover all non-secure DRAM at 0xC000_0000. Apply the
	 * same configuration to all filters in the TZC.
	 */
	region_base = LEO_DDR_BASE;
	region_top = region_base + LEO_SYSRAM_SIZE - 1; 
	tzc400_configure_region(LEO1_FILTER_BIT_ALL, 1,
				region_base,
				region_top,
				TZC_REGION_S_RDWR,
				TZC_REGION_NSEC_ALL_ACCESS_RDWR);
        /* Region 2 set to cover non-secure shared memory DRAM. */
	region_base = region_top+1;
	region_top = region_base + LEO_DDR_S_SIZE - 1; 
	//region_top = 0x85000000-1; 
//	region_top = (ddr_top)& (~LEO_DDR_BASE);
	tzc400_configure_region(LEO1_FILTER_BIT_ALL, 2,
				region_base,
				region_top,
				TZC_REGION_S_RDWR,
				0/*TZC_REGION_NSEC_ALL_ACCESS_RDWR*/);
	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);
	//tzc400_enable_filters(); kay , 20200311 we don't enable filter again..
}

/*******************************************************************************
 * Initialize the TrustZone Controller.
 * Early initialization create only one region with full access to secure.
 * This setting is used before and during DDR initialization.
 ******************************************************************************/
static void early_init_tzc400(void)
{
	tzc400_init(LEO1_TZC_BASE);
	tzc400_disable_filters();

	/* Region 1 set to cover Non-Secure DRAM at 0xC000_0000 */
	tzc400_configure_region(LEO1_FILTER_BIT_ALL, 1,
				LEO_DDR_BASE,
				LEO_DDR_BASE +
				(LEO_DDR_MAX_SIZE - 1U),
				TZC_REGION_S_NONE,
				TZC_REGION_ACCESS_RDWR(LEO1_TZC_A7_ID) |
				TZC_REGION_ACCESS_RDWR(LEO1_TZC_SDMMC_ID));

	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);

	tzc400_enable_filters();
}

/*******************************************************************************
 * Initialize the secure environment. At this moment only the TrustZone
 * Controller is initialized.
 ******************************************************************************/
void leo_arch_security_setup(void)
{
	early_init_tzc400();
}

/*******************************************************************************
 * Initialize the secure environment. At this moment only the TrustZone
 * Controller is initialized.
 ******************************************************************************/
void leo_security_setup(void)
{
	init_tzc400();
}
