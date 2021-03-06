/*
 * Copyright (C) 2018-2019, STMicroelectronics - All Rights Reserved
 * Copyright (c) 2018-2019, Linaro Limited
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LEO_COMMON_H
#define LEO_COMMON_H

#include <stdbool.h>

#include <arch_helpers.h>

/* Functions to save and get boot context address given by ROM code */
void LEO_save_boot_ctx_address(uintptr_t address);
uintptr_t LEO_get_boot_ctx_address(void);

/* Return the base address of the DDR controller */
uintptr_t LEO_ddrctrl_base(void);

/* Return the base address of the DDR PHY */
uintptr_t LEO_ddrphyc_base(void);

/* Return the base address of the PWR peripheral */
uintptr_t LEO_pwr_base(void);

/* Return the base address of the RCC peripheral */
uintptr_t LEO_rcc_base(void);

void leo_io_setup(void);

/*
 * Platform util functions for the GPIO driver
 * @bank: Target GPIO bank ID as per DT bindings
 *
 * Platform shall implement these functions to provide to leo_gpio
 * driver the resource reference for a target GPIO bank. That are
 * memory mapped interface base address, interface offset (see below)
 * and clock identifier.
 *
 * leo_get_gpio_bank_offset() returns a bank offset that is used to
 * check DT configuration matches platform implementation of the banks
 * description.
 */
uintptr_t leo_get_gpio_bank_base(unsigned int bank);
unsigned long leo_get_gpio_bank_clock(unsigned int bank);
uint32_t leo_get_gpio_bank_offset(unsigned int bank);

/*
 * Util for clock gating and to get clock rate for leo and platform drivers
 * @id: Target clock ID, ID used in clock DT bindings
 */
bool LEO_clk_is_enabled(unsigned long id);
void LEO_clk_enable(unsigned long id);
void LEO_clk_disable(unsigned long id);
unsigned long LEO_clk_get_rate(unsigned long id);

/* Initialise the IO layer and register platform IO devices */
void LEO_io_setup(void);

static inline uint64_t arm_cnt_us2cnt(uint32_t us)
{
	return ((uint64_t)us * (uint64_t)read_cntfrq()) / 1000000ULL;
}

static inline uint64_t timeout_init_us(uint32_t us)
{
	return read_cntpct_el0() + arm_cnt_us2cnt(us);
}

static inline bool timeout_elapsed(uint64_t expire)
{
	return read_cntpct_el0() > expire;
}

#endif /* LEO_COMMON_H */
