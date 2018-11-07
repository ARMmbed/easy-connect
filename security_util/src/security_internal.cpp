
#include "security_internal.h"

#include "mbedtls/sha256.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/cipher.h"

#include <stdio.h>
#include <stdlib.h>
 #include <string.h>
#ifndef	__NO_ASSERT_H__		/* Include assert.h only for internal use. */
#include <assert.h>		/* for assert() macro */
#endif

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

	while (len > 0) {
		use_len = len;
		if (use_len > sizeof(int))
			use_len = sizeof(int);

		/* coverity[dont_call] */
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
	security_int_params_t	*params)
{
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
/*
 * Pay attention:
 * according to mbedTLS rules, public key MUST start from this byte:
 * BER_OCTET_STRING tag 0x4
 */
	uint32_t public_size =
		params->remote_public_key_size + 1;
	uint8_t *buff = (uint8_t *)calloc(public_size, sizeof(uint8_t));

	assert(buff != NULL);
	buff[0] = 0x04; // adding BER_OCTET_STRING tag
	memcpy(buff + 1,
		params->remote_public_key,
		params->remote_public_key_size);

	mbedtls_ecdsa_init(ctx);
	if (params->security_grp_id == security_group_id_ecp_dp_256r1)
		ctx->grp.id = MBEDTLS_ECP_DP_SECP256R1;
	assert(mbedtls_ecp_group_load(&ctx->grp, ctx->grp.id) == 0);


	ret = mbedtls_ecp_point_read_binary(&ctx->grp, &ctx->Q,
						buff, public_size);
	free(buff);

	if (ret == 0) {
		ret = mbedtls_mpi_read_binary(&ctx->d,
			params->local_private_key,
			params->local_private_key_size);

	}
	return ret;
}

int32_t compute_hash_256(uint8_t *message, uint32_t	message_size,
	uint8_t *hash)
{
	return mbedtls_sha256_ret(message, message_size, hash, 0);
}

int32_t compute_hash_384(uint8_t *message, uint32_t	message_size,
	uint8_t *hash)
{
//	return mbedtls_sha384_ret(message, message_size, hash, 0);
	return SECURITY_UTIL_STATUS_FAILURE;
}

int32_t ECDSA_Sign(security_int_params_t	*params,
					uint8_t		*buffer,
					uint32_t	*buffer_size,
					uint8_t		*hash,
					uint32_t	hash_size)
{
//	printf("%s\n", __func__);
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	mbedtls_ecdsa_context mbedTls_ctx;
	uint8_t  *sig = NULL;
	size_t sig_len, len_r, len_s, max_half;
	mbedtls_mpi r, s;
	int i = 0;

	mbedtls_mpi_init(&r);
	mbedtls_mpi_init(&s);

	ret = init_mbedtls_context(&mbedTls_ctx, params);
	if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
		printf(" failed! init_mbedtls_context returned %d\n", (int)ret);
	} else {
		ret = mbedtls_ecdsa_sign
			(&mbedTls_ctx.grp, &r, &s, &mbedTls_ctx.d,
			hash, hash_size, myrand, NULL);

		len_r = mbedtls_mpi_size(&r);
		len_s = mbedtls_mpi_size(&s);
//		printf("%s: len_r=%d, len_s=%d\n",
//			__func__, (int)len_r, (int)len_s);
//		assert(len_r == len_s);
		assert((len_r <= HALF_SIG_SIZE) && (len_r > 0));
		assert((len_s <= HALF_SIG_SIZE) && (len_s > 0));

		max_half = (len_r > len_s) ? len_r : len_s;
		sig_len = 2 * max_half;

		assert(sig_len > 0);
		sig = (uint8_t *)calloc(sig_len, sizeof(uint8_t));
		assert(sig != NULL);

		if (len_r < max_half) {
			for (i = 0; i < max_half-len_r; i++)
				sig[i] = 0;
		}
		mbedtls_mpi_write_binary(&r, sig+max_half-len_r, len_r);

		if (len_s < max_half) {
			for (i = max_half; i < sig_len-len_s; i++)
				sig[i] = 0;
		}
		mbedtls_mpi_write_binary(&s, sig+sig_len-len_s, len_s);

		if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
			printf
			("mbedtls_ecdsa_write_signature returned %d\n", (int)ret);
		} else {
			memcpy(buffer, sig, sig_len);
			*buffer_size = sig_len;
		}
		free(sig);
	}

	mbedtls_ecdsa_free(&mbedTls_ctx);
	mbedtls_mpi_free(&r);
	mbedtls_mpi_free(&s);
	return ret;
}

