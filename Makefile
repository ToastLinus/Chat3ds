
TARGET  := Chat3DS
BUILD   := build
SOURCES := source
INCLUDES := include

ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard

CFLAGS := -g -Wall -O2 -mword-relocations \
          -ffunction-sections \
          $(ARCH) \
          -D__3DS__

CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions

LDFLAGS := -specs=3dsx.specs -g \
           -Wl,-Map,$(notdir $*.map)

LIBS := -lctru

include $(DEVKITPRO)/libctru/base_rules
