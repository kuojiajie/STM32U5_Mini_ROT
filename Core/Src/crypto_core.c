/**
* @file crypto_core.c
* @brief Real implementation of cryptographic verification using mbedTLS
*/

#include "rot_crypto.h"
#include <stdio.h>
#include <string.h>

// Include required mbedTLS headers for cryptographic operations
#include "mbedtls/sha256.h"
#include "mbedtls/rsa.h"
#include "mbedtls/pk.h"
#include "mbedtls/md.h"
#include "mbedtls/error.h"

// External variable declarations for compiler visibility
extern const uint32_t ROT_PUBKEY_LEN;

int ROT_Crypto_VerifySignature(const uint8_t* hash, const uint8_t* sig, const uint8_t* pubkey) {

    printf("\r\n[CRYPTO] Starting mbedTLS Verification...\r\n");

    int ret = 0;
    mbedtls_pk_context pk;

    // Initialize PK context for public key operations
    mbedtls_pk_init(&pk);

    // Parse DER-formatted public key
    printf("[CRYPTO] Parsing Public Key... ");
    ret = mbedtls_pk_parse_public_key(&pk, pubkey, ROT_PUBKEY_LEN);
    if (ret != 0) {
        printf("FAIL! (Key Parse Error: -0x%04X)\r\n", -ret);
        goto exit;
    }

    printf("OK\r\n");

    // Verify RSA signature against SHA-256 hash
    printf("[CRYPTO] Verifying Signature... ");

    // Pass 0 for hash length to let mbedTLS auto-detect
    ret = mbedtls_pk_verify(&pk, MBEDTLS_MD_SHA256, hash, 0, sig, 256);

    if (ret != 0) {
        // Display specific error code for debugging
        printf("FAIL! (Verify Error: -0x%04X)\r\n", -ret);
    } else {
        printf("PASS! (Signature Valid)\r\n");
    }

exit:
    mbedtls_pk_free(&pk);
    return ret;
}
