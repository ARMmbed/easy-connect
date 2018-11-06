
#include "security_internal.h"
#include "security_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef	__NO_ASSERT_H__		/* Include assert.h only for internal use. */
#include <assert.h>		/* for assert() macro */
#endif

#define MAX_SIZE					100
#define MAX_NUM_SECURE_SESSIONS		200

static uint8_t aes_gcm_128_alg[] = {0x60, 0x85, 0x74, 0x05, 0x08, 0x03, 0x00};
static uint8_t aes_gcm_256_alg[] = {0x60, 0x85, 0x74, 0x05, 0x08, 0x03, 0x01};

static session_secure_params_t sessions[MAX_NUM_SECURE_SESSIONS];

static security_key_pair_t ds_keys_local;
static security_key_pair_t ka_keys_local;

static void init_key_pair(security_key_pair_t *src_key_pair,
	security_key_pair_t *dst_key_pair)
{
	if (src_key_pair != NULL) {
		memcpy(dst_key_pair, src_key_pair,
			sizeof(security_key_pair_t));
	}
}

/*
 * This function should be called each time after key generation
 * NULL can be passed if one of the key pairs shouldn't be changed
 */
void init_security_util(security_key_pair_t *ds_key_pair,
	security_key_pair_t *ka_key_pair)
{
	init_key_pair(ds_key_pair, &ds_keys_local);
	init_key_pair(ka_key_pair, &ka_keys_local);

	memset(sessions, 0,
	MAX_NUM_SECURE_SESSIONS *
	sizeof(session_secure_params_t));
}

/*
 * This function should be called once at the exit
 */
void exit_security_util(void)
{

}

/*
 * compare src and dst association params
 * return 1 if identical, 0 otherwise
 */
static int32_t compare_association_params(
	const secured_association_params_t *src,
	const secured_association_params_t *dst)
{
	assert(src != NULL && dst != NULL);
	int32_t res =
	(src->local_wrapper_port == dst->local_wrapper_port) &&
	(src->remote_wrapper_port == dst->remote_wrapper_port) &&
	(src->remote_port == dst->remote_port) &&
	(src->remote_ip_address.type == dst->remote_ip_address.type) &&
	(src->remote_ip_address.choice.ipV4 ==
	dst->remote_ip_address.choice.ipV4);

	return res;
}

/*
 * find session with corresponding
 * association params
 * return index if found, -1 otherwise
 */
static int32_t find_session(const secured_association_params_t *id)
{
	for (int ind = 0; ind < MAX_NUM_SECURE_SESSIONS; ind++) {
		if (compare_association_params(id, &(sessions[ind].id)))
			return ind;
	}
	return -1;
}

/*
 * add new session with corresponding
 * association params
 * return index if added, -1 otherwise
 */
static int32_t find_or_add_session(
		const secured_association_params_t *id)
{
	int ind = find_session(id);

	if (ind < 0) {
		for (int ind = 0; ind < MAX_NUM_SECURE_SESSIONS; ind++) {
			if (sessions[ind].id.local_wrapper_port == 0) {
				sessions[ind].id.local_wrapper_port =
					id->local_wrapper_port;
				sessions[ind].id.remote_port =
					id->remote_port;
				sessions[ind].id.remote_wrapper_port =
					id->remote_wrapper_port;
				sessions[ind].id.remote_ip_address.type =
					id->remote_ip_address.type;
				sessions[ind].id.remote_ip_address.choice.ipV4 =
					id->remote_ip_address.choice.ipV4;
				return ind;
			}
		}
		return -1;
	}
	return ind;
}

/*
 * Set status of local DS crt sharing for given association
 * return SECURITY_UTIL_STATUS_FAILURE if session not found
 * and cannot be added
 */
int32_t set_DS_local_crt_sharing_status(
		const secured_association_params_t *session_id,
		e_key_sharing_status_t status)
{
	int ind = find_or_add_session(session_id);

	if (ind < 0)
		return SECURITY_UTIL_STATUS_FAILURE;
	sessions[ind].ds_key_sharing_status = status;
	return SECURITY_UTIL_STATUS_SUCCESS;
}

