
#include "security_internal.h"

#include "mbedtls/sha256.h"
#include "mbedtls/ecdsa.h"

#include <stdio.h>
#include <stdlib.h>
 #include <string.h>
#ifndef	__NO_ASSERT_H__		/* Include assert.h only for internal use. */
#include <assert.h>		/* for assert() macro */
#endif

#define HASH_SIZE			32
#define OCTET_STRING_TAG	9

/*
 * This function was taken from mbedTLS tests.
 * Replace by real rng function - TBD
 */
static int myrand(void *rng_state, unsigned char *output, size_t len)
{
	size_t use_len;
	int rnd;

	if (rng_state != NULL)
		rng_state  = NULL;

	while (len > 0) {
		use_len = len;
		if (use_len > sizeof(int))
			use_len = sizeof(int);

		rnd = rand();
		memcpy(output, &rnd, use_len);
		output += use_len;
		len -= use_len;
	}

	return (0);
}

/*
 * Auxiliary function - init_mbedtls_context,
 * create public and private keys from strings
 */
static int32_t init_mbedtls_context(mbedtls_ecdsa_context *ctx,
						ds_int_params_t	*params)
{
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;

	mbedtls_ecdsa_init(ctx);
	ctx->grp.id = MBEDTLS_ECP_DP_SECP256R1;
	mbedtls_ecp_group_load(&ctx->grp, MBEDTLS_ECP_DP_SECP256R1);
	ret = mbedtls_ecp_point_read_binary(&ctx->grp, &ctx->Q,
		params->public_key, params->public_key_size);

	if (ret == 0) {
		ret = mbedtls_mpi_read_binary(&ctx->d,
				params->private_key,
				params->private_key_size);

	}
	return ret;
}

int32_t ECDSA_Sign(ds_int_params_t	*params,
					uint8_t		*buffer,
					uint32_t	*buffer_size,
					uint8_t		*message,
					uint32_t	message_size)
{
	printf("%s\n", __func__);
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	mbedtls_ecdsa_context mbedTls_ctx;
	unsigned char *hash = NULL;
	unsigned char *sig = NULL;
	size_t sig_len;

	ret = init_mbedtls_context(&mbedTls_ctx, params);
	if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
		printf(" failed! init_mbedtls_context returned %d\n", ret);
		mbedtls_ecdsa_free(&mbedTls_ctx);
		return ret;
	}

	hash = (unsigned char *)calloc(HASH_SIZE, sizeof(unsigned char));
	assert(hash != NULL);

	sig = (unsigned char *)calloc(MBEDTLS_ECDSA_MAX_LEN,
							sizeof(unsigned char));
	assert(sig != NULL);

	ret = mbedtls_sha256_ret(message, message_size, hash, 0);

	if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
		printf(" failed! mbedtls_sha256_ret returned %d\n", ret);
	} else {
		ret = mbedtls_ecdsa_write_signature(&mbedTls_ctx,
			MBEDTLS_MD_SHA256, hash, HASH_SIZE, sig, &sig_len,
			myrand, NULL);
		if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
			printf
			("mbedtls_ecdsa_write_signature returned %d\n", ret);
		} else {
			uint32_t ind = 0;
/*
 *			for(int i = 0; i < sig_len; i+=4)
 *				printf("0x%x 0x%x 0x%x 0x%x\n",
 *				(unsigned int) sig[i],
 *				(unsigned int) sig[i+1],
 *				(unsigned int) sig[i+2],
 *				(unsigned int) sig[i+3]);
 */

			buffer[ind++] = OCTET_STRING_TAG;
			// The length should be less than 128 byte
			assert(sig_len < 0x80);
			buffer[ind++] = sig_len;

			memcpy(buffer + ind, sig, sig_len);
			*buffer_size = sig_len + ind;
		}
	}

	mbedtls_ecdsa_free(&mbedTls_ctx);
	free(hash);
	free(sig);
	return ret;
}

int32_t ECDSA_Verify(ds_int_params_t	*params,
					uint8_t		*buffer,
					uint32_t	buffer_size,
					uint8_t		*message,
					uint32_t	message_size)
{
	printf("%s\n", __func__);
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	mbedtls_ecdsa_context mbedTls_ctx;
	unsigned char *hash = NULL;

	init_mbedtls_context(&mbedTls_ctx, params);

	hash = (unsigned char *)calloc(HASH_SIZE, sizeof(unsigned char));
	assert(hash != NULL);

	ret = mbedtls_sha256_ret(message, message_size, hash, 0);
	if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
		printf(" failed! mbedtls_sha256_ret returned %d\n", ret);
	} else {
		uint32_t ind = 0;
		size_t sig_size;

		assert(buffer_size > 2);
		assert(buffer != NULL);
		if (buffer[ind++] == OCTET_STRING_TAG) {
			sig_size = buffer[ind++];
			assert(sig_size == buffer_size-2);
			ret = mbedtls_ecdsa_read_signature(&mbedTls_ctx,
							hash, HASH_SIZE,
							buffer + ind, sig_size);
			if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
				printf
				("mbedtls_ecdsa_read_signature returned %d\n",
				ret);
			}
		} else {
			printf("%s: wrong buffer format\n", __func__);
			ret = 1;
		}
	}

	mbedtls_ecdsa_free(&mbedTls_ctx);
	free(hash);
	return ret;
}
