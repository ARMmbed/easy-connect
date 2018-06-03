
#ifndef SECURITY_UTIL_INCLUDE_SECURITY_API_H_
#define SECURITY_UTIL_INCLUDE_SECURITY_API_H_

#include "security_structs.h"

/*
 * Extract public key (x, y) from buffer
 * public_key_x and public_key_y should be pre-allocated before
 * and have at least buffer_size/2 size
 */
void extract_public_key(uint8_t *buffer, uint32_t buffer_size,
			char	*public_key_x, char	*public_key_y,
			uint32_t	*public_key_size);
/*
 * Get pre-saved DS public key (x, y)
 * public_key_x and public_key_y should be pre-allocated (32 bytes)
 * return 0 if the key exists
 * return 1 if not
 */
uint8_t get_presaved_DS_public_key(char	*public_key_x, char	*public_key_y,
				uint32_t	*public_key_size);

/*
 * Get pre-saved KA public key (x, y)
 * public_key_x and public_key_y should be pre-allocated (32 bytes)
 * return 0 if the key exists
 * return 1 if not
 */
uint8_t get_presaved_KA_public_key(char	*public_key_x, char	*public_key_y,
					uint32_t	*public_key_size);

/*
 * Create digital signature from auth_params fields and add to the buffer
 * currently supported only Security Suite 1 && MechanismId 7
 */
int32_t create_HLS_authentication(hls_auth_params_t *auth_params,
							uint8_t *buffer,
							uint32_t *buffer_size,
							uint8_t is_originator,
							get_private_key func);

/*
 * Verify digital signature using auth_params fields
 * currently supported only Security Suite 1 && MechanismId 7
 */
int32_t verify_HLS_authentication(hls_auth_params_t *auth_params,
							uint8_t *buffer,
							uint32_t buffer_size,
							uint8_t is_originator,
							get_private_key func);

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
