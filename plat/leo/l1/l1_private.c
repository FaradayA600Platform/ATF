/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <lib/xlat_tables/xlat_tables_v2.h>


#define MAP_SRAM	MAP_REGION_FLAT(LEO_DDR_BASE, \
					LEO_DDR_S_SIZE, \
					MT_MEMORY | \
					MT_RW | \
					MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_DEVICE1	MAP_REGION_FLAT(LEO_DEVICE1_BASE, \
					LEO_DEVICE1_SIZE, \
					MT_DEVICE | \
					MT_RW | \
					MT_SECURE | \
					MT_EXECUTE_NEVER)
/*
#define MAP_DEVICE2	MAP_REGION_FLAT(LEO_DEVICE2_BASE, \
					LEO_DEVICE2_SIZE, \
					MT_DEVICE | \
					MT_RW | \
					MT_SECURE | \
					MT_EXECUTE_NEVER)
*/
#if defined(IMAGE_BL2)
static const mmap_region_t leo_mmap[] = {
	MAP_SRAM,
	MAP_DEVICE1,
//	MAP_DEVICE2,
	{0}
};
#endif
#if defined(IMAGE_BL32)
static const mmap_region_t leo_mmap[] = {
	MAP_SRAM,
	MAP_DEVICE1,
	MAP_DEVICE2,
	{0}
};
#endif

void configure_mmu(void)
{
	mmap_add(leo_mmap);
	init_xlat_tables();

	enable_mmu_svc_mon(0);
}
