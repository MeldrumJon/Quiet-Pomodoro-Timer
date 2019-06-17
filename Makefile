###
# Folders
###
ARDUINO_CORE := arduino-core
SRC_DIR := src
BUILD_DIR := build

PWD:=$(shell pwd)
# Create the build folder if it does not exist (note that placing this comment by the PWD variable adds spaces to the variable).
$(shell mkdir -p "$(PWD)/$(BUILD_DIR)")
$(shell mkdir -p "$(PWD)/$(BUILD_DIR)/$(SRC_DIR)")
$(shell mkdir -p "$(PWD)/$(BUILD_DIR)/$(ARDUINO_CORE)")

###
# Compiler Settings
###
CPP_FLAGS := -mmcu=atmega328p -g -Wall -O2 -D F_CPU=8000000 -I$(ARDUINO_CORE)
C_FLAGS := -mmcu=atmega328p -g -Wall -O2 -D F_CPU=8000000 -I$(ARDUINO_CORE)

###
# Build Program
###
HEADERS := $(shell find $(SRC_DIR) -name '*.h')
CPP_SOURCES := $(shell find $(SRC_DIR) -name '*.cpp')
C_SOURCES := $(shell find $(SRC_DIR) -name '*.c')

$(BUILD_DIR)/prgm.hex: $(BUILD_DIR)/prgm.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

$(BUILD_DIR)/prgm.elf: $(CPP_SOURCES) $(C_SOURCES) $(BUILD_DIR)/core.a # Must be last
	avr-gcc $(CPP_FLAGS) -o $@ $^ -lm

###
# Build Arduino Core
###
CORE_HEADERS := $(shell find $(ARDUINO_CORE) -name '*.h')
CORE_CPP_SOURCES := $(shell find $(ARDUINO_CORE) -name '*.cpp')
CORE_C_SOURCES := $(shell find $(ARDUINO_CORE) -name '*.c')
CORE_CPP_OBJECTS := $(patsubst $(ARDUINO_CORE)/%.cpp, $(BUILD_DIR)/$(ARDUINO_CORE)/%.cpp.o, $(CORE_CPP_SOURCES))
CORE_C_OBJECTS := $(patsubst $(ARDUINO_CORE)/%.c, $(BUILD_DIR)/$(ARDUINO_CORE)/%.c.o, $(CORE_C_SOURCES))

$(BUILD_DIR)/core.a: $(CORE_CPP_OBJECTS) $(CORE_C_OBJECTS)
	avr-ar rcs $@ $^

$(BUILD_DIR)/$(ARDUINO_CORE)/%.c.o: $(ARDUINO_CORE)/%.c $(CORE_HEADERS)
	avr-gcc -c $(C_FLAGS) $< -o $@
	
$(BUILD_DIR)/$(ARDUINO_CORE)/%.cpp.o: $(ARDUINO_CORE)/%.cpp $(CORE_HEADERS)
	avr-gcc -c $(CPP_FLAGS) $< -o $@

###
# Cleanup
###
clean:
	rm -rf $(BUILD_DIR)
	
### 
# Communication Functions
###

program: $(BUILD_DIR)/prgm.hex
	avrdude -v -c usbtiny -p m328p -b 19200 -U flash:w:$(BUILD_DIR)/prgm.hex
 
# serial:
# 	screen /dev/cu.usbserial-A5XK3RJT 38400
