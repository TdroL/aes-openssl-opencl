#ifndef _AES_OPT_LOCAL_TABLES_CL
#define _AES_OPT_LOCAL_TABLES_CL



__kernel void aes_opt_local_tables(
	__global uint *state,
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

	state += id * 4;

	uint s0, s1, s2, s3, t0, t1, t2, t3;
	uint4 s, t, o0, o8, o16, o24, vs;

	s0 = state[0]; //SWITCH_ORDER(state[0]);
	s1 = state[1]; //SWITCH_ORDER(state[1]);
	s2 = state[2]; //SWITCH_ORDER(state[2]);
	s3 = state[3]; //SWITCH_ORDER(state[3]);

	VECTOR_SWITCH_ORDER(vs);

	s0 = vs.s0 ^ roundkeys[0];
	s1 = vs.s1 ^ roundkeys[1];
	s2 = vs.s2 ^ roundkeys[2];
	s3 = vs.s3 ^ roundkeys[3];

	s = (uint4) (s0, s1, s2, s3);
	o0  = (s >>  0) & 0xff;
	o8  = (s >>  8) & 0xff;
	o16 = (s >> 16) & 0xff;
	o24 = (s >> 24) & 0xff;

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

	t0 ^= roundkeys[4];
	t1 ^= roundkeys[5];
	t2 ^= roundkeys[6];
	t3 ^= roundkeys[7];

	s = (uint4) (t0, t1, t2, t3);
	o0  = (s >>  0) & 0xff;
	o8  = (s >>  8) & 0xff;
	o16 = (s >> 16) & 0xff;
	o24 = (s >> 24) & 0xff;

	uint nrounds = NROUNDS;
	for (uint i = 0, r = nrounds / 2; i < r; i++)
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

		s0 ^= roundkeys[0];
		s1 ^= roundkeys[1];
		s2 ^= roundkeys[2];
		s3 ^= roundkeys[3];

		s = (uint4) (s0, s1, s2, s3);
		o0  = (s >>  0) & 0xff;
		o8  = (s >>  8) & 0xff;
		o16 = (s >> 16) & 0xff;
		o24 = (s >> 24) & 0xff;

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

		t0 ^= roundkeys[4];
		t1 ^= roundkeys[5];
		t2 ^= roundkeys[6];
		t3 ^= roundkeys[7];

		s = (uint4) (t0, t1, t2, t3);
		o0  = (s >>  0) & 0xff;
		o8  = (s >>  8) & 0xff;
		o16 = (s >> 16) & 0xff;
		o24 = (s >> 24) & 0xff;

		roundkeys += 8;
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

	VECTOR_SWITCH_ORDER(vs);

	state[0] = vs.s0;//SWITCH_ORDER(s0);
	state[1] = vs.s1;//SWITCH_ORDER(s1);
	state[2] = vs.s2;//SWITCH_ORDER(s2);
	state[3] = vs.s3;//SWITCH_ORDER(s3);
}

#endif
