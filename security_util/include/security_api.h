
#ifndef SECURITY_UTIL_INCLUDE_SECURITY_API_H_
#define SECURITY_UTIL_INCLUDE_SECURITY_API_H_

#include "security_structs.h"

/*create digital signature from auth_params fields and add to the buffer*/
int32_t create_HLS_authentication(hls_auth_params_t *auth_params,
							uint8_t *buffer,
							uint32_t *buffer_size,
							uint8_t is_originator);

/*verify digital signature using auth_params fields*/
int32_t verify_HLS_authentication(hls_auth_params_t *auth_params,
							uint8_t *buffer,
							uint32_t buffer_size,
							uint8_t is_originator);

/* calculate size of output_buffer for authenticated encryption*/
void auth_encrypt_get_output_size(kdf_params_t *params,
						uint32_t plain_buffer_size,
						uint32_t *cipher_buffer_size);

/*
 * authenticated encryption of input plain_buffer
 * output buffer allocated and freed by user - outside of the function
 */
uint8_t auth_encrypt(kdf_params_t *params,
						uint8_t *plain_buffer,
						uint32_t plain_buffer_size,
						uint8_t *cipher_buffer,
						uint32_t *cipher_buffer_size);


/* calculate size of output_buffer for authenticated decryption*/
void auth_decrypt_get_output_size(kdf_params_t *params,
						uint32_t cipher_buffer_size,
						uint32_t *plain_buffer_size);

/*
 * authenticated decryption of input cipher_buffer
 * output buffer allocated and freed by user - outside of the function
 */
uint8_t auth_decrypt(kdf_params_t *params,
						uint8_t *cipher_buffer,
						uint32_t cipher_buffer_size,
						uint8_t *plain_buffer,
						uint32_t *plain_buffer_size);

#endif /* SECURITY_UTIL_INCLUDE_SECURITY_API_H_ */
