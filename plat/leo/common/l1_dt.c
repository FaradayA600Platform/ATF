/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <libfdt.h>

#include <platform_def.h>

#include <common/debug.h>
#include <leo_dt.h>

static int fdt_checked = 1;
#define DT_DDR_COMPAT   "faraday,leo-ddr"

/*******************************************************************************
 * This function checks device tree file with its header.
 * Returns 0 on success and a negative FDT error code on failure.
 ******************************************************************************/
int dt_open_and_check(void)
{
  return 0;
}

/*******************************************************************************
 * This function gets the address of the DT.
 * If DT is OK, fdt_addr is filled with DT address.
 * Returns 1 if success, 0 otherwise.
 ******************************************************************************/
int fdt_get_address(void **fdt_addr)
{
	return fdt_checked;
}

/*******************************************************************************
 * This function check the presence of a node (generic use of fdt library).
 * Returns true if present, else return false.
 ******************************************************************************/
bool fdt_check_node(int node)
{
  return 0;
}

/*******************************************************************************
 * This function return global node status (generic use of fdt library).
 ******************************************************************************/
uint8_t fdt_get_status(int node)
{
  return 0;
}

/*******************************************************************************
 * This function reads a value of a node property (generic use of fdt
 * library).
 * Returns value if success, and a default value if property not found.
 * Default value is passed as parameter.
 ******************************************************************************/
uint32_t fdt_read_uint32_default(int node, const char *prop_name,
				 uint32_t dflt_value)
{
	return 0;
}

/*******************************************************************************
 * This function reads a series of parameters in a node property
 * (generic use of fdt library).
 * It reads the values inside the device tree, from property name and node.
 * The number of parameters is also indicated as entry parameter.
 * Returns 0 on success and a negative FDT error code on failure.
 * If success, values are stored at the third parameter address.
 ******************************************************************************/
int fdt_read_uint32_array(int node, const char *prop_name, uint32_t *array,
			  uint32_t count)
{
	return 0;
}

/*******************************************************************************
 * This function gets the stdout path node.
 * It reads the value indicated inside the device tree.
 * Returns node offset on success and a negative FDT error code on failure.
 ******************************************************************************/

/*******************************************************************************
 * This function fills the generic information from a given node.
 ******************************************************************************/
void dt_fill_device_info(struct dt_node_info *info, int node)
{
}

/*******************************************************************************
 * This function retrieve the generic information from DT.
 * Returns node on success and a negative FDT error code on failure.
 ******************************************************************************/
int dt_get_node(struct dt_node_info *info, int offset, const char *compat)
{
	return 0;
}

/*******************************************************************************
 * This function gets the UART instance info of stdout from the DT.
 * Returns node on success and a negative FDT error code on failure.
 ******************************************************************************/
int dt_get_stdout_uart_info(struct dt_node_info *info)
{
		return 0;
}

/*******************************************************************************
 * This function gets DDR size information from the DT.
 * Returns value in bytes on success, and 0 on failure.
 ******************************************************************************/
uint32_t dt_get_ddr_size(void)
{
		return 0;
}

/*******************************************************************************
 * This function gets DDRCTRL base address information from the DT.
 * Returns value on success, and 0 on failure.
 ******************************************************************************/
uintptr_t dt_get_ddrctrl_base(void)
{
		return 0;
}

/*******************************************************************************
 * This function gets DDRPHYC base address information from the DT.
 * Returns value on success, and 0 on failure.
 ******************************************************************************/
uintptr_t dt_get_ddrphyc_base(void)
{
		return 0;
}

/*******************************************************************************
 * This function gets PWR base address information from the DT.
 * Returns value on success, and 0 on failure.
 ******************************************************************************/
/*******************************************************************************
 * This function retrieves board model from DT
 * Returns string taken from model node, NULL otherwise
 ******************************************************************************/
const char *dt_get_board_model(void)
{
  return "leo";
}
