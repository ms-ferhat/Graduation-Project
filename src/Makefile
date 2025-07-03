

LVGL_PATH := lvgl
$(info LVGL_PATH is set to: $(LVGL_PATH))
include lvgl/lvgl.mk

# add your own source files here
CSRCS := $(wildcard *.c)


# Compiler and flags
CFLAGS += -Werror   -I$(LVGL_PATH)/..
COBJS := $(patsubst %.c, %.o, $(CSRCS))

# Add linking flags
LDFLAGS += -lbcm2835 -lcrypto

main: $(COBJS)
        $(CC) -o $@ $^ $(LDFLAGS)


clean:
        rm -f main $(COBJS)