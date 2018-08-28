#include <encDelta.h>
#include <Encoder.h>

#include <ringTimeCommon.h>

Encoder encoder(ENC_PIN_A, ENC_PIN_B);

int_fast8_t encDelta_getChange() {
  static int_fast16_t oldPosition = 0;
  int_fast16_t position = encoder.read();

  // Round change to the nearest multiple of COUNT_PER_DETENT, but don't shift back
  // so that we only count the number of detent changes.
  position = position - 2; // Do a little bit of rounding (don't always ceil): [0, 1] -> 0; [2, 3, 4, 5] -> 4
  position = position >> 2; 
  position = position + 1;

  if (position == oldPosition) {
    return 0;
  }
  int_fast16_t change = position - oldPosition;
  oldPosition = position;
  return change;
}