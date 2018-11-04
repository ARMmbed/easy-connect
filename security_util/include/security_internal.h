
#ifndef SECURITY_UTIL_INCLUDE_SECURITY_INTERNAL_H_
#define SECURITY_UTIL_INCLUDE_SECURITY_INTERNAL_H_

#include "security_structs.h"

int32_t compute_hash_256(uint8_t *message, uint32_t	message_size,
	uint8_t *hash);
int32_t compute_hash_384(uint8_t *message, uint32_t	message_size,
	uint8_t *hash);

int32_t ECDSA_Sign(security_int_params_t	*params,
					uint8_t		*buffer,
					uint32_t	*buffer_size,
					uint8_t		*message,
					uint32_t	message_size);

int32_t ECDSA_Verify(security_int_params_t	*params,
					uint8_t		*buffer,
					uint32_t	buffer_size,
					uint8_t		*message,
					uint32_t	message_size);

int32_t ECDH_compute_Z(security_int_params_t *params,
	uint8_t *z, uint32_t *z_len);

int32_t cipher_auth_encrypt(crypto_int_params_t *params,
		uint8_t *plain_buffer, uint32_t plain_buffer_size,
		uint8_t *cipher_buffer, uint32_t *cipher_buffer_size);
int32_t cipher_auth_decrypt(crypto_int_params_t *params,
		uint8_t *cipher_buffer, uint32_t cipher_buffer_size,
		uint8_t *plain_buffer, uint32_t *plain_buffer_size);

#endif /* SECURITY_UTIL_INCLUDE_SECURITY_INTERNAL_H_ */
