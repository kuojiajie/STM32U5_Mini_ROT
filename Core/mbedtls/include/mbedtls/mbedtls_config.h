#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

// 1. 系統設定：移除 OS 依賴
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES


// 2. 演算法開關
#define MBEDTLS_RSA_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_MD_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_OID_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C

// 3. 必要參數

#define MBEDTLS_PKCS1_V15

// 4. 記憶體優化
#define MBEDTLS_MPI_WINDOW_SIZE  1
#define MBEDTLS_MPI_MAX_SIZE     256 // 2048-bit RSA

#include "mbedtls/check_config.h"
#endif