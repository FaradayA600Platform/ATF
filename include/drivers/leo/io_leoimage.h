/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IO_STM32IMAGE_H
#define IO_STM32IMAGE_H

#include <drivers/io/io_driver.h>
#include <drivers/partition/partition.h>

#define MAX_LBA_SIZE		512
#define MAX_PART_NAME_SIZE	(EFI_NAMELEN + 1)
#define LEO_PART_NUM		6//(PLAT_PARTITION_MAX_ENTRIES - STM32_TF_A_COPIES)

struct leoimage_part_info {
	char name[MAX_PART_NAME_SIZE];
	uint32_t binary_type;
	uintptr_t part_offset;
	uint32_t bkp_offset;
};

struct leoimage_device_info {
	struct leoimage_part_info part_info[LEO_PART_NUM];
	uint32_t device_size;
	uint32_t lba_size;
};

int register_io_dev_leoimage(const io_dev_connector_t **dev_con);

#endif /* IO_STM32IMAGE_H */
