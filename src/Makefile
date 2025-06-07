LVGL_PATH := lvgl
$(info LVGL_PATH is set to: $(LVGL_PATH))
include lvgl/lvgl.mk

CSRCS += main.c xpt2046.c
CFLAGS += -Werror   -I$(LVGL_PATH)/..
COBJS := $(patsubst %.c, %.o, $(CSRCS))
# Add bcm2835 library and LVGL
LDFLAGS += -lbcm2835
main: $(COBJS)
        $(CC) -o $@ $^ $(LDFLAGS)


clean:
        rm -f main $(COBJS)