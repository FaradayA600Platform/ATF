/*
 * Copyright (c) 2016-2019, Faraday - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LEO_SMC_H
#define LEO_SMC_H

/*
 * LEO_SMC_BSEC call API
 *
 * Argument a0: (input) SMCC ID
 *		(output) status return code
 * Argument a1: (input) Service ID (LEO_SMC_BSEC_xxx)
 * Argument a2: (input) OTP index
 *		(output) OTP read value, if applicable
 * Argument a3: (input) OTP value if applicable
 */
#define LEO_SMC_BSEC			0x82001003

/* SMC function IDs for SiP Service queries */
#define LEO_SIP_SVC_CALL_COUNT	0x8200ff00
#define LEO_SIP_SVC_UID		0x8200ff01
/*					0x8200ff02 is reserved */
#define LEO_SIP_SVC_VERSION		0x8200ff03

/* LEO SiP Service Calls version numbers */
#define LEO_SIP_SVC_VERSION_MAJOR	0x0
#define LEO_SIP_SVC_VERSION_MINOR	0x1

/* Number of LEO SiP Calls implemented */
#define LEO_COMMON_SIP_NUM_CALLS	4

/* Service for BSEC */
#define LEO_SMC_READ_SHADOW		0x01
#define LEO_SMC_PROG_OTP		0x02
#define LEO_SMC_WRITE_SHADOW		0x03
#define LEO_SMC_READ_OTP		0x04

#endif /* LEO_SMC_H */
