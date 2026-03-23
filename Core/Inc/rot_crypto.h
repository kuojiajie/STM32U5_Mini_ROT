#ifndef ROT_CRYPTO_H
#define ROT_CRYPTO_H
#include <stdint.h>

/**
* @brief 驗證數位簽章 (mbedTLS Wrapper)
* @param hash   計算出的韌體雜湊 (32 bytes)
* @param sig    數位簽章 (256 bytes)
* @param pubkey 公鑰 (DER format)
* @return 0 = 驗證成功 (PASS), 非0 = 驗證失敗 (FAIL)
*/

int ROT_Crypto_VerifySignature(const uint8_t* hash, const uint8_t* sig, const uint8_t* pubkey);

#endif /* ROT_CRYPTO_H */