/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <common/tbbr/tbbr_img_def.h>
#include <lib/utils_def.h>
#include <plat/common/common_def.h>

#include "../a600_hw.h"

/* Special value used to verify platform parameters from BL2 to BL31 */
#define A600_BL31_PLAT_PARAM_VAL        ULL(0x0F1E2D3C4B5A6978)

#define PLATFORM_STACK_SIZE             ULL(0x1000)

#define PLATFORM_MAX_CPUS_PER_CLUSTER   U(4)
#define PLATFORM_CLUSTER_COUNT          U(1)
#define PLATFORM_CLUSTER0_CORE_COUNT    PLATFORM_MAX_CPUS_PER_CLUSTER
#define PLATFORM_CORE_COUNT             PLATFORM_CLUSTER0_CORE_COUNT

#define A600_PRIMARY_CPU                U(0x000)

#define PLAT_MAX_PWR_LVL                MPIDR_AFFLVL1
#define PLAT_NUM_PWR_DOMAINS            (PLATFORM_CLUSTER_COUNT + \
                                         PLATFORM_CORE_COUNT)

#define PLAT_MAX_RET_STATE              U(1)
#define PLAT_MAX_OFF_STATE              U(2)

/* Local power state for power domains in Run state. */
#define PLAT_LOCAL_STATE_RUN            U(0)
/* Local power state for retention. Valid only for CPU power domains */
#define PLAT_LOCAL_STATE_RET            U(1)
/*
 * Local power state for OFF/power-down. Valid for CPU and cluster power
 * domains.
 */
#define PLAT_LOCAL_STATE_OFF            U(2)

/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define PLAT_LOCAL_PSTATE_WIDTH         U(4)
#define PLAT_LOCAL_PSTATE_MASK          ((U(1) << PLAT_LOCAL_PSTATE_WIDTH) - 1)

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_SHIFT           U(6)
#define CACHE_WRITEBACK_GRANULE         (U(1) << CACHE_WRITEBACK_SHIFT)

/*
 * Partition memory into secure ROM, secure "SRAM", secure DRAM and
 * non-secure DRAM. Note that this is all actually DRAM with different names.
 */
#define SEC_ROM_BASE                    ULL(0x00000000)
#define SEC_ROM_SIZE                    ULL(0x00010000)

/* We have 512K of memory reserved starting at 0K */
#define SEC_SRAM_BASE                   ULL(0x28000000)
#define SEC_SRAM_SIZE                   ULL(0x00080000)

#define SEC_DRAM0_BASE                  ULL(0x80000000)
#define SEC_DRAM0_SIZE                  ULL(0x00800000)
/* End of reserved memory */

#define NS_DRAM0_BASE                   ULL(0x80800000)
#define NS_DRAM0_SIZE                   ULL(0x1F800000)

/*
 * BL33 entrypoint.
 */
#define PLAT_A600_NS_IMAGE_OFFSET       NS_DRAM0_BASE
#define PLAT_A600_NS_IMAGE_MAX_SIZE     NS_DRAM0_SIZE

/*
 * I/O registers.
 */
#define DEVICE0_BASE                    A600_IO_APB_BASE
#define DEVICE0_SIZE                    A600_IO_APB_SIZE

#define DEVICE1_BASE                    A600_IO_AHB_BASE
#define DEVICE1_SIZE                    A600_IO_AHB_SIZE

#define DEVICE2_BASE                    A600_IO_AXI_BASE
#define DEVICE2_SIZE                    A600_IO_AXI_SIZE

/*
 * Arm TF lives in SRAM, partition it here
 */
#define BL_RAM_BASE                     SEC_SRAM_BASE
#define BL_RAM_SIZE                     SEC_SRAM_SIZE

/*
 * Mailbox to control the secondary cores.All secondary cores are held in a wait
 * loop in cold boot. To release them perform the following steps (plus any
 * additional barriers that may be needed):
 *
 *     uint64_t *entrypoint = (uint64_t *)PLAT_A600_TM_ENTRYPOINT;
 *     *entrypoint = ADDRESS_TO_JUMP_TO;
 *
 *     uint64_t *mbox_entry = (uint64_t *)PLAT_A600_TM_HOLD_BASE;
 *     mbox_entry[cpu_id] = PLAT_A600_TM_HOLD_STATE_GO;
 *
 *     sev();
 */

/* The secure entry point to be used on warm reset by all CPUs. */
#define PLAT_A600_TM_ENTRYPOINT         0x28030008//PLAT_A600_TRUSTED_MAILBOX_BASE
#define PLAT_A600_TM_ENTRYPOINT_SIZE    ULL(8)

/* Hold entries for each CPU. */
#define PLAT_A600_TM_HOLD_BASE          (PLAT_A600_TM_ENTRYPOINT + \
                                         PLAT_A600_TM_ENTRYPOINT_SIZE)
