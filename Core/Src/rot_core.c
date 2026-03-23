/**
 * @file rot_core.c
 * @brief Root of Trust Core Implementation
 * @description Hardware-level secure boot control and verification logic
 */

#include "rot_core.h"
#include "main.h"
#include "rot_crypto.h"
#include "keys_data.h"
#include <stdio.h>

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
    printf("\r\n[ROT] Starting Secure Boot Sequence...\r\n");

    // Maintain system lockdown during verification
    printf("[ROT] System LOCKED. Verifying Signature...\r\n");
    HAL_Delay(1000); // Allow UART buffer to settle

    // Execute mbedTLS signature verification (critical security step)
    // Pass hardcoded cryptographic data from keys_data.h
    int result = ROT_Crypto_VerifySignature(FIRMWARE_HASH, FIRMWARE_SIGNATURE, ROT_PUBKEY);

    // Make security decision based on verification result
    if (result == 0) {
        // --- PASS: Signature verification successful ---
        printf("[ROT] PASS! Signature is Valid.\r\n");
        printf("[ROT] Releasing the Gate... Pi Booting.\r\n");

        // Release target system (Open Drain High = Float)
        HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_SET);
    } else {
        // --- FAIL: Signature verification failed ---
        printf("[ROT] FAIL! Invalid Signature (Error: -0x%04X)\r\n", -result);
        printf("[ROT] SYSTEM HALTED. Pi will NOT boot.\r\n");

        // Permanent lockdown (maintain reset line low)
        HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_RESET);

        // Security breach alert loop
        while(1) {
            HAL_Delay(1000);
            printf("[ROT] ALERT: Security Breach!\r\n");
        }
    }

    // Monitoring mode (only reached on successful verification)
    while(1) {
        HAL_Delay(5000);
        printf("[ROT] Status: System Running Securely.\r\n");
    }
}
