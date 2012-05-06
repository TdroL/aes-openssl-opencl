#ifndef KEY_SCHEDULE_H
#define KEY_SCHEDULE_H

#include <cstdint>

#define KEYLENGTH(keybits) ((keybits)/8)
#define RKLENGTH(keybits)  (((keybits)/8+28)*4)
#define NROUNDS(keybits)   ((keybits)/32+6)

typedef union {
	uint8_t c[4];
	uint32_t i;
} uint_u;

void key_schedule(uint8_t *keys, unsigned int rounds);
void key_schedule_core(uint8_t *block, unsigned int i);

#endif