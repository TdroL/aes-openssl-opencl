#ifndef _AES_ENCRYPT_VECTOR_CL
#define _AES_ENCRYPT_VECTOR_CL

#define SWITCH_ORDER(s)           \
	((((s) << 24) & 0xff000000) ^ \
	 (((s) <<  8) & 0x00ff0000) ^ \
	 (((s) >>  8) & 0x0000ff00) ^ \
	 (((s) >> 24) & 0x000000ff))

#define GET_OFFSETS(s)     \
	o0 = (s >>  0) & 0xff; \
	o1 = (s >>  8) & 0xff; \
	o2 = (s >> 16) & 0xff; \
	o3 = (s >> 24) & 0xff;

#define AES_ROUND_NAMED(source, target, nround, Te0, Te1, Te2, Te3) \
	GET_OFFSETS(source);                                            \
	target = (uint4) (Te0[o3.x], Te0[o3.y], Te0[o3.z], Te0[o3.w]) ^ \
	         (uint4) (Te1[o2.y], Te1[o2.z], Te1[o2.w], Te1[o2.x]) ^ \
	         (uint4) (Te2[o1.z], Te2[o1.w], Te2[o1.x], Te2[o1.y]) ^ \
	         (uint4) (Te3[o0.w], Te3[o0.x], Te3[o0.y], Te3[o0.z]) ^ \
	         roundkeys[nround];

#define AES_ROUND(source, target, nround)                           \
	AES_ROUND_NAMED(source, target, nround, Te0, Te1, Te2, Te3)

#endif