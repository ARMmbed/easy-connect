
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
#define HALF_SIG_SIZE		33

/*
 * This function was taken from mbedTLS tests.
 * Replace by real rng function - TBD
 */
static int myrand(void *rng_state, uint8_t *output, size_t len)
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
/*
 * Pay attention:
 * according to mbedTLS rules, public key MUST start from this byte:
 * BER_OCTET_STRING tag 0x4
 */
	uint32_t public_size = params->public_key_size + 1;
	uint8_t *buff = (uint8_t *)calloc(public_size, sizeof(uint8_t));

	assert(buff != NULL);
	buff[0] = 0x04; // adding BER_OCTET_STRING tag
	memcpy(buff + 1, params->public_key, params->public_key_size);

	mbedtls_ecdsa_init(ctx);
	ctx->grp.id = MBEDTLS_ECP_DP_SECP256R1;
	mbedtls_ecp_group_load(&ctx->grp, MBEDTLS_ECP_DP_SECP256R1);


	ret = mbedtls_ecp_point_read_binary(&ctx->grp, &ctx->Q,
						buff, public_size);

	if (ret == 0) {
		ret = mbedtls_mpi_read_binary(&ctx->d,
				params->private_key,
				params->private_key_size);

	}
	free(buff);
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
	uint8_t  *hash = NULL;
	uint8_t  *sig = NULL;
	size_t sig_len, len_r, len_s, max_half;
	mbedtls_mpi r, s;
	int i = 0;

	mbedtls_mpi_init(&r);
	mbedtls_mpi_init(&s);

	ret = init_mbedtls_context(&mbedTls_ctx, params);
	if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
		printf(" failed! init_mbedtls_context returned %d\n", ret);
		mbedtls_ecdsa_free(&mbedTls_ctx);
		return ret;
	}

	hash = (uint8_t *)calloc(HASH_SIZE, sizeof(uint8_t));
	assert(hash != NULL);

	ret = mbedtls_sha256_ret(message, message_size, hash, 0);

	if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
		printf(" failed! mbedtls_sha256_ret returned %d\n", ret);
	} else {

		ret = mbedtls_ecdsa_sign
			(&mbedTls_ctx.grp, &r, &s, &mbedTls_ctx.d,
			hash, HASH_SIZE, myrand, NULL);

		len_r = mbedtls_mpi_size(&r);
		len_s = mbedtls_mpi_size(&s);
		printf("%s: len_r=%d, len_s=%d\n",
			__func__, (int)len_r, (int)len_s);
//		assert(len_r == len_s);
		assert((len_r <= HALF_SIG_SIZE) && (len_r > 0));
		assert((len_s <= HALF_SIG_SIZE) && (len_s > 0));

		max_half = (len_r > len_s) ? len_r : len_s;
		sig_len = 2 * max_half;

		assert(sig_len > 0);
		sig = (uint8_t *)calloc(sig_len, sizeof(uint8_t));
		assert(sig != NULL);

		if (len_r < max_half) {
			for (i = 0; i < (int)(max_half-len_r); i++)
				sig[i] = 0;
		}
		mbedtls_mpi_write_binary(&r, sig+max_half-len_r, len_r);

		if (len_s < max_half) {
			for (i = max_half; i < (int)(sig_len-len_s); i++)
				sig[i] = 0;
		}
		mbedtls_mpi_write_binary(&s, sig+sig_len-len_s, len_s);

		if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
			printf
			("mbedtls_ecdsa_write_signature returned %d\n", ret);
		} else {
//			uint32_t ind = 0;

//			buffer[ind++] = OCTET_STRING_TAG;
			// The length should be less than 128 byte
//			assert(sig_len < 0x80);
//			buffer[ind++] = sig_len;

//			memcpy(buffer + ind, sig, sig_len);
//			*buffer_size = sig_len + ind;
			memcpy(buffer, sig, sig_len);
			*buffer_size = sig_len;
		}
		free(sig);
	}

	mbedtls_ecdsa_free(&mbedTls_ctx);
	free(hash);
	mbedtls_mpi_free(&r);
	mbedtls_mpi_free(&s);
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
	uint8_t *hash = NULL;
	mbedtls_mpi r, s;

	mbedtls_mpi_init(&r);
	mbedtls_mpi_init(&s);

	init_mbedtls_context(&mbedTls_ctx, params);

	hash = (uint8_t *)calloc(HASH_SIZE, sizeof(uint8_t));
	assert(hash != NULL);

	ret = mbedtls_sha256_ret(message, message_size, hash, 0);
	if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
		printf(" failed! mbedtls_sha256_ret returned %d\n", ret);
	} else {
//		uint32_t ind = 0;
		size_t sig_size;

		assert(buffer_size > 2);
		assert(buffer != NULL);

//		if (buffer[ind++] == OCTET_STRING_TAG) {
//			sig_size = buffer[ind++];
			sig_size = buffer_size;
			printf("buffer_size %d, sig_size=%d\n",
				(int)buffer_size, (int)sig_size);
//			assert(sig_size == buffer_size-2);
//			assert(sig_size % 2 == 0);

//			ret = mbedtls_mpi_read_binary
//					(&r, buffer + ind, sig_size/2);
			ret = mbedtls_mpi_read_binary
					(&r, buffer, sig_size/2);
			if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
				printf
				("read of r failed %d\n", ret);
			} else {
//				ret = mbedtls_mpi_read_binary
//					(&s, buffer + ind + sig_size/2,
//					sig_size/2);
				ret = mbedtls_mpi_read_binary
					(&s, buffer + sig_size/2,
					sig_size/2);
				if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
					printf
					("read of s failed %d\n", ret);
				} else {
					ret = mbedtls_ecdsa_verify
						(&mbedTls_ctx.grp,
						hash, HASH_SIZE,
						&mbedTls_ctx.Q, &r, &s);
					if (ret !=
						SECURITY_UTIL_STATUS_SUCCESS) {
						printf
						("verify failed %d\n", ret);
					}
				}
			}
//		}
//		else {
//			printf("%s: wrong buffer format\n", __func__);
//			ret = 1;
//		}
	}

	mbedtls_ecdsa_free(&mbedTls_ctx);
	free(hash);
	mbedtls_mpi_free(&r);
	mbedtls_mpi_free(&s);
	return ret;
}
