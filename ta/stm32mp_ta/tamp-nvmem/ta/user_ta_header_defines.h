/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2026, STMicroelectronics
 * STM32 TAMP Backup Register driver - public API
 */

#ifndef USER_TA_HEADER_DEFINES_H
#define USER_TA_HEADER_DEFINES_H

#include "include/tamp_nvmem_ta.h"

#define TA_UUID             TAMP_NVMEM_TA_UUID
#define TA_FLAGS            0
#define TA_STACK_SIZE       (4 * 1024)
#define TA_DATA_SIZE        (32 * 1024)
#define TA_VERSION          "1.0"
#define TA_DESCRIPTION      "TAMP NVMEM TA"
#define TA_COPYRIGHT        "Copyright (c) 2026, Mohamed Tabkioui"

#endif /* USER_TA_HEADER_DEFINES_H */
