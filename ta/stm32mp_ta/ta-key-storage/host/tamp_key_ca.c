// SPDX-License-Identifier: BSD-2-Clause

#include <stdio.h>
#include <string.h>
#include <err.h>
#include <tee_client_api.h>
#include "../ta/include/tamp_key_ta.h"

enum tamp_cmd_type {
        CMD_GENERATE = 0,
        CMD_STATUS,
        CMD_DELETE,
        CMD_UNKNOWN = -1
};

struct tamp_ctx {
	TEEC_Context ctx;
	TEEC_Session sess;
};

static void open_session(struct tamp_ctx *ctx)
{
        TEEC_UUID uuid = TAMP_KEY_TA_UUID;
        uint32_t origin = 0;
        TEEC_Result res = TEEC_InitializeContext(NULL, &ctx->ctx);

        if (res != TEEC_SUCCESS) 
                errx(1, "TEEC_InitializeContext failed: 0x%x", res);

        res = TEEC_OpenSession(&ctx->ctx,&ctx->sess, &uuid, 
                                TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
        
        if (res != TEEC_SUCCESS)
                errx(1, "TEEC_OpenSession failed: 0x%x, origin: 0x%x", 
                        res, origin);
}

static void close_session(struct tamp_ctx *ctx)
{
        TEEC_CloseSession(&ctx->sess);
        TEEC_FinalizeContext(&ctx->ctx);
}

static void cmd_generate(struct tamp_ctx *ctx)
{
        TEEC_Operation op = {0};
        uint32_t origin = 0;
        TEEC_Result res = TEEC_ERROR_GENERIC; 

        op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, 
                                        TEEC_NONE, TEEC_NONE);

        printf("Generating key in TA...\n");
        res = TEEC_InvokeCommand(&ctx->sess, TA_TAMP_CMD_GENERATE_KEY, 
                                &op, &origin);
        if (res != TEEC_SUCCESS)
                errx(1, "TEEC_InvokeCommand (GENERATE_KEY) failed: 0x%x, origin: 0x%x", res, origin);
        printf("Key generated and stored in Secure Storage\n");
}

static void cmd_status(struct tamp_ctx *ctx)
{
        TEEC_Operation op = {0};
        uint32_t origin = 0;
        TEEC_Result res = TEEC_ERROR_GENERIC;

        op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE, 
                                        TEEC_NONE, TEEC_NONE);

        printf("Getting key status from Secure Storage ...\n");
        res = TEEC_InvokeCommand(&ctx->sess, TA_TAMP_CMD_GET_STATUS, 
                                &op, &origin);

        if (res != TEEC_SUCCESS)
                errx(1, "TEEC_InvokeCommand (GET_STATUS) failed: 0x%x, origin: 0x%x", res, origin);

        if (op.params[0].value.a == 1)
                printf("Key status: EXISTS\n");
        else
                printf("Key status: GONE\n");
}

static void cmd_delete(struct tamp_ctx *ctx)
{
        TEEC_Operation op = {0};
        uint32_t origin = 0;
        TEEC_Result res = TEEC_ERROR_GENERIC;

        op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, 
                                        TEEC_NONE, TEEC_NONE);

        printf("Deleting key from Secure storage...\n");
        res = TEEC_InvokeCommand(&ctx->sess, TA_TAMP_CMD_DELETE_KEY, 
                                &op, &origin);
        if (res != TEEC_SUCCESS)
                errx(1, "TEEC_InvokeCommand (DELETE_KEY) failed: 0x%x, origin: 0x%x", res, origin);
        printf("Key deleted from Secure Storage\n");
}

int main(int argc, char *argv[])
{
        struct tamp_ctx ctx;
        enum tamp_cmd_type cmd = CMD_UNKNOWN;
        if (argc != 2) {
                fprintf(stderr, "CMD Available: %s <generate|status|delete>\n", argv[0]);
                return 1;
        }

        if(strcmp(argv[1], "generate") == 0)
                cmd = CMD_GENERATE;
        else if (strcmp(argv[1], "status") == 0)
                cmd = CMD_STATUS;
        else if (strcmp(argv[1], "delete") == 0)
                cmd = CMD_DELETE;
        else {
                fprintf(stderr, "Invalid command: %s\n", argv[1]);
                fprintf(stderr, "CMD Available: %s <generate|status|delete>\n", argv[0]);
                return 1;
        }

        open_session(&ctx);

        switch (cmd) {
        case CMD_GENERATE:
                cmd_generate(&ctx);
                break;
        case CMD_STATUS:
                cmd_status(&ctx);
                break;
        case CMD_DELETE:
                cmd_delete(&ctx);
                break;
        default:
                fprintf(stderr, "An unexpected error occurred.\n");
                close_session(&ctx);
                return 1;
        }

        close_session(&ctx);
        return 0;
}