/*
 * Get status of local DS crt sharing for given association
 * return SECURITY_UTIL_STATUS_FAILURE if session not found
 */
int32_t get_DS_local_crt_sharing_status(
		const secured_association_params_t *session_id,
		e_key_sharing_status_t *status)
{
	int ind = find_session(session_id);

	if (ind < 0)
		return SECURITY_UTIL_STATUS_FAILURE;
	*status = sessions[ind].ds_key_sharing_status;
	return SECURITY_UTIL_STATUS_SUCCESS;
}


/*
 * Set status of local KA crt sharing for given association
 * return SECURITY_UTIL_STATUS_FAILURE if session not found
 * and cannot be added
 */
int32_t set_KA_local_crt_sharing_status(
		const secured_association_params_t *session_id,
		e_key_sharing_status_t status)
{
	int ind = find_or_add_session(session_id);

	if (ind < 0)
		return SECURITY_UTIL_STATUS_FAILURE;
	sessions[ind].ka_key_sharing_status = status;
	return SECURITY_UTIL_STATUS_SUCCESS;
}

/*
 * Get status of local KA crt sharing for given association
 * return SECURITY_UTIL_STATUS_FAILURE if session not found
 */
int32_t get_KA_local_crt_sharing_status(
		const secured_association_params_t *session_id,
		e_key_sharing_status_t *status)
{
	int ind = find_session(session_id);

	if (ind < 0) {
		*status = key_sharing_status_failure;
		return SECURITY_UTIL_STATUS_FAILURE;
	}
	*status = sessions[ind].ka_key_sharing_status;
	return SECURITY_UTIL_STATUS_SUCCESS;
}

/*
 * Validate and Save remote ds crt
 * If validation ok -
 *	save it, return SECURITY_UTIL_STATUS_SUCCESS
 * If validation failed -
 *	don't save, return SECURITY_UTIL_STATUS_FAILURE
 */
int32_t validate_and_save_remote_ds_crt(
		const secured_association_params_t *session_id,
		const uint8_t *public_key, uint32_t public_key_size)
{
	assert(public_key_size == PUBLIC_KEY_SIZE);
	assert(public_key != NULL);
	int ind = find_or_add_session(session_id);

	if (ind < 0)
		return SECURITY_UTIL_STATUS_FAILURE;
	memcpy(sessions[ind].ds_pub_key_remote, public_key, public_key_size);
	sessions[ind].ds_pub_key_remote_size = public_key_size;
	return SECURITY_UTIL_STATUS_SUCCESS;
}

/*
 * Save remote KA crt
 * If validation ok -
 *	save it, return SECURITY_UTIL_STATUS_SUCCESS
 * If validation failed -
 *	don't save, return SECURITY_UTIL_STATUS_FAILURE
 */
int32_t validate_and_save_remote_ka_crt(
		const secured_association_params_t *session_id,
		const uint8_t *public_key, uint32_t public_key_size)
{
	assert(public_key != NULL);

	if (public_key_size != PUBLIC_KEY_SIZE)
		return SECURITY_UTIL_STATUS_FAILURE;

	int ind = find_or_add_session(session_id);

	if (ind < 0)
		return SECURITY_UTIL_STATUS_FAILURE;
	memcpy(sessions[ind].ka_pub_key_remote, public_key, public_key_size);
	sessions[ind].ka_pub_key_remote_size = public_key_size;
	return SECURITY_UTIL_STATUS_SUCCESS;
}


/*
 * Get pointer to pre-saved local DS public key
 */
const uint8_t *get_DS_public_key_local(
		uint32_t	*public_key_size)
{
	*public_key_size = ds_keys_local.public_key_size;
	return ds_keys_local.public_key;
}

/*
 * Get pointer to pre-saved remote DS public key
 */
