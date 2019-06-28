#include "encoder.h"
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>

// Mask for the register that handles the A/B pin input
#define ROTARY_A_MASK 0x10
#define ROTARY_B_MASK 0x20

// Contains the previous state of the encoder (bits[7:2]: unused)
// bit[1]: old state of A
// bit[0]: old state of B
volatile static uint_fast8_t state = 0;

// Contains the change in encoder position since the last read.
volatile static int_fast8_t g_delta = 0;

/**
 * This ISR is called when one of the pins on the rotary encoder change levels.
 * <code>
 *                     _______         _______       
 *            A ______|       |_______|       |______ A
 * CCW <---        _______         _______         __      --> CW
 *            B __|       |_______|       |_______|   B
 * </code>
 * <p>
 * Variable s retrieves the old state (stored in bits 1 and 0) and reads the new state
 * (stored in bits 3 and 2).  The table below shows how to calculate the change in the
 * encoder position from the previous and current state.
 * </p>
 * <code>
 *	    new	new old	old
 * case	 B	 A	 B	 A 	 Result
 * ---- --- --- --- ---  ------
 *   0   0	 0	 0	 0	 no movement
 *   1   0	 0	 0	 1	 +1
 *   2   0	 0	 1	 0	 -1
 *   3   0	 0	 1	 1	 +2  (assume A's edges only)
 *   4   0	 1	 0	 0	 -1
 *   5   0	 1	 0	 1	 no movement
 *   6   0	 1	 1	 0	 -2  (assume A's edges only)
 *   7   0	 1	 1	 1	 +1
 *   8   1	 0	 0	 0	 +1
 *   9   1	 0	 0	 1	 -2  (assume A's edges only)
 *  10   1	 0	 1	 0	 no movement
 *  11   1	 0	 1	 1	 -1
 *  12   1	 1	 0	 0	 +2  (assume A's edges only)
 *  13   1	 1	 0	 1	 -1
 *  14   1	 1	 1	 0	 +1
 *  15   1	 1	 1	 1	 no movement
 * </code>
 */
ISR(PCINT1_vect, ISR_BLOCK) {
	uint_fast8_t s = state;
	uint_fast8_t pins = PINC;
	if (pins & ROTARY_A_MASK) { s |= 0x4; }
	if (pins & ROTARY_B_MASK) { s |= 0x8; }
	switch (s) {
		case 0: case 5: case 10: case 15:
			break;
		case 1: case 7: case 8: case 14:
			g_delta++; break;
		case 2: case 4: case 11: case 13:
			g_delta--; break;
		case 3: case 12:
			g_delta += 2; break;
		default:
			g_delta -= 2; break;
	}
	state = (s >> 2);
}

/**
 * Enables pin change interrupts for the rotary encoder.
 */
void encoder_enable(void) {
	// Setup pin change interrupts (encoder)
	cli();
	DDRC &= ~(0x30); // Set pins as input
	PORTC |= 0x30; // Activate pull-ups
	PCICR |= 0x02; // Enable pin-change interrupt PCINT1
	PCMSK1 |= 0x30; // Enable interrupts for pins
	sei();
}

void encoder_disable(void) {
	cli();
	DDRC |= (0x30); // Set pins as output
	PORTC &= ~(0x30); // Drive pins low 
	PCICR &= ~(0x02); // Disable pin-change interrupt
	PCMSK1 &= ~(0x30); // Disable interrupts for the pins
	sei();
}

/**
 * Returns the number of detents moved since the last time encoder_delta was called.
 *
 * <p>
 * The global variable g_delta actually increments/decrements 4 times between detents.
 * This function rounds to the nearest detent.
 * </p>
 * <code>
 * [0, 1] -> 0
 * [2, 3, 4, 5] -> 4
 * </code>
 *
 * @return The number of detents turned since last call.  Negative values indicate
 * counter-clockwise turning.  Positive values indicate clockwise turning.
 */
int_fast8_t encoder_delta(void) {
	static int_fast8_t old_pos = 0; // Save the old position for the next call
	int_fast8_t zeroed_delta;
	int_fast8_t pos;

	cli();
	pos = g_delta;
	// Zero g_delta, except for the lower digits to avoid adding a weird offset if this 
	// function is called while the encoder is partway turned.
	g_delta = zeroed_delta = g_delta & 0x3;
	sei();

	pos = (pos + 2) >> 2; // Round position to nearest detent.
	int_fast8_t delta = (pos - old_pos);
	old_pos = (zeroed_delta + 2) >> 2; // Round "cleared" g_delta to nearest detent

	return delta;
}