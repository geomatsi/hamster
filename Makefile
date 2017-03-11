#
#
#

## Platform definitions

ARCH	= arm
PLAT	= stm32f0v1

# project and output directories

PRJ_DIR = $(shell pwd)
OBJ_DIR = $(PRJ_DIR)/out/$(PLAT).out
DEP_DIR = $(PRJ_DIR)/deps

## default build target: print info

all: info

info:
	@echo "ARCH = $(ARCH)"
	@echo "PLAT = $(PLAT)"
	@echo "CHIP = $(CHIP)"

## dependencies

# libopencm3

CM3_LIB_DIR	= $(DEP_DIR)/libopencm3
LIBCM3_INC	= -I$(CM3_LIB_DIR)/include
LIBCM3		= $(CM3_LIB_DIR)/lib/libopencm3_$(CHIP).a

# libnrf24

NRF24_LIB_DIR	= $(DEP_DIR)/libnrf24
LIBNRF24_INC	= -I$(NRF24_LIB_DIR)/include
LIBNRF24	= $(NRF24_LIB_DIR)/libnrf24_$(CHIP).a

# nanopb

NANOPB_DIR	= $(DEP_DIR)/nanopb
NANOPB_INC	= -I$(NANOPB_DIR)

## platform specific definitions

include $(PRJ_DIR)/zoo/$(PLAT)/platform.mk

## build rules for dependencies

libnrf24:
	make -C $(NRF24_LIB_DIR) \
		CROSS_COMPILE=$(CROSS_COMPILE)- \
		TARGET=$(CHIP) \
		PLT_FLAGS="$(PFLAGS)" \
		CFG_FLAGS="$(NRF24_CFG_FLAGS)"

libopencm3:
	make -C $(CM3_LIB_DIR) \
		PREFIX=$(CROSS_COMPILE) \
		FP_FLAGS=$(LIBCM3_FPFLAGS) \
		TARGETS="$(LIBCM3_TARGET)"

nanopb:
	make -C $(NANOPB_DIR)/generator/proto

## clean rules

clean:
	rm -rf $(OBJ_DIR)

distclean: clean
	make -C $(CM3_LIB_DIR) clean
	make -C $(NRF24_LIB_DIR) TARGET=$(CHIP) clean

.PHONY: libopencm3
.PHONY: libnrf24
.PHONY: nanopb
.PHONY: distclean
.PHONY: clean
