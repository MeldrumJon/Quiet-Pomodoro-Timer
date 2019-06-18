# Quiet-Pomodoro-Timer

## Adding the Arduino AVR core

1. Download [ArduinoCore-avr](https://github.com/arduino/ArduinoCore-avr)
1. In `arduino-core` add the files from the `core/arduino` directory and from 
the `variants/[your variant]` directory.
1. In `lib` add the files from the `libraries`.
	1. If the source files are in a subdirectory, move the files up to the library's main 
	directory.

## Adding third-party libraries.

This project used David Prentice's LCDWIKI forks, 
[LCDWIKI_gui](https://github.com/prenticedavid/LCDWIKI_gui)
and [LCDWIKI_SPI](https://github.com/prenticedavid/LCDWIKI_SPI).

Add the library folders to `lib` and make sure the source files are in the library's 
top directory.