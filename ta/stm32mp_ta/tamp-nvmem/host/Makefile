CC ?= $(CROSS_COMPILE)gcc
CFLAGS += -Wall -Wextra -Werror
CFLAGS += -I../ta/include
CFLAGS += -I$(TA_DEV_KIT_DIR)/host_include
CFLAGS += --sysroot=$(SDKTARGETSYSROOT)
LDFLAGS += -lteec

all: tamp_nvmem_ca

tamp_nvmem_ca: tamp_nvmem_ca.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f tamp_nvmem_ca *.o