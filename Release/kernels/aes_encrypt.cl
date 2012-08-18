#include "kernels/aes_encrypt_rijndael.cl"

#include "kernels/aes_encrypt_rijndael_constant_roundkeys.cl"
#include "kernels/aes_encrypt_rijndael_local_tables.cl"
#include "kernels/aes_encrypt_rijndael_local_roundkeys.cl"

#include "kernels/aes_encrypt_rijndael_opt_constant_roundkeys.cl"
#include "kernels/aes_encrypt_rijndael_opt_local_tables.cl"
#include "kernels/aes_encrypt_rijndael_opt_local_roundkeys.cl"

#include "kernels/aes_encrypt_vector.cl"
#include "kernels/aes_encrypt_vector_local_tables.cl"