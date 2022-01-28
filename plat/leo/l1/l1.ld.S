/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LEO_LD_S
#define LEO_LD_S

#include <lib/xlat_tables/xlat_tables_defs.h>
#include <platform_def.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)

ENTRY(__BL2_IMAGE_START__)

MEMORY {
	HEADER (rw) : ORIGIN = LEO_BINARY_BASE, LENGTH = 0x3000
	RAM (rwx) : ORIGIN = LEO_BINARY_BASE, LENGTH = LEO_BINARY_SIZE
}

SECTIONS
{
    /*
     * TF mapping must conform to ROM code specification.
     */
    .header : {
        __HEADER_START__ = .;
        KEEP(*(.header))
        . = ALIGN(4);
        __HEADER_END__ = .;
    } >HEADER

    . = LEO_BINARY_BASE;
    .data . : {
        . = ALIGN(PAGE_SIZE);
        __DATA_START__ = .;
        *(.data*)

        /*
         * bl2.
         * The strongest and only alignment contraint is MMU 4K page.
         * Indeed as images below will be removed, 4K pages will be re-used.
         */
        . = ( LEO_BL2_BASE - LEO_BINARY_BASE );
        __BL2_IMAGE_START__ = .;
        *(.bl2_image*)
        __BL2_IMAGE_END__ = .;

#ifndef AARCH32_SP_OPTEE
        /*
         * bl32 will be settled by bl2.
         * The strongest and only alignment constraint is 8 words to simplify
         * memraise8 assembly code.
         */
        . = ( LEO_BL32_BASE - LEO_BINARY_BASE );
        __BL32_IMAGE_START__ = .;
        *(.bl32_image*)
        __BL32_IMAGE_END__ = .;
#endif

        __DATA_END__ = .;
    } >RAM

    __TF_END__ = .;

}
#endif /* LEO_LD_S */
