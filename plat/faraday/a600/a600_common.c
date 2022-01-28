/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <bl31/interrupt_mgmt.h>
#include <drivers/arm/tzc400.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "a600_hw.h"
#include "a600_private.h"

#define MAP_DEVICE0	MAP_REGION_FLAT(DEVICE0_BASE,			\
					DEVICE0_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DEVICE1	MAP_REGION_FLAT(DEVICE1_BASE,			\
					DEVICE1_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DEVICE2	MAP_REGION_FLAT(DEVICE2_BASE,			\
					DEVICE2_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#ifdef A600_PRELOADED_DTB_BASE
#define MAP_NS_DTB	MAP_REGION_FLAT(A600_PRELOADED_DTB_BASE, 0x10000, \
					MT_MEMORY | MT_RW | MT_NS)
#endif

#define MAP_NS_DRAM0	MAP_REGION_FLAT(NS_DRAM0_BASE, NS_DRAM0_SIZE,	\
					MT_MEMORY | MT_RW | MT_NS)

#define MAP_BL32_MEM	MAP_REGION_FLAT(BL32_MEM_BASE, BL32_MEM_SIZE,	\
					MT_MEMORY | MT_RW | MT_SECURE)

#ifdef SPD_opteed
#define MAP_OPTEE_PAGEABLE	MAP_REGION_FLAT(		\
				A600_OPTEE_PAGEABLE_LOAD_BASE,	\
				A600_OPTEE_PAGEABLE_LOAD_SIZE,	\
				MT_MEMORY | MT_RW | MT_SECURE)
#endif

/*
 * Table of regions for various BL stages to map using the MMU.
 */
#ifdef IMAGE_BL1
static const mmap_region_t plat_a600_mmap[] = {
	MAP_DEVICE0,
	MAP_DEVICE1,
	MAP_DEVICE2,
#ifdef SPD_opteed
	MAP_OPTEE_PAGEABLE,
#endif
	{0}
};
#endif

#ifdef IMAGE_BL2
static const mmap_region_t plat_a600_mmap[] = {
	MAP_DEVICE0,
	MAP_DEVICE1,
	MAP_DEVICE2,
	MAP_NS_DRAM0,
#ifdef BL32_BASE
	MAP_BL32_MEM,
#endif
	{0}
};
#endif

#ifdef IMAGE_BL31
static const mmap_region_t plat_a600_mmap[] = {
	MAP_DEVICE0,
	MAP_DEVICE1,
	MAP_DEVICE2,
#ifdef A600_PRELOADED_DTB_BASE
	MAP_NS_DTB,
#endif
#ifdef BL32_BASE
	MAP_BL32_MEM,
#endif
	{0}
};
#endif

/*******************************************************************************
 * Function that sets up the console
 ******************************************************************************/
static console_16550_t a600_console;

void a600_console_init(void)
{
	int console_scope = CONSOLE_FLAG_BOOT;
#if A600_RUNTIME_UART != -1
	console_scope |= CONSOLE_FLAG_RUNTIME;
#endif
	int rc = console_16550_register(PLAT_A600_UART_BASE,
	                                PLAT_A600_UART_CLK_IN_HZ,
	                                PLAT_A600_UART_BAUDRATE,
	                                &a600_console);
	if (rc == 0) {
		/*
		 * The crash console doesn't use the multi console API, it uses
		 * the core console functions directly. It is safe to call panic
		 * and let it print debug information.
		 */
		panic();
	}

	console_set_scope(&a600_console.console, console_scope);
}

/*******************************************************************************
 * Function that sets up the ddr
 ******************************************************************************/
