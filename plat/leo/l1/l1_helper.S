/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>

	.globl	platform_mem_init
	.globl	plat_report_exception
	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_reset_handler
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_flush
	.globl	plat_crash_console_putc
	.globl	plat_panic_handler

func platform_mem_init
	/* Nothing to do, don't need to init SYSRAM */
	bx	lr
endfunc platform_mem_init

func plat_report_exception
	bx	lr
endfunc plat_report_exception

func plat_reset_handler
	bx	lr
endfunc plat_reset_handler

	/* ------------------------------------------------------------------
	 * unsigned long plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between a cold and warm
	 * boot.
	 *
	 * Currently supports only cold boot
	 * ------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	mov	r0, #0
	bx	lr
endfunc plat_get_my_entrypoint

	/* ---------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * Cold-booting secondary CPUs is not supported.
	 * ---------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	b	.
endfunc plat_secondary_cold_boot_setup

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary cpu.
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	ldcopr	r0, MPIDR
	ldr	r1, =(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	and	r0, r1
	cmp	r0, #LEO_PRIMARY_CPU
	moveq	r0, #1
	movne	r0, #0
	bx	lr
endfunc plat_is_my_cpu_primary

	/* -------------------------------------------
	 *  int plat_leo_get_core_pos(int mpidr);
	 *
	 *  Return CorePos = (ClusterId * 4) + CoreId
	 * -------------------------------------------
	 */
func plat_leo_get_core_pos
	and	r1, r0, #MPIDR_CPU_MASK
	and	r0, r0, #MPIDR_CLUSTER_MASK
	add	r0, r1, r0, LSR #6
	bx	lr
endfunc plat_leo_get_core_pos

	/* ------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 * ------------------------------------
	 */
func plat_my_core_pos
	ldcopr	r0, MPIDR
	b	plat_leo_get_core_pos
endfunc plat_my_core_pos

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 *
	 * Initialize the crash console without a C Runtime stack.
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	b	console_16550_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_flush(void)
	 *
	 * Flush the crash console without a C Runtime stack.
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	b	console_16550_core_flush
endfunc plat_crash_console_flush

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 *
	 * Print a character on the crash console without a C Runtime stack.
	 * Clobber list : r1 - r3
	 *
	 * In case of bootloading through uart, we keep console crash as this.
	 * Characters could be sent to the programmer, but will be ignored.
	 * No specific code in that case.
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	b	console_16550_putc
endfunc plat_crash_console_putc
