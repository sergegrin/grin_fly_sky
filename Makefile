# GCC (arm-none-eabi) build for FS-i6 (MKL16Z64).
# Usage: make            -> build/FSI6.elf/.hex/.bin
#        make clean

TARGET   := FSI6
BUILD    := build
PREFIX   ?= arm-none-eabi-
CC       := $(PREFIX)gcc
CXX      := $(PREFIX)g++
OBJCOPY  := $(PREFIX)objcopy
SIZE     := $(PREFIX)size

LDSCRIPT := gcc/MKL16Z64_flash.ld

# Shell commands: always cmd.exe on Windows (works from PowerShell, cmd,
# Git Bash and VS Code tasks alike), POSIX sh elsewhere.
ifeq ($(OS),Windows_NT)
  SHELL := cmd.exe
  # tolerate parallel jobs creating the same directory at once
  MKDIR = (if not exist "$(subst /,\,$(1))" mkdir "$(subst /,\,$(1))") 2>NUL & ver >NUL
  RMDIR = if exist "$(1)" rmdir /s /q "$(1)"
else
  MKDIR = mkdir -p $(1)
  RMDIR = rm -rf $(1)
endif

C_SRCS := \
  gcc/startup_MKL16Z4.c \
  Kinetis_KL/CMSIS/Device/Source/system_MKL16Z4.c

CXX_SRCS := \
  Sources/main.cpp Sources/lcd.cpp Sources/I6/BoardI6.cpp Sources/er9x.cpp \
  Sources/A7105_SPI.cpp Sources/AFHDS2A_a7105.cpp Sources/audio.cpp \
  Sources/drivers.cpp Sources/file.cpp Sources/menus.cpp Sources/pulses.cpp \
  Sources/stamp.cpp Sources/templates.cpp Sources/AFHDS.cpp \
  Sources/pers.cpp \
  Sources/crossfire/elrs.cpp Sources/crossfire/crc_crsf.cpp Sources/crossfire/crossfire.cpp

DEFS := -DMKL16Z64xxx4 -D__Kinetis_KL_FAMILY -D__KL1x_SUBFAMILY -DARM_MATH_CM0PLUS \
        -DFLASH_PLACEMENT=1 -DNDEBUG
INCS := -ICMSIS_4/CMSIS/Include -IKinetis_KL/CMSIS/Device/Include -ISources

CPU    := -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft
# -fno-inline-small-functions: smaller code
OPT    := -Os -fno-inline-small-functions
COMMON := $(CPU) $(OPT) -g -flto -ffunction-sections -fdata-sections -fshort-enums \
          -fno-common -funsigned-char -Wall -Wno-unused $(DEFS) $(INCS) -MMD -MP
CFLAGS   := $(COMMON) -std=gnu99
CXXFLAGS := $(COMMON) -std=gnu++11 -fno-exceptions -fno-rtti -fno-threadsafe-statics \
            -fno-use-cxa-atexit
LDFLAGS  := $(CPU) $(OPT) -flto -T$(LDSCRIPT) -nostartfiles --specs=nano.specs --specs=nosys.specs \
            -Wl,--gc-sections -Wl,--no-enum-size-warning -Wl,--no-warn-rwx-segments \
            -Wl,-Map=$(BUILD)/$(TARGET).map -Wl,--print-memory-usage

OBJS := $(addprefix $(BUILD)/obj/,$(C_SRCS:.c=.o) $(CXX_SRCS:.cpp=.o))

all: $(BUILD)/$(TARGET).hex $(BUILD)/$(TARGET).bin

$(BUILD)/$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)
	$(SIZE) $@

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/obj/%.o: %.c
	@$(call MKDIR,$(@D))
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/obj/%.o: %.cpp
	@$(call MKDIR,$(@D))
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@$(call RMDIR,$(BUILD))

-include $(OBJS:.o=.d)

.PHONY: all clean
