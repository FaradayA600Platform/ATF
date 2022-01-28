/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <lib/mmio.h>

#include "a600_hw.h"

/* Initial amount of values to discard */
#define RNG_WARMUP_COUNT	U(0x40000)

static void a600_rng_initialize(void)
{
	uint32_t int_mask, ctrl;

	/* Return if it is already enabled */
	ctrl = mmio_read_32(A600_RNG_BASE + A600_RNG_CTRL_OFFSET);
	if ((ctrl & A600_RNG_CTRL_ENABLE) != 0U) {
		return;
	}

	/* Mask interrupts */
	int_mask = mmio_read_32(A600_RNG_BASE + A600_RNG_INT_MASK_OFFSET);
	int_mask |= A600_RNG_INT_MASK_DISABLE;
	mmio_write_32(A600_RNG_BASE + A600_RNG_INT_MASK_OFFSET, int_mask);

	/* Discard several values when initializing to give it time to warmup */
	mmio_write_32(A600_RNG_BASE + A600_RNG_STATUS_OFFSET, RNG_WARMUP_COUNT);

	mmio_write_32(A600_RNG_BASE + A600_RNG_CTRL_OFFSET,
		      A600_RNG_CTRL_ENABLE);
}

static uint32_t a600_rng_get_word(void)
{
	size_t nwords;

	do {
		/* Get number of available words to read */
		nwords = (mmio_read_32(A600_RNG_BASE + A600_RNG_STATUS_OFFSET)
				       >> A600_RNG_STATUS_NUM_WORDS_SHIFT)
				       & A600_RNG_STATUS_NUM_WORDS_MASK;
	} while (nwords == 0U);

	return mmio_read_32(A600_RNG_BASE + A600_RNG_DATA_OFFSET);
}

void a600_rng_read(void *buf, size_t len)
{
	uint32_t data;
	size_t left = len;
	uint32_t *dst = buf;

	assert(buf != NULL);
	assert(len != 0U);
	assert(check_uptr_overflow((uintptr_t) buf, (uintptr_t) len) == 0);

	a600_rng_initialize();

	while (left >= sizeof(uint32_t)) {
		data = a600_rng_get_word();
		*dst++ = data;
		left -= sizeof(uint32_t);
	}

	if (left > 0U) {
		data = a600_rng_get_word();
		memcpy(dst, &data, left);
	}
}
