#
# platform description:
#   - arch: arm cortex-m0
#   - chip: stm32f030f4p6
#   - board: stm32f030f4p6 v1 board
#   - description: stm32 sensor node v1
#

## platform identifier

CHIP = stm32f0
APP = $(PLAT).app

## platform toolchain

CROSS_COMPILE ?= arm-none-eabi

CC	= $(CROSS_COMPILE)-gcc
LD	= $(CROSS_COMPILE)-ld
OBJCOPY	= $(CROSS_COMPILE)-objcopy
OBJSIZE	= $(CROSS_COMPILE)-size

## paths

VPATH += $(NANOPB_DIR)

VPATH += $(PRJ_DIR)/common
VPATH += $(PRJ_DIR)/zoo/$(PLAT)/app
VPATH += $(PRJ_DIR)/zoo/$(PLAT)/app

## platform dependencies

LIBCM3_TARGET	= stm32/f0
LIBCM3_FPFLAGS	= -msoft-float

PROTOC ?= protoc
PROTOBUF_SRC = $(PRJ_DIR)/zoo/$(PLAT)/app/protobuf

deps: libopencm3 libnrf24 nanopb

## sources

APP_SRCS :=		\
	main.c		\
	clock.c		\
	printf.c	\
	stdlib.c	\
	pb_common.c	\
	pb_encode.c	\
	msg.pb.c	\

APP_OBJS := $(APP_SRCS:.c=.o)
APP_OBJS := $(addprefix $(OBJ_DIR)/,$(APP_OBJS))

PFLAGS = \
	-mthumb			\
	-mcpu=cortex-m0		\
	-msoft-float		\
	-ffunction-sections	\

CFLAGS  = $(PFLAGS) -Wall -Werror -Os -DSTM32F0 -DPB_BUFFER_ONLY
CFLAGS += $(LIBCM3_INC) $(LIBNRF24_INC) $(NANOPB_INC)
CFLAGS += -I$(PRJ_DIR)/include
CFLAGS += -I$(OBJ_DIR)/nanopb

LIBS = $(LIBCM3) $(LIBNRF24)

LDSCRIPT = $(PRJ_DIR)/ld/$(PLAT).ld
LDFLAGS =  $(PFLAGS) -nostartfiles -T$(LDSCRIPT) -Wl,--gc-sections

## rules

app: $(OBJ_DIR)/$(APP).bin
	cp $(OBJ_DIR)/$(APP).bin $(OBJ_DIR)/test.bin
	$(OBJSIZE) $(OBJ_DIR)/$(APP).elf

%.hex: %.elf
	$(OBJCOPY) -O ihex $^ $@

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

$(OBJ_DIR)/$(APP).elf: $(APP_OBJS) $(LIBS) $(LDSCRIPT)
	$(CC) $(LDFLAGS) $(APP_OBJS) $(LIBS) -o $@

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)/msg.pb.h
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJ_DIR)/%.o: %.s $(OBJ_DIR)/msg.pb.h
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJ_DIR)/%.pb.c $(OBJ_DIR)/%.pb.h: $(PROTOBUF_SRC)/%.proto
	mkdir -p $(OBJ_DIR)/nanopb
	$(PROTOC) --proto_path=$(PROTOBUF_SRC) -o $(OBJ_DIR)/$*.pb $(PROTOBUF_SRC)/$*.proto
	python $(NANOPB_DIR)/generator/nanopb_generator.py $(OBJ_DIR)/$*.pb
	# FIXME: why msg.pb.h is removed unless stored in another directory ?
	mv $(OBJ_DIR)/msg.pb.h $(OBJ_DIR)/nanopb
## platform-specific flash rules

upload:
	openocd -f $(PRJ_DIR)/zoo/$(PLAT)/scripts/openocd-jlink-swd.cfg -c 'program ()'

debug:
	openocd -f $(PRJ_DIR)/zoo/$(PLAT)/scripts/openocd-jlink-swd.cfg -c 'attach ()'
