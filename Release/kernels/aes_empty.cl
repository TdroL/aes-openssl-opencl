#ifndef _AES_EMPTY_CL
#define _AES_EMPTY_CL



__kernel void aes_empty(
	__global uint4 *state,
	__global uint4 roundkeys[15]
) {
	const uint id = get_global_id(0);

	uint4 s, t, o0, o1, o2, o3;

	s = state[id] ^ roundkeys[1];

	AES_ROUND(s, t, 1);

	state[id] = s;
}

#endif