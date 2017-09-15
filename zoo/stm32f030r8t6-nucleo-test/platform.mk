#
# platform description:
#   - arch: arm cortex-m0
#   - chip: stm32f030f4p6
#   - board: stm32f030f4p6 v1
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

VPATH += $(PRJ_DIR)/common
VPATH += $(PRJ_DIR)/zoo/$(PLAT)/app

## platform dependencies

CM3_TARGET	= stm32/f0

# FIXME: commit std=c99 to libopencm3
CM3_FPFLAGS	= -msoft-float -std=c99

NRF24_PLT_FLAGS = $(APP_PLT_FLAGS)
NRF24_CFG_FLAGS = -DSPI_SINGLE_BYTE

deps: libopencm3 libnrf24 nanopb

## sources

APP_SRCS :=		\
	main.c		\
	delay.c		\
	radio.c		\
	rtc.c		\
	printf.c	\
	stdlib.c	\
	pb_common.c	\
	pb_encode.c	\
	msg.pb.c	\

APP_OBJS := $(APP_SRCS:.c=.o)
APP_OBJS := $(addprefix $(OBJ_DIR)/,$(APP_OBJS))

APP_PLT_FLAGS = \
	-mthumb			\
	-mcpu=cortex-m0		\
	-msoft-float		\
	-ffunction-sections	\

CFLAGS = $(APP_PLT_FLAGS)
CFLAGS += -std=c99 -Wall -Werror -Os -DSTM32F0

# set sensor node ID
ifneq (${NODE_ID},)
CFLAGS += -DNODE_ID=${NODE_ID}
else
CFLAGS += -DNODE_ID=1234
endif

# tweak nanopb size
CFLAGS += -DPB_BUFFER_ONLY

CFLAGS += $(CM3_INC) $(NRF24_INC) $(NANOPB_INC)
CFLAGS += -I$(PRJ_DIR)/include
CFLAGS += -I$(OBJ_DIR)/nanopb
CFLAGS += -I$(PRJ_DIR)/zoo/$(PLAT)/app

LIBS = $(CM3_LIB) $(NRF24_LIB)

LDSCRIPT = $(PRJ_DIR)/ld/$(PLAT).ld
LDFLAGS =  $(APP_PLT_FLAGS) -nostartfiles -T$(LDSCRIPT) -Wl,--gc-sections

## build rules

app: $(OBJ_DIR)/$(APP).bin
	cp $(OBJ_DIR)/$(APP).bin $(OBJ_DIR)/test.bin
	$(OBJSIZE) $(OBJ_DIR)/$(APP).elf

%.hex: %.elf
	$(OBJCOPY) -O ihex $^ $@

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

$(OBJ_DIR)/$(APP).elf: $(APP_OBJS) $(LIBS) $(LDSCRIPT)
	$(CC) $(CFLAGS) $(LDFLAGS) $(APP_OBJS) $(LIBS) -o $@

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

## platform-specific loader rules

upload:
	openocd -f $(PRJ_DIR)/zoo/$(PLAT)/scripts/openocd-jlink-swd.cfg -c 'program ()'

debug:
	openocd -f $(PRJ_DIR)/zoo/$(PLAT)/scripts/openocd-jlink-swd.cfg -c 'attach ()'
