#include <stdint.h>

extern uint_fast8_t flags;

#define FLAGS_SET(f) flags |= (f);
#define FLAGS_CLEAR(f) flags &= ~(f);
#define FLAGS_TEST(f) (flags & (f))
#define FLAGS_CLEAR_ALL(null) flags = 0;

#define FLAG_RESET 0x01