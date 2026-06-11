// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2026, STMicroelectronics
 * STM32 TAMP Backup Register driver
 *
 * Provides a generic interface to access BKP registers via NVMEM framework.
 * Acts as a DT consumer driver for "st,stm32mp-tamp-nvmem" compatible nodes.
 */

#include <drivers/nvmem.h>
#include <drivers/stm32_tamp_nvmem.h>
#include <initcall.h>
#include <kernel/dt.h>
#include <kernel/dt_driver.h>
#include <libfdt.h>
#include <malloc.h>
#include <trace.h>
#include <string_ext.h>

#define TAMP_NVMEM_MAX_CELLS 8
#define MAX_CELL_NAME_LEN 32
/* BHK cell name is write-only */
#define BHK_CELL_NAME "bhk_key"

/*
 * struct stm32_tamp_nvmem_cell : stores a parsed NVMEM cell with its name
 * @name: cell name from nvmem-cell-names
 * @cell: NVMEM cell pointer
 */
struct stm32_tamp_nvmem_cell {
	char name[MAX_CELL_NAME_LEN];
	struct nvmem_cell *cell;
	bool rng_only;
};

static struct stm32_tamp_nvmem_cell bkp_cells[TAMP_NVMEM_MAX_CELLS];
static size_t bkp_cells_count;
static bool initialized;

/*
 * Find a cell by name
 * TEE_Result nvmem_get_cell_by_name() : parse FDT each call
 * find_cell() : uses the cells already resolved in probe()
 */
static struct stm32_tamp_nvmem_cell *find_cell(const char *name)
{
	size_t i = 0;

	for (i = 0; i < bkp_cells_count; i++)
		if (strcmp(bkp_cells[i].name, name) == 0)
			return &bkp_cells[i];

	return NULL;
}

TEE_Result stm32_tamp_nvmem_cell_size(const char *name, size_t *size)
{
	struct stm32_tamp_nvmem_cell *entry = NULL;

	if (!name || !size)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!initialized)
		return TEE_ERROR_BAD_STATE;

	entry = find_cell(name);
	if (!entry)
		return TEE_ERROR_ITEM_NOT_FOUND;

	*size = entry->cell->len;
	return TEE_SUCCESS;
}

TEE_Result stm32_tamp_nvmem_read(const char *name, uint8_t *buf, size_t len)
{
	struct stm32_tamp_nvmem_cell *entry = NULL;

	if (!name || !buf || !len)
		return TEE_ERROR_BAD_PARAMETERS;

	if (strcmp(name, BHK_CELL_NAME) == 0) {
		EMSG("BHK cell '%s' is write-only", name);
		return TEE_ERROR_ACCESS_DENIED;
	}

	if (!initialized) {
		EMSG("Driver not initialized");
		return TEE_ERROR_BAD_STATE;
	}

	entry = find_cell(name);
	if (!entry) {
		EMSG("Cell '%s' not found", name);
		return TEE_ERROR_ITEM_NOT_FOUND;
	}

	if (len < entry->cell->len) {
		EMSG("Buffer too small: %zu < %zu", len, entry->cell->len);
		return TEE_ERROR_SHORT_BUFFER;
	}

	return nvmem_cell_read(entry->cell, buf);
}

TEE_Result stm32_tamp_nvmem_write(const char *name, const uint8_t *buf,
				  size_t len)
{
	struct stm32_tamp_nvmem_cell *entry = NULL;

	if (!name || !buf || !len)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!initialized) {
		EMSG("Driver not initialized");
		return TEE_ERROR_BAD_STATE;
	}

	entry = find_cell(name);
	if (!entry) {
		EMSG("Cell '%s' not found", name);
		return TEE_ERROR_ITEM_NOT_FOUND;
	}

	if (entry->rng_only) {
		EMSG("Cell '%s' is RNG-only", name);
		return TEE_ERROR_ACCESS_DENIED;
	}

	if (len != entry->cell->len) {
		EMSG("Wrong size: got %zu expected %zu", len, entry->cell->len);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	return nvmem_cell_write(entry->cell, (uint8_t *)(uintptr_t)buf, len);
}

