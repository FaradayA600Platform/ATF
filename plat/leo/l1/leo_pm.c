/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv2.h>
#include <dt-bindings/clock/leo-clks.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

static uintptr_t leo_sec_entrypoint;
static uint32_t cntfrq_core0;

/*******************************************************************************
 * LEO1 handler called when a CPU is about to enter standby.
 * call by core 1 to enter in wfi
 ******************************************************************************/
static void leo_cpu_standby(plat_local_state_t cpu_state)
{
	uint32_t interrupt = GIC_SPURIOUS_INTERRUPT;

	assert(cpu_state == ARM_LOCAL_STATE_RET);

	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	isb();
	dsb();
	while (interrupt == GIC_SPURIOUS_INTERRUPT) {
		wfi();

		/* Acknoledge IT */
		interrupt = gicv2_acknowledge_interrupt();
		/* If Interrupt == 1022 it will be acknowledged by non secure */
		if ((interrupt != PENDING_G1_INTID) &&
		    (interrupt != GIC_SPURIOUS_INTERRUPT)) {
			gicv2_end_of_interrupt(interrupt);
		}
	}
}

/*******************************************************************************
 * LEO1 handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 * call by core 0 to activate core 1
 ******************************************************************************/
static int leo_pwr_domain_on(u_register_t mpidr)
{
	unsigned long current_cpu_mpidr = read_mpidr_el1();
	uint32_t bkpr_core1_addr =
		tamp_bkpr(BOOT_API_CORE1_BRANCH_ADDRESS_TAMP_BCK_REG_IDX);
	uint32_t bkpr_core1_magic =
		tamp_bkpr(BOOT_API_CORE1_MAGIC_NUMBER_TAMP_BCK_REG_IDX);

	if (mpidr == current_cpu_mpidr) {
		return PSCI_E_INVALID_PARAMS;
	}

	if ((leo_sec_entrypoint < LEO_SYSRAM_BASE) ||
	    (leo_sec_entrypoint > (LEO_SYSRAM_BASE +
				     (LEO_SYSRAM_SIZE - 1)))) {
		return PSCI_E_INVALID_ADDRESS;
	}

	cntfrq_core0 = read_cntfrq_el0();

	/* Write entrypoint in backup RAM register */
	mmio_write_32(bkpr_core1_addr, leo_sec_entrypoint);

	/* Write magic number in backup register */
	mmio_write_32(bkpr_core1_magic, BOOT_API_A7_CORE1_MAGIC_NUMBER);

	/* Generate an IT to core 1 */
	gicv2_raise_sgi(ARM_IRQ_SEC_SGI_0, LEO_SECONDARY_CPU);

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * LEO1 handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
static void leo_pwr_domain_off(const psci_power_state_t *target_state)
{
	/* Nothing to do */
}

/*******************************************************************************
 * LEO1 handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
static void leo_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	/* Nothing to do, power domain is not disabled */
}

/*******************************************************************************
 * LEO1 handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 * call by core 1 just after wake up
 ******************************************************************************/
static void leo_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	leomp1_gic_pcpu_init();

	write_cntfrq_el0(cntfrq_core0);
}

/*******************************************************************************
 * LEO1 handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 ******************************************************************************/
static void leo_pwr_domain_suspend_finish(const psci_power_state_t
					    *target_state)
{
	/* Nothing to do, power domain is not disabled */
}

static void __dead2 leo_pwr_domain_pwr_down_wfi(const psci_power_state_t
						  *target_state)
{
	ERROR("leompu1 Power Down WFI: operation not handled.\n");
	panic();
}

static void __dead2 leo_system_off(void)
{
	ERROR("leompu1 System Off: operation not handled.\n");
	panic();
}

static void __dead2 leo_system_reset(void)
{
	mmio_setbits_32(leomp_rcc_base() + RCC_MP_GRSTCSETR,
			RCC_MP_GRSTCSETR_MPSYSRST);

	/* Loop in case system reset is not immediately caught */
	for ( ; ; ) {
		;
	}
}

static int leo_validate_power_state(unsigned int power_state,
				      psci_power_state_t *req_state)
{
	int pstate = psci_get_pstate_type(power_state);

	if (pstate != 0) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (psci_get_pstate_pwrlvl(power_state)) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (psci_get_pstate_id(power_state)) {
		return PSCI_E_INVALID_PARAMS;
	}

	req_state->pwr_domain_state[0] = ARM_LOCAL_STATE_RET;
	req_state->pwr_domain_state[1] = ARM_LOCAL_STATE_RUN;

	return PSCI_E_SUCCESS;
}

static int leo_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/* The non-secure entry point must be in DDR */
	if (entrypoint < LEO_DDR_BASE) {
		return PSCI_E_INVALID_ADDRESS;
	}

	return PSCI_E_SUCCESS;
}

static int leo_node_hw_state(u_register_t target_cpu,
			       unsigned int power_level)
{
	/*
	 * The format of 'power_level' is implementation-defined, but 0 must
	 * mean a CPU. Only allow level 0.
	 */
	if (power_level != MPIDR_AFFLVL0) {
		return PSCI_E_INVALID_PARAMS;
	}

	/*
	 * From psci view the CPU 0 is always ON,
	 * CPU 1 can be SUSPEND or RUNNING.
	 * Therefore do not manage POWER OFF state and always return HW_ON.
	 */

	return (int)HW_ON;
}

/*******************************************************************************
 * Export the platform handlers. The ARM Standard platform layer will take care
 * of registering the handlers with PSCI.
 ******************************************************************************/
static const plat_psci_ops_t leo_psci_ops = {
	.cpu_standby = leo_cpu_standby,
	.pwr_domain_on = leo_pwr_domain_on,
	.pwr_domain_off = leo_pwr_domain_off,
	.pwr_domain_suspend = leo_pwr_domain_suspend,
	.pwr_domain_on_finish = leo_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = leo_pwr_domain_suspend_finish,
	.pwr_domain_pwr_down_wfi = leo_pwr_domain_pwr_down_wfi,
	.system_off = leo_system_off,
	.system_reset = leo_system_reset,
	.validate_power_state = leo_validate_power_state,
	.validate_ns_entrypoint = leo_validate_ns_entrypoint,
	.get_node_hw_state = leo_node_hw_state
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	leo_sec_entrypoint = sec_entrypoint;
	*psci_ops = &leo_psci_ops;

	return 0;
}
