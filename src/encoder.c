#include "encoder.h"

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#define ROTARY_A_MASK 0x4
#define ROTARY_B_MASK 0x8

//                     _______         _______       
//            A ______|       |_______|       |______ A
// CCW <---        _______         _______         __      --> CW
//            B __|       |_______|       |_______|   B

// Contains the previous state of the encoder (bits[7:2]: unused)
// bit[1]: old state of A
// bit[0]: old state of B
volatile static uint_fast8_t state = 0;
volatile static int_fast8_t position = 0;

// Variable s retrieves the old state and current state, stored in bits[3:0]
// as shown below.  The switch statements implements the table below.
//	    new	new old	old
// case	 B	 A	 B	 A 	 Result
// ---- --- --- --- ---  ------
//   0   0	 0	 0	 0	 no movement
//   1   0	 0	 0	 1	 +1
//   2   0	 0	 1	 0	 -1
//   3   0	 0	 1	 1	 +2  (assume A's edges only)
//   4   0	 1	 0	 0	 -1
//   5   0	 1	 0	 1	 no movement
//   6   0	 1	 1	 0	 -2  (assume A's edges only)
//   7   0	 1	 1	 1	 +1
//   8   1	 0	 0	 0	 +1
//   9   1	 0	 0	 1	 -2  (assume A's edges only)
//  10   1	 0	 1	 0	 no movement
//  11   1	 0	 1	 1	 -1
//  12   1	 1	 0	 0	 +2  (assume A's edges only)
//  13   1	 1	 0	 1	 -1
//  14   1	 1	 1	 0	 +1
//  15   1	 1	 1	 1	 no movement
ISR(PCINT2_vect, ISR_BLOCK) {
	uint_fast8_t s = state;
	uint_fast8_t pins = PIND;
	if (pins & ROTARY_A_MASK) { s |= 0x4; }
	if (pins & ROTARY_B_MASK) { s |= 0x8; }
	switch (s) {
		case 0: case 5: case 10: case 15:
			break;
		case 1: case 7: case 8: case 14:
			position++; break;
		case 2: case 4: case 11: case 13:
			position--; break;
		case 3: case 12:
			position += 2; break;
		default:
			position -= 2; break;
	}
	state = (s >> 2);
}

// ISR(INT0_vect) {
// 	update();
// }
// 
// ISR(INT1_vect) {
// 	update();
// }

void encoder_init(void) {
// 	// Setup pin change interrupts (encoder)
// 	EICRA = 0x5;
// 	EIMSK = 0x3;
	cli();
	PCICR = 0x04;
	PCMSK2 = 0x0C;
	sei();
	// PD3 (B) = PCINT19
	// PD2 (A) = PCINT18
}

// There are 4 changes per detent, so when position is, closest to zero, we should use 0.
// If position is closest to 4, we should use 4
// [0, 1] -> 0
// [2, 3, 4, 5] -> 4
// (_0_ + 2) & 0xFC = 0
// (_1_ + 2) & 0xFC = 0
// (_2_ + 2) & 0xFC = 4
// (_3_ + 2) & 0xFC = 4
// (_4_ + 2) & 0xFC = 4
// (_5_ + 2) & 0xFC = 4
// (_6_ + 2) & 0xFC = 8

int_fast8_t encoder_delta(void) {
	static int_fast8_t old_pos = 0;
	int_fast8_t next_old_pos = 0;
	int_fast8_t pos;
	cli();
	pos = position;
	position = position & 0x3;
	sei();
	next_old_pos = ((pos & 0x3) + 2) >> 2;
	pos = (pos + 2) >> 2;
	int_fast8_t delta = (pos - old_pos);
	old_pos = next_old_pos;
	return delta;
}
