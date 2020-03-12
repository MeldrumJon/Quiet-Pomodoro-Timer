# Visual-Timer

Shows timer progress on screen.  Flashes a bright LED when the timer expires 
instead of ringing or vibrating.  Useful for time management in quiet offices 
or libraries.

![Visual Timer](../assets/Image.jpg)
![Time's Up](../assets/Flash.png)

## Parts

- [16x2 Reflective LCD Character Display](https://www.mouser.com/ProductDetail/668-S16204XRGHS10B)
- ATMega328p-PU
    - With fuses set to use internal 8MHz clock (use Makefile: `make fuses`)
- [Rotary encoder with push button switch](https://www.mouser.com/ProductDetail/652-PEC11R4315KS0012)
- 2x [Bright LEDs](https://www.mouser.com/ProductDetail/78-TLHK46Q1R2)
- 10uF electrolytic capacitor
- 3x 0.1uF ceramic capacitors
- 2x 220 ohm resistors
- 10k ohm potentiometer
- Breakaway headers
- Battery holder for 3xAA or 3xAAA batteries

## Tools

- AVR ISP Programmer or [arduinoISP](https://www.arduino.cc/en/tutorial/arduinoISP)
- USB to TTY serial adapter for debugging.

## Schematic

![Visual Timer's schematic](../assets/Schematic.png?raw=true)

## Setup

1. Install the AVR-GCC toolchain (and update your `PATH` variable, if necessary)
2. Connect the AVR ISP programmer pins to the ISP header on the PCB.
3. Connect the ATMega328p's TX pin to RX on the TTY serial adapter (optional)
4. Update the `program` and `fuses` recipes in the makefile to use your AVR 
ISP programmer.
5. Update the `serial` recipe in the makefile to use your USB to TTY serial 
adapter.
6. Run `make fuses` to set the fuses on the ATMega328p.
    - If the ATMega328p was previously set to use an external clock or crystal 
    oscillator, an external clock or crystal oscillator will be needed to set 
    the fuses.

## Programming

Run `make program` to program the ATMega328p.

## PCB

KiCAD files are in [the `hardware` Folder](./hardware).


