/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2026, Mohamed Tabkioui
 * TAMP NVMEM PTA : public header for TAs & CAs
 */

#ifndef PTA_STM32MP_TAMP_NVMEM_H
#define PTA_STM32MP_TAMP_NVMEM_H

#define PTA_NAME "tamp_nvmem.pta"
#define PTA_TAMP_NVMEM_UUID                                            \
	{                                                              \
		0x2d6d308b, 0x8ff1, 0x4083,                            \
		{                                                      \
			0xae, 0x7c, 0x58, 0x61, 0x14, 0xc4, 0xcc, 0xd5 \
		}                                                      \
	}

#define PTA_TAMP_NVMEM_CMD_READ 0x0
#define PTA_TAMP_NVMEM_CMD_WRITE 0x1
#define PTA_TAMP_NVMEM_CMD_GENERATE 0x2

#define MAX_NAME_LEN 32

#endif /* PTA_STM32MP_TAMP_NVMEM_H */
