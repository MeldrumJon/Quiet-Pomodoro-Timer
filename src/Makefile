BUILD_DIR := build

FLAGS = -mmcu=atmega328p 
FLAGS += -DF_CPU=8000000UL
FLAGS += -O2 -Wall -Wl,--gc-sections -ffunction-sections -fdata-sections
FLAGS += -fno-exceptions -Wno-error=narrowing
FLAGS += -fno-fat-lto-objects -flto -fuse-linker-plugin

HEADERS := $(shell find . -name '*.h')
SOURCES := $(shell find . -name '*.cpp' -o -name '*.c')

$(BUILD_DIR)/prgm.hex: $(HEADERS) $(SOURCES)
	mkdir -p $(BUILD_DIR)
	avr-gcc $(FLAGS) -o $(BUILD_DIR)/prgm.elf $(SOURCES) $(LIB_SOURCES) $(CORE_SOURCES)
	avr-objcopy -j .text -j .data -O ihex $(BUILD_DIR)/prgm.elf $(BUILD_DIR)/prgm.hex

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: program
program: $(BUILD_DIR)/prgm.hex
	avrdude -v -c usbtiny -p m328p -B5 -U flash:w:$(BUILD_DIR)/prgm.hex
	
.PHONY: fuses
fuses:
	avrdude -v -c usbtiny -p m328p -B5 -U efuse:w:0xFF:m -U hfuse:w:0xD9:m -U lfuse:w:0xE2:m

.PHONY: serial
serial:
	screen /dev/cu.usbserial-A5XK3RJT 38400

