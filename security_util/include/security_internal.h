
#ifndef SECURITY_UTIL_INCLUDE_SECURITY_INTERNAL_H_
#define SECURITY_UTIL_INCLUDE_SECURITY_INTERNAL_H_

#include "security_structs.h"

int32_t ECDSA_Sign(ds_int_params_t	*params,
					uint8_t		*buffer,
					uint32_t	*buffer_size,
					uint8_t		*message,
					uint32_t	message_size);

int32_t ECDSA_Verify(ds_int_params_t	*params,
					uint8_t		*buffer,
					uint32_t	buffer_size,
					uint8_t		*message,
					uint32_t	message_size);

void ECDH_compute_Z(shared_secret_params_t *params);

void NIST_KDF(kdf_params_t *params);

#endif /* SECURITY_UTIL_INCLUDE_SECURITY_INTERNAL_H_ */