TEE_Result stm32_tamp_nvmem_write_bhk_rng(const char *name, const uint8_t *buf,
					  size_t len)
{
	struct stm32_tamp_nvmem_cell *entry = NULL;

	if (!name || !buf || !len)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!initialized)
		return TEE_ERROR_BAD_STATE;

	entry = find_cell(name);
	if (!entry)
		return TEE_ERROR_ITEM_NOT_FOUND;

	if (len != entry->cell->len)
		return TEE_ERROR_BAD_PARAMETERS;

	return nvmem_cell_write(entry->cell, (uint8_t *)(uintptr_t)buf, len);
}

/*
 * probe() : parse DTS and get NVMEM cells
 * Called at boot by DT driver framework
 */
static TEE_Result stm32_tamp_nvmem_probe(const void *fdt, int node,
					 const void *compat_data __unused)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	const char *cell_name = NULL;
	struct nvmem_cell *cell = NULL;
	int count = 0;
	int i = 0;
	size_t copied = 0;

	if (initialized) {
		IMSG("stm32_tamp_nvmem: already initialized");
		return TEE_SUCCESS;
	}

	IMSG("stm32_tamp_nvmem: probe started");

	/* Get number of cells */
	count = fdt_stringlist_count(fdt, node, "nvmem-cell-names");
	if (count <= 0) {
		EMSG("No nvmem-cell-names found");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if ((size_t)count > TAMP_NVMEM_MAX_CELLS) {
		EMSG("Too many cells: %d > %d", count, TAMP_NVMEM_MAX_CELLS);
		return TEE_ERROR_EXCESS_DATA;
	}

	/* Parse each cell */
	for (i = 0; i < count; i++) {
		cell_name = fdt_stringlist_get(fdt, node, "nvmem-cell-names", i,
					       NULL);
		if (!cell_name) {
			EMSG("Failed to get cell name at index %d", i);
			res = TEE_ERROR_GENERIC;
			goto err_free;
		}

		copied = strlcpy(bkp_cells[i].name, cell_name,
				 sizeof(bkp_cells[i].name));
		if (copied >= sizeof(bkp_cells[i].name)) {
			EMSG("Cell name too long: '%s'", cell_name);
			res = TEE_ERROR_EXCESS_DATA;
			goto err_free;
		}

		res = nvmem_get_cell_by_index(fdt, node, i, &cell);
		if (res) {
			EMSG("Failed to get cell '%s': 0x%x", cell_name, res);
			goto err_free;
		}

		bkp_cells[i].cell = cell;
		bkp_cells_count++;

		/* BHK est write-only via RNG */
		if (strcmp(cell_name, BHK_CELL_NAME) == 0)
			bkp_cells[i].rng_only = true;

		IMSG("Cell '%s' registered (offset=0x%lx len=%zu)", cell_name,
		     (unsigned long)cell->offset, cell->len);
	}

	initialized = true;
	IMSG("stm32_tamp_nvmem: %zu cells registered", bkp_cells_count);
	return TEE_SUCCESS;

err_free:
	for (i = 0; i < (int)bkp_cells_count; i++) {
		nvmem_put_cell(bkp_cells[i].cell);
		bkp_cells[i].cell = NULL;
		bkp_cells[i].name[0] = '\0';
	}
	bkp_cells_count = 0;
	return res;
}

static const struct dt_device_match stm32_tamp_nvmem_match[] = {
	{ .compatible = "st,stm32mp-tamp-nvmem" },
	{}
};

DEFINE_DT_DRIVER(stm32_tamp_nvmem_dt_driver) = {
	.name = "stm32_tamp_nvmem",
	.match_table = stm32_tamp_nvmem_match,
	.probe = stm32_tamp_nvmem_probe,
};
