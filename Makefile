# 1. VariablesOBJS_LL =  $(BUILD_DIR)/stm32f4xx_ll_pwr.o \
           $(BUILD_DIR)/stm32f4xx_ll_rcc.o \
           $(BUILD_DIR)/stm32f4xx_ll_utils.o \
           $(BUILD_DIR)/stm32f4xx_ll_dma.o \
           $(BUILD_DIR)/stm32f4xx_ll_usart.o \
           $(BUILD_DIR)/system_stm32f4xx.o
CC         = arm-none-eabi-gcc
GDB        = arm-none-eabi-gdb
OBJCOPY    = arm-none-eabi-objcopy
MACH       = cortex-m4
BUILD_DIR  = ./build
SRC_DIR    = ./src
LL_SRC_DIR = ./src/LL
ASM_SRC_DIR = ./src/asm
INIT_SRC_DIR = ./src/init
FLASH_PIC32MX_SRC_DIR = ./src/flash_pic32mx

# --- STARTUP CHOICE ---
# Change this to 's' to use the Assembly startup
# Change this to 'c' to use the C startup
STARTUP_TYPE = c
# ----------------------

# 2. Paths
INC_DIR    = ./inc
CMSIS_DEV  = /home/tuna/projects/stm32f446-libraries/CMSIS-DEVICE-F4
CMSIS_CORE = /home/tuna/projects/stm32f446-libraries/CMSIS-CORE
LL_DIR     = /home/tuna/projects/stm32f4xx-hal-driver/Inc 

# 3. Flags
INCLUDES   = -I$(INC_DIR) -I$(CMSIS_DEV) -I$(CMSIS_CORE)
INCLUDES_LL = -I$(LL_DIR)

CFLAGS     = -c -mcpu=$(MACH) -mthumb -std=gnu11 -O0 -g -DSTM32F446xx -DUSE_FULL_LL_DRIVER $(INCLUDES) $(INCLUDES_LL)
LDFLAGS = -mcpu=$(MACH) -mthumb --specs=nano.specs --specs=nosys.specs -T stm32f446re.ld -Wl,-Map=$(BUILD_DIR)/final.map

# 4. Logic to select the startup object file
ifeq ($(STARTUP_TYPE), c)
    STARTUP_OBJ = $(BUILD_DIR)/startup_c.o
else
    STARTUP_OBJ = $(BUILD_DIR)/startup_s.o
endif

# ST Low-Level Drivers
OBJS_LL =  $(BUILD_DIR)/stm32f4xx_ll_pwr.o \
           $(BUILD_DIR)/stm32f4xx_ll_rcc.o \
           $(BUILD_DIR)/stm32f4xx_ll_utils.o \
           $(BUILD_DIR)/stm32f4xx_ll_dma.o \
           $(BUILD_DIR)/stm32f4xx_ll_usart.o \
           $(BUILD_DIR)/system_stm32f4xx.o

# Project Object Files (Mapped to their respective subfolders)
OBJS    =  $(BUILD_DIR)/main.o \
           $(BUILD_DIR)/timer.o \
	   $(BUILD_DIR)/flash_engine.o\
	   $(BUILD_DIR)/uart_dma_function_manager.o\
	   $(BUILD_DIR)/init_dma.o\
	   $(BUILD_DIR)/init_gpio.o\
	   $(BUILD_DIR)/icsp_bitbang.o\
	   $(BUILD_DIR)/init_usart.o\
	   $(BUILD_DIR)/pseudo_operations.o\
	   $(BUILD_DIR)/pic32_pe.o\
	   $(BUILD_DIR)/enter_enhanced_icsp.o\
	   $(BUILD_DIR)/check_device_status.o\
	   $(BUILD_DIR)/erase_device.o\
	   $(BUILD_DIR)/enter_serial_exec.o\
	   $(BUILD_DIR)/download_pe.o\
	   $(BUILD_DIR)/exit_programming_mode.o\
           $(STARTUP_OBJ)


all: $(BUILD_DIR)/final.bin

$(BUILD_DIR)/final.bin: $(BUILD_DIR)/final.elf
	$(OBJCOPY) -O binary $< $(BUILD_DIR)/final.bin

$(BUILD_DIR)/final.elf: $(OBJS) $(OBJS_LL)
	$(CC) $(LDFLAGS) -o $@ $^

# --- Pattern Rules for Source Subdirectories ---

# Pattern rule for C files in root src/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

# Pattern rule for C files in src/init/
$(BUILD_DIR)/%.o: $(INIT_SRC_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

# Pattern rule for C files in src/flash_pic32mx/
$(BUILD_DIR)/%.o: $(FLASH_PIC32MX_SRC_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

# Pattern rule for Assembly files in src/asm/ 
$(BUILD_DIR)/%.o: $(ASM_SRC_DIR)/%.s
	$(CC) $(CFLAGS) -o $@ $<

# Pattern rule for ST LL source files in src/LL/
$(BUILD_DIR)/%.o: $(LL_SRC_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

# Rule for C startup (if selected)
$(BUILD_DIR)/startup_c.o: startup_stm32f446xx.c
	$(CC) $(CFLAGS) -o $@ $<

# Rule for Assembly startup (if selected)
$(BUILD_DIR)/startup_s.o: startup_stm32f446xx.s
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)/*

# --- Debug & Flash Targets ---
load:
	openocd -f board/st_nucleo_f4.cfg

run_semi: $(BUILD_DIR)/final.elf
	openocd -f board/st_nucleo_f4.cfg \
		-c "init" \
		-c "arm semihosting enable" \
		-c "program $< verify reset"

flash_debug: $(BUILD_DIR)/final.elf
	$(GDB) \
		-ex "target remote localhost:3333" \
		-ex "monitor arm semihosting enable" \
		-ex "monitor reset init" \
		-ex "load" \
		-ex "monitor reset halt" \
		$<

debug: $(BUILD_DIR)/final.elf
	$(GDB) -ex "target remote localhost:3333" \
		-ex "monitor reset init" \
		$<

flash: $(BUILD_DIR)/final.bin
	openocd -f board/st_nucleo_f4.cfg -c "program $< verify reset exit 0x08000000"
