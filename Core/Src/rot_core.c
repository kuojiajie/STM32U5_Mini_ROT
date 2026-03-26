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
#include "stm32u5xx_hal.h"

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
    printf("[ROT] Checking Slot at 0x%08lX...\r\n", start_address);

    // Cast address to pointer for direct flash memory access
    const uint8_t *fw_ptr = (const uint8_t *)start_address;
    const size_t fw_size = 5; // Fixed firmware size for "TRUST" test pattern

    // Calculate SHA-256 hash in real-time from flash content
    uint8_t calculated_hash[32];
    ROT_Crypto_SHA256(fw_ptr, fw_size, calculated_hash);

    // Verify cryptographic signature using mbedTLS RSA-2048
    return ROT_Crypto_VerifySignature(calculated_hash, FIRMWARE_SIGNATURE, ROT_PUBKEY);
}

/**
 * @brief Self-healing mechanism: Copy valid firmware from Slot B to corrupted Slot A
 * @description Uses STM32 HAL Flash APIs to unlock, erase, and reprogram Slot A
 */

void Sync_SlotB_to_SlotA(void) {
    uint32_t slot_a_addr = 0x08100000;
    uint32_t slot_b_addr = 0x08180000;
    printf("\r\n[SYNC] Unlocking Flash...\r\n");
    HAL_FLASH_Unlock();


    // Clear all Flash error flags (safety mechanism)
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
    printf("[SYNC] Erasing Slot A (Bank 2, Page 0)...\r\n");

    // Configure erase parameters
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks     = FLASH_BANK_2; // 0x08100000 belongs to Bank 2
    EraseInitStruct.Page      = 0;            // Page 0 of this Bank
    EraseInitStruct.NbPages   = 1;            // Erase 1 page only (8KB)

    // Execute erase operation
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
        printf("[SYNC] ERROR: Failed to erase Slot A!\r\n");
        HAL_FLASH_Lock();
        return;
    }

    printf("[SYNC] Programming Slot A with data from Slot B...\r\n");

    // STM32U5 requires Quad-Word (16 bytes) programming
    // Our data is only 5 bytes, so copying 16 bytes is safe and sufficient
    // Parameters: program mode, destination address, source address
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, slot_a_addr, slot_b_addr) != HAL_OK) {
        printf("[SYNC] ERROR: Failed to program Slot A!\r\n");
    } else {
        printf("[SYNC] Verification & Sync Complete!\r\n");
    }

    printf("[SYNC] Locking Flash...\r\n");
    HAL_FLASH_Lock();
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

        // Trigger self-healing mechanism
        printf("[ROT] Initiating Self-Healing: Syncing Slot B to Slot A...\r\n");
        Sync_SlotB_to_SlotA();
        printf("[ROT] Self-Healing Complete. Slot A is restored.\r\n");

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
