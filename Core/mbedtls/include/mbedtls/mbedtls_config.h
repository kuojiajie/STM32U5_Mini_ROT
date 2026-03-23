#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

// 1. System configuration: Remove OS dependencies
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES


// 2. Algorithm configuration
#define MBEDTLS_RSA_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_MD_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_OID_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C

// 3. Required parameters

#define MBEDTLS_PKCS1_V15

// 4. Memory optimization for embedded systems
#define MBEDTLS_MPI_WINDOW_SIZE  1
#define MBEDTLS_MPI_MAX_SIZE     256 // Maximum size for 2048-bit RSA

#include "mbedtls/check_config.h"
#endif