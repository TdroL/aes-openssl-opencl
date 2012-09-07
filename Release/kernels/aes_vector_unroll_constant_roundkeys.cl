#ifndef _AES_VECTOR_UNROLL_CONSTANT_ROUNDKEYS_CL
#define _AES_VECTOR_UNROLL_CONSTANT_ROUNDKEYS_CL



__kernel void aes_vector_unroll_constant_roundkeys(
	__global uint4 *state,
	__global uint4 *roundkeys
) {
	const uint id = get_global_id(0);

	uint4 s, t, o0, o1, o2, o3;

	s = SWITCH_ORDER(state[id]) ^ roundkeys[0];

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

	// final round
	GET_OFFSETS(t);
	s = (((uint4) (Te4[o3.x], Te4[o3.y], Te4[o3.z], Te4[o3.w])) & 0xff000000) ^
	    (((uint4) (Te4[o2.y], Te4[o2.z], Te4[o2.w], Te4[o2.x])) & 0x00ff0000) ^
	    (((uint4) (Te4[o1.z], Te4[o1.w], Te4[o1.x], Te4[o1.y])) & 0x0000ff00) ^
	    (((uint4) (Te4[o0.w], Te4[o0.x], Te4[o0.y], Te4[o0.z])) & 0x000000ff) ^
	    roundkeys[NROUNDS];

	state[id] = SWITCH_ORDER(s);
}

#endif