int32_t ECDSA_Verify(security_int_params_t	*params,
					uint8_t		*buffer,
					uint32_t	buffer_size,
					uint8_t		*hash,
					uint32_t	hash_size)
{
//	printf("%s\n", __func__);
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	mbedtls_ecdsa_context mbedTls_ctx;
	mbedtls_mpi r, s;

	mbedtls_mpi_init(&r);
	mbedtls_mpi_init(&s);

	ret = init_mbedtls_context(&mbedTls_ctx, params);

	if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
		printf(" failed! init_mbedtls_context returned %d\n", (int)ret);
	} else {
		size_t sig_size;

		assert(buffer_size > 2);
		assert(buffer != NULL);
		sig_size = buffer_size;
//		printf("buffer_size %d, sig_size=%d\n",
//			(int)buffer_size, (int)sig_size);

		ret = mbedtls_mpi_read_binary
					(&r, buffer, sig_size/2);
		if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
			printf
				("read of r failed %d\n", (int)ret);
		} else {
			ret = mbedtls_mpi_read_binary
					(&s, buffer + sig_size/2,
					sig_size/2);
			if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
				printf
					("read of s failed %d\n", (int)ret);
			} else {
				ret = mbedtls_ecdsa_verify
						(&mbedTls_ctx.grp,
						hash, hash_size,
						&mbedTls_ctx.Q, &r, &s);

				if (ret !=
					SECURITY_UTIL_STATUS_SUCCESS) {
					printf("verify failed %d\n", (int)ret);
				}
			}
		}
	}

	mbedtls_ecdsa_free(&mbedTls_ctx);
	mbedtls_mpi_free(&r);
	mbedtls_mpi_free(&s);
	return ret;
}

int32_t ECDH_compute_Z(security_int_params_t *params,
	uint8_t *z, uint32_t *z_len)
{
	mbedtls_ecdh_context ecdh;
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;

	printf("%s\n", __func__);

	mbedtls_ecdh_init(&ecdh);

	if (params->security_grp_id == security_group_id_ecp_dp_256r1)
		ecdh.grp.id = MBEDTLS_ECP_DP_SECP256R1;
	assert(mbedtls_ecp_group_load(&ecdh.grp, ecdh.grp.id) == 0);

/*
 * Pay attention:
 * according to mbedTLS rules, public key MUST start from this byte:
 * BER_OCTET_STRING tag 0x4
 */
	uint32_t public_size = params->remote_public_key_size + 1;
	uint8_t *buff = (uint8_t *)calloc(public_size, sizeof(uint8_t));

	assert(buff != NULL);
	buff[0] = 0x04; // adding BER_OCTET_STRING tag
	memcpy(buff + 1,
		params->remote_public_key,
		params->remote_public_key_size);

	ret = mbedtls_ecp_point_read_binary(&ecdh.grp, &ecdh.Qp,
						buff, public_size);

	free(buff);

	if (ret == 0) {
		ret = mbedtls_mpi_read_binary(&ecdh.d,
			params->local_private_key,
			params->local_private_key_size);

		if (ret == 0) {
			size_t actual_size;

			ret = mbedtls_ecdh_calc_secret(&ecdh, &actual_size, z,
				*z_len, myrand, NULL);
			if (ret == 0)
				*z_len = actual_size;
		}
	}

	mbedtls_ecdh_free(&ecdh);
	return ret;
}


