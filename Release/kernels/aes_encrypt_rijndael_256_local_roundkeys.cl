#ifndef _AES_ENCRYPT_RIJNDAEL_256_LOCAL_ROUNDKEYS_CL
#define _AES_ENCRYPT_RIJNDAEL_256_LOCAL_ROUNDKEYS_CL

#include "aes_encrypt_tables.cl"
#include "aes_encrypt_rijndael.cl"

__kernel void aes_encrypt_rijndael_256_local_roundkeys(
	__global uint *state,
	__global uint *roundkeys
) {
	const uint id = get_global_id(0);
	const uint local_id = get_local_id(0);

	__local uint lroundkeys[256/8];
	uint rk = 0;

	((uchar *) lroundkeys)[local_id] = ((uchar *) roundkeys)[local_id];

	barrier(CLK_LOCAL_MEM_FENCE);

	state += id * 16;

	uint s0, s1, s2, s3, t0, t1, t2, t3;
	uint nrounds = 14;

	s0 = GETU32(state     ) ^ lroundkeys[rk+0];
	s1 = GETU32(state +  4) ^ lroundkeys[rk+1];
	s2 = GETU32(state +  8) ^ lroundkeys[rk+2];
	s3 = GETU32(state + 12) ^ lroundkeys[rk+3];

	uint r = nrounds >> 1;
	for (;;)
	{
		t0 =
			Te0[(s0 >> 24)       ] ^
			Te1[(s1 >> 16) & 0xff] ^
			Te2[(s2 >>  8) & 0xff] ^
			Te3[(s3      ) & 0xff] ^
			lroundkeys[rk+4];
		t1 =
			Te0[(s1 >> 24)       ] ^
			Te1[(s2 >> 16) & 0xff] ^
			Te2[(s3 >>  8) & 0xff] ^
			Te3[(s0      ) & 0xff] ^
			lroundkeys[rk+5];
		t2 =
			Te0[(s2 >> 24)       ] ^
			Te1[(s3 >> 16) & 0xff] ^
			Te2[(s0 >>  8) & 0xff] ^
			Te3[(s1      ) & 0xff] ^
			lroundkeys[rk+6];
		t3 =
			Te0[(s3 >> 24)       ] ^
			Te1[(s0 >> 16) & 0xff] ^
			Te2[(s1 >>  8) & 0xff] ^
			Te3[(s2      ) & 0xff] ^
			lroundkeys[rk+7];

		rk += 8;
		if (--r == 0)
					break;
		s0 =
			Te0[(t0 >> 24)       ] ^
			Te1[(t1 >> 16) & 0xff] ^
			Te2[(t2 >>  8) & 0xff] ^
			Te3[(t3      ) & 0xff] ^
			lroundkeys[rk+0];
		s1 =
			Te0[(t1 >> 24)       ] ^
			Te1[(t2 >> 16) & 0xff] ^
			Te2[(t3 >>  8) & 0xff] ^
			Te3[(t0      ) & 0xff] ^
			lroundkeys[rk+1];
		s2 =
			Te0[(t2 >> 24)       ] ^
			Te1[(t3 >> 16) & 0xff] ^
			Te2[(t0 >>  8) & 0xff] ^
			Te3[(t1      ) & 0xff] ^
			lroundkeys[rk+2];
		s3 =
			Te0[(t3 >> 24)       ] ^
			Te1[(t0 >> 16) & 0xff] ^
			Te2[(t1 >>  8) & 0xff] ^
			Te3[(t2      ) & 0xff] ^
			lroundkeys[rk+3];
	}

	s0 =
		(Te4[(t0 >> 24)       ] & 0xff000000) ^
		(Te4[(t1 >> 16) & 0xff] & 0x00ff0000) ^
		(Te4[(t2 >>  8) & 0xff] & 0x0000ff00) ^
		(Te4[(t3      ) & 0xff] & 0x000000ff) ^
		lroundkeys[rk+0];
	s1 =
		(Te4[(t1 >> 24)       ] & 0xff000000) ^
		(Te4[(t2 >> 16) & 0xff] & 0x00ff0000) ^
		(Te4[(t3 >>  8) & 0xff] & 0x0000ff00) ^
		(Te4[(t0      ) & 0xff] & 0x000000ff) ^
		lroundkeys[rk+1];
	s2 =
		(Te4[(t2 >> 24)       ] & 0xff000000) ^
		(Te4[(t3 >> 16) & 0xff] & 0x00ff0000) ^
		(Te4[(t0 >>  8) & 0xff] & 0x0000ff00) ^
		(Te4[(t1      ) & 0xff] & 0x000000ff) ^
		lroundkeys[rk+2];
	s3 =
		(Te4[(t3 >> 24)       ] & 0xff000000) ^
		(Te4[(t0 >> 16) & 0xff] & 0x00ff0000) ^
		(Te4[(t1 >>  8) & 0xff] & 0x0000ff00) ^
		(Te4[(t2      ) & 0xff] & 0x000000ff) ^
		lroundkeys[rk+3];

	PUTU32(state     , s0);
	PUTU32(state +  4, s1);
	PUTU32(state +  8, s2);
	PUTU32(state + 12, s3);
}

#endif