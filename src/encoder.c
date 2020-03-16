#include "encoder.h"
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>

// Mask for the register that handles the A/B pin input
#define ROTARY_A_DMASK 0x10
#define ROTARY_B_DMASK 0x08
#define PCINT2_EN_MASK 0x04

// Contains the previous state of the encoder (bits[7:2]: unused)
// bit[1]: old state of A
// bit[0]: old state of B
volatile static uint8_t state = 0;

// Contains the change in encoder position since the last read.
volatile static int8_t g_delta = 0;

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
int8_t delta_lut[16] = {
    0,
    +1,
    -1,
    +2,
    -1,
    0,
    -2,
    +1,
    +1,
    -2,
    0,
    -1,
    +2,
    -1,
    +1,
    0
};

ISR(PCINT2_vect, ISR_BLOCK) {
	uint8_t s = state;
	if (PIND & ROTARY_A_DMASK) { s |= 0x4; }
	if (PIND & ROTARY_B_DMASK) { s |= 0x8; }
	g_delta += delta_lut[s];
	state = (s >> 2);
}

/**
 * Enables pin change interrupts for the rotary encoder.
 */
void encoder_enable(void) {
	// Setup pin change interrupts (encoder)
	cli();
	DDRD &= ~(ROTARY_A_DMASK | ROTARY_B_DMASK); // Set pins as input
	PORTD |= (ROTARY_A_DMASK | ROTARY_B_DMASK); // Activate pull-ups
	PCICR |= PCINT2_EN_MASK; // Enable pin-change interrupt PCINT2
	PCMSK2 |= (ROTARY_A_DMASK | ROTARY_B_DMASK); // Enable interrupts for pins
	sei();
}

void encoder_disable(void) {
	cli();
	DDRD |= (ROTARY_A_DMASK | ROTARY_B_DMASK); // Set pins as output
	PORTD &= ~(ROTARY_A_DMASK | ROTARY_B_DMASK); // Drive pins low 
	PCICR &= ~(PCINT2_EN_MASK); // Disable pin-change interrupt
	PCMSK2 &= ~(ROTARY_A_DMASK | ROTARY_B_DMASK); // Disable interrupts for the pins
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
int8_t encoder_delta(void) {
	static int8_t old_pos = 0; // Save the old position for the next call
	int8_t zeroed_delta;
	int8_t pos;

	cli();
	pos = g_delta;
	// Zero g_delta, except for the lower digits to avoid adding a weird offset if this 
	// function is called while the encoder is partway turned.
	g_delta = zeroed_delta = g_delta & 0x3;
	sei();

	pos = (pos + 2) >> 2; // Round position to nearest detent.
	int8_t delta = (pos - old_pos);
	old_pos = (zeroed_delta + 2) >> 2; // Round "cleared" g_delta to nearest detent

	return delta;
}
