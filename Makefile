###
# Folders
###
ARDUINO_DIR := arduino-core
LIB_DIR := lib
SRC_DIR := src
BUILD_DIR := build

#####
# Files
#####

HEADERS := $(shell find $(SRC_DIR) -name '*.h')
SOURCES := $(shell find $(SRC_DIR) -name '*.cpp' -o -name '*.c')
OBJECTS := $(patsubst %, $(BUILD_DIR)/%.o, $(SOURCES))

CORE_HEADERS := $(shell find $(ARDUINO_DIR) -name '*.h')
CORE_SOURCES := $(shell find $(ARDUINO_DIR) -name '*.cpp' -o -name '*.c')
CORE_OBJECTS := $(patsubst %, $(BUILD_DIR)/%.o, $(CORE_SOURCES))

LIB_HEADERS := $(shell find $(LIB_DIR) -name '*.h')
LIB_SOURCES := $(shell find $(LIB_DIR) -name '*.cpp' -o -name '*.c')
LIB_OBJECTS := $(patsubst %, $(BUILD_DIR)/%.o, $(LIB_SOURCES))

# Library include paths for the compiler
LIB_PATHS := $(shell ls -d -1 "$(LIB_DIR)/"**/)
LIB_PARAMS := $(foreach d, $(LIB_PATHS), -I$d)

#####
# Compiler Settings
#####
FLAGS := -mmcu=atmega328p -D F_CPU=8000000 -D ARDUINO=9999999 -I$(ARDUINO_DIR) -I$(SRC_DIR) $(LIB_PARAMS)
CPP_FLAGS := -g -Wall -Os $(FLAGS)
C_FLAGS := -g -Wall -Os $(FLAGS)

#####
# Build Program
#####

$(BUILD_DIR)/prgm.hex: $(BUILD_DIR)/prgm.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

$(BUILD_DIR)/prgm.elf: $(SOURCES) $(BUILD_DIR)/lib.a $(BUILD_DIR)/core.a # core.a must be last
	avr-gcc $(CPP_FLAGS) -o $@ $(SOURCES) $(BUILD_DIR)/lib.a -lm $(BUILD_DIR)/core.a -lm

#####
# Build Libraries
#####

$(BUILD_DIR)/lib.a: $(LIB_OBJECTS)
	avr-ar rcs $@ $^

$(LIB_OBJECTS): $(LIB_HEADERS) $(BUILD_DIR)/core.a # Recompile if headers or arduino-core change

#####
# Build Arduino Core
#####

$(BUILD_DIR)/core.a: $(CORE_OBJECTS)
	avr-ar rcs $@ $(CORE_OBJECTS)

$(CORE_OBJECTS): $(CORE_HEADERS) # Recompile if headers change

#####
# Compile Objects
#####

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(@D)
	avr-gcc -c $(C_FLAGS) $< -o $@
	
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(@D)
	avr-gcc -c $(CPP_FLAGS) $< -o $@

#####
# Cleanup
#####
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	
#####
# Communication Functions
#####

.PHONY: program
program: $(BUILD_DIR)/prgm.hex
	avrdude -v -c usbtiny -p m328p -B 0.5 -U flash:w:$(BUILD_DIR)/prgm.hex

.PHONY: serial
serial:
	screen /dev/cu.usbserial-A5XK3RJT 38400
