# Chip Info
F_CPU := 8000000
MCU := atmega328p

# Compiler Options
FLAGS := "-O2 -Wall -mmcu=atmega328p -DF_CPU=8000000L"

# Folders
PWD=$(shell pwd) # Create the build folder if it does not exist
$(shell mkdir -p $(PWD)/$(BUILD_DIR))

ARDUINO_CORE := arduino-core
SRC_DIR := src
BUILD_DIR := build

# Build arduino core
CORE_SOURCES := $(shell find $(ARDUINO_CORE) -name '*.cpp' -o -name '*.c')
CORE_HEADERS := $(shell find $(ARDUINO_CORE) -name '*.h')
CORE_OBJECTS := $(patsubst $(ARDUINO_CORE)/%.c,$(ARDUINO_CORE)/%.o,$(CORE_SOURCES))

$(BUILD_DIR)/%.o: $(CORE_SOURCES)
	avr-gcc $(FLAGS) -I$(ARDUINO_CORE) -o $@ $<

$(BUILD_DIR)/core.a: $(CORE_OBJECTS)
	echo $(CORE_OBJECTS)

C_FILES := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*/*.c) $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp)
H_FILES := $(wildcard $(SRC_DIR)/*.h) $(wildcard $(SRC_DIR)/*/*.h)

$(BUILD_DIR)/prgm.hex: $(BUILD_DIR)/prgm.elf
	avr-objcopy -j .text -j .data -O ihex $(BUILD_DIR)/prgm.elf $(BUILD_DIR)/prgm.hex

$(BUILD_DIR)/prgm.elf: $(C_FILES) $(H_FILES)
	avr-gcc -mmcu=atmega328p -g -Wall -O2 -D F_CPU=8000000 -o $(BUILD_DIR)/prgm.elf $(C_FILES)
	
$(BUILD_DIR)/prgm.lst: $(BUILD_DIR)/prgm.elf
	avr-objdump -h -S $(BUILD_DIR)/prgm.elf > $(BUILD_DIR)/prgm.lst
	
listing: $(BUILD_DIR)/prgm.lst

clean:
	rm -rf $(BUILD_DIR)

program: $(BUILD_DIR)/prgm.hex
	avrdude -v -c usbtiny -p m328p -b 19200 -U flash:w:$(BUILD_DIR)/prgm.hex

serial:
	screen /dev/cu.usbserial-A5XK3RJT 38400
