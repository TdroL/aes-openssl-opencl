#include "kernels/aes_encrypt_rijndael_128.cl"
#include "kernels/aes_encrypt_rijndael_192.cl"
#include "kernels/aes_encrypt_rijndael_256.cl"

#include "kernels/aes_encrypt_rijndael_256_constant_roundkeys.cl"
#include "kernels/aes_encrypt_rijndael_256_local_roundkeys.cl"

#include "kernels/aes_encrypt_rijndael_256_opt.cl"
#include "kernels/aes_encrypt_rijndael_256_opt_constant_roundkeys.cl"
#include "kernels/aes_encrypt_rijndael_256_opt_local_roundkeys.cl"
#include "kernels/aes_encrypt_rijndael_256_local.cl"

#include "kernels/aes_encrypt_vector_128.cl"
#include "kernels/aes_encrypt_vector_192.cl"
#include "kernels/aes_encrypt_vector_256.cl"

#include "kernels/aes_encrypt_vector_128_local.cl"
#include "kernels/aes_encrypt_vector_192_local.cl"
#include "kernels/aes_encrypt_vector_256_local.cl"