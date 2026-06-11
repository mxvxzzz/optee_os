/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2026, STMicroelectronics
 * STM32 TAMP Backup Register driver - public API
 */

#ifndef __DRIVERS_STM32_TAMP_NVMEM_H__
#define __DRIVERS_STM32_TAMP_NVMEM_H__

#include <tee_api_types.h>
#include <stddef.h>
#include <stdint.h>

/**
 * stm32_tamp_nvmem_cell_size() : Get the size of a BKP cell
 * @name: Cell name as defined in DTS nvmem-cell-names
 * @size:  Buffer size in bytes
 * Return TEE_SUCCESS or error code
 */
TEE_Result stm32_tamp_nvmem_cell_size(const char *name, size_t *size);

/**
 * stm32_tamp_nvmem_read() : Read a BKP cell by name
 * @name: Cell name as defined in DTS nvmem-cell-names
 * @buf:  Output buffer
 * @len:  Buffer size in bytes
 * Return TEE_SUCCESS or error code
 *
 * NOTE: "bhk_key" is write-only
 */
TEE_Result stm32_tamp_nvmem_read(const char *name, uint8_t *buf, size_t len);

/**
 * stm32_tamp_nvmem_write() : Write a BKP cell by name
 * @name: Cell name as defined in DTS nvmem-cell-names
 * @buf:  Input buffer
 * @len:  Buffer size in bytes
 * Return TEE_SUCCESS or error code
 */
TEE_Result stm32_tamp_nvmem_write(const char *name, const uint8_t *buf,
				  size_t len);

/**
 * stm32_tamp_nvmem_write_bhk_rng() : Write a BKP BHK cell by name
 * @name: Cell name as defined in DTS nvmem-cell-names
 * @buf:  Input buffer
 * @len:  Buffer size in bytes
 * Return TEE_SUCCESS or error code
 */
TEE_Result stm32_tamp_nvmem_write_bhk_rng(const char *name, const uint8_t *buf,
					  size_t len);

#endif /* __DRIVERS_STM32_TAMP_NVMEM_H__ */
