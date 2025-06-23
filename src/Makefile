LVGL_PATH := lvgl
$(info LVGL_PATH is set to: $(LVGL_PATH))
include lvgl/lvgl.mk

CSRCS += main.c xpt2046.c messages.c transfer.c encryption.c ui.c
# Compiler and flags
CFLAGS += -Werror   -I$(LVGL_PATH)/..
COBJS := $(patsubst %.c, %.o, $(CSRCS))
# Add bcm2835 library and LVGL
LDFLAGS += -lbcm2835
# add cyrpto library
LDFLAGS += -lcrypto
main: $(COBJS)
        $(CC) -o $@ $^ $(LDFLAGS)


clean:
        rm -f main $(COBJS)