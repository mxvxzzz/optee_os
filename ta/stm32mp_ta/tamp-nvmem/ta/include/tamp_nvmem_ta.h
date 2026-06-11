/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2026, STMicroelectronics
 * STM32 TAMP Backup Register driver - public API
 */

#ifndef TAMP_NVMEM_TA_H
#define TAMP_NVMEM_TA_H

/* UUID TA with uuidgen */
#define TAMP_NVMEM_TA_UUID \
	{ 0x67a60c43, 0x5e3b, 0x4981, \
		{ 0x86, 0x9f, 0xf7, 0xb9, 0x17, 0x14, 0xe1, 0xa0 } }

/* TA commands */
#define TAMP_NVMEM_CMD_READ		0x0
#define TAMP_NVMEM_CMD_WRITE		0x1
#define TAMP_NVMEM_CMD_GENERATE	0x2

#endif /* TAMP_NVMEM_TA_H */
