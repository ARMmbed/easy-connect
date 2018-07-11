
#include "security_internal.h"
#include "security_api.h"

#include <stdio.h>
#include <stdlib.h>
 #include <string.h>
#ifndef	__NO_ASSERT_H__		/* Include assert.h only for internal use. */
#include <assert.h>		/* for assert() macro */
#endif

#define MAX_SIZE			512

static const uint8_t temp_key[] = {
		0x71, 0xed, 0x42, 0xeb, 0x83, 0xb7, 0x50, 0xe4,
		0xc3, 0xa4, 0x3f, 0xaf, 0x2c, 0x34, 0xc1, 0x14,
		0xe9, 0x5a, 0x13, 0xcf, 0x75, 0xd1, 0xe0, 0xab,
		0xa3, 0x82, 0x44, 0x80, 0xa6, 0xc3, 0xf8, 0xbc
};

/* stub for secure storage functions */
const uint8_t *key(uint32_t *size)
{
	*size = sizeof(temp_key);
	return temp_key;
}

/*
 * Extract public key from buffer
 * public_key should be pre-allocated before
 * and have at least buffer_size size
 */
void extract_public_key(uint8_t *buffer, uint32_t buffer_size,
			uint8_t *public_key,
			uint32_t	*public_key_size)
{
	assert(buffer != NULL);
	assert(buffer_size == 64);
	assert(public_key != NULL);

	*public_key_size = buffer_size;
	memcpy(public_key, buffer, *public_key_size);
}

/*
 * Get pre-saved DS public key
 * public_key should be pre-allocated (64 bytes)
 * return 0 if the key exists
 * return 1 if not
 */
uint8_t get_presaved_DS_public_key(uint8_t *public_key,
				uint32_t	*public_key_size)
{
	assert(public_key != NULL);
	return SECURITY_UTIL_STATUS_FAILURE;
}

/*
 * Get pre-saved KA public key
 * public_key should be pre-allocated (64 bytes)
 * return 0 if the key exists
 * return 1 if not
 */
uint8_t get_presaved_KA_public_key(uint8_t *public_key,
					uint32_t	*public_key_size)
{
	assert(public_key != NULL);
	return SECURITY_UTIL_STATUS_FAILURE;
}

/*
 * message buffer should be pre-allocated before and have at least
 * MAX_SIZE size
 */
static void create_originator_message_for_ds
			(hls_auth_params_t *auth_params,
			uint8_t *message, uint32_t *message_size)
{
	uint32_t msg_size = 0;

	*message_size = 0;
	assert(auth_params != NULL);
	assert(message != NULL);

	memcpy(message + msg_size, auth_params->originator_sys_title.buf,
		auth_params->originator_sys_title.size);
	msg_size += auth_params->originator_sys_title.size;
	assert(msg_size <= MAX_SIZE);
	memcpy(message + msg_size, auth_params->recipient_sys_title.buf,
			auth_params->recipient_sys_title.size);
	msg_size += auth_params->recipient_sys_title.size;
	assert(msg_size <= MAX_SIZE);
	memcpy(message + msg_size, auth_params->recipient_challenge.buf,
			auth_params->recipient_challenge.size);
	msg_size += auth_params->recipient_challenge.size;
	assert(msg_size <= MAX_SIZE);
	memcpy(message + msg_size, auth_params->originator_challenge.buf,
			auth_params->originator_challenge.size);
	msg_size += auth_params->originator_challenge.size;
	assert(msg_size <= MAX_SIZE);
	*message_size = msg_size;
}

/*
 * message buffer should be pre-allocated before and have at least
 * MAX_SIZE size
 */
static void create_recipient_message_for_ds
			(hls_auth_params_t *auth_params,
			uint8_t *message, uint32_t *message_size)
{
	uint32_t msg_size = 0;

	*message_size = 0;
	assert(auth_params != NULL);
	assert(message != NULL);

	memcpy(message + msg_size, auth_params->recipient_sys_title.buf,
			auth_params->recipient_sys_title.size);
	msg_size += auth_params->recipient_sys_title.size;
	assert(msg_size <= MAX_SIZE);
	memcpy(message + msg_size, auth_params->originator_sys_title.buf,
		auth_params->originator_sys_title.size);
	msg_size += auth_params->originator_sys_title.size;
	assert(msg_size <= MAX_SIZE);
	memcpy(message + msg_size, auth_params->originator_challenge.buf,
			auth_params->originator_challenge.size);
	msg_size += auth_params->originator_challenge.size;
	assert(msg_size <= MAX_SIZE);
	memcpy(message + msg_size, auth_params->recipient_challenge.buf,
			auth_params->recipient_challenge.size);
	msg_size += auth_params->recipient_challenge.size;
	assert(msg_size <= MAX_SIZE);
	*message_size = msg_size;
}