#define PLAT_A600_TM_HOLD_ENTRY_SIZE    ULL(8)
#define PLAT_A600_TM_HOLD_SIZE          (PLAT_A600_TM_HOLD_ENTRY_SIZE * \
                                         PLATFORM_CORE_COUNT)

#define PLAT_A600_TRUSTED_MAILBOX_SIZE  (PLAT_A600_TM_ENTRYPOINT_SIZE + \
                                         PLAT_A600_TM_HOLD_SIZE)

#define PLAT_A600_TM_HOLD_STATE_WAIT    ULL(0)
#define PLAT_A600_TM_HOLD_STATE_GO      ULL(1)

/*
 * BL1 specific defines.
 *
 * BL1 RW data is relocated from ROM to RAM at runtime so we need 2 sets of
 * addresses.
 *
 * Put BL1 RW at the top of the Secure SRAM. BL1_RW_BASE is calculated using
 * the current BL1 RW debug size plus a little space for growth.
 */
#define PLAT_MAX_BL1_RW_SIZE            ULL(0x10000)

#define BL1_RO_BASE                     SEC_ROM_BASE
#define BL1_RO_LIMIT                    (SEC_ROM_BASE + SEC_ROM_SIZE)
#define BL1_RW_BASE                     (BL1_RW_LIMIT - PLAT_MAX_BL1_RW_SIZE)
#define BL1_RW_LIMIT                    (BL_RAM_BASE + BL_RAM_SIZE)

/*
 * BL2 specific defines.
 *
 * Put BL2 just below BL31. BL2_BASE is calculated using the current BL2 debug
 * size plus a little space for growth.
 */
#define PLAT_MAX_BL2_SIZE               ULL(0x10000)

#define BL2_BASE                        SEC_SRAM_BASE
#define BL2_LIMIT                       (SEC_SRAM_BASE + PLAT_MAX_BL2_SIZE)

/*
 * BL31 specific defines.
 *
 * Put BL31 at the top of the Trusted SRAM. BL31_BASE is calculated using the
 * current BL31 debug size plus a little space for growth.
 */
#define PLAT_MAX_BL31_SIZE              ULL(0x10000)

#define BL31_BASE                       ULL(0xFFFE0000)
#define BL31_LIMIT                      ULL(0xFFFFFFFF)
#define BL31_PROGBITS_LIMIT             ULL(0xFFFF0000)

/*
 * BL32 specific defines.
 *
 * BL32 can execute from Secure SRAM or Secure DRAM.
 */
#define PLAT_MAX_BL32_SIZE              ULL(0x200000)

#define BL32_DRAM_BASE                  ULL(0xFFDE0000)
#define BL32_DRAM_LIMIT                 ULL(0xFFFFFFFF)

#ifdef SPD_opteed
/* Load pageable part of OP-TEE at end of allocated DRAM space for BL32 */
#define A600_OPTEE_PAGEABLE_LOAD_SIZE   0x080000 /* 512KB */
#define A600_OPTEE_PAGEABLE_LOAD_BASE   (BL32_DRAM_LIMIT - \
                                         A600_OPTEE_PAGEABLE_LOAD_SIZE)
#endif

#define SEC_SRAM_ID                     0
#define SEC_DRAM_ID                     1

#define BL32_MEM_BASE                   SEC_DRAM0_BASE
#define BL32_MEM_SIZE                   SEC_DRAM0_SIZE
#define BL32_BASE                       BL32_DRAM_BASE
#define BL32_LIMIT                      BL32_DRAM_LIMIT
#define BL32_SIZE                       (BL32_LIMIT - BL32_BASE)

#define BL33_BASE                       0x80800000
#define BL33_LIMIT                      0x80880000
#define BL33_SIZE                       (BL33_LIMIT - BL33_BASE)

#ifdef SPD_none
#undef BL32_BASE
#endif /* SPD_none */

/*
 * Other memory-related defines.
 */
#define PLAT_PHY_ADDR_SPACE_SIZE        (ULL(1) << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE       (ULL(1) << 32)

#define MAX_MMAP_REGIONS                8
#define MAX_XLAT_TABLES                 4

#define MAX_IO_DEVICES                  U(3)
#define MAX_IO_HANDLES                  U(4)

#define MAX_IO_BLOCK_DEVICES            U(1)

/*
 * DDR-related constants.
 */
#define PLAT_A600_DDRC_BASE             A600_DDRC_BASE

/*
 * Serial-related constants.
 */
#define PLAT_A600_UART_BASE             A600_UART0_BASE
#define PLAT_A600_UART_CLK_IN_HZ        A600_UART_CLK_IN_HZ
#define PLAT_A600_UART_BAUDRATE         ULL(115200)

/*
 * System counter
 */
#define SYS_COUNTER_FREQ_IN_TICKS       ULL(375000000)

#endif /* PLATFORM_DEF_H */
