#ifndef _AES_ENCRYPT_RIJNDAEL_256_OPT_LOCAL_ROUNDKEYS_CL
#define _AES_ENCRYPT_RIJNDAEL_256_OPT_LOCAL_ROUNDKEYS_CL

#include "aes_encrypt_tables.cl"
#include "aes_encrypt_rijndael.cl"

__kernel void aes_encrypt_rijndael_256_opt_local_roundkeys(
	__global uint *state,
	__global uint *roundkeys
) {
	const uint id = get_global_id(0);
	const uint local_id = get_local_id(0);

	__local uint lroundkeys[256/8];
	uint rk = 0;

	((uchar *) lroundkeys)[local_id] = ((uchar *) roundkeys)[local_id];

	barrier(CLK_LOCAL_MEM_FENCE);

	state += id * 4;

	uint s0, s1, s2, s3, t0, t1, t2, t3;
	uint4 s, t, o0, o8, o16, o24;

	s0 = state[0];
	s1 = state[1];
	s2 = state[2];
	s3 = state[3];

	state[0] = s0;
	state[1] = s1;
	state[2] = s2;
	state[3] = s3;

	s0 ^= lroundkeys[rk+0];
	s1 ^= lroundkeys[rk+1];
	s2 ^= lroundkeys[rk+2];
	s3 ^= lroundkeys[rk+3];

	s = (uint4) (s0, s1, s2, s3);
	o0  = (s >>  0) & 0xff;
	o8  = (s >>  8) & 0xff;
	o16 = (s >> 16) & 0xff;
	o24 = (s >> 24) & 0xff;

	t0 =
		Te0[o24.s0] ^
		Te1[o16.s1] ^
		Te2[ o8.s2] ^
		Te3[ o0.s3];
	t1 =
		Te0[o24.s1] ^
		Te1[o16.s2] ^
		Te2[ o8.s3] ^
		Te3[ o0.s0];
	t2 =
		Te0[o24.s2] ^
		Te1[o16.s3] ^
		Te2[ o8.s0] ^
		Te3[ o0.s1];
	t3 =
		Te0[o24.s3] ^
		Te1[o16.s0] ^
		Te2[ o8.s1] ^
		Te3[ o0.s2];

	t0 ^= lroundkeys[rk+4];
	t1 ^= lroundkeys[rk+5];
	t2 ^= lroundkeys[rk+6];
	t3 ^= lroundkeys[rk+7];

	s = (uint4) (t0, t1, t2, t3);
	o0  = (s >>  0) & 0xff;
	o8  = (s >>  8) & 0xff;
	o16 = (s >> 16) & 0xff;
	o24 = (s >> 24) & 0xff;

	for (uint i = 0; i < 6; i++)
	{
		s0 =
			Te0[o24.s0] ^
			Te1[o16.s1] ^
			Te2[ o8.s2] ^
			Te3[ o0.s3];
		s1 =
			Te0[o24.s1] ^
			Te1[o16.s2] ^
			Te2[ o8.s3] ^
			Te3[ o0.s0];
		s2 =
			Te0[o24.s2] ^
			Te1[o16.s3] ^
			Te2[ o8.s0] ^
			Te3[ o0.s1];
		s3 =
			Te0[o24.s3] ^
			Te1[o16.s0] ^
			Te2[ o8.s1] ^
			Te3[ o0.s2];

		s0 ^= lroundkeys[rk+0];
		s1 ^= lroundkeys[rk+1];
		s2 ^= lroundkeys[rk+2];
		s3 ^= lroundkeys[rk+3];

		s = (uint4) (s0, s1, s2, s3);
		o0  = (s >>  0) & 0xff;
		o8  = (s >>  8) & 0xff;
		o16 = (s >> 16) & 0xff;
		o24 = (s >> 24) & 0xff;

		t0 =
			Te0[o24.s0] ^
			Te1[o16.s1] ^
			Te2[ o8.s2] ^
			Te3[ o0.s3];
		t1 =
			Te0[o24.s1] ^
			Te1[o16.s2] ^
			Te2[ o8.s3] ^
			Te3[ o0.s0];
		t2 =
			Te0[o24.s2] ^
			Te1[o16.s3] ^
			Te2[ o8.s0] ^
			Te3[ o0.s1];
		t3 =
			Te0[o24.s3] ^
			Te1[o16.s0] ^
			Te2[ o8.s1] ^
			Te3[ o0.s2];

		t0 ^= lroundkeys[rk+4];
		t1 ^= lroundkeys[rk+5];
		t2 ^= lroundkeys[rk+6];
		t3 ^= lroundkeys[rk+7];

		s = (uint4) (t0, t1, t2, t3);
		o0  = (s >>  0) & 0xff;
		o8  = (s >>  8) & 0xff;
		o16 = (s >> 16) & 0xff;
		o24 = (s >> 24) & 0xff;

		rk += 8;
	}

	s0 =
		(Te4[o24.s0] & 0xff000000) ^
		(Te4[o16.s1] & 0x00ff0000) ^
		(Te4[ o8.s2] & 0x0000ff00) ^
		(Te4[ o0.s3] & 0x000000ff);
	s1 =
		(Te4[o24.s1] & 0xff000000) ^
		(Te4[o16.s2] & 0x00ff0000) ^
		(Te4[ o8.s3] & 0x0000ff00) ^
		(Te4[ o0.s0] & 0x000000ff);
	s2 =
		(Te4[o24.s2] & 0xff000000) ^
		(Te4[o16.s3] & 0x00ff0000) ^
		(Te4[ o8.s0] & 0x0000ff00) ^
		(Te4[ o0.s1] & 0x000000ff);
	s3 =
		(Te4[o24.s3] & 0xff000000) ^
		(Te4[o16.s0] & 0x00ff0000) ^
		(Te4[ o8.s1] & 0x0000ff00) ^
		(Te4[ o0.s2] & 0x000000ff);

	s0 ^= lroundkeys[rk+0];
	s1 ^= lroundkeys[rk+1];
	s2 ^= lroundkeys[rk+2];
	s3 ^= lroundkeys[rk+3];

	state[0] = s0;
	state[1] = s1;
	state[2] = s2;
	state[3] = s3;
}

#endif