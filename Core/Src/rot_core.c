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

/**
 * @brief Verify firmware integrity at specified flash memory slot
 * @param start_address Base address of firmware slot in flash memory (e.g., 0x08100000)
 * @return 0 on verification success, non-zero on verification failure
 */

int ROT_Verify_Slot(uint32_t start_address) {

    printf("[ROT] Checking Slot at 0x%08X...\r\n", start_address);

    // Cast address to pointer for direct flash memory access
    const uint8_t *fw_ptr = (const uint8_t *)start_address;
    const size_t fw_size = 5; // Fixed firmware size for "TRUST" test pattern

    // Calculate SHA-256 hash in real-time from flash content
    uint8_t calculated_hash[32];
    ROT_Crypto_SHA256(fw_ptr, fw_size, calculated_hash);

    // Verify cryptographic signature using mbedTLS RSA-2048
    return ROT_Crypto_VerifySignature(calculated_hash, FIRMWARE_SIGNATURE, ROT_PUBKEY);
}

void ROT_SecureBoot_Sequence(void) {

    printf("\r\n=======================================\r\n");
    printf("[ROT] A/B Dual Image Auto-Recovery\r\n");
    printf("=======================================\r\n");
    HAL_Delay(500);

    // ----------------------------------------------------
    // Primary Boot Attempt: Slot A (Active Firmware)
    // ----------------------------------------------------
    printf("\r\n[ROT] >>> Attempting to Boot from Slot A <<<\r\n");
    if (ROT_Verify_Slot(0x08100000) == 0) {
        printf("[ROT] SUCCESS: Slot A is VALID.\r\n");
        printf("[ROT] Releasing Pi Gate.\r\n");
        HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_SET);
        return; // Boot successful, exit sequence
    }

    // ----------------------------------------------------
    // Fallback Recovery: Slot B (Backup Firmware)
    // ----------------------------------------------------
    printf("\r\n[ROT] WARNING: Slot A Corrupted or Tampered!\r\n");
    printf("[ROT] Initiating Auto-Recovery Procedure...\r\n");
    HAL_Delay(1000); // Simulate hardware state transition

    printf("\r\n[ROT] >>> Attempting to Boot from Slot B (Backup) <<<\r\n");
    if (ROT_Verify_Slot(0x08180000) == 0) {
        printf("[ROT] SUCCESS: Slot B is VALID.\r\n");
        printf("[ROT] System Recovered! Releasing Pi Gate.\r\n");
        HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_SET);
        return; // Recovery successful, boot from backup
    }

    // ----------------------------------------------------
    // Critical Failure: Both Slots Corrupted - System Lockdown
    // ----------------------------------------------------
    printf("\r\n[ROT] FATAL ERROR: Both Slot A and Slot B are Corrupted!\r\n");
    printf("[ROT] System Locked. Contact Administrator.\r\n");
    HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_RESET);

    while(1) {
        // Infinite loop: Maintain system lockdown state
        HAL_Delay(1000);
    }
}