const uint8_t *get_DS_public_key_remote(
		const secured_association_params_t *session_id,
		uint32_t	*public_key_size)
{
	int ind = find_session(session_id);

	if (ind < 0 || sessions[ind].ds_pub_key_remote_size == 0)
		return NULL;

	*public_key_size = sessions[ind].ds_pub_key_remote_size;
	return sessions[ind].ds_pub_key_remote;
}

/*
 * Get pointer to pre-saved local KA public key
 */
const uint8_t *get_KA_public_key_local(
		uint32_t	*public_key_size)
{
	*public_key_size = ka_keys_local.public_key_size;
	return ka_keys_local.public_key;
}
/*
 * Get pointer to pre-saved remote KA public key
 */
const uint8_t *get_KA_public_key_remote(
		const secured_association_params_t *session_id,
		uint32_t	*public_key_size)
{
	int ind = find_session(session_id);

	if (ind < 0 || sessions[ind].ka_pub_key_remote_size == 0)
		return NULL;

	*public_key_size = sessions[ind].ka_pub_key_remote_size;
	return sessions[ind].ka_pub_key_remote;
}

static inline void copy_and_validate(uint8_t *to, uint32_t *to_size,
	uint8_t *from, uint32_t from_size, uint32_t max_size)
{
	memcpy(to + *to_size, from, from_size);
	*to_size += from_size;
	assert(*to_size <= max_size);
}
/*
 * message buffer should be pre-allocated before and have at least
 * MAX_SIZE size
 */
static void create_originator_message_for_ds
			(hls_auth_params_t *auth_params,
			uint8_t *message, uint32_t *message_size)
{
	*message_size = 0;
	assert(auth_params != NULL);
	assert(message != NULL);

	copy_and_validate(message, message_size,
		auth_params->originator_sys_title.buf,
		auth_params->originator_sys_title.size, MAX_SIZE);

	copy_and_validate(message, message_size,
		auth_params->recipient_sys_title.buf,
		auth_params->recipient_sys_title.size, MAX_SIZE);

	copy_and_validate(message, message_size,
		auth_params->recipient_challenge.buf,
		auth_params->recipient_challenge.size, MAX_SIZE);

	copy_and_validate(message, message_size,
		auth_params->originator_challenge.buf,
		auth_params->originator_challenge.size, MAX_SIZE);
}

/*
 * message buffer should be pre-allocated before and have at least
 * MAX_SIZE size
 */
static void create_recipient_message_for_ds
			(hls_auth_params_t *auth_params,
			uint8_t *message, uint32_t *message_size)
{
	*message_size = 0;
	assert(auth_params != NULL);
	assert(message != NULL);

	copy_and_validate(message, message_size,
		auth_params->recipient_sys_title.buf,
		auth_params->recipient_sys_title.size, MAX_SIZE);

	copy_and_validate(message, message_size,
		auth_params->originator_sys_title.buf,
		auth_params->originator_sys_title.size, MAX_SIZE);

	copy_and_validate(message, message_size,
		auth_params->originator_challenge.buf,
		auth_params->originator_challenge.size, MAX_SIZE);

	copy_and_validate(message, message_size,
		auth_params->recipient_challenge.buf,
		auth_params->recipient_challenge.size, MAX_SIZE);
}


/*
 * message buffer should be pre-allocated before and have at least
 * MAX_SIZE size
 */
static void create_message_for_kdf(kdf_params_t *kdf_params,
		int32_t session_ind, uint8_t *message, uint32_t *message_size)
{
	*message_size = 0;
	assert(kdf_params != NULL);
	assert(message != NULL);
	assert(session_ind >= 0);

	// key length in bytes = shared secret length
	message[(*message_size)++] = sessions[session_ind].shared_secret_size;
	assert(*message_size <= MAX_SIZE);

	copy_and_validate(message, message_size,
		sessions[session_ind].shared_secret,
		sessions[session_ind].shared_secret_size, MAX_SIZE);

	// authentication tag length in bytes
	message[(*message_size)++] = AUTH_TAG_SIZE;
	assert(*message_size <= MAX_SIZE);

	copy_and_validate(message, message_size,
		kdf_params->algorithm_id.buf,
		kdf_params->algorithm_id.size, MAX_SIZE);

	copy_and_validate(message, message_size,
		kdf_params->originator_sys_title->buf,
		kdf_params->originator_sys_title->size, MAX_SIZE);

	// 1 byte of transaction id size
	message[(*message_size)++] = kdf_params->transaction_id->size;
	assert(*message_size <= MAX_SIZE);

	copy_and_validate(message, message_size,
		kdf_params->transaction_id->buf,
		kdf_params->transaction_id->size, MAX_SIZE);

	copy_and_validate(message, message_size,
		kdf_params->recipient_sys_title->buf,
		kdf_params->recipient_sys_title->size, MAX_SIZE);
}
/*
 * message buffer should be pre-allocated before and have at least
 * MAX_SIZE size
 */
