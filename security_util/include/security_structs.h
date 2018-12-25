
#ifndef SECURITY_UTIL_INCLUDE_SECURITY_STRUCTS_H_
#define SECURITY_UTIL_INCLUDE_SECURITY_STRUCTS_H_

#include <inttypes.h>

#define SECURITY_UTIL_STATUS_SUCCESS 0
#define SECURITY_UTIL_STATUS_FAILURE 1

#define PUBLIC_KEY_SIZE		64
#define PRIVATE_KEY_SIZE	32
#define MAX_Z_SIZE			512
#define AUTH_TAG_SIZE		12
#define IV_SIZE				12
#define HASH_SIZE			48

#define TRANSACTION_SIZE	8

typedef enum e_security_group_id {
	security_group_id_ecp_dp_256r1 = 0,  /* MBEDTLS_ECP_DP_SECP256R1 */
	security_group_id_last = 1
} e_security_group_id_t;

typedef enum e_key_sharing_status {
	key_sharing_status_failure = 0,
	key_sharing_status_success = 1
} e_key_sharing_status_t;

typedef enum e_security_mode {
	security_mode_none = 0,			// none
	security_mode_auth = 1,			// Authentication only
	security_mode_encrypt = 2,		// Encryption only
	security_mode_auth_encrypt = 3	// Authenticated Encryption
} e_security_mode_t;

typedef struct security_octet_string {
	uint8_t *buf; /* Buffer with consecutive Octet_string bits */
	int32_t size; /* Size of the buffer */
} security_octet_string_t;

typedef struct security_ipv6 {
	uint32_t x1;
	uint32_t x2;
	uint32_t x3;
	uint32_t x4;
} security_ipv6_t;

typedef enum e_security_ip_address_type {
	security_IP_address_not_selected,
	security_IP_address_ipV4,
	security_IP_address_ipV6
} e_security_ip_address_type_t;

typedef struct security_ip_address {
	e_security_ip_address_type_t type;
	union security_ip_address_u {
		uint32_t	 ipV4;
		security_ipv6_t	 ipV6;
	} choice;
} security_ip_address_t;

typedef struct security_suite_params {
	uint8_t			*algorithm_id;
	uint32_t		algorithm_id_size;
	uint32_t		key_data_len;
	uint32_t		hash_len;
} security_suite_params_t;

typedef struct security_key_pair {
	const uint8_t *public_key;
	uint32_t public_key_size;
	const uint8_t *private_key;
	uint32_t private_key_size;
} security_key_pair_t;

typedef struct secured_association_params {
	security_ip_address_t	remote_ip_address;
	uint16_t				remote_port;
	uint16_t				remote_wrapper_port;
	uint16_t				local_wrapper_port;
} secured_association_params_t;

/**
 * Contains certificate significant fields which
 * are needed for the certificate usage
 */
typedef struct security_key_crt {
	uint8_t	is_initialized;
	/* Start time of certificate validity */
	uint8_t	*pValid_from;
	/* End time of certificate validity */
	uint8_t	*pValid_to;
	/* Container for the public key */
	uint8_t	*Q;
}
security_key_crt;

typedef struct session_secure_params {
	secured_association_params_t	id;
	e_key_sharing_status_t	ds_key_sharing_status;
	e_key_sharing_status_t	ka_key_sharing_status;
	security_key_crt		ka_crt_remote;
	security_key_crt		ds_crt_remote;
	uint8_t			shared_secret[PRIVATE_KEY_SIZE];
	uint32_t		shared_secret_size;
} session_secure_params_t;

/* internal parameters for Digital Signature sign/verify
 * and key operations
 */
typedef struct security_int_params {
	e_security_group_id_t	security_grp_id;
	void *remote_ecp_public_key;
	const uint8_t *local_private_key;
	uint32_t local_private_key_size;
} security_int_params_t;

/* HLS authentication Parameters */
typedef struct hls_auth_params {
	uint8_t			security_suite;
	uint8_t			mechanism_id;
	security_octet_string_t	originator_sys_title;
	security_octet_string_t	originator_challenge;
	security_octet_string_t	recipient_sys_title;
	security_octet_string_t	recipient_challenge;
} hls_auth_params_t;

/* Encrypt/Decrypt Parameters */
typedef struct crypto_params {
	uint8_t				security_suite;
	e_security_mode_t	security_mode;
	uint8_t				*invocation_counter;
	security_octet_string_t		originator_sys_title;
	security_octet_string_t		transaction_id;
	security_octet_string_t		recipient_sys_title;
} crypto_params_t;

/* Encrypt/Decrypt internal Parameters */
typedef struct crypto_int_params {
	const uint8_t	*session_key;
	uint32_t		key_size;
	const uint8_t	*iv;
	uint32_t		iv_size;
	const uint8_t	*add_data;
	uint32_t		add_size;
} crypto_int_params_t;

/* Key Derivation Parameters */
typedef struct kdf_params {
	security_octet_string_t	algorithm_id;
	security_octet_string_t	*originator_sys_title;
	security_octet_string_t	*transaction_id;
	security_octet_string_t	*recipient_sys_title;
} kdf_params_t;

#endif /* SECURITY_UTIL_INCLUDE_SECURITY_STRUCTS_H_ */
