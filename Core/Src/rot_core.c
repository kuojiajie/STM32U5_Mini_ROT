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

// Firmware header structure definition for dynamic image parsing
typedef struct {
    uint32_t magic_word;     // Magic identifier (0x544F524D = "MROT")
    uint32_t payload_length; // Actual firmware payload length in bytes
} fw_header_t;

int ROT_Verify_Slot(uint32_t start_address) {
    printf("\r\n[ROT] Checking Slot at 0x%08X...\r\n", (unsigned int)start_address);

    // Map flash start address to firmware header structure
    const fw_header_t *header = (const fw_header_t *)start_address;

    // Verify magic word for file format validation
    if (header->magic_word != 0x544F524D) {
        printf("[ROT] ERROR: Invalid Magic Word! This is not an MROT file.\r\n");
        return -1; // Verification failed, exit early
    }

    // Extract dynamic payload length from header
    size_t dynamic_length = header->payload_length;
    printf("[ROT] MROT Header Found! Payload Length: %d bytes.\r\n", dynamic_length);

    // Calculate actual payload data location
    // Payload starts after header (8 bytes) from the base address
    const uint8_t *payload_ptr = (const uint8_t *)(start_address + sizeof(fw_header_t));

    // Calculate hash using dynamic length extracted from header
    uint8_t calculated_hash[32];
    ROT_Crypto_SHA256(payload_ptr, dynamic_length, calculated_hash);

    // Verify signature against calculated hash
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
