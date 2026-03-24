#ifndef ROT_CRYPTO_H
#define ROT_CRYPTO_H
#include <stdint.h>
#include <stddef.h>

/**
* @brief Verify digital signature using mbedTLS wrapper
* @param hash   Computed firmware hash (32 bytes)
* @param sig    Digital signature (256 bytes)
* @param pubkey Public key in DER format
* @return 0 = Verification successful (PASS), non-zero = Verification failed (FAIL)
*/

// Digital signature verification interface
int ROT_Crypto_VerifySignature(const uint8_t* hash, const uint8_t* sig, const uint8_t* pubkey);

// SHA-256 hash calculation interface
void ROT_Crypto_SHA256(const uint8_t* start_addr, size_t size, uint8_t* output);

#endif /* ROT_CRYPTO_H */