/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef A600_HW_H
#define A600_HW_H

#include <lib/utils_def.h>

/*
 * Peripherals
 */
#define A600_IO_APB_BASE            ULL(0x20000000)
#define A600_IO_APB_SIZE            ULL(0x04000000)

#define A600_IO_AHB_BASE            ULL(0x24000000)
#define A600_IO_AHB_SIZE            ULL(0x04000000)

#define A600_IO_AXI_BASE            ULL(0x28000000)
#define A600_IO_AXI_SIZE            ULL(0x08000000)

/*
 * Power management, reset controller, watchdog.
 */
#define A600_IO_PM_OFFSET		ULL(0x00100000)
#define A600_PM_BASE			(A600_IO_APB_BASE + A600_IO_PM_OFFSET)
/* Registers on top of A600_PM_BASE. */
#define A600_PM_RSTC_OFFSET		ULL(0x0000001C)
#define A600_PM_RSTS_OFFSET		ULL(0x00000020)
#define A600_PM_WDOG_OFFSET		ULL(0x00000024)
/* Watchdog constants */
#define A600_PM_PASSWORD		U(0x5A000000)
#define A600_PM_RSTC_WRCFG_MASK		U(0x00000030)
#define A600_PM_RSTC_WRCFG_FULL_RESET	U(0x00000020)
/*
 * The RSTS register is used by the VideoCore firmware when booting the
 * Raspberry Pi to know which partition to boot from. The partition value is
 * formed by bits 0, 2, 4, 6, 8 and 10. Partition 63 is used by said firmware
 * to indicate halt.
 */
#define A600_PM_RSTS_WRCFG_HALT		U(0x00000555)

/*
 * Hardware random number generator.
 */
#define A600_IO_RNG_OFFSET		ULL(0x00104000)
#define A600_RNG_BASE			(A600_IO_APB_BASE + A600_IO_RNG_OFFSET)
#define A600_RNG_CTRL_OFFSET		ULL(0x00000000)
#define A600_RNG_STATUS_OFFSET		ULL(0x00000004)
#define A600_RNG_DATA_OFFSET		ULL(0x00000008)
#define A600_RNG_INT_MASK_OFFSET	ULL(0x00000010)
/* Enable/disable RNG */
#define A600_RNG_CTRL_ENABLE		U(0x1)
#define A600_RNG_CTRL_DISABLE		U(0x0)
/* Number of currently available words */
#define A600_RNG_STATUS_NUM_WORDS_SHIFT	U(24)
#define A600_RNG_STATUS_NUM_WORDS_MASK	U(0xFF)
/* Value to mask interrupts caused by the RNG */
#define A600_RNG_INT_MASK_DISABLE	U(0x1)

/*
 * I2C controller
 */
#define A600_IO_I2C0_OFFSET         ULL(0x00100000)
#define A600_IO_I2C1_OFFSET         ULL(0x00200000)
#define A600_IO_I2C2_OFFSET         ULL(0x00300000)
#define A600_IO_I2C3_OFFSET         ULL(0x00400000)
#define A600_I2C0_BASE              (A600_IO_APB_BASE + A600_IO_I2C0_OFFSET)
#define A600_I2C1_BASE              (A600_IO_APB_BASE + A600_IO_I2C1_OFFSET)
#define A600_I2C2_BASE              (A600_IO_APB_BASE + A600_IO_I2C2_OFFSET)
#define A600_I2C3_BASE              (A600_IO_APB_BASE + A600_IO_I2C3_OFFSET)

/*
 * Serial port
 */
#define A600_IO_UART0_OFFSET        ULL(0x00700000)
#define A600_IO_UART1_OFFSET        ULL(0x00800000)
#define A600_IO_UART2_OFFSET        ULL(0x00900000)
#define A600_IO_UART3_OFFSET        ULL(0x00A00000)
#define A600_UART0_BASE             (A600_IO_APB_BASE + A600_IO_UART0_OFFSET)
#define A600_UART1_BASE             (A600_IO_APB_BASE + A600_IO_UART1_OFFSET)
#define A600_UART2_BASE             (A600_IO_APB_BASE + A600_IO_UART2_OFFSET)
#define A600_UART3_BASE             (A600_IO_APB_BASE + A600_IO_UART3_OFFSET)
#define A600_UART_CLK_IN_HZ         ULL(58823529)

/*
 * TZC
 */
#define A600_IO_TZC0_OFFSET         ULL(0x05200000)
#define A600_IO_TZC1_OFFSET         ULL(0x05300000)
#define A600_TZC0_BASE             (A600_IO_AXI_BASE + A600_IO_TZC0_OFFSET)
#define A600_TZC1_BASE             (A600_IO_AXI_BASE + A600_IO_TZC1_OFFSET)

/*
 * GPIO controller
 */
#define A600_IO_GPIO_OFFSET         ULL(0x00E00000)
#define A600_GPIO_BASE              (A600_IO_APB_BASE + A600_IO_GPIO_OFFSET)

/*
 * Interrupt controller
 */
#define A600_IO_GICD_OFFSET         ULL(0x00601000)
#define A600_IO_GICC_OFFSET         ULL(0x00602000)
#define A600_GICD_BASE              (A600_IO_AXI_BASE + A600_IO_GICD_OFFSET)
#define A600_GICC_BASE              (A600_IO_AXI_BASE + A600_IO_GICC_OFFSET)

/*
 * Local interrupt controller
 */
#define A600_INTC_BASE_ADDRESS			ULL(0x40000000)
/* Registers on top of A600_INTC_BASE_ADDRESS */
#define A600_INTC_CONTROL_OFFSET		ULL(0x00000000)
#define A600_INTC_PRESCALER_OFFSET		ULL(0x00000008)
#define A600_INTC_MBOX_CONTROL_OFFSET		ULL(0x00000050)
#define A600_INTC_MBOX_CONTROL_SLOT3_FIQ	ULL(0x00000080)
#define A600_INTC_PENDING_FIQ_OFFSET		ULL(0x00000070)
#define A600_INTC_PENDING_FIQ_MBOX3		ULL(0x00000080)

#endif /* A600_HW_H */
