__kernel void main(
	__global char *buffer,
	__global char *roundkeys,
	const uint limit
) {
    const uint i = get_global_id(0);

	buffer[i] = 2;

	if (i < limit) {
		buffer[i] = 1;
	}
}