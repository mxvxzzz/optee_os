/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2026, Mohamed Tabkioui
 * TAMP Backup Register PTA - Tamper flag in BKP12R
 */

#ifndef __PTA_STM32MP_TAMP_BKP_H
#define __PTA_STM32MP_TAMP_BKP_H

/*
 * UUID generated with uuidgen
 */
#define PTA_TAMP_BKP_UUID { 0x2b7e151b, 0x16f1, 0x4a6f, \
    { 0xa5, 0x2e, 0xd1, 0x00, 0x00, 0x00, 0x00, 0x01 } }

#define PTA_NAME "tamp_bkp.pta"

/*
 * Write tamper health flag (0xA5A5A5A5) into BKP12R
 * Called at boot to initialize the flag
 *
 * No parameters
 *
 * Return codes:
 * TEE_SUCCESS            - Flag written successfully
 * TEE_ERROR_NOT_SUPPORTED - NVMEM not available
 */
#define PTA_TAMP_BKP_CMD_WRITE_FLAG   0x0

/*
 * Read tamper health flag from BKP12R
 *
 * [out] value[0].a  Flag value :
 *                   0xA5A5A5A5 = system healthy
 *                   0x00000000 = tamper occurred !
 *
 * Return codes:
 * TEE_SUCCESS            - Flag read successfully
 * TEE_ERROR_NOT_SUPPORTED - NVMEM not available
 */
#define PTA_TAMP_BKP_CMD_READ_FLAG    0x1

/* Health flag value written at boot */
#define PTA_TAMP_BKP_HEALTH_FLAG      0xA5A5A5A5U

#endif /* __PTA_STM32MP_TAMP_BKP_H */