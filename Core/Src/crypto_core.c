/**
* @file crypto_core.c
* @brief Real implementation of cryptographic verification using mbedTLS
*/

#include "rot_crypto.h"
#include <stdio.h>
#include <string.h>

// 1. 補齊所有必要的 mbedTLS 標頭檔
#include "mbedtls/sha256.h"
#include "mbedtls/rsa.h"
#include "mbedtls/pk.h"
#include "mbedtls/md.h"
#include "mbedtls/error.h"

// 2. 明確宣告外部變數 (這樣編譯器才找得到)
extern const uint32_t ROT_PUBKEY_LEN;

int ROT_Crypto_VerifySignature(const uint8_t* hash, const uint8_t* sig, const uint8_t* pubkey) {

    printf("\r\n[CRYPTO] Starting mbedTLS Verification...\r\n");

    int ret = 0;
    mbedtls_pk_context pk;

    // 初始化 PK Context
    mbedtls_pk_init(&pk);

    // 解析公鑰
    printf("[CRYPTO] Parsing Public Key... ");
    ret = mbedtls_pk_parse_public_key(&pk, pubkey, ROT_PUBKEY_LEN);
    if (ret != 0) {
        printf("FAIL! (Key Parse Error: -0x%04X)\r\n", -ret);
        goto exit;
    }

    printf("OK\r\n");

    // 驗證簽章
    printf("[CRYPTO] Verifying Signature... ");

    // 關鍵修正：這裡傳入 0 讓 mbedTLS 自動判斷 Hash 長度
    // 這是最標準的用法
    ret = mbedtls_pk_verify(&pk, MBEDTLS_MD_SHA256, hash, 0, sig, 256);

    if (ret != 0) {
        // 如果還是失敗，我們會印出具體錯誤碼，這次不會有編譯錯誤了
        printf("FAIL! (Verify Error: -0x%04X)\r\n", -ret);
    } else {
        printf("PASS! (Signature Valid)\r\n");
    }

exit:
    mbedtls_pk_free(&pk);
    return ret;
}
