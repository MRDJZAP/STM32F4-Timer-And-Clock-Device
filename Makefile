# Toolchain configuration
PREFIX   ?= arm-none-eabi-
CC       := $(PREFIX)gcc
LD       := $(PREFIX)gcc

# Target binary name
TARGET_NAME := app.elf

# Directory definitions
SRC_DIR   := src
INC_DIR   := inc
OBJ_DIR   := obj
BUILD_DIR := build

# Output target ELF path
TARGET    := $(BUILD_DIR)/$(TARGET_NAME)

# MCU Architecture Flags (e.g., STM32F411 Cortex-M4)
MCUFLAGS  := -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

# Compiler & Linker Flags
CFLAGS    := $(MCUFLAGS) -Wall -I$(INC_DIR)/chip_headers/CMSIS/Include/ -I$(INC_DIR)/chip_headers/CMSIS/Device/ST/STM32F4xx/Include/ -Iinc/
LDFLAGS   := $(MCUFLAGS) -T $(SRC_DIR)/stm32_ls.ld -Wl,-Map=$(BUILD_DIR)/app.map -nostartfiles

# Automatically locate all .c files
C_SRCS    := $(wildcard $(SRC_DIR)/*.c)

# Map source file paths in src/ to object file paths in obj/
OBJS      := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_SRCS))

# Default build rule
all: $(TARGET)

# Rule to link object files into the final .elf target
$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) $^ -o $@

# Rule to compile C files into obj/*.o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Order-only rule to create output directories on the fly
$(OBJ_DIR) $(BUILD_DIR):
	mkdir -p $@

# Clean rule to remove output directories
clean:
	rm -rf $(OBJ_DIR)/*.* $(BUILD_DIR)/*.*

load: build/app.elf
	openocd -f st_nucleo_f4.cfg -c "init" -c "reset init" -c "flash write_image erase build/app.elf" -c "verify_image build/app.elf" -c "reset run" -c "shutdown"
