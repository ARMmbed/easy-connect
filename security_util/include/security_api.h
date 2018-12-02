
#ifndef SECURITY_UTIL_INCLUDE_SECURITY_API_H_
#define SECURITY_UTIL_INCLUDE_SECURITY_API_H_

#include "security_structs.h"

/*
 * This function should be called each time after key generation
 * NULL can be passed if one of the key pairs shouldn't be changed
 */
int init_security_util(security_key_pair_t *ds_key_pair,
	security_key_pair_t *ka_key_pair,
	uint8_t *ka_certificate, uint32_t ka_certificate_size);
/*
 * This function should be called once at the exit
 */
void exit_security_util(void);

/*
 * Set status of local DS crt sharing for given association
 * return SECURITY_UTIL_STATUS_FAILURE if session not found
 * and cannot be added
 */
int32_t set_DS_local_crt_sharing_status(
		const secured_association_params_t *session_id,
		e_key_sharing_status_t status);

/*
 * Get status of local DS crt sharing for given association
 * return SECURITY_UTIL_STATUS_FAILURE if session not found
 */
int32_t get_DS_local_crt_sharing_status(
		const secured_association_params_t *session_id,
		e_key_sharing_status_t *status);


/*
 * Set status of local KA crt sharing for given association
 * return SECURITY_UTIL_STATUS_FAILURE if session not found
 * and cannot be added
 */
int32_t set_KA_local_crt_sharing_status(
		const secured_association_params_t *session_id,
		e_key_sharing_status_t status);
/*
 * Get status of local KA crt sharing for given association
 * return SECURITY_UTIL_STATUS_FAILURE if session not found
 */
int32_t get_KA_local_crt_sharing_status(
		const secured_association_params_t *session_id,
		e_key_sharing_status_t *status);

/*
 * Validate and Save remote ds crt
 * If validation ok -
 *	save it, return SECURITY_UTIL_STATUS_SUCCESS
 * If validation failed -
 *	don't save, return SECURITY_UTIL_STATUS_FAILURE
 */
int32_t validate_and_save_remote_ds_crt(
		const secured_association_params_t *session_id,
		const uint8_t *public_key, uint32_t public_key_size);

/*
 * Validate and Save remote ka crt
 * If validation ok -
 *	save it, return SECURITY_UTIL_STATUS_SUCCESS
 * If validation failed -
 *	don't save, return SECURITY_UTIL_STATUS_FAILURE
 */
int32_t validate_and_save_remote_ka_crt(
		const secured_association_params_t *session_id,
		const uint8_t *remote_ka_crt, uint32_t remote_ka_crt_size);

/*
 * free remote ka crt
 * If found -
 *	free it, return SECURITY_UTIL_STATUS_SUCCESS
 * If not found  -
 *	return SECURITY_UTIL_STATUS_FAILURE
 */
int32_t free_remote_ka_crt(
		const secured_association_params_t *session_id);
/*
 * Get pointer to pre-saved local DS public key
 */
const uint8_t *get_DS_public_key_local(
		uint32_t	*public_key_size);

/*
 * Get pointer to pre-saved remote DS public key
 */
const uint8_t *get_DS_public_key_remote(
		const secured_association_params_t *session_id,
		uint32_t	*public_key_size);
/*
 * Get pointer to pre-saved local KA public key
 */
const uint8_t *get_KA_public_key_local(
		uint32_t	*public_key_size);

/*
 * Get pointer to pre-saved local KA certificate
 */
const uint8_t *get_KA_certificate_local(
		uint32_t	*certificate_size);

/*
 * Get pointer to pre-saved remote KA certificate
 */
const uint8_t *get_KA_certificate_remote(
		const secured_association_params_t *session_id,
		uint32_t	*certificate_size);
/*
 * Create digital signature from auth_params fields and add to the buffer
 * currently supported only Security Suite 1 && MechanismId 7
 */
int32_t create_HLS_authentication(hls_auth_params_t *auth_params,
							uint8_t *buffer,
							uint32_t *buffer_size,
							uint8_t is_originator);

/*
 * Verify digital signature using auth_params fields
 * currently supported only Security Suite 1 && MechanismId 7
 */
int32_t verify_HLS_authentication(hls_auth_params_t *auth_params,
							uint8_t *buffer,
							uint32_t buffer_size,
							uint8_t is_originator);

/*
 * Calculate shared secret Z using shared_secret_params fields
 */
int32_t calculate_shared_secret(
		const secured_association_params_t *session_id);

/* calculate size of output_buffer for authenticated encryption*/
void auth_encrypt_get_output_size(e_security_mode_t mode,
						uint32_t plain_buffer_size,
						uint32_t *cipher_buffer_size);

/*
 * authenticated encryption of input plain_buffer
 * output buffer allocated and freed by user - outside of the function
 */
int32_t auth_encrypt(
		const secured_association_params_t *session_id,
		crypto_params_t *params,
		uint8_t *plain_buffer,
		uint32_t plain_buffer_size,
		uint8_t *cipher_buffer,
		uint32_t *cipher_buffer_size);


/* calculate size of output_buffer for authenticated decryption*/
void auth_decrypt_get_output_size(e_security_mode_t mode,
						uint32_t cipher_buffer_size,
						uint32_t *plain_buffer_size);

/*
 * authenticated decryption of input cipher_buffer
 * output buffer allocated and freed by user - outside of the function
 */
int32_t auth_decrypt(
		const secured_association_params_t *session_id,
		crypto_params_t *params,
		uint8_t *cipher_buffer,
		uint32_t cipher_buffer_size,
		uint8_t *plain_buffer,
		uint32_t *plain_buffer_size);

#endif /* SECURITY_UTIL_INCLUDE_SECURITY_API_H_ */
