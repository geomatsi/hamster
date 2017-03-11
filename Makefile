#
#
#

## Platform definitions

ARCH	= arm
PLAT	= stm32f0v1

# project and output directories

PRJ_DIR = $(shell pwd)
OBJ_DIR = $(PRJ_DIR)/out/$(PLAT).out

## default build target: print info

all: info

info:
	@echo "ARCH = $(ARCH)"
	@echo "PLAT = $(PLAT)"
	@echo "CHIP = $(CHIP)"

## dependencies

# libopencm3

CM3_LIB_DIR	= libopencm3
LIBCM3_INC	= -I$(CM3_LIB_DIR)/include
LIBCM3		= $(CM3_LIB_DIR)/lib/libopencm3_$(CHIP).a

# libnrf24

NRF24_LIB_DIR	= libnrf24
LIBNRF24_INC	= -I$(NRF24_LIB_DIR)/include
LIBNRF24	= $(NRF24_LIB_DIR)/libnrf24_$(CHIP).a

# nanopb

NANOPB_DIR		= nanopb
NANOPB_INC		= -I$(NANOPB_DIR)

## platform specific definitions

include $(PRJ_DIR)/zoo/$(PLAT)/platform.mk

## build rules for dependencies

libnrf24:
	make -C libnrf24 \
		CROSS_COMPILE=$(CROSS_COMPILE)- \
		TARGET=$(CHIP) \
		PLT_FLAGS="$(PFLAGS)" \
		CFG_FLAGS="$(NRF24_CFG_FLAGS)"

libopencm3:
	make -C libopencm3 \
		PREFIX=$(CROSS_COMPILE) \
		FP_FLAGS=$(LIBCM3_FPFLAGS) \
		TARGETS="$(LIBCM3_TARGET)"

nanopb:
	make -C nanopb/generator/proto

## clean rules

clean:
	rm -rf $(OBJ_DIR)

distclean:
	make -C libopencm3 clean
	make -C libnrf24 TARGET=$(CHIP) clean
	rm -rf $(OBJ_DIR)

.PHONY: libopencm3
.PHONY: libnrf24
.PHONY: nanopb
.PHONY: distclean
.PHONY: clean
