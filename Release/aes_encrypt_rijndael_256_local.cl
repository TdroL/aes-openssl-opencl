#ifndef _AES_ENCRYPT_RIJNDAEL_256_LOCAL_CL
#define _AES_ENCRYPT_RIJNDAEL_256_LOCAL_CL

#include "aes_encrypt_tables.cl"
#include "aes_encrypt_rijndael.cl"

#ifdef cl_nv_pragma_unroll
	#pragma OPENCL EXTENSION cl_nv_pragma_unroll : enable
#endif

__kernel void aes_encrypt_rijndael_256_local(
	__global uint *state,
	__constant uint *roundkeys
) {
	const uint id = get_global_id(0);
	/*
	const uint local_id = get_local_id(0);

	__local uint lTe0[256], lTe1[256], lTe2[256], lTe3[256], lTe4[256];

	lTe0[local_id] = Te0[local_id];
	lTe1[local_id] = Te1[local_id];
	lTe2[local_id] = Te2[local_id];
	lTe3[local_id] = Te3[local_id];
	lTe4[local_id] = Te4[local_id];

	barrier(CLK_LOCAL_MEM_FENCE);
	*/

	#define lTe0 Te0
	#define lTe1 Te1
	#define lTe2 Te2
	#define lTe3 Te3
	#define lTe4 Te4

	state += id * 4;
	//state += id ;

	uint s0, s1, s2, s3, t0, t1, t2, t3;
	uint4 s, t, o0, o8, o16, o24;

	s0 = state[0];
	s1 = state[1];
	s2 = state[2];
	s3 = state[3];

	//s = state[id] ^ roundkeys[0];

	s = (uint4) (s0 ^ roundkeys[0], s1 ^ roundkeys[1], s2 ^ roundkeys[2], s3 ^ roundkeys[3]);
	o0  = (s >>  0) & 0xff;
	o8  = (s >>  8) & 0xff;
	o16 = (s >> 16) & 0xff;
	o24 = (s >> 24) & 0xff;

	roundkeys += 4;
	//roundkeys += 1;

	t0 =
		lTe0[o24.s0] ^
		lTe1[o16.s1] ^
		lTe2[ o8.s2] ^
		lTe3[ o0.s3];
	t1 =
		lTe0[o24.s1] ^
		lTe1[o16.s2] ^
		lTe2[ o8.s3] ^
		lTe3[ o0.s0];
	t2 =
		lTe0[o24.s2] ^
		lTe1[o16.s3] ^
		lTe2[ o8.s0] ^
		lTe3[ o0.s1];
	t3 =
		lTe0[o24.s3] ^
		lTe1[o16.s0] ^
		lTe2[ o8.s1] ^
		lTe3[ o0.s2];

	/*
	t = (uint4) (lTe0[o24.s0], lTe0[o24.s1], lTe0[o24.s2], lTe0[o24.s3]) ^
	    (uint4) (lTe1[o16.s1], lTe1[o16.s2], lTe1[o16.s3], lTe1[o16.s0]) ^
		(uint4) (lTe2[ o8.s2], lTe2[ o8.s3], lTe2[ o8.s0], lTe2[ o8.s1]) ^
		(uint4) (lTe3[ o0.s3], lTe3[ o0.s0], lTe3[ o0.s1], lTe3[ o0.s2]);
	*/

	s = (uint4) (t0 ^ roundkeys[0], t1 ^ roundkeys[1], t2 ^	roundkeys[2], t3 ^ roundkeys[3]);
	o0  = (s >>  0) & 0xff;
	o8  = (s >>  8) & 0xff;
	o16 = (s >> 16) & 0xff;
	o24 = (s >> 24) & 0xff;

	roundkeys += 4;

	for (uint i = 0; i < 6; i++)
	{
		s0 =
			lTe0[o24.s0] ^
			lTe1[o16.s1] ^
			lTe2[ o8.s2] ^
			lTe3[ o0.s3];

		s1 =
			lTe0[o24.s1] ^
			lTe1[o16.s2] ^
			lTe2[ o8.s3] ^
			lTe3[ o0.s0];
		s2 =
			lTe0[o24.s2] ^
			lTe1[o16.s3] ^
			lTe2[ o8.s0] ^
			lTe3[ o0.s1];
		s3 =
			lTe0[o24.s3] ^
			lTe1[o16.s0] ^
			lTe2[ o8.s1] ^
			lTe3[ o0.s2];

		s = (uint4) (s0 ^ roundkeys[0], s1 ^ roundkeys[1], s2 ^ roundkeys[2], s3 ^ roundkeys[3]);
		o0  = (s >>  0) & 0xff;
		o8  = (s >>  8) & 0xff;
		o16 = (s >> 16) & 0xff;
		o24 = (s >> 24) & 0xff;

		roundkeys += 4;

		t0 =
			lTe0[o24.s0] ^
			lTe1[o16.s1] ^
			lTe2[ o8.s2] ^
			lTe3[ o0.s3];
		t1 =
			lTe0[o24.s1] ^
			lTe1[o16.s2] ^
			lTe2[ o8.s3] ^
			lTe3[ o0.s0];
		t2 =
			lTe0[o24.s2] ^
			lTe1[o16.s3] ^
			lTe2[ o8.s0] ^
			lTe3[ o0.s1];
		t3 =
			lTe0[o24.s3] ^
			lTe1[o16.s0] ^
			lTe2[ o8.s1] ^
			lTe3[ o0.s2];

		s = (uint4) (t0 ^ roundkeys[0], t1 ^ roundkeys[1], t2 ^	roundkeys[2], t3 ^ roundkeys[3]);
		o0  = (s >>  0) & 0xff;
		o8  = (s >>  8) & 0xff;
		o16 = (s >> 16) & 0xff;
		o24 = (s >> 24) & 0xff;

		roundkeys += 4;
	}

	s0 =
		(lTe4[o24.s0] & 0xff000000) ^
		(lTe4[o16.s1] & 0x00ff0000) ^
		(lTe4[ o8.s2] & 0x0000ff00) ^
		(lTe4[ o0.s3] & 0x000000ff);
	s1 =
		(lTe4[o24.s1] & 0xff000000) ^
		(lTe4[o16.s2] & 0x00ff0000) ^
		(lTe4[ o8.s3] & 0x0000ff00) ^
		(lTe4[ o0.s0] & 0x000000ff);
	s2 =
		(lTe4[o24.s2] & 0xff000000) ^
		(lTe4[o16.s3] & 0x00ff0000) ^
		(lTe4[ o8.s0] & 0x0000ff00) ^
		(lTe4[ o0.s1] & 0x000000ff);
	s3 =
		(lTe4[o24.s3] & 0xff000000) ^
		(lTe4[o16.s0] & 0x00ff0000) ^
		(lTe4[ o8.s1] & 0x0000ff00) ^
		(lTe4[ o0.s2] & 0x000000ff);

	s0 ^= roundkeys[0];
	s1 ^= roundkeys[1];
	s2 ^= roundkeys[2];
	s3 ^= roundkeys[3];

	state[0] = s0;
	state[1] = s1;
	state[2] = s2;
	state[3] = s3;
}

#endif