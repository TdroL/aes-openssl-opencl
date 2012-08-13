#ifndef CL_ENCRYPT_H
#define CL_ENCRYPT_H

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

bool cl_init(char *kernel_name, size_t state_length, size_t roundkeys_length, bool silent = false);

void cl_print_platform_info();
void cl_print_device_info();

void cl_update(uint8_t *text, uint32_t *roundkeys);

void cl_release_kernel();

void cl_release_all();

#endif