TOOLCHAIN_DIR = /opt/st/stm32mp2/5.0.15-openstlinux-6.6-yocto-scarthgap-mpu-v26.02.18/sysroots/x86_64-ostl_sdk-linux/usr/bin/aarch64-ostl-linux
SYSROOT       = /opt/st/stm32mp2/5.0.15-openstlinux-6.6-yocto-scarthgap-mpu-v26.02.18/sysroots/cortexa35-ostl-linux

CC      = $(TOOLCHAIN_DIR)/aarch64-ostl-linux-gcc
CFLAGS  += -Wall -Wextra -Werror -I../ta/include --sysroot=$(SYSROOT)
LDFLAGS += -lteec

BINARY  = tamp_key_ca
SRC     = tamp_key_ca.c

all: $(BINARY)

$(BINARY): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(BINARY) *.o