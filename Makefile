###### Build #####
SRC_DIR := ./src
BUILD_DIR := ./build

# Create the build folder if it does not exist
PWD=$(shell pwd)
$(shell mkdir -p $(PWD)/$(BUILD_DIR))

C_FILES := $(wildcard $(SRC_DIR)/*.c)
H_FILES := $(wildcard $(SRC_DIR)/*.h)

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
	avrdude -v -c arduino -p m328p -b 19200 -P /dev/cu.usbmodem14101 -U flash:w:$(BUILD_DIR)/prgm.hex

serial:
	screen /dev/cu.usbserial-A5XK3RJT 9600
