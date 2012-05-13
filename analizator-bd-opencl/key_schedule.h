#ifndef KEY_SCHEDULE_H
#define KEY_SCHEDULE_H

#include <cstdint>

#ifndef KEYLENGTH
	#define KEYLENGTH(keybits) ((keybits)/8)
#endif
#ifndef RKLENGTH
	#define RKLENGTH(keybits)  ((keybits)/8+28)
#endif
#ifndef NROUNDS
	#define NROUNDS(keybits)   ((keybits)/32+6)
#endif

typedef union {
	uint8_t c[4];
	uint32_t i;
} uint_u;

void key_schedule(uint32_t *keys, unsigned int rounds);
void key_schedule_core(uint_u *block, unsigned int i);

#endif