static void create_additional_data(crypto_params_t *params,
		int32_t session_ind, uint32_t key_size, uint8_t sc,
		uint8_t *message, uint32_t *message_size)
{
	*message_size = 0;
	assert(params != NULL);
	assert(message != NULL);
	assert(session_ind >= 0);

	message[(*message_size)++] = sc;
	assert(*message_size <= MAX_SIZE);
	// part of shared secret used as authentication key (AK)
	copy_and_validate(message, message_size,
		sessions[session_ind].shared_secret, key_size, MAX_SIZE);

	// 1 byte of transaction id size
	message[(*message_size)++] = params->transaction_id.size;
	assert(*message_size <= MAX_SIZE);
	// transactionId
	copy_and_validate(message, message_size,
		params->transaction_id.buf,
		params->transaction_id.size, MAX_SIZE);

	// 1 byte of originator sys title size
	message[(*message_size)++] = params->originator_sys_title.size;
	assert(*message_size <= MAX_SIZE);
	// originator sys title
	copy_and_validate(message, message_size,
		params->originator_sys_title.buf,
		params->originator_sys_title.size, MAX_SIZE);

	// 1 byte of recipient sys title size
	message[(*message_size)++] = params->recipient_sys_title.size;
	assert(*message_size <= MAX_SIZE);
	// recipient sys title
	copy_and_validate(message, message_size,
		params->recipient_sys_title.buf,
		params->recipient_sys_title.size, MAX_SIZE);

	// date-time
	message[(*message_size)++] = 0;
	// other information
	message[(*message_size)++] = 0;

	assert(*message_size <= MAX_SIZE);
}

/*
 * message buffer should be pre-allocated before and have at least
 * MAX_SIZE size
 * The length of the IV shall be 96 bits (12 octets): len(IV) = 96.
 * Within this:
 * the leftmost 64 bits (8 octets) shall hold the fixed field.
 * It shall contain the originator system title
 * the rightmost 32 bits shall hold the invocation field.
 */
static void create_initialization_vector(crypto_params_t *params,
		uint8_t *message)
{
	uint32_t msg_size = 0;

	assert(params != NULL);
	assert(message != NULL);

	copy_and_validate(message, &msg_size,
			params->originator_sys_title.buf,
			params->originator_sys_title.size, MAX_SIZE);

	// fill in invocation counter
	copy_and_validate(message, &msg_size, params->invocation_counter, 4,
			MAX_SIZE);
}


static uint8_t get_security_suite_params(uint8_t security_suite_id,
	security_suite_params_t *params)
{
	uint8_t ret = SECURITY_UTIL_STATUS_SUCCESS;

	switch (security_suite_id) {
	case 0:
	{
		params->algorithm_id = aes_gcm_128_alg;
		params->algorithm_id_size = sizeof(aes_gcm_128_alg);
		params->hash_len = 0;
		params->key_data_len = 128;
		break;
	}
	case 1:
	{
		params->algorithm_id = aes_gcm_128_alg;
		params->algorithm_id_size = sizeof(aes_gcm_128_alg);
		params->hash_len = 256;
		params->key_data_len = 128;
		break;
	}
	case 2:
	{
		params->algorithm_id = aes_gcm_256_alg;
		params->algorithm_id_size = sizeof(aes_gcm_256_alg);
		params->hash_len = 384;
		params->key_data_len = 256;
		break;
	}
	default:
		return SECURITY_UTIL_STATUS_FAILURE;
	}

	return ret;
}

