#ifndef _AES_ENCRYPT_VECTOR_128_CL
#define _AES_ENCRYPT_VECTOR_128_CL

#include "aes_encrypt_tables.cl"
#include "aes_encrypt_vector.cl"

__kernel void aes_encrypt_vector_128(__global uint4 *state, __constant uint4 *roundkeys) {
	const uint id = get_global_id(0);

	uint4 s, t, o0, o1, o2, o3;
	uint r = 10;

	s = SWITCH_ORDER(state[id]) ^ roundkeys[0];

#ifdef FULL_UNROLL

	AES_ROUND(s, t,  1);
	AES_ROUND(t, s,  2);
	AES_ROUND(s, t,  3);
	AES_ROUND(t, s,  4);
	AES_ROUND(s, t,  5);
	AES_ROUND(t, s,  6);
	AES_ROUND(s, t,  7);
	AES_ROUND(t, s,  8);
	AES_ROUND(s, t,  9);
	AES_ROUND(t, s, 10);
	AES_ROUND(s, t, 11);
	AES_ROUND(t, s, 12);
	AES_ROUND(s, t, 13);

	roundkeys += r;

#else

	r >>= 1;
	for (;;) {
		AES_ROUND(s, t, 1);

		roundkeys += 2;
		if (--r == 0) {
			break;
		}

		AES_ROUND(t, s, 0);
	}

#endif

	// final round
	GET_OFFSETS(t);
	s = (uint4) (Te4[o3.x] & 0xff000000, Te4[o3.y] & 0xff000000, Te4[o3.z] & 0xff000000, Te4[o3.w] & 0xff000000) ^
	    (uint4) (Te4[o2.y] & 0x00ff0000, Te4[o2.z] & 0x00ff0000, Te4[o2.w] & 0x00ff0000, Te4[o2.x] & 0x00ff0000) ^
	    (uint4) (Te4[o1.z] & 0x0000ff00, Te4[o1.w] & 0x0000ff00, Te4[o1.x] & 0x0000ff00, Te4[o1.y] & 0x0000ff00) ^
	    (uint4) (Te4[o0.w] & 0x000000ff, Te4[o0.x] & 0x000000ff, Te4[o0.y] & 0x000000ff, Te4[o0.z] & 0x000000ff) ^
	    roundkeys[0];

	state[id] = SWITCH_ORDER(s);
}


#endif