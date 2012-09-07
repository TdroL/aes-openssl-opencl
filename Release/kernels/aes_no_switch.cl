#ifndef _AES_NO_SWITCH_CL
#define _AES_NO_SWITCH_CL



__kernel void aes_no_switch(
	__global uint4 *state,
	__global uint4 rk[15]
) {
	uint global_id = get_global_id(0);
	uint4 s, t;

	s = state[global_id] ^ rk[0];

	uint rounds = NROUNDS;

	    uint r = rounds >> 1;
	uint4 offset0, offset1, offset2, offset3;
	    for (;;) {
	offset0 = s & 0xff;
	offset1 = (s.yzwx >> 8) & 0xff;
	offset2 = (s.zwxy >> 16) & 0xff;
	offset3 = (s.wxyz >> 24);
	t = (uint4)(Te0[offset0.x], Te0[offset0.y], Te0[offset0.z], Te0[offset0.w]) ^
	(uint4)(Te1[offset1.x], Te1[offset1.y], Te1[offset1.z], Te1[offset1.w]) ^
	(uint4)(Te2[offset2.x], Te2[offset2.y], Te2[offset2.z], Te2[offset2.w]) ^
	(uint4)(Te3[offset3.x], Te3[offset3.y], Te3[offset3.z], Te3[offset3.w]) ^
	rk[1];

	        rk += 2;
	        if (--r == 0) {
	            break;
	        }

	offset0 = t & 0xff;
	offset1 = (t.yzwx >> 8) & 0xff;
	offset2 = (t.zwxy >> 16) & 0xff;
	offset3 = (t.wxyz >> 24);
	s = (uint4)(Te0[offset0.x], Te0[offset0.y], Te0[offset0.z], Te0[offset0.w]) ^
	(uint4)(Te1[offset1.x], Te1[offset1.y], Te1[offset1.z], Te1[offset1.w]) ^
	(uint4)(Te2[offset2.x], Te2[offset2.y], Te2[offset2.z], Te2[offset2.w]) ^
	(uint4)(Te3[offset3.x], Te3[offset3.y], Te3[offset3.z], Te3[offset3.w]) ^
	rk[0];
	    }

	offset0 = (t.zwxy >> 16) & 0xff;
	offset1 = (t.wxyz >> 24);
	offset2 = t & 0xff;
	offset3 = (t.yzwx >> 8) & 0xff;

	s = ((uint4)(Te2[offset2.x], Te2[offset2.y], Te2[offset2.z], Te2[offset2.w]) & 0x000000ff) ^
	((uint4)(Te3[offset3.x], Te3[offset3.y], Te3[offset3.z], Te3[offset3.w]) & 0x0000ff00) ^
	((uint4)(Te0[offset0.x], Te0[offset0.y], Te0[offset0.z], Te0[offset0.w]) & 0x00ff0000) ^
	((uint4)(Te1[offset1.x], Te1[offset1.y], Te1[offset1.z], Te1[offset1.w]) & 0xff000000) ^
	rk[0];

	state[global_id] = s;
}

#endif