// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2026, STMicroelectronics
 * TAMP NVMEM CA : test client
 *
 * Usage:
 *		tamp_nvmem_ca read		<cell_name>
 *		tamp_nvmem_ca write		<cell_name> <hex_value>
 *		tamp_nvmem_ca generate	<BHK_cell_name>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <tee_client_api.h>
#include <pta_stm32mp_tamp_nvmem.h>

struct tamp_nvmem_ctx {
	TEEC_Context ctx;
	TEEC_Session sess;
};

static void open_session(struct tamp_nvmem_ctx *ctx)
{
	TEEC_UUID uuid = PTA_TAMP_NVMEM_UUID;
	uint32_t origin = 0;

	TEEC_Result res = TEEC_InitializeContext(NULL, &ctx->ctx);

	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed: 0x%x", res);

	res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid, TEEC_LOGIN_PUBLIC,
			       NULL, NULL, &origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_OpenSession failed: 0x%x origin: 0x%x", res,
		     origin);
}

static void close_session(struct tamp_nvmem_ctx *ctx)
{
	TEEC_CloseSession(&ctx->sess);
	TEEC_FinalizeContext(&ctx->ctx);
}

static void do_read(struct tamp_nvmem_ctx *ctx, const char *cell_name)
{
	TEEC_Operation op = { 0 };
	uint32_t origin = 0;
	uint8_t buf[32] = {};
	size_t i = 0;
	size_t read_len = 0;

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE,
					 TEEC_NONE);

	op.params[0].tmpref.buffer = (void *)cell_name;
	op.params[0].tmpref.size = strlen(cell_name);
	op.params[1].tmpref.buffer = buf;
	op.params[1].tmpref.size = sizeof(buf);

	TEEC_Result res = TEEC_InvokeCommand(&ctx->sess,
		PTA_TAMP_NVMEM_CMD_READ, &op, &origin);

	if (res != TEEC_SUCCESS)
		errx(1, "Read '%s' failed: 0x%x origin: 0x%x", cell_name, res,
		     origin);

	/* for printf : update tmpref.size with the reel size */
	read_len = op.params[1].tmpref.size;

	printf("Cell '%s' (%zu bytes) : 0x", cell_name, read_len);

	for (i = 0; i < read_len; i++)
		printf("%02x", buf[i]);
	printf("\n");
}

static void do_write(struct tamp_nvmem_ctx *ctx, const char *cell_name,
		     const char *hex_val)
{
	TEEC_Operation op = { 0 };
	uint32_t origin = 0;
	uint8_t buf[32] = {};
	size_t i, len, hex_len = 0;
	unsigned int byte = 0;

	if (hex_val[0] == '0' && (hex_val[1] == 'x' || hex_val[1] == 'X'))
		hex_val += 2;

	hex_len = strlen(hex_val);

	if (hex_len == 0 || hex_len % 2 != 0)
		errx(1, "Invalid hex value : must be even number of hex chars");

	len = hex_len / 2;

	if (len > sizeof(buf))
		errx(1, "Value too long : max 32 bytes");

	for (i = 0; i < len; i++) {
		if (sscanf(hex_val + 2 * i, "%02x", &byte) != 1)
			errx(1, "Invalid hex char at position %zu",
			     i * 2);
		buf[i] = (uint8_t)byte;
	}

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
					 TEEC_NONE);

	op.params[0].tmpref.buffer = (void *)cell_name;
	op.params[0].tmpref.size = strlen(cell_name);
	op.params[1].tmpref.buffer = buf;
	op.params[1].tmpref.size = len;

	TEEC_Result res = TEEC_InvokeCommand(&ctx->sess,
					     PTA_TAMP_NVMEM_CMD_WRITE,
					     &op, &origin);

	if (res != TEEC_SUCCESS)
		errx(1, "Write '%s' failed: 0x%x origin: 0x%x", cell_name, res,
		     origin);

	printf("Cell '%s' (%zu bytes) written successfully\n", cell_name, len);
}

static void do_generate(struct tamp_nvmem_ctx *ctx, const char *cell_name)
{
	TEEC_Operation op = { 0 };
	uint32_t origin = 0;

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = (void *)cell_name;
	op.params[0].tmpref.size = strlen(cell_name);

	TEEC_Result res = TEEC_InvokeCommand(&ctx->sess,
		PTA_TAMP_NVMEM_CMD_GENERATE, &op, &origin);

	if (res != TEEC_SUCCESS)
		errx(1, "Generate '%s' failed: 0x%x origin: 0x%x", cell_name,
		     res, origin);

	printf("Cell '%s' generated successfully via RNG\n", cell_name);
}

int main(int argc, char *argv[])
{
	struct tamp_nvmem_ctx ctx;

	if (argc < 3) {
		fprintf(stderr,
			"Usage:\n"
			"  %s read     <cell_name>\n"
			"  %s write    <cell_name> <hex_value>\n"
			"  %s generate <cell_name>\n",
			argv[0], argv[0], argv[0]);
		return 1;
	}

	open_session(&ctx);

	if (strcmp(argv[1], "read") == 0) {
		if (argc != 3) {
			fprintf(stderr, "Usage: %s read <cell_name>\n",
				argv[0]);
			close_session(&ctx);
			return 1;
		}
		do_read(&ctx, argv[2]);
	} else if (strcmp(argv[1], "write") == 0) {
		if (argc != 4) {
			fprintf(stderr,
				"Usage: %s write <cell_name> <hex_value>\n",
				argv[0]);
			close_session(&ctx);
			return 1;
		}
		do_write(&ctx, argv[2], argv[3]);
	} else if (strcmp(argv[1], "generate") == 0) {
		if (argc != 3) {
			fprintf(stderr, "Usage: %s generate <cell_name>\n",
				argv[0]);
			close_session(&ctx);
			return 1;
		}
		do_generate(&ctx, argv[2]);
	} else {
		fprintf(stderr, "Unknown command: %s\n", argv[1]);
		close_session(&ctx);
		return 1;
	}

	close_session(&ctx);
	return 0;
}
