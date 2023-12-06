V = 1
BIN = samrh71_base_project

SAMRH71_CMSIS_DIR = /home/rolandcl/Projects/EmbeddedSystems/cmsis_packs/Microchip.SAMRH71_DFP.2.5.142/samrh71c
CORTEX_CMSIS_DIR = /home/rolandcl/Projects/EmbeddedSystems/cmsis_packs/ARM.CMSIS.5.6.0/CMSIS

INC = -I$(SAMRH71_CMSIS_DIR)/include -I$(CORTEX_CMSIS_DIR)/Include
SRC_C += $(wildcard src/*.c)
SRC_LD = link.ld

# FreeRTOS
#FREERTOS_DIR = /home/rolandcl/Projects/EmbeddedSystems/FreeRTOSv10.2.1/FreeRTOS/Source
FREERTOS_DIR = /home/rolandcl/Projects/EmbeddedSystems/FreeRTOS/FreeRTOS-Kernel
INC += -I$(FREERTOS_DIR)/include -I$(FREERTOS_DIR)/portable/GCC/ARM_CM7/r0p1
SRC_C += $(wildcard $(FREERTOS_DIR)/*.c)
SRC_C += $(FREERTOS_DIR)/portable/MemMang/heap_5.c
SRC_C += $(FREERTOS_DIR)/portable/GCC/ARM_CM7/r0p1/port.c
SRC_C += src/opt/printf-stdarg.c
#SRC_C += src/opt/scanf.c

ARCHFLAGS = -mcpu=cortex-m7
FPFLAGS = -mfloat-abi=softfp -mfpu=fpv5-d16
OPTFLAGS = -O1
BOOT_OPTFLAGS = -O1  # Avoid 'memcpy' in code & data relocation loops (boot.c)
#DBGFLAGS = -ggdb
# Use newlib-nano, include syscalls stubs (nosys)
SPECSFLAGS = --specs=nano.specs --specs=nosys.specs -fno-builtin-printf

DEF = -D__ATSAMRH71F20C__ -D"COMPONENT_TYPEDEF_STYLE='N'" -DDONT_USE_CMSIS_INIT

# Tools selection
CROSS_COMPILE = arm-none-eabi-

BUILD_DIR = ./build

JLINKEXE = JLinkExe -device ATSAMRH71F20C -if SWD -speed 4000 -autoconnect 1

###############################################################################################
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AS := $(CROSS_COMPILE)gcc
NM := $(CROSS_COMPILE)nm
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
SIZE := $(CROSS_COMPILE)size
GDB := $(CROSS_COMPILE)gdb
LD := $(CROSS_COMPILE)gcc

# Include directories
INC += -I.

# Compiler and linker flags
ARCHFLAGS += -mthumb -mabi=aapcs $(FPFLAGS)
WARNFLAGS = -Wall -Wextra -Wundef -Wshadow -Wimplicit-function-declaration \
             -Wredundant-decls -Wstrict-prototypes -Wmissing-prototypes \
             -Wconversion -Wno-sign-conversion -Wdouble-promotion \
			 -Wfloat-conversion
PREPFLAGS = -MD -MP

# This should be only enabled during development:
#WARNFLAGS += -Werror

CFLAGS = $(ARCHFLAGS) $(OPTFLAGS) $(DBGFLAGS) $(WARNFLAGS) $(PREPFLAGS) \
         -std=gnu99 \
         -ffunction-sections -fdata-sections -fno-strict-aliasing \
		 -save-temps=obj\
         $(SPECSFLAGS)

BOOT_CFLAGS = $(ARCHFLAGS) $(BOOT_OPTFLAGS) $(DBGFLAGS) $(WARNFLAGS) $(PREPFLAGS) \
         -std=gnu99 \
         -ffunction-sections -fdata-sections -fno-strict-aliasing \
         $(SPECSFLAGS)

LDFLAGS = $(ARCHFLAGS) $(OPTFLAGS) $(DBGFLAGS) \
          -Wl,-Map=$(BUILD_DIR)/$(BIN).map -Wl,--gc-sections \
          $(SPECSFLAGS)

FILENAMES_C = $(notdir $(SRC_C))
OBJS_C = $(addprefix $(BUILD_DIR)/, $(FILENAMES_C:.c=.o))
vpath %.c $(dir $(SRC_C))

OUTPUTS = $(BUILD_DIR)/$(BIN).elf
OUTPUTS += $(BUILD_DIR)/$(BIN).bin
#OUTPUTS += $(BUILD_DIR)/$(BIN).openocd.gdb

ifeq ($(V), 0)
	CMD_ECHO = @
else
	CMD_ECHO =
endif

###############################################################################################
.PHONY: default
default: all

.PHONY: all
all: $(BUILD_DIR) $(OUTPUTS)
	@echo ""
	$(CMD_ECHO) $(SIZE) $(BUILD_DIR)/$(BIN).elf

$(BUILD_DIR):
	$(CMD_ECHO) mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c
	@echo "  CC      $(notdir $@)"
	$(CMD_ECHO) $(CC) $(CFLAGS) $(DEF) $(INC) -c -o $@ $<

$(BUILD_DIR)/boot.o: boot.c
	@echo "  CC      $(notdir $@)"
	$(CMD_ECHO) $(CC) $(BOOT_CFLAGS) $(DEF) $(INC) -c -o $@ $<

$(BUILD_DIR)/$(BIN).elf: $(OBJS_ASM) $(OBJS_C) $(OBJS_CXX) | $(SRC_LD)
	@echo "  LD      $(notdir $@)"
	$(CMD_ECHO) $(LD) $^ $(LDFLAGS) $(INC) -T$(SRC_LD) -o $@

$(BUILD_DIR)/$(BIN).bin: $(BUILD_DIR)/$(BIN).elf
	@echo "  OBJCOPY      $(notdir $@)"
	$(CMD_ECHO) $(OBJCOPY) -O binary $(BUILD_DIR)/$(BIN).elf $@

	

#$(BUILD_DIR)/$(BIN).elf: $(OBJS_ASM) $(OBJS_C) $(OBJS_CXX) | $(SRC_LD)
#    @echo "  OBJCOPY      $(notdir $@)"
#    $(CMD_ECHO) $(OBJCOPY) -O binary $(BUILD_DIR)/$(BIN).elf $(BUILD_DIR)/$(BIN).bin

.PHONY: flash
flash: $(BUILD_DIR)/$(BIN).bin
	$(CMD_ECHO) $(JLINKEXE) -CommandFile jlink_flash.bat

.PHONY: clean
clean:
	rm -f $(OUTPUTS)
	rm -f $(BUILD_DIR)/$(BIN).map $(BUILD_DIR)/*.o
