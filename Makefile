#
#
#

## supported platform

## stm32f0v1
ARCH		?= arm
PLAT		?= stm32f0v1
NODE_ID		?= 101
NODE_TEMP	?= 1
NODE_RANGE	?= 1
UART_DBG	?= 1

## stm32f0v1
#ARCH		?= arm
#PLAT		?= stm32f0v1
#NODE_ID	?= 102
#NODE_TEMP	?= 1
#NODE_RANGE	?= 1

# attiny85v1
#ARCH		?= avr
#PLAT		?= attiny85v1
#NODE_ID	?= 201

# devduino2
#ARCH		?= avr
#PLAT		?= devduino2
#NODE_ID	?= 301


# stm32f030r8t6-nucleo: test node for hub debug
#ARCH		?= arm
#PLAT		?= stm32f030r8t6-nucleo-test
#NODE_ID	?= 10

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
	@echo "NODE = $(NODE_ID)"

## dependencies

include $(DEP_DIR)/deps.mk

## protobuf

include $(PRJ_DIR)/proto/proto.mk

## platforms

include $(PRJ_DIR)/zoo/$(PLAT)/platform.mk

## clean rules

clean:
	rm -rf $(OBJ_DIR)

distclean: clean depsclean

.PHONY: distclean
.PHONY: depsclean
.PHONY: clean
