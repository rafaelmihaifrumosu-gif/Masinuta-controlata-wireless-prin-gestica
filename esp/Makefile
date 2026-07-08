# --- Configurare Toolchain ---
# Calea este setată conform locației identificate pe sistemul tău
ESP_TOOLCHAIN_PATH = /home/rafael/.espressif/tools/xtensa-esp-elf/esp-16.1.0_20260609/xtensa-esp-elf/bin/
CC = $(ESP_TOOLCHAIN_PATH)xtensa-esp-elf-gcc
OBJCOPY = $(ESP_TOOLCHAIN_PATH)xtensa-esp-elf-objcopy

# --- Porturi și Programmer ---
PORT = /dev/ttyUSB0
BAUD = 921600

# --- Steaguri de compilare ---
# -mlongcalls este esențial pentru arhitectura ESP32
# Am adăugat -Idrivers/uart
CFLAGS = -mlongcalls -Wno-frame-address -Os -ffunction-sections -fdata-sections
CFLAGS += -DESP32 -std=gnu99 -I. -Idrivers/gpio -Idrivers/uart -Idrivers/inc -Iutils

# --- Fișiere și Directoare ---
SRC = src/main.c drivers/gpio/gpio.c drivers/uart/uart.c utils/delay.c
OBJDIR = obj
BINDIR = bin
OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))
TARGET = $(BINDIR)/main

# --- Reguli ---
all: directories $(TARGET).bin

directories:
	@mkdir -p $(BINDIR)
	@mkdir -p $(OBJDIR)/src $(OBJDIR)/drivers/gpio $(OBJDIR)/drivers/uart $(OBJDIR)/utils

# Regula pentru compilare
$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Regula pentru linkare (.elf)
$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) -Wl,--gc-sections -T esp32.ld -o $@ $^

# Regula pentru generare binar (.bin)
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Regula pentru Flash (încarcă binarul pe ESP32)
flash: all
	esptool.py --chip esp32 --port $(PORT) --baud $(BAUD) write_flash 0x10000 $(TARGET).bin

# Curățare fișiere generate
clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: all clean directories flash