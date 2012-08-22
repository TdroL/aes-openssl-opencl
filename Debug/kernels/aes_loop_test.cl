#ifndef _AES_LOOP_TEST_CL
#define _AES_LOOP_TEST_CL



__kernel void aes_loop_test(
	__global uint4 *state,
	__constant uint4 *roundkeys,
	const uint nrounds
) {
	#ifdef NROUNDS
	uint r = NROUNDS;
	#else
	uint r = nrounds;
	#endif

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
// #ifdef NROUNDS
// 	#if NROUNDS > 10
// 	AES_ROUND(t, s, 10);
// 	AES_ROUND(s, t, 11);
// 	#endif
// 	#if NROUNDS > 12
// 	AES_ROUND(t, s, 12);
// 	AES_ROUND(s, t, 13);
// 	#endif
// #else
	if (nrounds > 10)
	{
		AES_ROUND(t, s, 10);
		AES_ROUND(s, t, 11);

		if (nrounds > 12)
		{
			AES_ROUND(t, s, 12);
			AES_ROUND(s, t, 13);
		}
	}
// #endif

	// final round
	GET_OFFSETS(t);
	s = (uint4) (Te4[o3.x] & 0xff000000, Te4[o3.y] & 0xff000000, Te4[o3.z] & 0xff000000, Te4[o3.w] & 0xff000000) ^
	    (uint4) (Te4[o2.y] & 0x00ff0000, Te4[o2.z] & 0x00ff0000, Te4[o2.w] & 0x00ff0000, Te4[o2.x] & 0x00ff0000) ^
	    (uint4) (Te4[o1.z] & 0x0000ff00, Te4[o1.w] & 0x0000ff00, Te4[o1.x] & 0x0000ff00, Te4[o1.y] & 0x0000ff00) ^
	    (uint4) (Te4[o0.w] & 0x000000ff, Te4[o0.x] & 0x000000ff, Te4[o0.y] & 0x000000ff, Te4[o0.z] & 0x000000ff) ^
	    roundkeys[r];

	state[id] = SWITCH_ORDER(s);
}

#endif