/*
 * Create digital signature from auth_params fields and add to the buffer
 * currently supported only Security Suite 1 && MechanismId 7
 */
int32_t	create_HLS_authentication(hls_auth_params_t *auth_params,
							uint8_t *buffer,
							uint32_t *buffer_size,
							uint8_t is_originator,
							get_private_key func)
{
	assert(auth_params != NULL);
	assert(auth_params->security_suite == 1);
	assert(auth_params->mechanism_id == 7);
	assert(buffer != NULL);
	assert(buffer_size != 0);

	ds_int_params_t ds_params;
	uint32_t message_size = 0, key_size = 0;
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	uint8_t *message = (uint8_t *)calloc(MAX_SIZE, sizeof(uint8_t));

	assert(message != NULL);

	if (is_originator)
		create_originator_message_for_ds
			(auth_params, message, &message_size);
	else
		create_recipient_message_for_ds
			(auth_params, message, &message_size);

	ds_params.public_key_size =
			auth_params->public_key_size;
	ds_params.public_key = auth_params->public_key;
	if (func)
		ds_params.private_key = func(&key_size);
	else
		ds_params.private_key = key(&key_size);
	ds_params.private_key_size = key_size;

/*
 *	printf("private key:\n%u %u %u %u\n",
 *		ds_params.private_key[0],
 *		ds_params.private_key[1],
 *		ds_params.private_key[2],
 *		ds_params.private_key[3]);
 */
	ret = ECDSA_Sign(&ds_params, buffer, buffer_size,
						message, message_size);
	free(message);
	return ret;
}


static void PrintfBuffName(const char *name, const unsigned char *ptr, int size)
{
	printf("###### %s ####\n", name);
	for(int i = 0 ; i < size ; ++i)
	{
		printf("%02x ", *ptr++);
		if((i+1)% 8 == 0) printf("\n");
	}
	if(size% 8 != 0) printf("\n");
	printf("#########################\n\n");
}

static void PrintfAuthParams(hls_auth_params_t *auth_params)
{
	PrintfBuffName("originator_sys_title", auth_params->originator_sys_title.buf, auth_params->originator_sys_title.size);
	PrintfBuffName("originator_challenge", auth_params->originator_challenge.buf, auth_params->originator_challenge.size);
	PrintfBuffName("recipient_sys_title", auth_params->recipient_sys_title.buf, auth_params->recipient_sys_title.size);
	PrintfBuffName("recipient_challenge", auth_params->recipient_challenge.buf, auth_params->recipient_challenge.size);
	PrintfBuffName("public_key", auth_params->public_key, auth_params->public_key_size);
}

static void PrintfDSParams(ds_int_params_t *ds_params)
{
	PrintfBuffName("public_key", ds_params->public_key, ds_params->public_key_size);
	PrintfBuffName("private_key", ds_params->private_key, ds_params->private_key_size);
}

/*
 * Verify digital signature using auth_params fields
 * currently supported only Security Suite 1 && MechanismId 7
 */
int32_t verify_HLS_authentication(hls_auth_params_t *auth_params,
							uint8_t *buffer,
							uint32_t buffer_size,
							uint8_t is_originator,
							get_private_key func)
{
	assert(auth_params != NULL);
	assert(auth_params->security_suite == 1);
	assert(auth_params->mechanism_id == 7);
	assert(buffer != NULL);
	assert(buffer_size != 0);

	ds_int_params_t ds_params;
	uint32_t message_size = 0, key_size = 0;
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	uint8_t *message = (uint8_t *)calloc(MAX_SIZE, sizeof(uint8_t));

	assert(message != NULL);

	// for verification use opposite direction
	if (is_originator)
		create_recipient_message_for_ds
			(auth_params, message, &message_size);
	else
		create_originator_message_for_ds
			(auth_params, message, &message_size);

	ds_params.public_key_size =
			auth_params->public_key_size;
	ds_params.public_key = auth_params->public_key;
	if (func)
		ds_params.private_key = func(&key_size);
	else
		ds_params.private_key = key(&key_size);
	ds_params.private_key_size = key_size;
	
	PrintfAuthParams(auth_params);
	PrintfDSParams(&ds_params);

	ret = ECDSA_Verify(&ds_params, buffer, buffer_size,
						message, message_size);
	free(message);
	return ret;
}
