#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
//#include "tee_api_defines.h"
//#include "tee_api_types.h"
//#include "trace.h"
#include "include/tamp_key_ta.h"

TEE_Result TA_CreateEntryPoint(void)
{
        IMSG("Key TA: EntryPoint Created");
        return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
        IMSG("Key TA: EntryPoint Destroyed");
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
                                        TEE_Param __unused params[4],
                                        void __unused **sess_ctx)
{
        uint32_t exp_parm_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                                        TEE_PARAM_TYPE_NONE,
                                        TEE_PARAM_TYPE_NONE,
                                        TEE_PARAM_TYPE_NONE);
        if (param_types != exp_parm_types)
                return TEE_ERROR_BAD_PARAMETERS;

        IMSG("Key TA: Session opened");

        return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __unused *sess_ctx)
{
        IMSG("Key TA: Session closed");
}
/* CMD GENERATE KEY AND STORE IT IN SECURE STORAGE REE */
static TEE_Result cmd_generate_key(uint32_t param_types)
{
        TEE_ObjectHandle key_handle = TEE_HANDLE_NULL;
        TEE_ObjectHandle obj_handle = TEE_HANDLE_NULL;
        TEE_Result res = TEE_ERROR_GENERIC;
        uint8_t key_buf[32];
        size_t key_size = sizeof(key_buf);

        uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);
        
        if (param_types != exp_param_types)
                return TEE_ERROR_BAD_PARAMETERS;
        
        /* Allocate a transient AES object (container temporary in memory secure world) */
        res = TEE_AllocateTransientObject(TEE_TYPE_AES,TA_TAMP_KEY_SIZE_BITS,&key_handle);
        if (res != TEE_SUCCESS){
                EMSG("AllocateTransisentObject failed: 0x%x", res);
                return res;
        }

        res = TEE_GenerateKey(key_handle, TA_TAMP_KEY_SIZE_BITS, NULL, 0);
        if (res != TEE_SUCCESS){
                EMSG("GenerateKey failed: 0x%x", res);
                goto exit;
        }
        res = TEE_GetObjectBufferAttribute(key_handle,
                                           TEE_ATTR_SECRET_VALUE,
                                           key_buf, &key_size);

        if (res != TEE_SUCCESS) {
                EMSG("GetObjectBufferAttribute failed: 0x%x", res);
                goto exit;
        }

        res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
                                                TA_TAMP_KEY_OBJ_ID,
                                                sizeof(TA_TAMP_KEY_OBJ_ID),
                                                TEE_DATA_FLAG_ACCESS_READ |
                                                TEE_DATA_FLAG_ACCESS_WRITE |
                                                TEE_DATA_FLAG_ACCESS_WRITE_META |
                                                TEE_DATA_FLAG_OVERWRITE,
                                                TEE_HANDLE_NULL,
                                                key_buf, key_size,
                                                &obj_handle);
        if (res != TEE_SUCCESS) {
                EMSG("CreatePersistentObject failed: 0x%x", res);
                goto exit;
        }
exit: 
        TEE_MemFill(key_buf, 0, sizeof(key_buf));
        TEE_FreeTransientObject(key_handle);
        return res;
}

static TEE_Result cmd_get_status(uint32_t param_types, TEE_Param params[4])
{
        TEE_ObjectHandle obj_handle = TEE_HANDLE_NULL;
        TEE_Result res = TEE_ERROR_GENERIC;

        uint32_t exp_parm_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT,
                                        TEE_PARAM_TYPE_NONE,
                                        TEE_PARAM_TYPE_NONE,
                                        TEE_PARAM_TYPE_NONE);
        if (param_types != exp_parm_types)
                return TEE_ERROR_BAD_PARAMETERS;

        res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
                                                TA_TAMP_KEY_OBJ_ID,
                                                sizeof(TA_TAMP_KEY_OBJ_ID),
                                                TEE_DATA_FLAG_ACCESS_READ,
                                                &obj_handle);
        if (res == TEE_SUCCESS) {
                /* Key exists */
                params[0].value.a = 1;
                TEE_CloseObject(obj_handle);
        } else if (res == TEE_ERROR_ITEM_NOT_FOUND) {
                params[0].value.a = 0;
        } else {
                EMSG("OpenPersistentObject failed: 0x%x", res);
                return res;
        }

        return TEE_SUCCESS;
}

static TEE_Result cmd_delete_key(uint32_t param_types)
{
        TEE_ObjectHandle obj_handle = TEE_HANDLE_NULL;
        TEE_Result res = TEE_ERROR_GENERIC;

        uint32_t exp_parm_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                                        TEE_PARAM_TYPE_NONE,
                                        TEE_PARAM_TYPE_NONE,
                                        TEE_PARAM_TYPE_NONE);
        if (param_types != exp_parm_types)
                return TEE_ERROR_BAD_PARAMETERS;

        res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
                                                TA_TAMP_KEY_OBJ_ID,
                                                sizeof(TA_TAMP_KEY_OBJ_ID),
                                                TEE_DATA_FLAG_ACCESS_READ |
                                                TEE_DATA_FLAG_ACCESS_WRITE_META,
                                                &obj_handle);
        
        if (res == TEE_ERROR_ITEM_NOT_FOUND) {
                IMSG("Key already deleted");
                return TEE_SUCCESS;
        } 
        if (res != TEE_SUCCESS) {
                EMSG("OpenPersistentObject failed: 0x%x", res);
                return res;
        }
        else {
                res = TEE_CloseAndDeletePersistentObject1(obj_handle);
                if (res != TEE_SUCCESS) {
                        EMSG("CloseAndDeletePersistentObject failed: 0x%x", res);
                        return res;
                }
                IMSG("Key deleted from Secure Storage");
        }
        return TEE_SUCCESS;
}

TEE_Result TA_InvokeCommandEntryPoint(void __unused *sess_ctx,
                                        uint32_t cmd_id,
                                        uint32_t param_types,
                                        TEE_Param params[4])
{
        switch (cmd_id) {
        case TA_TAMP_CMD_GENERATE_KEY:
                return cmd_generate_key(param_types);
        case TA_TAMP_CMD_GET_STATUS:
                return cmd_get_status(param_types, params);
        case TA_TAMP_CMD_DELETE_KEY:
                return cmd_delete_key(param_types);
        default:
                return TEE_ERROR_NOT_SUPPORTED;
        }
}