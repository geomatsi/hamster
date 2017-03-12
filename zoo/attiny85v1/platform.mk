#
# platform description
#   - arch: avr8
#   - chip: attiny85
#   - board: attiny85 v1
#   - description: attiny85 sensor node v1
#

CHIP		= attiny85
CLK_FREQ	= 1000000L
APP		= $(PLAT).app

## platform toolchain

CROSS_COMPILE ?= avr

CC	= $(CROSS_COMPILE)-gcc
LD	= $(CROSS_COMPILE)-ld
OBJCOPY = $(CROSS_COMPILE)-objcopy
OBJDUMP	= $(CROSS_COMPILE)-objdump

## paths

VPATH += $(NANOPB_DIR)

VPATH += $(PRJ_DIR)/drivers/w1
VPATH += $(PRJ_DIR)/zoo/$(PLAT)/app

## platform dependencies

NRF24_CFG_FLAGS = -DNRF24_LOG_LEVEL=1 -DCONFIG_LIB_RF24_SIZEOPT
NRF24_PLT_FLAGS = $(APP_PLT_FLAGS)

PROTOC ?= protoc
PROTOBUF_SRC = $(PRJ_DIR)/zoo/$(PLAT)/app/protobuf

deps: libnrf24 nanopb

## sources

APP_SRCS := \
	main.c \
	clock.c \
	radio.c \
	spi.c \
	adc.c \
	w1core.c \
	ds18b20.c \
	pb_common.c \
	pb_encode.c \
	msg.pb.c \

APP_OBJS := $(APP_SRCS:.c=.o)
APP_OBJS := $(addprefix $(OBJ_DIR)/,$(APP_OBJS))

APP_PLT_FLAGS = \
	-mmcu=$(CHIP) \
	-DF_CPU=$(CLK_FREQ) \
	-Os -fshort-enums -ffunction-sections -Wl,--gc-sections

CFLAGS	= $(APP_PLT_FLAGS)
CFLAGS	+= --std=c99 -Wall -Werror

# tweak nanopb size to fit into attiny
CFLAGS += -DPB_BUFFER_ONLY -DPB_NO_ERRMSG

CFLAGS += $(NRF24_INC) $(NANOPB_INC)
CFLAGS += -I$(PRJ_DIR)/include
CFLAGS += -I$(OBJ_DIR)/nanopb
CFLAGS += -I$(PRJ_DIR)/zoo/$(PLAT)/app

LIBS = $(NRF24_LIB)

LDFLAGS	= -Wl,-Map,$(OBJ_DIR)/$(APP).map

## build rules

app: $(OBJ_DIR)/$(APP).hex $(OBJ_DIR)/$(APP).bin
	cp $(OBJ_DIR)/$(APP).hex $(OBJ_DIR)/firmware.hex
	avr-size --mcu=$(CHIP) --format=avr $(OBJ_DIR)/$(APP).elf

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

$(OBJ_DIR)/$(APP).elf: $(APP_OBJS) $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(APP_OBJS) $(LIBS) -o $@

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)/msg.pb.h
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJ_DIR)/%.pb.c $(OBJ_DIR)/%.pb.h: $(PROTOBUF_SRC)/%.proto
	mkdir -p $(OBJ_DIR)/nanopb
	$(PROTOC) --proto_path=$(PROTOBUF_SRC) -o $(OBJ_DIR)/$*.pb $(PROTOBUF_SRC)/$*.proto
	python $(NANOPB_DIR)/generator/nanopb_generator.py $(OBJ_DIR)/$*.pb
	# FIXME: why msg.pb.h is removed unless stored in another directory ?
	mv $(OBJ_DIR)/msg.pb.h $(OBJ_DIR)/nanopb

## platform-specific loader flags

# UM232H programmer

SERIAL_PORT = /dev/ttyUSB0
PROGRAMMER_UM232H = UM232H
SPI_BIT_CLK = 100000

DUDE_OPTIONS_UM232H =		\
	-p $(CHIP)		\
	-P $(SERIAL_PORT)	\
	-c $(PROGRAMMER_UM232H)	\
	-b $(SPI_BIT_CLK)

# FT232RL programmer

FTDI_PORT = ft0
PROGRAMMER_FT232RL  = arduino-ft232r
FTDI_BBSYNC_RATE = 2400

DUDE_OPTIONS_FT232RL =		\
	-p $(CHIP)		\
	-P $(FTDI_PORT)		\
	-c $(PROGRAMMER_FT232RL)\
	-B $(FTDI_BBSYNC_RATE)

upload: upload-ft232rl

upload-ft232rl:
	avrdude $(DUDE_OPTIONS_FT232RL) -U flash:w:out/firmware.hex -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

upload-um232h:
	avrdude $(DUDE_OPTIONS_UM232H) -U flash:w:out/firmware.hex -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

# Fuse bits:
#	- to set 8MHz:  -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
#	- to set 1MHz:	-U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
