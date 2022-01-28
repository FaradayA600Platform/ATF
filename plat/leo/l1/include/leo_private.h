/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LEO_PRIVATE_H
#define LEO_PRIVATE_H

#include <stdint.h>

void configure_mmu(void);

void leo_arch_security_setup(void);
void leo_security_setup(void);

void leo_gic_pcpu_init(void);
void leo_gic_init(void);

void leo_syscfg_init(void);
void leo_syscfg_enable_io_compensation(void);
void leo_syscfg_disable_io_compensation(void);

#endif /* LEO_PRIVATE_H */