static uint8_t create_security_ctrl_byte(uint8_t security_suite,
	e_security_mode_t	security_mode)
{
	return ((uint8_t)security_mode << 4 | security_suite);
}

static int32_t kdf(int32_t session_ind, crypto_params_t *params,
		uint8_t *session_key, uint32_t *key_size)
{
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	kdf_params_t kdf_params;
	security_suite_params_t security_suite_params;
	uint32_t	message_size;
	uint8_t message[MAX_SIZE];
	uint8_t *hash = NULL;
	uint32_t hash_size;

	assert(params);
	// currently suppoirt only suite1 and AE mode
	assert(params->security_suite == 1);
	assert(params->security_mode == security_mode_auth_encrypt);

	ret =
		get_security_suite_params(params->security_suite,
			&security_suite_params);

	if (ret != SECURITY_UTIL_STATUS_SUCCESS)
		return ret;

	hash_size = security_suite_params.hash_len >> 3;
	*key_size = security_suite_params.key_data_len >> 3;
	assert(hash_size <= HASH_SIZE);
	assert(*key_size <= hash_size);

	kdf_params.algorithm_id.buf = security_suite_params.algorithm_id;
	kdf_params.algorithm_id.size = security_suite_params.algorithm_id_size;
	kdf_params.originator_sys_title = &params->originator_sys_title;
	kdf_params.recipient_sys_title = &params->recipient_sys_title;
	kdf_params.transaction_id = &params->transaction_id;

	create_message_for_kdf(&kdf_params, session_ind,
		message, &message_size);

	hash = (uint8_t *)calloc(hash_size, sizeof(uint8_t));
	assert(hash);
	if (hash_size == 32)
		ret = compute_hash_256(message, message_size, hash);
	else if (hash_size == 48)
		ret = compute_hash_384(message, message_size, hash);
	else
		ret = SECURITY_UTIL_STATUS_FAILURE;

	if (ret == SECURITY_UTIL_STATUS_SUCCESS)
		memcpy(session_key, hash, *key_size);

	free(hash);
	return ret;
}

/*
 * Create digital signature from auth_params fields and add to the buffer
 * currently supported only Security Suite 1 && MechanismId 7
 */
int32_t	create_HLS_authentication(hls_auth_params_t *auth_params,
							uint8_t *buffer,
							uint32_t *buffer_size,
							uint8_t is_originator)
{
	assert(auth_params != NULL);
	assert(auth_params->security_suite == 1);
	assert(auth_params->mechanism_id == 7);
	assert(buffer != NULL);
	assert(buffer_size != 0);

	security_int_params_t int_params;
	security_suite_params_t security_suite_params;
	uint32_t message_size = 0;
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	uint8_t message[MAX_SIZE];
	uint8_t *hash = NULL;
	uint32_t hash_size;

	ret =
		get_security_suite_params(auth_params->security_suite,
			&security_suite_params);
	if (ret != SECURITY_UTIL_STATUS_SUCCESS)
		return ret;

	if (is_originator)
		create_originator_message_for_ds
			(auth_params, message, &message_size);
	else
		create_recipient_message_for_ds
			(auth_params, message, &message_size);

	hash_size = security_suite_params.hash_len >> 3;
	hash = (uint8_t *)calloc(hash_size, sizeof(uint8_t));
	if (security_suite_params.hash_len == 256)
		ret = compute_hash_256(message, message_size, hash);
	else if (security_suite_params.hash_len == 384)
		ret = compute_hash_384(message, message_size, hash);
	else
		ret = SECURITY_UTIL_STATUS_FAILURE;

	if (ret == SECURITY_UTIL_STATUS_SUCCESS) {
		int_params.security_grp_id = security_group_id_ecp_dp_256r1;
		int_params.remote_public_key_size =
				auth_params->public_key_size;
		int_params.remote_public_key =
				auth_params->public_key;
		int_params.local_private_key =
				ds_keys_local.private_key;
		int_params.local_private_key_size =
				ds_keys_local.private_key_size;

		ret = ECDSA_Sign(&int_params, buffer, buffer_size,
				hash, security_suite_params.hash_len);
	}
	free(hash);
	return ret;
}

