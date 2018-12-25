
#ifndef SECURITY_UTIL_INCLUDE_SECURITY_INTERNAL_H_
#define SECURITY_UTIL_INCLUDE_SECURITY_INTERNAL_H_

#include "security_structs.h"
#include "mbedtls/oid.h"
#include "mbedtls/x509_crt.h"

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

int FindCommonName(mbedtls_x509_name *issuer,
							const uint8_t *pcn,
							uint32_t cn_len);

int32_t init_security_key_crt(
		security_key_crt * p_security_key_crt,
		mbedtls_x509_crt *p_mbed_tls_crt);

void free_security_key_crt(security_key_crt *p_security_key_crt);

#endif /* SECURITY_UTIL_INCLUDE_SECURITY_INTERNAL_H_ */
