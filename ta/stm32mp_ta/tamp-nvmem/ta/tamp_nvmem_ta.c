// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2026, STMicroelectronics
 * TAMP NVMEM TA
 * Test TA for tamp_nvmem PTA
 */

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <pta_stm32mp_tamp_nvmem.h>
#include <string.h>
#include "include/tamp_nvmem_ta.h"

TEE_Result TA_CreateEntryPoint(void)
{
	IMSG("TAMP NVMEM TA: Created");
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
	IMSG("TAMP NVMEM TA: Destroyed");
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
				    TEE_Param params[4] __unused,
				    void **sess_ctx __unused)
{
	uint32_t exp = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
				       TEE_PARAM_TYPE_NONE,
				       TEE_PARAM_TYPE_NONE);
	if (param_types != exp)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("TAMP NVMEM TA: Session opened");
	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *sess_ctx __unused)
{
	IMSG("TAMP NVMEM TA: Session closed");
}

/*
 * Helper : call PTA with cell name + buffer
 */
static TEE_Result call_pta(uint32_t cmd, const char *name, void *buf,
			   size_t len, bool is_write)
{
	TEE_TASessionHandle pta_sess = TEE_HANDLE_NULL;
	TEE_UUID pta_uuid = PTA_TAMP_NVMEM_UUID;
	TEE_Param pta_params[4] = {};
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
	uint32_t param_types = 0;

	res = TEE_OpenTASession(&pta_uuid, TEE_TIMEOUT_INFINITE,
				TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE),
				NULL, &pta_sess, &ret_origin);

	if (res != TEE_SUCCESS) {
		EMSG("Failed to open PTA session: 0x%x", res);
		return res;
	}

	/* params[0] = cell name */
	pta_params[0].memref.buffer = (void *)name;
	pta_params[0].memref.size = strlen(name);

	if (cmd == PTA_TAMP_NVMEM_CMD_GENERATE) {
		param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
					      TEE_PARAM_TYPE_NONE,
					      TEE_PARAM_TYPE_NONE,
					      TEE_PARAM_TYPE_NONE);
	} else {
		pta_params[1].memref.buffer = buf;
		pta_params[1].memref.size = len;

		param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
					      is_write ?
					      TEE_PARAM_TYPE_MEMREF_INPUT :
					      TEE_PARAM_TYPE_MEMREF_OUTPUT,
					      TEE_PARAM_TYPE_NONE,
					      TEE_PARAM_TYPE_NONE);
	}

	res = TEE_InvokeTACommand(pta_sess, TEE_TIMEOUT_INFINITE, cmd,
				  param_types, pta_params, &ret_origin);
	TEE_CloseTASession(pta_sess);

	if (res != TEE_SUCCESS)
		EMSG("PTA command 0x%x failed: 0x%x", cmd, res);

	return res;
}

static TEE_Result cmd_read(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
					     TEE_PARAM_TYPE_MEMREF_OUTPUT,
					     TEE_PARAM_TYPE_NONE,
					     TEE_PARAM_TYPE_NONE);
	char name[32] = {};

	if (param_types != exp)
		return TEE_ERROR_BAD_PARAMETERS;

	if (params[0].memref.size == 0 || params[0].memref.size >= sizeof(name))
		return TEE_ERROR_BAD_PARAMETERS;

	memcpy(name, params[0].memref.buffer, params[0].memref.size);
	name[params[0].memref.size] = '\0';

	IMSG("TA: read cell '%s'", name);

	return call_pta(PTA_TAMP_NVMEM_CMD_READ, name, params[1].memref.buffer,
			params[1].memref.size, false);
}

static TEE_Result cmd_write(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
					     TEE_PARAM_TYPE_MEMREF_INPUT,
					     TEE_PARAM_TYPE_NONE,
					     TEE_PARAM_TYPE_NONE);
	char name[32] = {};

	if (param_types != exp)
		return TEE_ERROR_BAD_PARAMETERS;

	if (params[0].memref.size == 0 || params[0].memref.size >= sizeof(name))
		return TEE_ERROR_BAD_PARAMETERS;

	memcpy(name, params[0].memref.buffer, params[0].memref.size);
	name[params[0].memref.size] = '\0';

	IMSG("TA: write cell '%s'", name);

	return call_pta(PTA_TAMP_NVMEM_CMD_WRITE, name, params[1].memref.buffer,
			params[1].memref.size, true);
}

static TEE_Result cmd_generate(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
					     TEE_PARAM_TYPE_NONE,
					     TEE_PARAM_TYPE_NONE,
					     TEE_PARAM_TYPE_NONE);
	char name[32] = {};

	if (param_types != exp)
		return TEE_ERROR_BAD_PARAMETERS;

	if (params[0].memref.size == 0 || params[0].memref.size >= sizeof(name))
		return TEE_ERROR_BAD_PARAMETERS;

	memcpy(name, params[0].memref.buffer, params[0].memref.size);
	name[params[0].memref.size] = '\0';

	IMSG("TA: generate cell '%s'", name);

	return call_pta(PTA_TAMP_NVMEM_CMD_GENERATE, name, NULL, 0, false);
}

TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx __unused, uint32_t cmd_id,
				      uint32_t param_types, TEE_Param params[4])
{
	switch (cmd_id) {
	case TAMP_NVMEM_CMD_READ:
		return cmd_read(param_types, params);
	case TAMP_NVMEM_CMD_WRITE:
		return cmd_write(param_types, params);
	case TAMP_NVMEM_CMD_GENERATE:
		return cmd_generate(param_types, params);
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