void a600_ddr_init(void)
{
	unsigned int val;
	/*
	 * ==================================
	 *  FTDDR440 subsystem
	 * ==================================
	 *  addr[19:16] = 4'h0 ; cha_dfi
	 *  addr[19:16] = 4'h1 ; chb_dfi
	 *  addr[19:16] = 4'h2 ; cha_ctrl
	 *  addr[19:16] = 4'h3 ; chb_ctrl
	 *  addr[19:16] = 4'h4 ; cphy
	 *  addr[19:16] = 4'h5 ; cha_aphy
	 *  addr[19:16] = 4'h6 ; chb_aphy
	 *  addr[19:16] = 4'h7 ; cha_dphy_b0
	 *  addr[19:16] = 4'h8 ; cha_dphy_b1
	 *  addr[19:16] = 4'h9 ; chb_dphy_b0
	 *  addr[19:16] = 4'hA ; chb_dphy_b1
	 * ==================================
	 */

	mmio_write_32(0x2D080148, 0x00000000);	// disable DDR interleave and assert PHY reset (DDR0_CTRL of secure EXTREG)
//	mmio_write_32(0x2D080148, 0x00000002);	// enable DDR interleave and assert PHY reset (DDR0_CTRL of secure EXTREG)
	mdelay(100);
	mmio_write_32(0x2D080148, 0x00000001);	// disable DDR interleave and release PHY reset (DDR0_CTRL of secure EXTREG)
//	mmio_write_32(0x2D080148, 0x00000003);	// enable DDR interleave and release PHY reset (DDR0_CTRL of secure EXTREG)

	// ------------------- Step1 - change DDR PLL M/N - start -------------------------------

	// cha_dphy reset assert
	mmio_write_32(0x2A6700cc, 0x00000002);	//a_dphy_b0
	mmio_write_32(0x2A6800cc, 0x00000002);	//a_dphy_b1
	// chb_dphy reset assert
	mmio_write_32(0x2A6900cc, 0x00000002);	//b_dphy_b0
	mmio_write_32(0x2A6a00cc, 0x00000002);	//b_dphy_b1
	// cha_aphy reset assert
	mmio_write_32(0x2A6500a0, 0x00000002);	//a_aphy
	// chb_aphy reset assert
	mmio_write_32(0x2A6600a0, 0x00000002);	//b_aphy
	// cphy reset assert
	mmio_write_32(0x2A640058, 0x00000002);	//cphy

	mdelay(100);

	// cphy PLL setting
	// cphy PLL_CTRL1           0x44= 0x083f4006
	// m = 10'd533 = 10'h215 =
	// p = 6'd3
	// s = 3'd0
//	mmio_write_32(0x2A640044, 0x123f6403);	//cphy (400mhz)
//	mmio_write_32(0x2A640044, 0x0A3f6403);	//cphy (800mhz)
//	mmio_write_32(0x2A640044, 0x0A3F8543);	//cphy (1066mhz)
//	mmio_write_32(0x2A640044, 0x013F6403);	//cphy (1600mhz)
//	mmio_write_32(0x2A640044, 0x013F7483);	//cphy (1866mhz)
	mmio_write_32(0x2A640044, 0x013F8543);	//cphy (2133mhz)

	mdelay(100);

	// cha_dphy reset assert
	mmio_write_32(0x2A6700cc, 0x00000003);	//a_dphy_b0
	mmio_write_32(0x2A6800cc, 0x00000003);	//a_dphy_b1
	// chb_dphy reset assert
	mmio_write_32(0x2A6900cc, 0x00000003);	//b_dphy_b0
	mmio_write_32(0x2A6a00cc, 0x00000003);	//b_dphy_b1
	// cha_aphy reset de-assert
	mmio_write_32(0x2A6500a0, 0x00000003);	//a_aphy
	// chb_aphy reset de-assert
	mmio_write_32(0x2A6600a0, 0x00000003);	//b_aphy
	// cphy reset de-assert
	mmio_write_32(0x2A640058, 0x00000003);	//cphy

	// wait to aphy ready - channel A
	val = mmio_read_32(0x2A6500E0);
	val &= 0x00100000;
	while (val != 0x00100000)
	{
		val = mmio_read_32(0x2A6500E0);
		val &= 0x00100000;
	}

	// wait to aphy ready - channel B
	val = mmio_read_32(0x2A6600E0);
	val &= 0x00100000;
	while (val != 0x00100000)
	{
		val = mmio_read_32(0x2A6600E0);
		val &= 0x00100000;
	}

	// ------------------- Step1 - change DDR PLL M/N - end -------------------------------

	// ------------------- Step2 - Set FTDDR440 control register - start -------------------------------

	// Channel A ctrl setting
	mmio_write_32(0x2A620000, 0x08058100);	//Memory Controller Configure (LPDDR4 mode, 16-bits memory width)
	mmio_write_32(0x2A620020, 0x00103f7c);	//LPDDRx Mode Register Set Values for MR1/MR2/MR3/MR4
	mmio_write_32(0x2A620038, 0x00000002);	//LPDDRx Additive Latency Register
	mmio_write_32(0x2A62003c, 0x08000075);	//Rank Information Register (0x80000000, 16x10x3, 1GB)
	mmio_write_32(0x2A620040, 0x16162217);	//Timing Parameter 0
	mmio_write_32(0x2A620044, 0x64a18c6a);	//Timing Parameter 1
	mmio_write_32(0x2A620048, 0x03039a7f);	//Timing Parameter 2
	mmio_write_32(0x2A62004c, 0x00ff8764);	//Timing Parameter 3
	mmio_write_32(0x2A620050, 0x0000050a);	//Timing Parameter 4
	mmio_write_32(0x2A62005c, 0x50000000);	//Timing Parameter 7
	mmio_write_32(0x2A620060, 0x0000000e);	//Initialization of Waiting Cycle Count 1
	mmio_write_32(0x2A620064, 0x000000d4);	//Initialization of Waiting Cycle Count 2
	mmio_write_32(0x2A620068, 0x00000190);	//LPDDR4 2 us Wait Cycle Register
	mmio_write_32(0x2A62006c, 0x00061a80);	//LPDDR4 2 ms Wait Cycle Register
	mmio_write_32(0x2A620074, 0x00000000);	//EXIT SRF Control Register
	mmio_write_32(0x2A620100, 0x001600e5);	//Channel Arbitration Setup Register
	mmio_write_32(0x2A620104, 0x0f050506);	//Channel Arbiter Grant Count Register - A
	mmio_write_32(0x2A620108, 0x05080305);	//Channel Arbiter Grant Count Register - B
	mmio_write_32(0x2A620118, 0x00000002);	//Bandwidth Control Register
	mmio_write_32(0x2A62011c, 0x03030404);	//Bandwidth Control Command Count Register - A
	mmio_write_32(0x2A620120, 0x01010202);	//Bandwidth Control Command Count Register - B
	mmio_write_32(0x2A62012c, 0x00000000);	//Channel QoS Mapping Register
	mmio_write_32(0x2A620130, 0x00000000);	//AXI4 QoS Mapping Register 0
	mmio_write_32(0x2A620200, 0x10052008);	//DDRx PHY Write/Read Data Timing Control Register
	mmio_write_32(0x2A620080, 0x0000aa00);	//Command Flush Control Register
	mmio_write_32(0x2A620170, 0x00000000);	//Traffic Monitor Clock Cycle Register
	mmio_write_32(0x2A62008c, 0x81818181);	//AHB INCR Read Prefetch Length 1
	mmio_write_32(0x2A620090, 0x81818181);	//AHB INCR Read Prefetch Length 2
	mmio_write_32(0x2A6200a0, 0x00000001);	//DDR ELASTIC FIFO Control Register

	mmio_write_32(0x2A620204, 0x00000010);	//Channel A DFI Training Control Register
	mmio_write_32(0x2A620210, 0x00000019);
	mmio_write_32(0x2A620218, 0x00000043);
	mmio_write_32(0x2A620300, 0x007f007c);	//FSP1 MR2 and MR1
	mmio_write_32(0x2A620304, 0x00660030);	//FSP1 MR11 and MR3
	mmio_write_32(0x2A620308, 0x0000000e);	//FSP1 MR13 and MR12
	mmio_write_32(0x2A62030c, 0x0000000e);	//FSP1 MR22 and MR14
	mmio_write_32(0x2A620310, 0xa0093818);
	mmio_write_32(0x2A620314, 0x26847c9a);
	mmio_write_32(0x2A620318, 0x03031030);
	mmio_write_32(0x2A62031c, 0x00938764);
	mmio_write_32(0x2A620320, 0x00000a18);
	mmio_write_32(0x2A620324, 0x00000000);
	mmio_write_32(0x2A620328, 0x00000000);
	mmio_write_32(0x2A62032c, 0x32331414);
	mmio_write_32(0x2A620330, 0x00000002);
	mmio_write_32(0x2A620334, 0x000e2011);	//FSP1 wren & trddata_en
	mmio_write_32(0x2A620338, 0x00000a0a);
	mmio_write_32(0x2A62033C, 0x000fffff);
	mmio_write_32(0x2A620340, 0x001b003c);	//FSP2 MR2 and MR1
	mmio_write_32(0x2A620344, 0x00660010);	//FSP2 MR11 and MR3
	mmio_write_32(0x2A620348, 0x0000000e);	//FSP2 MR13 and MR12
	mmio_write_32(0x2A62034c, 0x0000000e);	//FSP2 MR22 and MR14
	mmio_write_32(0x2A620350, 0x300a120c);
	mmio_write_32(0x2A620354, 0x02033524);
	mmio_write_32(0x2A620358, 0x03022421);
	mmio_write_32(0x2A62035c, 0x00641109);
	mmio_write_32(0x2A620360, 0x0000090a);
	mmio_write_32(0x2A620364, 0x00000000);
	mmio_write_32(0x2A620368, 0x00000000);
	mmio_write_32(0x2A62036c, 0x32331414);
	mmio_write_32(0x2A620370, 0x00000002);
	mmio_write_32(0x2A620374, 0x00052005);	//FSP2 wren & trddata_en
	mmio_write_32(0x2A620378, 0x00000a0a);
	mmio_write_32(0x2A62037C, 0x000fffff);

	// Channel B ctrl setting
	mmio_write_32(0x2A630000, 0x08058100);	//Memory Controller Configure (LPDDR4 mode, 16-bits memory width)
	mmio_write_32(0x2A630020, 0x00103f7c);	//LPDDRx Mode Register Set Values for MR1/MR2/MR3/MR4
	mmio_write_32(0x2A630038, 0x00000002);	//LPDDRx Additive Latency Register
	mmio_write_32(0x2A63003c, 0x0C000075);	//Rank Information Register (0xC0000000, 16x10x3, 1GB)
	mmio_write_32(0x2A630040, 0x16162217);	//Timing Parameter 0
	mmio_write_32(0x2A630044, 0x64a18c6a);	//Timing Parameter 1
	mmio_write_32(0x2A630048, 0x03039a7f);	//Timing Parameter 2
	mmio_write_32(0x2A63004c, 0x00ff8764);	//Timing Parameter 3
	mmio_write_32(0x2A630050, 0x0000050a);	//Timing Parameter 4
	mmio_write_32(0x2A63005c, 0x50000000);	//Timing Parameter 7
	mmio_write_32(0x2A630060, 0x0000000e);	//Initialization of Waiting Cycle Count 1
	mmio_write_32(0x2A630064, 0x000000d4);	//Initialization of Waiting Cycle Count 2
	mmio_write_32(0x2A630068, 0x00000190);	//LPDDR4 2 us Wait Cycle Register
	mmio_write_32(0x2A63006c, 0x00061a80);	//LPDDR4 2 ms Wait Cycle Register
	mmio_write_32(0x2A630074, 0x00000000);	//EXIT SRF Control Register
	mmio_write_32(0x2A630100, 0x001600e5);	//Channel Arbitration Setup Register
	mmio_write_32(0x2A630104, 0x0f050506);	//Channel Arbiter Grant Count Register - A
	mmio_write_32(0x2A630108, 0x05080305);	//Channel Arbiter Grant Count Register - B
	mmio_write_32(0x2A630118, 0x00000002);	//Bandwidth Control Register
	mmio_write_32(0x2A63011c, 0x03030404);	//Bandwidth Control Command Count Register - A
	mmio_write_32(0x2A630120, 0x01010202);	//Bandwidth Control Command Count Register - B
	mmio_write_32(0x2A63012c, 0x00000000);	//Channel QoS Mapping Register
	mmio_write_32(0x2A630130, 0x00000000);	//AXI4 QoS Mapping Register 0
	mmio_write_32(0x2A630200, 0x10052008);	//DDRx PHY Write/Read Data Timing Control Register
	mmio_write_32(0x2A630080, 0x0000aa00);	//Command Flush Control Register
	mmio_write_32(0x2A630170, 0x00000000);	//Traffic Monitor Clock Cycle Register
	mmio_write_32(0x2A63008c, 0x81818181);	//AHB INCR Read Prefetch Length 1
	mmio_write_32(0x2A630090, 0x81818181);	//AHB INCR Read Prefetch Length 2
	mmio_write_32(0x2A6300a0, 0x00000001);	//DDR ELASTIC FIFO Control Register

	mmio_write_32(0x2A630204, 0x00000010);	//Channel B DFI Training Control Register
	mmio_write_32(0x2A630210, 0x00000019);
	mmio_write_32(0x2A630218, 0x00000043);
	mmio_write_32(0x2A630300, 0x007f007c);	//FSP1 MR2 and MR1
	mmio_write_32(0x2A630304, 0x00660030);	//FSP1 MR11 and MR3
	mmio_write_32(0x2A630308, 0x0000000e);	//FSP1 MR13 and MR12
	mmio_write_32(0x2A63030c, 0x0000000e);	//FSP1 MR22 and MR14
	mmio_write_32(0x2A630310, 0xa0093818);
	mmio_write_32(0x2A630314, 0x26847c9a);
	mmio_write_32(0x2A630318, 0x03031030);
	mmio_write_32(0x2A63031c, 0x00938764);
	mmio_write_32(0x2A630320, 0x00000a18);
	mmio_write_32(0x2A630324, 0x00000000);
	mmio_write_32(0x2A630328, 0x00000000);
	mmio_write_32(0x2A63032c, 0x32331414);
	mmio_write_32(0x2A630330, 0x00000002);
	mmio_write_32(0x2A630334, 0x000e2011);	//FSP1 wren & trddata_en
	mmio_write_32(0x2A630338, 0x00000a0a);
	mmio_write_32(0x2A63033C, 0x000fffff);
	mmio_write_32(0x2A630340, 0x001b003c);	//FSP2 MR2 and MR1
	mmio_write_32(0x2A630344, 0x00660010);	//FSP2 MR11 and MR3
	mmio_write_32(0x2A630348, 0x0000000e);	//FSP2 MR13 and MR12
	mmio_write_32(0x2A63034c, 0x0000000e);	//FSP2 MR22 and MR14
	mmio_write_32(0x2A630350, 0x300a120c);
	mmio_write_32(0x2A630354, 0x02033524);
	mmio_write_32(0x2A630358, 0x03032421);
	mmio_write_32(0x2A63035c, 0x00641109);
	mmio_write_32(0x2A630360, 0x0000090a);
	mmio_write_32(0x2A630364, 0x00000000);
	mmio_write_32(0x2A630368, 0x00000000);
	mmio_write_32(0x2A63036c, 0x32331414);
	mmio_write_32(0x2A630370, 0x00000002);
	mmio_write_32(0x2A630374, 0x00052005);
	mmio_write_32(0x2A630378, 0x00000a0a);
	mmio_write_32(0x2A63037C, 0x000fffff);

	// ------------------- Step2 - Set FTDDR440 control register - end -------------------------------

	// ------------------- Step3 - Set pin swap - start -------------------------------

//	mmio_write_32(0x2A6001B0, 0x00000001);	//D PHY Byte Swap
	mmio_write_32(0x2A600160, 0x02105423);	//CA PHY Pin Swap Register 0
	mmio_write_32(0x2A600164, 0x0000001e);	//CA PHY Pin Swap Register 1
	mmio_write_32(0x2A6001C0, 0x32104756);	//D PHY Byte0 Swap regsiter 0
	mmio_write_32(0x2A6001C4, 0x20137654);	//D PHY Byte0 Swap regsiter 1
	mmio_write_32(0x2A6001C8, 0x32104756);	//D PHY Byte0 Swap regsiter 2
	mmio_write_32(0x2A6001D0, 0x45673021);	//D PHY Byte1 Swap regsiter 0
	mmio_write_32(0x2A6001D4, 0x45673102);	//D PHY Byte1 Swap regsiter 1
	mmio_write_32(0x2A6001D8, 0x45673021);	//D PHY Byte1 Swap regsiter 2
//	mmio_write_32(0x2A6101B0, 0x00000001);	//D PHY Byte Swap
	mmio_write_32(0x2A610160, 0x02105243);	//CA PHY Pin Swap Register 0
	mmio_write_32(0x2A610164, 0x0000001b);	//CA PHY Pin Swap Register 1
	mmio_write_32(0x2A6101C0, 0x45673021);	//D PHY Byte0 Swap regsiter 0
	mmio_write_32(0x2A6101C4, 0x45673102);	//D PHY Byte0 Swap regsiter 1
	mmio_write_32(0x2A6101C8, 0x45673021);	//D PHY Byte0 Swap regsiter 2
	mmio_write_32(0x2A6101D0, 0x32104756);	//D PHY Byte1 Swap regsiter 0
	mmio_write_32(0x2A6101D4, 0x20137654);	//D PHY Byte1 Swap regsiter 1
	mmio_write_32(0x2A6101D8, 0x32104756);	//D PHY Byte1 Swap regsiter 2

	// ------------------- Step3 - Set pin swap - end -------------------------------

	// ------------------- Step4 - Set FTDFIW400 control register - start -------------------------------

	// Channel A setting
	mmio_write_32(0x2A600004, 0x00031c00);	//set freq_set_point
	mmio_write_32(0x2A60000c, 0x00000000);	//disable update control
	mmio_write_32(0x2A600010, 0x00000000);	//disable update control
	mmio_write_32(0x2A600014, 0x00000000);	//disable update control
	mmio_write_32(0x2A600020, 0x00120e5c);	//cbt
	mmio_write_32(0x2A600028, 0x01030014);	//weye/reye per-bit
	mmio_write_32(0x2A60002C, 0x10000054);	//weye setting
	mmio_write_32(0x2A600030, 0x55555555);	//weye external pattern
	mmio_write_32(0x2A600034, 0x5523201c);	//rvref setting
	mmio_write_32(0x2A60003C, 0x01180100);	//wvref setting
	mmio_write_32(0x2A600040, 0x00040204);	//FSP2's DFI timing setting
	mmio_write_32(0x2A600044, 0x00080210);	//FSP1's DFI timing setting
	mmio_write_32(0x2A600048, 0x0d0b060e);	//FSP2's timing setting
	mmio_write_32(0x2A60004c, 0x1e1e1e1e);	//FSP1's timing setting
	mmio_write_32(0x2A600050, 0x25241102);	//FSP2's timing setting
	mmio_write_32(0x2A600054, 0x858a1e0f);	//FSP1's timing setting
	mmio_write_32(0x2A600058, 0x40050325);	//FSP2's timing setting
	mmio_write_32(0x2A60005c, 0xf0100f25);	//FSP1's timing setting
	mmio_write_32(0x2A600060, 0x20040507);	//FSP2's timing setting
	mmio_write_32(0x2A600064, 0xa00f100f);	//FSP1's timing setting
	mmio_write_32(0x2A600068, 0x00002b56);	//FSP2's timing setting
	mmio_write_32(0x2A60006c, 0x0000ffff);	//FSP1's timing setting
	mmio_write_32(0x2A600080, 0x0010001b);	//FSP2's MR3, MR2, RL=20, WL=10
	mmio_write_32(0x2A600084, 0x0010007f);	//FSP1's MR3, MR2, RL=36, WL=34
	mmio_write_32(0x2A600088, 0x00000000);	//MR13
	mmio_write_32(0x2A600100, 0x07ff07ff);	//FSP0's caphdly and csdly
	mmio_write_32(0x2A600238, 0x00002000);	//FSP2's R0 B0 RVREF
	mmio_write_32(0x2A60023C, 0x00002000);	//FSP1's R0 B0 RVREF
	mmio_write_32(0x2A600338, 0x00002000);	//FSP2's R0 B1 RVREF
	mmio_write_32(0x2A60033C, 0x00002000);	//FSP1's R0 B1 RVREF

	// Channel B setting
	mmio_write_32(0x2A610004, 0x00031c00);	//set freq_set_point
	mmio_write_32(0x2A61000c, 0x00000000);	//disable update control
	mmio_write_32(0x2A610010, 0x00000000);	//disable update control
	mmio_write_32(0x2A610014, 0x00000000);	//disable update control
	mmio_write_32(0x2A610020, 0x00120e5c);	//cbt
	mmio_write_32(0x2A610028, 0x01030014);	//weye/reye per-bit
	mmio_write_32(0x2A61002C, 0x10000054);	//weye setting
	mmio_write_32(0x2A610030, 0x55555555);	//weye external pattern
	mmio_write_32(0x2A610034, 0x5523201c);	//rvref setting
	mmio_write_32(0x2A61003C, 0x01180100);	//wvref setting
	mmio_write_32(0x2A610040, 0x00040204);	//FSP2's DFI timing setting
	mmio_write_32(0x2A610044, 0x00080210);	//FSP1's DFI timing setting
	mmio_write_32(0x2A610048, 0x0d0b060e);	//FSP2's timing setting
	mmio_write_32(0x2A61004c, 0x1e1e1e1e);	//FSP1's timing setting
	mmio_write_32(0x2A610050, 0x25241102);	//FSP2's timing setting
	mmio_write_32(0x2A610054, 0x858a1e0f);	//FSP1's timing setting
	mmio_write_32(0x2A610058, 0x40050325);	//FSP2's timing setting
	mmio_write_32(0x2A61005c, 0xf0100f25);	//FSP1's timing setting
	mmio_write_32(0x2A610060, 0x20040507);	//FSP2's timing setting
	mmio_write_32(0x2A610064, 0xa00f100f);	//FSP1's timing setting
	mmio_write_32(0x2A610068, 0x00002b56);	//FSP2's timing setting
	mmio_write_32(0x2A61006c, 0x0000ffff);	//FSP1's timing setting
	mmio_write_32(0x2A610080, 0x0010001b);	//FSP2's MR3, MR2, RL=20, WL=10
	mmio_write_32(0x2A610084, 0x0010007f);	//FSP1's MR3, MR2, RL=36, WL=34
	mmio_write_32(0x2A610088, 0x00000000);	//MR13
	mmio_write_32(0x2A610100, 0x07ff07ff);	//FSP0's caphdly and csdly
	mmio_write_32(0x2A610238, 0x00002000);	//FSP2's R0 B0 RVREF
	mmio_write_32(0x2A61023C, 0x00002000);	//FSP1's R0 B0 RVREF
	mmio_write_32(0x2A610338, 0x00002000);	//FSP2's R0 B1 RVREF
	mmio_write_32(0x2A61033C, 0x00002000);	//FSP1's R0 B1 RVREF

	// ------------------- Step4 - Set FTDFIW400 control register - end -------------------------------

	// ------------------- Step5 - Change to boot frequency and enable the training - start -------------------------------

	mmio_write_32(0x2A600004, 0x00039c7f);	//enable wrlvl, reye, wl2nd, weye
	mmio_write_32(0x2A610004, 0x00039c7f);	//enable wrlvl, reye, wl2nd, weye

	// wait to cha_dfi boot sequence completed
	val = mmio_read_32(0x2A600004);
	val &= 0x00008000;
	while (val == 0x00008000)
	{
		val = mmio_read_32(0x2A600004);
		val &= 0x00008000;
	}

	// wait to chb_dfi boot sequence completed
	val = mmio_read_32(0x2A610004);
	val &= 0x00008000;
	while (val == 0x00008000)
	{
		val = mmio_read_32(0x2A610004);
		val &= 0x00008000;
	}

	// ------------------- Step5 - Change to boot frequency and enable the training - end -------------------------------

	// ------------------- Step6 - Trigger DDR initialization and Training - start -------------------------------

	mmio_write_32(0x2A620204, 0x00000011);	//DFI Training Control Register (dfi_auto_init_en)
	mmio_write_32(0x2A630204, 0x00000011);	//DFI Training Control Register (dfi_auto_init_en)
	mmio_write_32(0x2A620004, 0x00000001);	//Memory Controller Command Register (Initial command)
	mmio_write_32(0x2A630004, 0x00000001);	//Memory Controller Command Register (Initial command)

	// wait to DDR0 init_ok
	// @(posedge AXI_TB.AXI_Core.U_DDR_TOP.init_ok_u0);
	val = mmio_read_32(0x2D080148);
	val &= 0x80000000;
	while (val != 0x80000000)
	{
		val = mmio_read_32(0x2D080148);
		val &= 0x80000000;
	}

	// wait to DDR1 init_ok
	// @(posedge AXI_TB.AXI_Core.U_DDR_TOP.init_ok_u1);
	val = mmio_read_32(0x2D08014C);
	val &= 0x80000000;
	while (val != 0x80000000)
	{
		val = mmio_read_32(0x2D08014C);
		val &= 0x80000000;
	}
//	mdelay(3000);
	// ------------------- Step6 - Trigger DDR initialization and Training - end -------------------------------
}

