/**
 * @file rot_core.c
 * @brief Root of Trust Core Implementation
 * @description Hardware-level secure boot control and verification logic
 */

#include "main.h"
#include <stdio.h>
#include "rot_core.h"
#include "rot_crypto.h"
#include "keys_data.h"

void ROT_System_Init(void) {
    // Logging only - GPIO initialization handled in main.c
    // Double assurance: ensure reset line remains low
    HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_RESET);

    printf("\r\n========================================\r\n");
    printf("[ROT] System Initialized. Gate is CLOSED (PF15 Low).\r\n");
    printf("[ROT] Pi is currently HELD in RESET.\r\n");
    printf("========================================\r\n");
}

void ROT_SecureBoot_Sequence(void) {

    printf("\r\n[ROT] Real Firmware Verification...\r\n");
    HAL_Delay(1000);

    // 1. Define firmware location in Flash memory
    // Point to firmware image programmed at 0x08100000
    const uint8_t *FW_PTR = (const uint8_t *)0x08100000;
    const size_t FW_SIZE = 5; // "TRUST" firmware consists of 5 characters

    // 2. Calculate hash in real-time from Flash content
    uint8_t calculated_hash[32];
    ROT_Crypto_SHA256(FW_PTR, FW_SIZE, calculated_hash);

    // 3. Verify signature using calculated hash (not hardcoded)
    int result = ROT_Crypto_VerifySignature(calculated_hash, FIRMWARE_SIGNATURE, ROT_PUBKEY);

    // 4. Security decision based on verification result
    if (result == 0) {
        printf("[ROT] PASS. Releasing Pi.\r\n");
        HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_SET);
    } else {
        printf("[ROT] FAIL. System Locked.\r\n");
        HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_RESET);
        while(1) { HAL_Delay(1000); }
    }
}
