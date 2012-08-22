#ifndef _AES_LOCAL_TABLES_CL
#define _AES_LOCAL_TABLES_CL



__kernel void aes_local_tables(
	__global uchar *state,
	__global uint *roundkeys
) {
	const uint id = get_global_id(0);
	const uint lid = get_local_id(0);

	__local uint lTe0[256], lTe1[256], lTe2[256], lTe3[256], lTe4[256];

	lTe0[lid] = Te0[lid];
	lTe1[lid] = Te1[lid];
	lTe2[lid] = Te2[lid];
	lTe3[lid] = Te3[lid];
	lTe4[lid] = Te4[lid];

	barrier(CLK_LOCAL_MEM_FENCE);

	state += id * 16;

	uint s0, s1, s2, s3, t0, t1, t2, t3;

	s0 = GETU32(state     ) ^ roundkeys[0];
	s1 = GETU32(state +  4) ^ roundkeys[1];
	s2 = GETU32(state +  8) ^ roundkeys[2];
	s3 = GETU32(state + 12) ^ roundkeys[3];

	uint nrounds = NROUNDS;
	uint r = nrounds / 2;
	for (;;)
	{
		t0 =
			lTe0[(s0 >> 24)       ] ^
			lTe1[(s1 >> 16) & 0xff] ^
			lTe2[(s2 >>  8) & 0xff] ^
			lTe3[(s3      ) & 0xff] ^
			roundkeys[4];
		t1 =
			lTe0[(s1 >> 24)       ] ^
			lTe1[(s2 >> 16) & 0xff] ^
			lTe2[(s3 >>  8) & 0xff] ^
			lTe3[(s0      ) & 0xff] ^
			roundkeys[5];
		t2 =
			lTe0[(s2 >> 24)       ] ^
			lTe1[(s3 >> 16) & 0xff] ^
			lTe2[(s0 >>  8) & 0xff] ^
			lTe3[(s1      ) & 0xff] ^
			roundkeys[6];
		t3 =
			lTe0[(s3 >> 24)       ] ^
			lTe1[(s0 >> 16) & 0xff] ^
			lTe2[(s1 >>  8) & 0xff] ^
			lTe3[(s2      ) & 0xff] ^
			roundkeys[7];

		roundkeys += 8;
		if (--r == 0)
			break;

		s0 =
			lTe0[(t0 >> 24)       ] ^
			lTe1[(t1 >> 16) & 0xff] ^
			lTe2[(t2 >>  8) & 0xff] ^
			lTe3[(t3      ) & 0xff] ^
			roundkeys[0];
		s1 =
			lTe0[(t1 >> 24)       ] ^
			lTe1[(t2 >> 16) & 0xff] ^
			lTe2[(t3 >>  8) & 0xff] ^
			lTe3[(t0      ) & 0xff] ^
			roundkeys[1];
		s2 =
			lTe0[(t2 >> 24)       ] ^
			lTe1[(t3 >> 16) & 0xff] ^
			lTe2[(t0 >>  8) & 0xff] ^
			lTe3[(t1      ) & 0xff] ^
			roundkeys[2];
		s3 =
			lTe0[(t3 >> 24)       ] ^
			lTe1[(t0 >> 16) & 0xff] ^
			lTe2[(t1 >>  8) & 0xff] ^
			lTe3[(t2      ) & 0xff] ^
			roundkeys[3];
	}

	s0 =
		(lTe4[(t0 >> 24)       ] & 0xff000000) ^
		(lTe4[(t1 >> 16) & 0xff] & 0x00ff0000) ^
		(lTe4[(t2 >>  8) & 0xff] & 0x0000ff00) ^
		(lTe4[(t3      ) & 0xff] & 0x000000ff) ^
		roundkeys[0];
	s1 =
		(lTe4[(t1 >> 24)       ] & 0xff000000) ^
		(lTe4[(t2 >> 16) & 0xff] & 0x00ff0000) ^
		(lTe4[(t3 >>  8) & 0xff] & 0x0000ff00) ^
		(lTe4[(t0      ) & 0xff] & 0x000000ff) ^
		roundkeys[1];
	s2 =
		(lTe4[(t2 >> 24)       ] & 0xff000000) ^
		(lTe4[(t3 >> 16) & 0xff] & 0x00ff0000) ^
		(lTe4[(t0 >>  8) & 0xff] & 0x0000ff00) ^
		(lTe4[(t1      ) & 0xff] & 0x000000ff) ^
		roundkeys[2];
	s3 =
		(lTe4[(t3 >> 24)       ] & 0xff000000) ^
		(lTe4[(t0 >> 16) & 0xff] & 0x00ff0000) ^
		(lTe4[(t1 >>  8) & 0xff] & 0x0000ff00) ^
		(lTe4[(t2      ) & 0xff] & 0x000000ff) ^
		roundkeys[3];

	PUTU32(state     , s0);
	PUTU32(state +  4, s1);
	PUTU32(state +  8, s2);
	PUTU32(state + 12, s3);
}

#endif