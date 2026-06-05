#ifndef TAMP_KEY_TA_H
#define TAMP_KEY_TA_H

/* UUID using uuidegen 
        5e4f4d2e-0535-47df-b4ca-7539293ff8b6
*/
#define TAMP_KEY_TA_UUID \
    { 0x5e4f4d2e, 0x0535, 0x47df, \
        { 0xb4, 0xca, 0x75, 0x39, 0x29, 0x3f, 0xf8, 0xb6 } }

#define TA_TAMP_KEY_OBJ_ID "tamp_aes_key"
#define TA_TAMP_KEY_SIZE_BITS 256

#define TA_TAMP_CMD_GENERATE_KEY 0
#define TA_TAMP_CMD_GET_STATUS 1
#define TA_TAMP_CMD_DELETE_KEY 2

#endif /* TAMP_KEY_TA_H */