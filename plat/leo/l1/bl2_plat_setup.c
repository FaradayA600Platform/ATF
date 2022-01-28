/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/st/bsec.h>
#include <drivers/leo/uart/uart_16550.h>
#include <lib/mmio.h>
#include <lib/optee_utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <leo_common.h>
#include <leo_context.h>

static console_16550_t leo_console;
void leo_console_init(void)                                                                                                                                   
{
    int console_scope = CONSOLE_FLAG_BOOT;
#if RPI3_RUNTIME_UART != -1
    console_scope |= CONSOLE_FLAG_RUNTIME;
#endif
    int rc = console_16550_register(LEO_DEBUG_USART_BASE,
             LEO_DEBUG_USART_CLK_FRQ,
             LEO_UART_BAUDRATE,
             &leo_console);
    if (rc == 0) {
    /*
     * The crash console doesn't use the multi console API, it uses
     * the core console functions directly. It is safe to call panic
     * and let it print debug information.
     */
         panic();
    }
    console_set_scope(&leo_console.console, console_scope);
}


void bl2_el3_early_platform_setup(u_register_t arg0,
				  u_register_t arg1 __unused,
				  u_register_t arg2 __unused,
				  u_register_t arg3 __unused)
{
    leo_console_init();    	
}

void bl2_plat_arch_setup(void)
{
}

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1, u_register_t arg2, u_register_t arg3)
{
}

void bl2_platform_setup(void)
{
#ifdef AARCH32_SP_OPTEE
	INFO("BL2 runs OP-TEE setup\n");
	/* Initialize tzc400 after DDR initialization */
	leo_security_setup();
#else
	INFO("BL2 runs SP_MIN setup\n");
#endif
}

void bl2_el3_plat_arch_setup(void)
{

	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);
	/* system */
	mmap_add_region(LEO1_TZC_BASE, LEO1_TZC_BASE,
			0x10000,
			MT_MEMORY | MT_RW | MT_SECURE);

	configure_mmu();
	if (dt_open_and_check() < 0) {
		panic();
	}

	leo_io_setup();
}

#if defined(AARCH32_SP_OPTEE)
/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
	bl_mem_params_node_t *bl32_mem_params;
	bl_mem_params_node_t *pager_mem_params;
	bl_mem_params_node_t *paged_mem_params;
	INFO("plat handle post image load IMAGE_ID %x\n",image_id);
	assert(bl_mem_params != NULL);

	switch (image_id) {
	case BL32_IMAGE_ID:
		INFO("bl32 : \n");
		bl_mem_params->ep_info.pc =bl_mem_params->image_info.image_base;
		INFO("bl_mem_params image base %lx\n",bl_mem_params->ep_info.pc);
		pager_mem_params = get_bl_mem_params_node(BL32_EXTRA1_IMAGE_ID);
		assert(pager_mem_params != NULL);
		pager_mem_params->image_info.image_base = LEO_OPTEE_BASE;
		pager_mem_params->image_info.image_max_size =
			LEO_OPTEE_SIZE;

		paged_mem_params = get_bl_mem_params_node(BL32_EXTRA2_IMAGE_ID);
		assert(paged_mem_params != NULL);
		paged_mem_params->image_info.image_base = LEO_DDR_BASE +
			(LEO_DDR_MAX_SIZE - LEO_DDR_S_SIZE -
			 LEO_DDR_SHMEM_SIZE);
		paged_mem_params->image_info.image_max_size =
			LEO_DDR_S_SIZE;
		INFO("OPTEE IMAGE\n");		
		err = parse_optee_header(&bl_mem_params->ep_info,
					 &pager_mem_params->image_info,
					 &paged_mem_params->image_info);
		if (err) {
			ERROR("OPTEE header parse error.\n");
			panic();
		}

		/* Set optee boot info from parsed header data */
		bl_mem_params->ep_info.pc =
				pager_mem_params->image_info.image_base;
		bl_mem_params->ep_info.args.arg0 =
				paged_mem_params->image_info.image_base;
		bl_mem_params->ep_info.args.arg1 = 0; /* Unused */
		bl_mem_params->ep_info.args.arg2 = 0; /* No DT supported */
		break;

	case BL33_IMAGE_ID:
		bl32_mem_params = get_bl_mem_params_node(BL33_IMAGE_ID);
		assert(bl32_mem_params != NULL);
		bl32_mem_params->ep_info.lr_svc = bl_mem_params->ep_info.pc;
		INFO("BL33: bl_mem parames epinfo pc %lx\n",bl_mem_params->ep_info.pc);
		break;

	default:
		/* Do nothing in default case */
		break;
	}

	return err;
}
#endif
