#ifndef KEY_SCHEDULE_H
#define KEY_SCHEDULE_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>

// based on http://www.samiam.org/key-schedule.html

void key_schedule(const uint8_t *key, unsigned int n, uint8_t *expanded_key);
void key_schedule_core(uint8_t *block, unsigned int i);

#endif