#ifndef CL_ENCRYPT_H
#define CL_ENCRYPT_H

#include <cstdint>
#include "CL/cl.h"

void cl_init(char *kernel_name, uint8_t *state_ptr, unsigned int state_length,
			 unsigned int roundkeys_length);

void cl_encrypt(uint8_t *roundkeys, unsigned int rk_length, uint8_t *text,
	            unsigned int text_length);

void cl_release_kernel();

void cl_release_all();

#endif