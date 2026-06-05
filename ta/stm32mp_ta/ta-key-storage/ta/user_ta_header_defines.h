#ifndef USER_TA_HEADER_DEFINES_H
#define USER_TA_HEADER_DEFINES_H

#include "include/tamp_key_ta.h"

#define TA_UUID TAMP_KEY_TA_UUID
//#define TA_FLAGS (TA_FLAG_USER_MODE | TA_FLAG_EXEC_DDR | TA_FLAG_SINGLE_INSTANCE)
#define TA_FLAGS 0
#define TA_STACK_SIZE (4 * 1024)
#define TA_DATA_SIZE (32 * 1024)

#define TA_VERSION "1.0"
#define TA_DESCRIPTION "TAMP Key TA"
#define TA_COPYRIGHT "Copyright (c) 2024, OP-TEE Authors. All rights reserved."
#endif /* USER_TA_HEADER_DEFINES_H */