int32_t cipher_auth_encrypt(crypto_int_params_t *params,
		uint8_t *plain_buffer, uint32_t plain_buffer_size,
		uint8_t *cipher_buffer, uint32_t *cipher_buffer_size)
{
	mbedtls_cipher_context_t ctx;
	size_t output_size = 0;
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	const mbedtls_cipher_info_t *cipher_info;
	uint32_t key_bit_size;

	assert(params);
	assert(cipher_buffer != NULL);
	assert(plain_buffer != NULL && plain_buffer_size > 0);

	printf("%s\n", __func__);

	key_bit_size = params->key_size << 3;
	// Setup AES-GCM contex
	mbedtls_cipher_init(&ctx);
	cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_GCM);
	if (cipher_info == NULL)
		return SECURITY_UTIL_STATUS_FAILURE;

	ret = mbedtls_cipher_setup(&ctx, cipher_info);
	if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
		printf("mbedtls_cipher_setup() returned -0x%04X\r\n", -ret);
		return SECURITY_UTIL_STATUS_FAILURE;
	}

	ret = mbedtls_cipher_setkey(&ctx,
		params->session_key, key_bit_size, MBEDTLS_ENCRYPT);
	if (ret != 0) {
		printf("mbedtls_cipher_setkey() returned -0x%04X\r\n", -ret);
		return SECURITY_UTIL_STATUS_FAILURE;
	}

	ret = mbedtls_cipher_auth_encrypt(&ctx, params->iv, params->iv_size,
		params->add_data, params->add_size,
		plain_buffer, plain_buffer_size, cipher_buffer, &output_size,
		cipher_buffer + plain_buffer_size, AUTH_TAG_SIZE);

	if (ret != 0) {
		printf("mbedtls_cipher_auth_encrypt returned -0x%04X\r\n",
			-ret);
		return SECURITY_UTIL_STATUS_FAILURE;
	}

	assert(output_size == plain_buffer_size);
	*cipher_buffer_size = output_size + AUTH_TAG_SIZE;

	mbedtls_cipher_free(&ctx);
	return ret;
}

int32_t cipher_auth_decrypt(crypto_int_params_t *params,
		uint8_t *cipher_buffer, uint32_t cipher_buffer_size,
		uint8_t *plain_buffer, uint32_t *plain_buffer_size)
{
	mbedtls_cipher_context_t ctx;
	size_t cipher_size = 0;
	size_t output_size = 0;
	int32_t ret = SECURITY_UTIL_STATUS_SUCCESS;
	const mbedtls_cipher_info_t *cipher_info;
	uint32_t key_bit_size;

	assert(params);
	assert(cipher_buffer != NULL && cipher_buffer_size > 0);
	assert(plain_buffer != NULL);

	printf("%s\n", __func__);

	cipher_size = cipher_buffer_size - AUTH_TAG_SIZE;
	key_bit_size = params->key_size << 3;
	// Setup AES-GCM contex
	mbedtls_cipher_init(&ctx);
	cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_GCM);
	if (cipher_info == NULL) {
		mbedtls_cipher_free(&ctx);
		return SECURITY_UTIL_STATUS_FAILURE;
	}

	ret = mbedtls_cipher_setup(&ctx, cipher_info);
	if (ret != SECURITY_UTIL_STATUS_SUCCESS) {
		printf("mbedtls_cipher_setup() returned -0x%04X\r\n", -ret);
		mbedtls_cipher_free(&ctx);
		return SECURITY_UTIL_STATUS_FAILURE;
	}

	ret = mbedtls_cipher_setkey(&ctx,
		params->session_key, key_bit_size, MBEDTLS_ENCRYPT);
	if (ret != 0) {
		printf("mbedtls_cipher_setkey() returned -0x%04X\r\n", -ret);
		mbedtls_cipher_free(&ctx);
		return SECURITY_UTIL_STATUS_FAILURE;
	}

	ret = mbedtls_cipher_auth_decrypt(&ctx, params->iv, params->iv_size,
		params->add_data, params->add_size,
		cipher_buffer, cipher_buffer_size - AUTH_TAG_SIZE,
		plain_buffer, &output_size,
		cipher_buffer + cipher_size, AUTH_TAG_SIZE);

	if (ret != 0) {
		printf("mbedtls_cipher_auth_decrypt returned -0x%04X\r\n",
			-ret);
		mbedtls_cipher_free(&ctx);
		return SECURITY_UTIL_STATUS_FAILURE;
	}

	assert(output_size == cipher_size);
	*plain_buffer_size = cipher_size;

	mbedtls_cipher_free(&ctx);
	return ret;
}