/*******************************************************************************
 * Function that sets up the ddr
 ******************************************************************************/
void a600_tzc_init(void)
{
	unsigned long long region_base, region_top;

	/* Initialize TZC0 */
	tzc400_init(A600_TZC0_BASE);

	/* Disable filters. */
	tzc400_disable_filters();

	/* Region 0 set to no access by default */
	tzc400_configure_region0(TZC_REGION_S_NONE, 0);

	/* Region 1 set to cover all non-secure DRAM */
	region_base = 0x00000000;
	region_top  = 0xBFFFFFFF;
	tzc400_configure_region(0xF, 1,
	                        region_base,
	                        region_top,
	                        TZC_REGION_S_RDWR,
	                        0xFFFFFFFF);

	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);

	/* Enable filters. */
	tzc400_enable_filters();

	/* Initialize TZC1 */
	tzc400_init(A600_TZC1_BASE);

	/* Disable filters. */
	tzc400_disable_filters();

	/* Region 0 set to no access by default */
	tzc400_configure_region0(TZC_REGION_S_NONE, 0);

	/* Region 1 set to cover all secure DRAM */
	region_base = 0xFF800000;
	region_top  = 0xFFFFFFFF;
	tzc400_configure_region(0xF, 1,
	                        region_base,
	                        region_top,
	                        TZC_REGION_S_RDWR,
	                        0);

	/* Region 2 set to cover all non-secure shared memory DRAM. */
	region_base = 0xFF800000;
	region_top  = 0xFF83FFFF;
	tzc400_configure_region(0xF, 2,
	                        region_base,
	                        region_top,
	                        TZC_REGION_S_RDWR,
	                        0xFFFFFFFF);

	/* Region 3 set to cover all non-secure DRAM. */
	region_base = 0x80000000;
	region_top  = 0xFF7FFFFF;
	tzc400_configure_region(0xF, 3,
	                        region_base,
	                        region_top,
	                        TZC_REGION_S_RDWR,
	                        0xFFFFFFFF);



	/* Enable filters. */
	tzc400_enable_filters();
}

