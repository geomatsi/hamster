#
#
#

## deps paths

# libopencm3

CM3_DIR		= $(DEP_DIR)/libopencm3
CM3_INC		= -I$(CM3_DIR)/include
CM3_LIB		= $(CM3_DIR)/lib/libopencm3_$(CHIP).a

# libnrf24

NRF24_DIR	= $(DEP_DIR)/libnrf24
NRF24_INC	= -I$(NRF24_DIR)/include
NRF24_LIB	= $(NRF24_DIR)/libnrf24_$(CHIP).a

# nanopb

NANOPB_DIR	= $(DEP_DIR)/nanopb
NANOPB_INC	= -I$(NANOPB_DIR)

## build rules for dependencies

libnrf24:
	make -C $(NRF24_DIR) \
		CROSS_COMPILE=$(CROSS_COMPILE)- \
		TARGET=$(CHIP) \
		PLT_FLAGS="$(NRF24_PLT_FLAGS)" \
		CFG_FLAGS="$(NRF24_CFG_FLAGS)"

libopencm3:
	make -C $(CM3_DIR) \
		PREFIX=$(CROSS_COMPILE) \
		FP_FLAGS=$(CM3_FPFLAGS) \
		TARGETS="$(CM3_TARGET)"

nanopb:
	make -C $(NANOPB_DIR)/generator/proto

depsclean:
	make -C $(CM3_DIR) clean
	make -C $(NRF24_DIR) TARGET=$(CHIP) clean

.PHONY: libopencm3
.PHONY: libnrf24
.PHONY: nanopb

