/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef A600_PRIVATE_H
#define A600_PRIVATE_H

#include <stdint.h>

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/

/* Utility functions */
void a600_platform_init(void);
void a600_console_init(void);
void a600_ddr_init(void);
void a600_tzc_init(void);
void a600_setup_page_tables(uintptr_t total_base, size_t total_size,
			    uintptr_t code_start, uintptr_t code_limit,
			    uintptr_t rodata_start, uintptr_t rodata_limit
#if USE_COHERENT_MEM
			    , uintptr_t coh_start, uintptr_t coh_limit
#endif
			    );

/* Optional functions required in the Raspberry Pi 3 port */
unsigned int plat_a600_calc_core_pos(u_register_t mpidr);

/* BL2 utility functions */
uint32_t a600_get_spsr_for_bl32_entry(void);
uint32_t a600_get_spsr_for_bl33_entry(void);

/* IO storage utility functions */
void plat_a600_io_setup(void);

/* Hardware RNG functions */
void a600_rng_read(void *buf, size_t len);

/* VideoCore firmware commands */
int a600_vc_hardware_get_board_revision(uint32_t *revision);

#endif /* A600_PRIVATE_H */
