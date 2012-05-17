#ifndef _AES_ENCRYPT_RIJNDAEL_128_CL
#define _AES_ENCRYPT_RIJNDAEL_128_CL

#include "aes_encrypt_tables.cl"
#include "aes_encrypt_rijndael.cl"

__kernel void aes_encrypt_rijndael_128(
	__global uchar *state,
	__global const uint *roundkeys
) {
	const uint id = get_global_id(0);

	state += id * 16;

	uint s0, s1, s2, s3, t0, t1, t2, t3;
	uint nrounds = 10;

	s0 = GETU32(state     ) ^ roundkeys[0];
	s1 = GETU32(state +  4) ^ roundkeys[1];
	s2 = GETU32(state +  8) ^ roundkeys[2];
	s3 = GETU32(state + 12) ^ roundkeys[3];

	#ifdef FULL_UNROLL

		t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ roundkeys[ 4];
		t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ roundkeys[ 5];
		t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ roundkeys[ 6];
		t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ roundkeys[ 7];

		s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >>  8) & 0xff] ^ Te3[t3 & 0xff] ^ roundkeys[ 8];
		s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >>  8) & 0xff] ^ Te3[t0 & 0xff] ^ roundkeys[ 9];
		s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >>  8) & 0xff] ^ Te3[t1 & 0xff] ^ roundkeys[10];
		s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >>  8) & 0xff] ^ Te3[t2 & 0xff] ^ roundkeys[11];

		t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ roundkeys[12];
		t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ roundkeys[13];
		t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ roundkeys[14];
		t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ roundkeys[15];

		s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >>  8) & 0xff] ^ Te3[t3 & 0xff] ^ roundkeys[16];
		s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >>  8) & 0xff] ^ Te3[t0 & 0xff] ^ roundkeys[17];
		s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >>  8) & 0xff] ^ Te3[t1 & 0xff] ^ roundkeys[18];
		s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >>  8) & 0xff] ^ Te3[t2 & 0xff] ^ roundkeys[19];

		t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ roundkeys[20];
		t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ roundkeys[21];
		t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ roundkeys[22];
		t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ roundkeys[23];

		s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >>  8) & 0xff] ^ Te3[t3 & 0xff] ^ roundkeys[24];
		s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >>  8) & 0xff] ^ Te3[t0 & 0xff] ^ roundkeys[25];
		s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >>  8) & 0xff] ^ Te3[t1 & 0xff] ^ roundkeys[26];
		s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >>  8) & 0xff] ^ Te3[t2 & 0xff] ^ roundkeys[27];

		t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ roundkeys[28];
		t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ roundkeys[29];
		t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ roundkeys[30];
		t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ roundkeys[31];

		s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >>  8) & 0xff] ^ Te3[t3 & 0xff] ^ roundkeys[32];
		s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >>  8) & 0xff] ^ Te3[t0 & 0xff] ^ roundkeys[33];
		s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >>  8) & 0xff] ^ Te3[t1 & 0xff] ^ roundkeys[34];
		s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >>  8) & 0xff] ^ Te3[t2 & 0xff] ^ roundkeys[35];

		t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ roundkeys[36];
		t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ roundkeys[37];
		t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ roundkeys[38];
		t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ roundkeys[39];
		if (nrounds > 10)
		{
			s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >>  8) & 0xff] ^ Te3[t3 & 0xff] ^ roundkeys[40];
			s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >>  8) & 0xff] ^ Te3[t0 & 0xff] ^ roundkeys[41];
			s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >>  8) & 0xff] ^ Te3[t1 & 0xff] ^ roundkeys[42];
			s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >>  8) & 0xff] ^ Te3[t2 & 0xff] ^ roundkeys[43];

			t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ roundkeys[44];
			t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ roundkeys[45];
			t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ roundkeys[46];
			t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ roundkeys[47];
			if (nrounds > 12)
			{
				s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >>  8) & 0xff] ^ Te3[t3 & 0xff] ^ roundkeys[48];
				s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >>  8) & 0xff] ^ Te3[t0 & 0xff] ^ roundkeys[49];
				s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >>  8) & 0xff] ^ Te3[t1 & 0xff] ^ roundkeys[50];
				s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >>  8) & 0xff] ^ Te3[t2 & 0xff] ^ roundkeys[51];

				t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ roundkeys[52];
				t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ roundkeys[53];
				t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ roundkeys[54];
				t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ roundkeys[55];
			}
		}
		roundkeys += nrounds << 2;
	#else
		uint r = nrounds >> 1;
		for (;;)
		{
			t0 =
				Te0[(s0 >> 24)       ] ^
				Te1[(s1 >> 16) & 0xff] ^
				Te2[(s2 >>  8) & 0xff] ^
				Te3[(s3      ) & 0xff] ^
				roundkeys[4];
			t1 =
				Te0[(s1 >> 24)       ] ^
				Te1[(s2 >> 16) & 0xff] ^
				Te2[(s3 >>  8) & 0xff] ^
				Te3[(s0      ) & 0xff] ^
				roundkeys[5];
			t2 =
				Te0[(s2 >> 24)       ] ^
				Te1[(s3 >> 16) & 0xff] ^
				Te2[(s0 >>  8) & 0xff] ^
				Te3[(s1      ) & 0xff] ^
				roundkeys[6];
			t3 =
				Te0[(s3 >> 24)       ] ^
				Te1[(s0 >> 16) & 0xff] ^
				Te2[(s1 >>  8) & 0xff] ^
				Te3[(s2      ) & 0xff] ^
				roundkeys[7];
				roundkeys += 8;
				if (--r == 0)
						break;
			s0 =
				Te0[(t0 >> 24)       ] ^
				Te1[(t1 >> 16) & 0xff] ^
				Te2[(t2 >>  8) & 0xff] ^
				Te3[(t3      ) & 0xff] ^
				roundkeys[0];
			s1 =
				Te0[(t1 >> 24)       ] ^
				Te1[(t2 >> 16) & 0xff] ^
				Te2[(t3 >>  8) & 0xff] ^
				Te3[(t0      ) & 0xff] ^
				roundkeys[1];
			s2 =
				Te0[(t2 >> 24)       ] ^
				Te1[(t3 >> 16) & 0xff] ^
				Te2[(t0 >>  8) & 0xff] ^
				Te3[(t1      ) & 0xff] ^
				roundkeys[2];
			s3 =
				Te0[(t3 >> 24)       ] ^
				Te1[(t0 >> 16) & 0xff] ^
				Te2[(t1 >>  8) & 0xff] ^
				Te3[(t2      ) & 0xff] ^
				roundkeys[3];
		}
	#endif

	s0 =
		(Te4[(t0 >> 24)       ] & 0xff000000) ^
		(Te4[(t1 >> 16) & 0xff] & 0x00ff0000) ^
		(Te4[(t2 >>  8) & 0xff] & 0x0000ff00) ^
		(Te4[(t3      ) & 0xff] & 0x000000ff) ^
		roundkeys[0];
	s1 =
		(Te4[(t1 >> 24)       ] & 0xff000000) ^
		(Te4[(t2 >> 16) & 0xff] & 0x00ff0000) ^
		(Te4[(t3 >>  8) & 0xff] & 0x0000ff00) ^
		(Te4[(t0      ) & 0xff] & 0x000000ff) ^
		roundkeys[1];
	s2 =
		(Te4[(t2 >> 24)       ] & 0xff000000) ^
		(Te4[(t3 >> 16) & 0xff] & 0x00ff0000) ^
		(Te4[(t0 >>  8) & 0xff] & 0x0000ff00) ^
		(Te4[(t1      ) & 0xff] & 0x000000ff) ^
		roundkeys[2];
	s3 =
		(Te4[(t3 >> 24)       ] & 0xff000000) ^
		(Te4[(t0 >> 16) & 0xff] & 0x00ff0000) ^
		(Te4[(t1 >>  8) & 0xff] & 0x0000ff00) ^
		(Te4[(t2      ) & 0xff] & 0x000000ff) ^
		roundkeys[3];

	PUTU32(state     , s0);
	PUTU32(state +  4, s1);
	PUTU32(state +  8, s2);
	PUTU32(state + 12, s3);
}

#endif