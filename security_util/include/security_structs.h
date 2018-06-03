
#ifndef SECURITY_UTIL_INCLUDE_SECURITY_STRUCTS_H_
#define SECURITY_UTIL_INCLUDE_SECURITY_STRUCTS_H_

#include <inttypes.h>

#define SECURITY_UTIL_STATUS_SUCCESS 0
#define SECURITY_UTIL_STATUS_FAILURE 1

typedef const uint8_t *(*get_private_key)(uint32_t *size);

typedef struct octet_string {
	uint8_t *buf; /* Buffer with consecutive Octet_string bits */
	int32_t size; /* Size of the buffer */
} octet_string_t;

typedef enum e_security_group_id {
	security_group_id_ecp_dp_256r1 = 0,  /* MBEDTLS_ECP_DP_SECP256R1 */
	security_group_id_last = 1
} e_security_group_id_t;

/* internal parameters for Digital Signature sign/verify */
typedef struct ds_int_params {
	e_security_group_id_t	security_grp_id;
	const unsigned char	*private_key;
	uint32_t	private_key_size;
	const unsigned char	*public_key;
	uint32_t	public_key_size;
} ds_int_params_t;

/* Shared Secret Calculation Parameters */
typedef struct shared_secret_params {
	e_security_group_id_t	security_grp_id;
	unsigned char	*public_key;
	uint32_t	public_key_size;
} shared_secret_params_t;

/* HLS authentication Parameters */
typedef struct hls_auth_params {
	uint8_t			security_suite;
	uint8_t			mechanism_id;
	octet_string_t	originator_sys_title;
	octet_string_t	originator_challenge;
	octet_string_t	recipient_sys_title;
	octet_string_t	recipient_challenge;
	unsigned char	*public_key;
	uint32_t	public_key_size;
} hls_auth_params_t;

/* Key Derivation Parameters */
typedef struct kdf_params {
	octet_string_t	algorithm_id;
	uint32_t		key_data_len;
	uint32_t		hash_len;
	octet_string_t	originator_sys_title;
	octet_string_t	transaction_id;
	octet_string_t	recipient_sys_title;
} kdf_params_t;


#endif /* SECURITY_UTIL_INCLUDE_SECURITY_STRUCTS_H_ */
