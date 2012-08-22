#ifndef _AES_VECTOR_LOCAL_TABLES_CL
#define _AES_VECTOR_LOCAL_TABLES_CL



__kernel void aes_vector_local_tables(
	__global uint4 *state,
	__global uint4 *roundkeys
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

	#define Te0 lTe0
	#define Te1 lTe1
	#define Te2 lTe2
	#define Te3 lTe3
	#define Te4 lTe4

	uint4 s, t, o0, o1, o2, o3, s0, s1, s2, s3;

	s = SWITCH_ORDER(state[id]) ^ roundkeys[0];

#if 0

	AES_ROUND(s, t,  1);
	AES_ROUND(t, s,  2);
	AES_ROUND(s, t,  3);
	AES_ROUND(t, s,  4);
	AES_ROUND(s, t,  5);
	AES_ROUND(t, s,  6);
	AES_ROUND(s, t,  7);
	AES_ROUND(t, s,  8);
	AES_ROUND(s, t,  9);

	if (NROUNDS > 10)
	{
		AES_ROUND(t, s, 10);
		AES_ROUND(s, t, 11);

		if (NROUNDS > 12)
		{
			AES_ROUND(t, s, 12);
			AES_ROUND(s, t, 13);
		}
	}

#else


	uint r = NROUNDS / 2;
	uint rk = 0;

	for (;;) {
		AES_ROUND(s, t, rk + 1);

		rk += 2;
		if (--r == 0) {
			break;
		}

		AES_ROUND(t, s, rk + 0);
	}

#endif

	// final round
	GET_OFFSETS(t);
	s = (((uint4) (Te4[o3.x], Te4[o3.y], Te4[o3.z], Te4[o3.w])) & 0xff000000) ^
	    (((uint4) (Te4[o2.y], Te4[o2.z], Te4[o2.w], Te4[o2.x])) & 0x00ff0000) ^
	    (((uint4) (Te4[o1.z], Te4[o1.w], Te4[o1.x], Te4[o1.y])) & 0x0000ff00) ^
	    (((uint4) (Te4[o0.w], Te4[o0.x], Te4[o0.y], Te4[o0.z])) & 0x000000ff) ^
	    roundkeys[NROUNDS];

	state[id] = SWITCH_ORDER(s);

	#undef Te0
	#undef Te1
	#undef Te2
	#undef Te3
	#undef Te4
}

#endif