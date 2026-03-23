#ifndef ROT_CRYPTO_H
#define ROT_CRYPTO_H
#include <stdint.h>

/**
* @brief Verify digital signature using mbedTLS wrapper
* @param hash   Computed firmware hash (32 bytes)
* @param sig    Digital signature (256 bytes)
* @param pubkey Public key in DER format
* @return 0 = Verification successful (PASS), non-zero = Verification failed (FAIL)
*/

int ROT_Crypto_VerifySignature(const uint8_t* hash, const uint8_t* sig, const uint8_t* pubkey);

#endif /* ROT_CRYPTO_H */