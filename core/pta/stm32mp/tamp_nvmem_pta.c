// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2026, STMicroelectronics
 * TAMP NVMEM PTA
 * Manages TAMP Backup Registers via NVMEM framework
 * Uses stm32_tamp_nvmem driver
 *
 */

#include <tee_api_defines.h>
#include <drivers/stm32_tamp_nvmem.h>
#include <pta_stm32mp_tamp_nvmem.h>
#include <kernel/pseudo_ta.h>
#include <kernel/ts_manager.h>
#include <string.h>
#include <crypto/crypto.h>

static TEE_Result pta_read(uint32_t param_types,
			   TEE_Param params[TEE_NUM_PARAMS])
{
	size_t cell_size = 0;
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_MEMREF_OUTPUT,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE);
	char name[MAX_NAME_LEN] = {};
	TEE_Result res = TEE_ERROR_GENERIC;

	if (param_types != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

	if (params[0].memref.size == 0 || params[0].memref.size >= MAX_NAME_LEN)
		return TEE_ERROR_BAD_PARAMETERS;

	memcpy(name, params[0].memref.buffer, params[0].memref.size);
	name[params[0].memref.size] = '\0';

	/* extract the reel size of cell */
	res = stm32_tamp_nvmem_cell_size(name, &cell_size);
	if (res)
		return res;

	/* check size of buffer CA  */
	if (params[1].memref.size < cell_size)
		return TEE_ERROR_SHORT_BUFFER;

	IMSG("Read cell '%s'", name);

	res = stm32_tamp_nvmem_read(name, params[1].memref.buffer,
				    params[1].memref.size);
	if (res) {
		EMSG("Read cell '%s' failed: 0x%x", name, res);
		return res;
	}

	/* update the size */
	params[1].memref.size = cell_size;

	return TEE_SUCCESS;
}

static TEE_Result pta_write(uint32_t param_types,
			    TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE);
	char name[MAX_NAME_LEN] = {};
	TEE_Result res = TEE_ERROR_GENERIC;

	if (param_types != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

	/* validate name length */
	if (params[0].memref.size == 0 || params[0].memref.size >= MAX_NAME_LEN)
		return TEE_ERROR_BAD_PARAMETERS;

	memcpy(name, params[0].memref.buffer, params[0].memref.size);
	name[params[0].memref.size] = '\0';

	IMSG("Write cell '%s'", name);

	res = stm32_tamp_nvmem_write(name, params[1].memref.buffer,
				     params[1].memref.size);
	if (res)
		EMSG("Write cell '%s' failed: 0x%x", name, res);

	return res;
}

static TEE_Result pta_generate(uint32_t param_types,
			       TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE);
	char name[MAX_NAME_LEN] = {};
	uint8_t buf[32] = {};
	size_t cell_size = 0;
	TEE_Result res = TEE_ERROR_GENERIC;

	if (param_types != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

	if (params[0].memref.size == 0 || params[0].memref.size >= MAX_NAME_LEN)
		return TEE_ERROR_BAD_PARAMETERS;

	memcpy(name, params[0].memref.buffer, params[0].memref.size);
	name[params[0].memref.size] = '\0';

	res = stm32_tamp_nvmem_cell_size(name, &cell_size);
	if (res)
		return res;

	/* generate key 256 bits via RNG */
	res = crypto_rng_read(buf, cell_size);
	if (res) {
		EMSG("RNG failed: 0x%x", res);
		return res;
	}

	IMSG("Generated %zu random bytes for cell '%s'", cell_size, name);

	res = stm32_tamp_nvmem_write_bhk_rng(name, buf, cell_size);
	if (res)
		EMSG("Write cell '%s' failed: 0x%x", name, res);
	else
		IMSG("Cell '%s' generated and written", name);

	/* erase buf */
	memset(buf, 0, sizeof(buf));

	return res;
}

static TEE_Result
pta_tamp_nvmem_open_session(uint32_t ptypes __unused,
			    TEE_Param par[TEE_NUM_PARAMS] __unused,
			    void **session __unused)
{
	/* Access only for TAs */

	/*
	 * struct ts_session *caller = ts_get_calling_session();
	 *
	 * if (caller && is_user_ta_ctx(caller->ctx))
	 *	return TEE_SUCCESS;
	 *
	 * EMSG("Access denied: only TAs can use tamp_nvmem PTA");
	 * return TEE_ERROR_ACCESS_DENIED;
	 */

	return TEE_SUCCESS;
}

static TEE_Result
pta_tamp_nvmem_invoke_command(void *sess __unused, uint32_t cmd_id,
			      uint32_t param_types,
			      TEE_Param params[TEE_NUM_PARAMS])
{
	IMSG(PTA_NAME " command 0x%x", cmd_id);

	switch (cmd_id) {
	case PTA_TAMP_NVMEM_CMD_READ:
		return pta_read(param_types, params);
	case PTA_TAMP_NVMEM_CMD_WRITE:
		return pta_write(param_types, params);
	case PTA_TAMP_NVMEM_CMD_GENERATE:
		return pta_generate(param_types, params);
	default:
		return TEE_ERROR_NOT_IMPLEMENTED;
	}
}

pseudo_ta_register(.uuid = PTA_TAMP_NVMEM_UUID, .name = PTA_NAME,
		   .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
		   .open_session_entry_point = pta_tamp_nvmem_open_session,
		   .invoke_command_entry_point = pta_tamp_nvmem_invoke_command);
