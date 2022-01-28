/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_dummy.h>
#include <drivers/io/io_storage.h>
#include <drivers/partition/partition.h>
#include <drivers/leo/io_leoimage.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

/* IO devices */
static const io_dev_connector_t *dummy_dev_con;
static uintptr_t dummy_dev_handle;
static uintptr_t dummy_dev_spec;

//static uintptr_t image_dev_handle;
#define MMC_BLOCK_SIZE 512
static io_block_spec_t gpt_block_spec = {
	.offset = 0,
	.length = 34 * MMC_BLOCK_SIZE, /* Size of GPT table */
};

static uintptr_t storage_dev_handle;

#ifdef AARCH32_SP_OPTEE
static const struct leoimage_part_info optee_header_partition_spec = {
	.name = OPTEE_HEADER_IMAGE_NAME,
	.binary_type = OPTEE_HEADER_BINARY_TYPE,
};

static const struct leoimage_part_info optee_pager_partition_spec = {
	.name = OPTEE_PAGER_IMAGE_NAME,
	.binary_type = OPTEE_PAGER_BINARY_TYPE,
};

static const struct leoimage_part_info optee_paged_partition_spec = {
	.name = OPTEE_PAGED_IMAGE_NAME,
	.binary_type = OPTEE_PAGED_BINARY_TYPE,
};
#else
static const io_block_spec_t bl32_block_spec = {
	.offset = BL32_BASE,
	.length = LEO_BL32_SIZE
};
#endif

static const io_block_spec_t bl2_block_spec = {
	.offset = BL2_BASE,
	.length = LEO_BL2_SIZE,
};

static const struct leoimage_part_info bl33_partition_spec = {
	.name = BL33_IMAGE_NAME,
	.binary_type = BL33_BINARY_TYPE,
};

enum IMG_BALA{
	IMG_IDX_BL33,
#ifdef AARCH32_SP_OPTEE
	IMG_IDX_OPTEE_HEADER,
	IMG_IDX_OPTEE_PAGER,
	IMG_IDX_OPTEE_PAGED,
#endif
	IMG_IDX_NUM
};

static io_block_spec_t leoimage_block_spec = {
	.offset = 0,
	.length = 0,
};

static int open_dummy(const uintptr_t spec);
//static int open_image(const uintptr_t spec);
static int open_storage(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

static const struct plat_io_policy policies[] = {
	[BL2_IMAGE_ID] = {
		.dev_handle = &dummy_dev_handle,
		.image_spec = (uintptr_t)&bl2_block_spec,
		.check = open_dummy
	},
#ifdef AARCH32_SP_OPTEE
	[BL32_IMAGE_ID] = {
		.dev_handle = &dummy_dev_handle,
		.image_spec = (uintptr_t)&optee_header_partition_spec,
		.check = open_dummy
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		.dev_handle = &dummy_dev_handle,
		.image_spec = (uintptr_t)&optee_pager_partition_spec,
		.check = open_dummy
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		.dev_handle = &dummy_dev_handle,
		.image_spec = (uintptr_t)&optee_paged_partition_spec,
		.check = open_dummy
	},
#else
	[BL32_IMAGE_ID] = {
		.dev_handle = &dummy_dev_handle,
		.image_spec = (uintptr_t)&bl32_block_spec,
		.check = open_dummy
	},
#endif
	[BL33_IMAGE_ID] = {
		.dev_handle = &dummy_dev_handle,
		.image_spec = (uintptr_t)&bl33_partition_spec,
		.check = open_dummy
	},
	[GPT_IMAGE_ID] = {
		.dev_handle = &storage_dev_handle,
		.image_spec = (uintptr_t)&gpt_block_spec,
		.check = open_storage
	},
	[STM32_IMAGE_ID] = {
		.dev_handle = &storage_dev_handle,
		.image_spec = (uintptr_t)&leoimage_block_spec,
		.check = open_storage
	}
};

static int open_dummy(const uintptr_t spec)
{
	return io_dev_init(dummy_dev_handle, 0);
}
/*
static int open_image(const uintptr_t spec)
{
	return io_dev_init(image_dev_handle, 0);
}
*/

static int open_storage(const uintptr_t spec)
{
	return io_dev_init(storage_dev_handle, 0);
}

void leo_io_setup(void)
{
	int io_result __unused;

	io_result = register_io_dev_dummy(&dummy_dev_con);
	assert(io_result == 0);

	io_result = io_dev_open(dummy_dev_con, dummy_dev_spec,
				&dummy_dev_handle);
	assert(io_result == 0);

}

/*
 * Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy.
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int rc;
	const struct plat_io_policy *policy;

	assert(image_id < ARRAY_SIZE(policies));

	policy = &policies[image_id];
	rc = policy->check(policy->image_spec);
	if (rc == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	}

	return rc;
}