void a600_platform_init(void)
{
	mmio_write_32(0x2D000060, 0xFFFFFFFF);	// enable PCLK (APB clock control register of the SCU)
	mmio_write_32(0x2D000050, 0xFFFFFFFF);	// enable HCLK (AHB clock control register of the SCU)
	mmio_write_32(0x2D000080, 0xFFFFFFFF);	// enable ACLK (AXI clock control register of the SCU)
	mmio_write_32(0x2D080020, 0xFFFFFFFF);	// enable IP clock (IP_EN of the secure EXTREG)
	mmio_write_32(0x2AE00050, 0xFFFFFFFF);	// enable IP clock (IP_EN of the non-secure EXTREG)

	mmio_write_32(0x2AE00064, 0xFFFFFFFF);	// release IP reset (sw_rstn of the non-secure EXTREG)
	mmio_write_32(0x2AE0006C, 0xFFFFFFFF);	// release IP reset (sw2_rstn of the non-secure EXTREG)
	mmio_write_32(0x2D080038, 0xFFFFFFFF);	// release IP reset (sw_rstn of the secure EXTREG)

	mmio_write_32(0x2D080144, 0x00000000);	// config all of ports to non-secure (securecontrol of secure EXTREG)

	mmio_write_32(0x2AE00188, 0x00008003);	// adjust rds_delay_adj of hb_ctrl

	mmio_write_32(0x28300034, 0x00000008);	// bypass spi cache

	mmio_write_32(0x2AE00158, 0x00003401);	// usb0 pwr rst
	mmio_write_32(0x2AE00164, 0x00003401);	// usb1 pwr rst

	mmio_write_32(0x2AE00150, 0x1EA1C044);	// usb0 select extvbus
	mmio_write_32(0x2AE0015C, 0x1EA1C044);	// usb1 select extvbus

	mmio_write_32(0x2CC04000, 0x00000001);	// enable timestamp counter of coresight
	mmio_write_32(0x2CC07000, 0x00000001);	// enable timestamp counter of coresight
}

