// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2026, Mohamed Tabkioui
 * TAMP Backup Register PTA
 * Manages the tamper flag in BKP12R via NVMEM API
 */

#include <drivers/nvmem.h>
#include <kernel/pseudo_ta.h>
#include <pta_stm32mp_tamp_bkp.h>
#include <libfdt.h>
#include <kernel/dt.h>
#include <string.h>

/* BKP12R offset = 12 * 4 = 0x30 */
#define TAMP_FLAG_BKP_OFFSET    0x30U
#define TAMP_FLAG_BKP_SIZE      4U

/*
 * Helper : get the nvmem cell for the tamper flag
 * Caller must call nvmem_put_cell() after use

 * for memo : change in dts and fdt_node_offset_by_compatible (st,stm32mp25-tamp-nvram // st,tamp-nvram )
 */
static TEE_Result get_tamper_flag_cell(struct nvmem_cell **cell)
{
    const void *fdt = NULL;
    int node = -1;
    TEE_Result res = TEE_ERROR_GENERIC;

    fdt = get_embedded_dt();
    if (!fdt)
        return TEE_ERROR_NOT_SUPPORTED;

    node = fdt_node_offset_by_compatible(fdt, -1,
                                          "st,tamp-nvram");
    if (node < 0){
        EMSG("tamp-nvram node not found");
        return TEE_ERROR_NOT_SUPPORTED;
    }

    res = nvmem_get_cell_by_name(fdt, node, "tamper_flag", cell);
    if (res)
        return res;

    return TEE_SUCCESS;
}

/*
 * CMD_WRITE_FLAG : write 0xA5A5A5A5 into BKP12R
 * Called at boot to initialize the flag
 */
static TEE_Result pta_write_tamper_flag(uint32_t param_types,
                                         TEE_Param params[TEE_NUM_PARAMS]
                                         __unused)
{
    const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                                             TEE_PARAM_TYPE_NONE,
                                             TEE_PARAM_TYPE_NONE,
                                             TEE_PARAM_TYPE_NONE);
    struct nvmem_cell *cell = NULL;
    uint32_t flag = PTA_TAMP_BKP_HEALTH_FLAG;
    TEE_Result res = TEE_ERROR_GENERIC;

    if (param_types != exp_pt)
        return TEE_ERROR_BAD_PARAMETERS;

    res = get_tamper_flag_cell(&cell);
    if (res)
        return res;

    res = nvmem_cell_write(cell, (uint8_t *)&flag, sizeof(flag));
    nvmem_put_cell(cell);

    if (res)
        EMSG("Failed to write tamper flag: 0x%x", res);
    else
        IMSG("Tamper health flag written: 0x%08x", flag);

    return res;
}

/*
 * CMD_READ_FLAG : read BKP12R and return value

 *  Returns 
 *          0xA5A5A5A5 if OK!
 *          0x0 if tamper occurred
 */
static TEE_Result pta_read_tamper_flag(uint32_t param_types,
                                        TEE_Param params[TEE_NUM_PARAMS])
{
    const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT,
                                             TEE_PARAM_TYPE_NONE,
                                             TEE_PARAM_TYPE_NONE,
                                             TEE_PARAM_TYPE_NONE);
    struct nvmem_cell *cell = NULL;
    uint32_t flag = 0;
    TEE_Result res = TEE_ERROR_GENERIC;

    if (param_types != exp_pt)
        return TEE_ERROR_BAD_PARAMETERS;

    res = get_tamper_flag_cell(&cell);
    if (res)
        return res;

    res = nvmem_cell_read(cell, (uint8_t *)&flag);
    nvmem_put_cell(cell);

    if (res) {
        EMSG("Failed to read tamper flag: 0x%x", res);
        return res;
    }

    IMSG("Tamper flag read: 0x%08x", flag);
    params[0].value.a = flag;

    return TEE_SUCCESS;
}

// Only TAs can access mais pas Linux
static TEE_Result pta_tamp_bkp_open_session(uint32_t ptypes __unused,
                                              TEE_Param par[TEE_NUM_PARAMS] __unused,
                                              void **session __unused)
{
    struct ts_session *caller = ts_get_calling_session();

    /* Accept any Trusted Application only */
    if (caller && is_user_ta_ctx(caller->ctx))
        return TEE_SUCCESS;

    EMSG("Access denied: only TAs can use tamp_bkp PTA");
    return TEE_ERROR_ACCESS_DENIED;
}
static TEE_Result pta_tamp_bkp_invoke_command(void *sess __unused,
                                               uint32_t cmd_id,
                                               uint32_t param_types,
                                               TEE_Param params[TEE_NUM_PARAMS])
{
    IMSG(PTA_NAME " command 0x%x", cmd_id);

    switch (cmd_id) {
    case PTA_TAMP_BKP_CMD_WRITE_FLAG:
        return pta_write_tamper_flag(param_types, params);
    case PTA_TAMP_BKP_CMD_READ_FLAG:
        return pta_read_tamper_flag(param_types, params);
    default:
        return TEE_ERROR_NOT_IMPLEMENTED;
    }
}

pseudo_ta_register(
    .uuid = PTA_TAMP_BKP_UUID,
    .name = PTA_NAME,
    .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
    .open_session_entry_point = pta_tamp_bkp_open_session,
    .invoke_command_entry_point = pta_tamp_bkp_invoke_command
);