/*
 * Verify digital signature using auth_params fields
 * currently supported only Security Suite 1 && MechanismId 7
 */
int32_t verify_HLS_authentication(hls_auth_params_t *auth_params,
							uint8_t *buffer,
							uint32_t buffer_size,
							uint8_t is_originator)
{
	assert(auth_params != NULL);
	assert(auth_params->security_suite == 1);
	assert(auth_params->mechanism_id == 7);
	assert(buffer != NULL);
	assert(buffer_size != 0);

	security_int_params_t int_params;
	security_suite_params_t security_suite_params;
	uint32_t message_size = 0;
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	uint8_t message[MAX_SIZE];
	uint8_t *hash = NULL;
	uint32_t hash_size;

	ret =
		get_security_suite_params(auth_params->security_suite,
			&security_suite_params);
	if (ret != SECURITY_UTIL_STATUS_SUCCESS)
		return ret;

	// for verification use opposite direction
	if (is_originator)
		create_recipient_message_for_ds
			(auth_params, message, &message_size);
	else
		create_originator_message_for_ds
			(auth_params, message, &message_size);

	hash_size = security_suite_params.hash_len >> 3;
	hash = (uint8_t *)calloc(hash_size, sizeof(uint8_t));
	if (security_suite_params.hash_len == 256)
		ret = compute_hash_256(message, message_size, hash);
	else if (security_suite_params.hash_len == 384)
		ret = compute_hash_384(message, message_size, hash);
	else
		ret = SECURITY_UTIL_STATUS_FAILURE;

	if (ret == SECURITY_UTIL_STATUS_SUCCESS) {
		int_params.security_grp_id = security_group_id_ecp_dp_256r1;
		int_params.remote_public_key_size =
				auth_params->public_key_size;
		int_params.remote_public_key =
				auth_params->public_key;
		int_params.local_private_key =
				ds_keys_local.private_key;
		int_params.local_private_key_size =
				ds_keys_local.private_key_size;

		ret = ECDSA_Verify(&int_params, buffer, buffer_size,
			hash, security_suite_params.hash_len);
	}
	free(hash);
	return ret;
}

/*
 * Calculate shared secret Z using shared_secret_params fields
 */
int32_t calculate_shared_secret(
		const secured_association_params_t *session_id)
{
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	security_int_params_t int_params;
	int ind = find_session(session_id);

	if (ind < 0)
		return SECURITY_UTIL_STATUS_FAILURE;

	int_params.security_grp_id = security_group_id_ecp_dp_256r1;
	int_params.remote_public_key_size =
			sessions[ind].ka_pub_key_remote_size;
	int_params.remote_public_key =
			sessions[ind].ka_pub_key_remote;
	int_params.local_private_key =
			ka_keys_local.private_key;
	int_params.local_private_key_size =
			ka_keys_local.private_key_size;

	sessions[ind].shared_secret_size = PRIVATE_KEY_SIZE;
	ret = ECDH_compute_Z(&int_params,
		sessions[ind].shared_secret,
		&sessions[ind].shared_secret_size);
	return ret;
}

