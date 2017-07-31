#
# platform description
#   - arch: avr8
#   - chip: atmega328p
#   - platform: devduino v2
#   - description: http://www.seeedstudio.com/wiki/DevDuino_Sensor_Node_V2.0_(ATmega_328)
#

CHIP		= atmega328p
CLK_FREQ	= 16000000L
APP		= $(PLAT).app

## platform toolchain

CROSS_COMPILE ?= avr

CC	= $(CROSS_COMPILE)-gcc
LD	= $(CROSS_COMPILE)-ld
OBJCOPY	= $(CROSS_COMPILE)-objcopy
OBJDUMP	= $(CROSS_COMPILE)-objdump

## paths

VPATH += $(PRJ_DIR)/zoo/$(PLAT)/app

## platform dependencies

NRF24_CFG_FLAGS = -DNRF24_LOG_LEVEL=2 -DCONFIG_LIB_RF24_SIZEOPT -DSPI_SINGLE_BYTE
NRF24_PLT_FLAGS = $(APP_PLT_FLAGS)

deps: libnrf24 nanopb

## platform compile flags

APP_SRCS := \
	main.c	\
	uart.c	\
	leds.c	\
	spi.c	\
	clock.c	\
	radio.c \
	vcc.c	\
	adc.c	\
	temp_mcp9700.c \
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

# tweak nanopb size
CFLAGS += -DPB_BUFFER_ONLY

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

# programming using arduino bootloader

SERIAL_PORT	= /dev/ttyUSB0
PROGRAMMER	= arduino
BAUDRATE	= 57600

DUDE_OPTIONS_SERIAL =		\
	-p $(CHIP)		\
	-P $(SERIAL_PORT)	\
	-c $(PROGRAMMER)	\
	-b $(BAUDRATE)		\
	-F

# programming using ft232rl based ISP programmer

FTDI_PORT = ft0
PROGRAMMER  = arduino-ft232r
FTDI_BBSYNC_RATE = 2400

DUDE_OPTIONS_FT232RL =		\
	-p $(CHIP)		\
	-P $(FTDI_PORT)		\
	-c $(PROGRAMMER)	\
	-B $(FTDI_BBSYNC_RATE)

## upload rules

upload-serial:
	avrdude $(DUDE_OPTIONS_SERIAL) -U flash:w:out/firmware.hex

upload:
	avrdude $(DUDE_OPTIONS_FT232RL) -U flash:w:out/firmware.hex
