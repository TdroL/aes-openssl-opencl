#ifndef _AES_ENCRYPT_VECTOR_128_LOCAL_CL
#define _AES_ENCRYPT_VECTOR_128_LOCAL_CL

#include "aes_encrypt_tables.cl"
#include "aes_encrypt_vector.cl"

__kernel void aes_encrypt_vector_128_local(__global uint4 *state, __constant uint4 *roundkeys) {
	const uint id = get_global_id(0);
	const uint local_id = get_local_id(0);

	__local uint lTe0[256], lTe1[256], lTe2[256], lTe3[256], lTe4[256];

	lTe0[local_id] = Te0[local_id];
	lTe1[local_id] = Te1[local_id];
	lTe2[local_id] = Te2[local_id];
	lTe3[local_id] = Te3[local_id];
	lTe4[local_id] = Te4[local_id];

	barrier(CLK_LOCAL_MEM_FENCE);

	uint4 s, t, o0, o1, o2, o3, s0, s1, s2, s3;
	uint r = 10;

	s = SWITCH_ORDER(state[id]) ^ roundkeys[0];

#ifdef FULL_UNROLL

	AES_ROUND_NAMED(s, t,  1, lTe0, lTe1, lTe2, lTe3);
	AES_ROUND_NAMED(t, s,  2, lTe0, lTe1, lTe2, lTe3);
	AES_ROUND_NAMED(s, t,  3, lTe0, lTe1, lTe2, lTe3);
	AES_ROUND_NAMED(t, s,  4, lTe0, lTe1, lTe2, lTe3);
	AES_ROUND_NAMED(s, t,  5, lTe0, lTe1, lTe2, lTe3);
	AES_ROUND_NAMED(t, s,  6, lTe0, lTe1, lTe2, lTe3);
	AES_ROUND_NAMED(s, t,  7, lTe0, lTe1, lTe2, lTe3);
	AES_ROUND_NAMED(t, s,  8, lTe0, lTe1, lTe2, lTe3);
	AES_ROUND_NAMED(s, t,  9, lTe0, lTe1, lTe2, lTe3);
	AES_ROUND_NAMED(t, s, 10, lTe0, lTe1, lTe2, lTe3);
	AES_ROUND_NAMED(s, t, 11, lTe0, lTe1, lTe2, lTe3);
	AES_ROUND_NAMED(t, s, 12, lTe0, lTe1, lTe2, lTe3);
	AES_ROUND_NAMED(s, t, 13, lTe0, lTe1, lTe2, lTe3);

	roundkeys += r;

#else

	r >>= 1;
	for (;;) {
		AES_ROUND_NAMED(s, t, 1, lTe0, lTe1, lTe2, lTe3);

		roundkeys += 2;
		if (--r == 0) {
			break;
		}

		AES_ROUND_NAMED(t, s, 0, lTe0, lTe1, lTe2, lTe3);
	}

#endif

	// final round
	GET_OFFSETS(t);
	s = (uint4) (lTe4[o3.x] & 0xff000000, lTe4[o3.y] & 0xff000000, lTe4[o3.z] & 0xff000000, lTe4[o3.w] & 0xff000000) ^
	    (uint4) (lTe4[o2.y] & 0x00ff0000, lTe4[o2.z] & 0x00ff0000, lTe4[o2.w] & 0x00ff0000, lTe4[o2.x] & 0x00ff0000) ^
	    (uint4) (lTe4[o1.z] & 0x0000ff00, lTe4[o1.w] & 0x0000ff00, lTe4[o1.x] & 0x0000ff00, lTe4[o1.y] & 0x0000ff00) ^
	    (uint4) (lTe4[o0.w] & 0x000000ff, lTe4[o0.x] & 0x000000ff, lTe4[o0.y] & 0x000000ff, lTe4[o0.z] & 0x000000ff) ^
	    roundkeys[0];

	state[id] = SWITCH_ORDER(s);
}

#endif