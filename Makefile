#
#
#

## supported platform

# stm32f0v1
ARCH	= arm
PLAT	= stm32f0v1

# attiny85v1
#ARCH	= avr
#PLAT	= attiny85v1

# devduino2
#ARCH	= avr
#PLAT	= devduino2

# project and output directories

PRJ_DIR = $(shell pwd)
DEP_DIR = $(PRJ_DIR)/deps
OBJ_DIR = $(PRJ_DIR)/out/$(PLAT).out

## default build target: print info

all: info

info:
	@echo "ARCH = $(ARCH)"
	@echo "PLAT = $(PLAT)"
	@echo "CHIP = $(CHIP)"

## dependencies

include $(DEP_DIR)/deps.mk

## platforms

include $(PRJ_DIR)/zoo/$(PLAT)/platform.mk

## clean rules

clean:
	rm -rf $(OBJ_DIR)

distclean: clean depsclean

.PHONY: distclean
.PHONY: depsclean
.PHONY: clean