/* calculate size of output_buffer for authenticated encryption*/
void auth_encrypt_get_output_size(e_security_mode_t mode,
		uint32_t plain_buffer_size,
		uint32_t *cipher_buffer_size)
{
	switch (mode) {
	case security_mode_none:
	case security_mode_encrypt:
		*cipher_buffer_size = plain_buffer_size;
		break;
	case security_mode_auth:
	case security_mode_auth_encrypt:
		*cipher_buffer_size = plain_buffer_size + AUTH_TAG_SIZE;
		break;
	default:
		*cipher_buffer_size = 0;
		break;
	}
}

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
		uint32_t *cipher_buffer_size)
{
	uint32_t key_size = 0;
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	uint8_t *session_key = NULL;
	uint8_t sc;
	int32_t session_ind;
	uint8_t ad[MAX_SIZE], iv[IV_SIZE];
	uint32_t ad_size;
	crypto_int_params_t int_params;

	assert(session_id);
	session_ind = find_session(session_id);
	assert(session_ind >= 0);

	switch (params->security_suite) {
	case 0:
	case 1:
	{
		key_size = 16;
		break;
	}
	case 2:
	{
		key_size = 32;
		break;
	}
	default:
		return SECURITY_UTIL_STATUS_FAILURE;
	}

	session_key = (uint8_t *)calloc(key_size, sizeof(uint8_t));
	assert(session_key);
	ret = kdf(session_ind, params, session_key, &key_size);

	if (ret == SECURITY_UTIL_STATUS_SUCCESS) {
		sc = create_security_ctrl_byte(
			params->security_suite, params->security_mode);
		create_additional_data(params, session_ind,
			key_size, sc, ad, &ad_size);
		create_initialization_vector(params, iv);

		int_params.session_key = session_key;
		int_params.key_size = key_size;
		int_params.iv = iv;
		int_params.iv_size = IV_SIZE;
		int_params.add_data = ad;
		int_params.add_size = ad_size;

		ret = cipher_auth_encrypt(&int_params,
			plain_buffer, plain_buffer_size,
			cipher_buffer, cipher_buffer_size);

	}

	free(session_key);
	return ret;
}


/* calculate size of output_buffer for authenticated decryption*/
void auth_decrypt_get_output_size(e_security_mode_t mode,
						uint32_t cipher_buffer_size,
						uint32_t *plain_buffer_size)
{

	if (cipher_buffer_size <= AUTH_TAG_SIZE) {
		*plain_buffer_size = 0;
		return;
	}

	*plain_buffer_size = cipher_buffer_size - AUTH_TAG_SIZE;

	switch (mode) {
	case security_mode_none:
	case security_mode_encrypt:
		assert(cipher_buffer_size > 0);
		*plain_buffer_size = cipher_buffer_size;
		break;
	case security_mode_auth:
	case security_mode_auth_encrypt:
		*plain_buffer_size = cipher_buffer_size - AUTH_TAG_SIZE;
		break;
	default:
		*plain_buffer_size = 0;
		break;
	}
}

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
		uint32_t *plain_buffer_size)
{
	uint32_t key_size = 0;
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	uint8_t *session_key = NULL;
	uint8_t sc;
	int32_t session_ind;
	uint8_t ad[MAX_SIZE], iv[IV_SIZE];
	uint32_t ad_size;
	crypto_int_params_t int_params;

	assert(session_id);
	session_ind = find_session(session_id);
	if (session_ind == -1)
		return SECURITY_UTIL_STATUS_FAILURE;

	switch (params->security_suite) {
	case 0:
	case 1:
	{
		key_size = 16;
		break;
	}
	case 2:
	{
		key_size = 32;
		break;
	}
	default:
		return SECURITY_UTIL_STATUS_FAILURE;
	}

	session_key = (uint8_t *)calloc(key_size, sizeof(uint8_t));
	assert(session_key);
	ret = kdf(session_ind, params, session_key, &key_size);

	if (ret == SECURITY_UTIL_STATUS_SUCCESS) {
		sc = create_security_ctrl_byte(
				params->security_suite, params->security_mode);
		create_additional_data(params, session_ind,
			key_size, sc, ad, &ad_size);
		create_initialization_vector(params, iv);

		int_params.session_key = session_key;
		int_params.key_size = key_size;
		int_params.iv = iv;
		int_params.iv_size = IV_SIZE;
		int_params.add_data = ad;
		int_params.add_size = ad_size;

		ret = cipher_auth_decrypt(&int_params,
			cipher_buffer, cipher_buffer_size,
			plain_buffer, plain_buffer_size);
	}

	free(session_key);
	return ret;
}