/*******************************************************************************
 * Function that sets up the translation tables.
 ******************************************************************************/
void a600_setup_page_tables(uintptr_t total_base, size_t total_size,
			    uintptr_t code_start, uintptr_t code_limit,
			    uintptr_t rodata_start, uintptr_t rodata_limit
#if USE_COHERENT_MEM
			    , uintptr_t coh_start, uintptr_t coh_limit
#endif
			    )
{
	/*
	 * Map the Trusted SRAM with appropriate memory attributes.
	 * Subsequent mappings will adjust the attributes for specific regions.
	 */
	VERBOSE("Trusted SRAM seen by this BL image: %p - %p\n",
		(void *) total_base, (void *) (total_base + total_size));
	mmap_add_region(total_base, total_base,
			total_size,
			MT_MEMORY | MT_RW | MT_SECURE);

	/* Re-map the code section */
	VERBOSE("Code region: %p - %p\n",
		(void *) code_start, (void *) code_limit);
	mmap_add_region(code_start, code_start,
			code_limit - code_start,
			MT_CODE | MT_SECURE);

	/* Re-map the read-only data section */
	VERBOSE("Read-only data region: %p - %p\n",
		(void *) rodata_start, (void *) rodata_limit);
	mmap_add_region(rodata_start, rodata_start,
			rodata_limit - rodata_start,
			MT_RO_DATA | MT_SECURE);

#if USE_COHERENT_MEM
	/* Re-map the coherent memory region */
	VERBOSE("Coherent region: %p - %p\n",
		(void *) coh_start, (void *) coh_limit);
	mmap_add_region(coh_start, coh_start,
			coh_limit - coh_start,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif

	mmap_add(plat_a600_mmap);

	init_xlat_tables();
}

/*******************************************************************************
 * Return entrypoint of BL33.
 ******************************************************************************/
uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return PLAT_A600_NS_IMAGE_OFFSET;
#endif
}

/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
uint32_t a600_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	return 0;
}

/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
uint32_t a600_get_spsr_for_bl33_entry(void)
{
#if A600_BL33_IN_AARCH32
	INFO("BL33 will boot in Non-secure AArch32 Hypervisor mode\n");
	return SPSR_MODE32(MODE32_hyp, SPSR_T_ARM, SPSR_E_LITTLE,
			   DISABLE_ALL_EXCEPTIONS);
#else
	return SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
#endif
}

unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}

uint32_t plat_ic_get_pending_interrupt_type(void)
{
	ERROR("a600: Interrupt routed to EL3.\n");
	return INTR_TYPE_INVAL;
}

uint32_t plat_interrupt_type_to_line(uint32_t type, uint32_t security_state)
{
	assert((type == INTR_TYPE_S_EL1) || (type == INTR_TYPE_EL3) ||
	       (type == INTR_TYPE_NS));

	assert(sec_state_is_valid(security_state));

	/* Non-secure interrupts are signalled on the IRQ line always. */
	if (type == INTR_TYPE_NS)
		return __builtin_ctz(SCR_IRQ_BIT);

	/* Secure interrupts are signalled on the FIQ line always. */
	return  __builtin_ctz(SCR_FIQ_BIT);
}
