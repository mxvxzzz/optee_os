CFG_TEE_TA_LOG_LEVEL ?= 4 # 0 (no logs), 1 (error), 2 (warning), 3 (info), 4 (debug)
CFG_TEE_OPTEE_CORE_API_COMPACT_1_1 ?= 1
CFG_TA_ASLR ?= y
CFG_TA_GPROF ?= n # Enable gprof profiling support in the TA. 
CFG_SECURE_DATA_PATH ?= n # SDP : Enable secure data path support in the TA. 
BINARY = 5e4f4d2e-0535-47df-b4ca-7539293ff8b6

-include $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk
ifeq ($(wildcard $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk),)
$(error "Cannot find OP-TEE OS TA development kit, please set TA_DEV_KIT_DIR")
clean:
	@echo 'Note: $$(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk not found, cannot clean'
	@echo 'Note: TA_DEV_KIT_DIR=$(TA_DEV_KIT_DIR)'
endif