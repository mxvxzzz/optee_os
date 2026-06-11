# stm32_tamp_nvmem : BKP Register NVMEM Consumer Driver for OP-TEE

## Overview

This driver implements a DT consumer driver for STM32MP2 Backup (BKP) registers
via the OP-TEE NVMEM framework. It exposes BKP cells defined in the Device Tree
to Trusted Applications via a Pseudo TA (PTA).

BKP registers are part of the TAMP peripheral and are protected by the RIF
(Resource Isolation Framework). Zone 1 registers (BKP0R-BKP23R) are accessible
only by OP-TEE (CID1) and are automatically erased on tamper detection.

## Architecture

```
CA Linux userspace (EL0-NS)
	|
	| libteec / /dev/tee0
	|
TEE Driver Linux kernel (EL1-NS)
	|
	| SMC
	|
TF-A BL31 (EL3)
	|
OP-TEE Kernel (EL1-S)
	|
tamp_nvmem_pta.c         <-- Pseudo TA (EL1-S) (PTA added)
	|
stm32_tamp_nvmem.c       <-- DT consumer driver (driver added)
	|
nvmem.c                  <-- OP-TEE NVMEM framework
	|
stm32_tamp_nvram.c       <-- NVMEM provider driver
	|
BKP Registers Hardware   <-- TAMP @ 0x46010000
```

## BKP Cell Mapping

|   Cell name  |     Registers   |   Size  |        Access        |
|--------------|-----------------|---------|----------------------|
| bhk_key      | BKP0R - BKP7R   | 32 bytes| RNG-only, write-only |
| tamper_bkp   | BKP12R          | 4 bytes | Read/Write           |

## Security Properties

- `bhk_key`    : Written only via `generate` command (RNG)
- `bhk_key`    : Read is denied
- `tamper_bkp` : Read/Write Exact size (4 bytes) enforced on write
- Tamper event : Zone 1 is automatically erased by hardware

---

## Development Environment

### Important note — How we built OP-TEE

This driver was developed and tested using a **direct OP-TEE source build**.

Instead, we used the **ST SDK** to cross-compile OP-TEE directly from source
and flash only the FIP partition.

### Platform

- Board  : STM32MP257F-EV1
- CPU    : Cortex-A35 (ARMv8-A)
- OS     : OpenSTLinux v6.6
- OP-TEE : 4.0.0-stm32mp-r3
- Branch : `maho/stm32mp2/tamp-lab`

### SDK

```bash
# ST SDK path
/opt/st/stm32mp2/5.0.15-openstlinux-6.6-yocto-scarthgap-mpu-v26.02.18/

# Source the SDK environment
source /opt/st/stm32mp2/5.0.15-openstlinux-6.6-yocto-scarthgap-mpu-v26.02.18/environment-setup-cortexa35-ostl-linux
```

### Required Environment Variables

```bash
# Cross compiler (set by SDK environment-setup)
export CROSS_COMPILE=aarch64-ostl-linux-

# SDK sysroot (set by SDK environment-setup)
export SDKTARGETSYSROOT=/opt/st/stm32mp2/5.0.15-openstlinux-6.6-yocto-scarthgap-mpu-v26.02.18/sysroots/cortexa35-ostl-linux

# TA Dev Kit (generated after OP-TEE build)
export TA_DEV_KIT_DIR=~/hmac/optee_os_maho/out/arm-plat-stm32mp2/export-ta_arm64

# Board IP address
export IP=<board_ip>
```

---

## Build

### Build OP-TEE OS + FIP

```bash
cd ~/hmac/optee_os_maho
make -f Makefile.sdk.stm32mp2 optee
make -f Makefile.sdk.stm32mp2 fip-update
```

### Deploy FIP to board

```bash
# Copy FIP to board and flash manually
make -f Makefile.sdk.stm32mp2 scp-fip IP=$IP

# On the board (SD card boots from mmcblk0p5 = fip-a):
dd if=/path/to/fip.bin of=/dev/disk/by-partlabel/fip-a bs=1M conv=fdatasync
reboot
```

### Build the CA (host application)

```bash
cd ta/stm32mp_ta/tamp-nvmem/host
make TA_DEV_KIT_DIR=~/hmac/optee_os_maho/out/arm-plat-stm32mp2/export-ta_arm64
```

### Deploy the CA to board

```bash
scp ta/stm32mp_ta/tamp-nvmem/host/tamp_nvmem_ca root@$IP:/usr/bin/
```

---

## DTS Configuration

We used the **External-DT** as submodule git.

Adds the following to the OP-TEE external DTS:

```dts
&nvram {
	tamper_bkp: tamp-bkp@30 {
		reg = <0x30 0x4>;    /* BKP12R */
	};
	bhk_key: tamp-bkp@0 {
		reg = <0x0 0x20>;    /* BKP0R - BKP7R */
	};
};

/ {
	bkp-regs {
		compatible = "st,stm32mp-tamp-nvmem";
		nvmem-cells = <&tamper_bkp>, <&bhk_key>;
		nvmem-cell-names = "tamper_bkp", "bhk_key";
	};
};
```

---

## Usage

### Write tamper flag

```bash
tamp_nvmem_ca write tamper_bkp 0xa5a5a5a5
```

### Read tamper flag

```bash
tamp_nvmem_ca read tamper_bkp
# Cell 'tamper_bkp' (4 bytes) : 0xa5a5a5a5
```

### Generate BHK via RNG

```bash
tamp_nvmem_ca generate bhk_key
# Cell 'bhk_key' generated successfully via RNG
```

### Error cases

```bash
# Wrong size
tamp_nvmem_ca write tamper_bkp 0xFF
# Wrong size: got 1 expected 4

# RNG-only cell
tamp_nvmem_ca write bhk_key 0xFF...
# Cell 'bhk_key' is RNG-only

# BHK is write-only
tamp_nvmem_ca read bhk_key
# BHK cell 'bhk_key' is write-only
```

---

## Boot log

On successful probe, OP-TEE prints:

```
I/TC: stm32_tamp_nvmem: probe started
I/TC: Cell 'tamper_bkp' registered (offset=0x30 len=4)
I/TC: Cell 'bhk_key' registered (offset=0x0 len=32)
I/TC: stm32_tamp_nvmem: 2 cells registered
```

The repeated `probe started` messages before the final success not an error
this is the OP-TEE deferred driver initialization mechanism
(`TEE_ERROR_DEFER_DRIVER_INIT`) waiting for the NVMEM provider
(`stm32_tamp_nvram`) to be registered first.

---

## Files

| File | Description |
|------|-------------|
| `core/drivers/stm32_tamp_nvmem.c` | DT consumer driver |
| `core/include/drivers/stm32_tamp_nvmem.h` | Driver public API |
| `core/pta/stm32mp/tamp_nvmem_pta.c` | Pseudo TA |
| `lib/libutee/include/pta_stm32mp_tamp_nvmem.h` | PTA public header |
| `ta/stm32mp_ta/tamp-nvmem/host/tamp_nvmem_ca.c` | Host CA application |
| `Makefile.sdk.stm32mp2` | Build and deploy helper |

---

## Author

Mohamed Tabkioui - ST Microelectronics Le Mans internship 2026
