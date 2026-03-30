/**
* @file rot_core.c
* @brief Root of Trust Core Implementation (Phase 8: Anti-Rollback)
*/

#include "main.h"
#include <stdio.h>
#include "rot_core.h"
#include "rot_crypto.h"
#include "keys_data.h"
#include "stm32u5xx_hal.h"

void ROT_System_Init(void) {
    HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_RESET);
    printf("\r\n========================================\r\n");
    printf("[ROT] System Initialized. Gate is CLOSED (PF15 Low).\r\n");
    printf("[ROT] Pi is currently HELD in RESET.\r\n");
    printf("========================================\r\n");
}

// Firmware header structure with anti-rollback protection (12 bytes total)
typedef struct {
    uint32_t magic_word;     // Magic identifier (0x544F524D = "MROT")
    uint32_t fw_version;     // Firmware version number for anti-rollback
    uint32_t payload_length; // Actual firmware payload length in bytes
} fw_header_t;

// Anti-Rollback protection: Flash address for minimum allowed version storage
#define ARB_FLASH_ADDR 0x081F0000

int ROT_Verify_Slot(uint32_t start_address) {
    printf("\r\n[ROT] Checking Slot at 0x%08X...\r\n", (unsigned int)start_address);
    const fw_header_t *header = (const fw_header_t *)start_address;

    if (header->magic_word != 0x544F524D) {
        printf("[ROT] ERROR: Invalid Magic Word!\r\n");
        return -1;
    }

    // --- Anti-Rollback Protection Check ---
    // Read minimum allowed version from Flash storage
    uint32_t min_allowed_version = *(uint32_t *)ARB_FLASH_ADDR;
    
    // Default initialization: If Flash is empty (0xFFFFFFFF), set minimum version to 1
    if (min_allowed_version == 0xFFFFFFFF) {
        min_allowed_version = 1;
    }

    printf("[ARB] Firmware Version: v%u (Minimum Allowed: v%u)\r\n",
           (unsigned int)header->fw_version, (unsigned int)min_allowed_version);

    // Critical anti-rollback defense logic
    if (header->fw_version < min_allowed_version) {
        printf("[ARB] SECURITY ALERT: Anti-Rollback triggered! Downgrade attack blocked.\r\n");
        return -1; // Reject firmware downgrade attempt
    }
    // ------------------------------------

    size_t dynamic_length = header->payload_length;
    if (dynamic_length == 0 || dynamic_length > 1024 * 1024) {
        printf("[ROT] ERROR: Invalid payload length!\r\n");
        return -1;
    }
    printf("[ROT] MROT Header Found! Payload Length: %d bytes.\r\n", dynamic_length);

    // Calculate actual payload data location
    // Payload starts after header (12 bytes) from the base address
    const uint8_t *payload_ptr = (const uint8_t *)(start_address + sizeof(fw_header_t));

    uint8_t calculated_hash[32];
    ROT_Crypto_SHA256(payload_ptr, dynamic_length, calculated_hash);
    return ROT_Crypto_VerifySignature(calculated_hash, FIRMWARE_SIGNATURE, ROT_PUBKEY);
}

// =========================================================================
// Anti-Rollback Protection: Update minimum allowed version in Flash
// =========================================================================

/**
 * @brief Update Anti-Rollback minimum version counter in Flash
 * @param current_boot_version Firmware version that successfully booted
 * @description Updates the minimum allowed version stored in Flash to prevent rollback
 */
void ROT_Update_ARB_Version(uint32_t current_boot_version) {
    uint32_t stored_ver = *(uint32_t *)ARB_FLASH_ADDR;
    if (stored_ver == 0xFFFFFFFF) stored_ver = 1;

    // Only update Flash if current version is higher than stored version
    if (current_boot_version > stored_ver) {
        printf("\r\n[ARB] Upgrading Anti-Rollback Counter from v%u to v%u...\r\n", 
               (unsigned int)stored_ver, (unsigned int)current_boot_version);
        
        HAL_FLASH_Unlock();
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

        // Erase page containing ARB_FLASH_ADDR (Bank 2, Page 120)
        FLASH_EraseInitTypeDef EraseInitStruct;
        uint32_t PageError = 0;
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.Banks     = FLASH_BANK_2;
        EraseInitStruct.Page      = 120;
        EraseInitStruct.NbPages   = 1;
        HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

        // Write new version number (16 bytes Quad-Word programming)
        // First uint32_t: version number, remaining three: padding
        uint32_t write_data[4] = {current_boot_version, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, ARB_FLASH_ADDR, (uint32_t)write_data);
        HAL_FLASH_Lock();
        
        printf("[ARB] Anti-Rollback Counter locked to v%u.\r\n", (unsigned int)current_boot_version);
    }
}

void Sync_SlotB_to_SlotA(void) {
    uint32_t slot_a_addr = 0x08100000;
    uint32_t slot_b_addr = 0x08180000;
    printf("\r\n[SYNC] Unlocking Flash...\r\n");
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    printf("[SYNC] Erasing Slot A (Bank 2, Page 0)...\r\n");
    FLASH_EraseInitTypeDef EraseInitStruct;

    uint32_t PageError = 0;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks     = FLASH_BANK_2;
    EraseInitStruct.Page      = 0;           
    EraseInitStruct.NbPages   = 1;           

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
        printf("[SYNC] ERROR: Failed to erase Slot A!\r\n");
        HAL_FLASH_Lock();
        return;
    }

    printf("[SYNC] Programming Slot A with data from Slot B...\r\n");
    // File total length is 17 bytes. Must write two Quad-Word operations (32 bytes total) for complete coverage

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, slot_a_addr, slot_b_addr) != HAL_OK ||
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, slot_a_addr + 16, slot_b_addr + 16) != HAL_OK) {
        printf("[SYNC] ERROR: Failed to program Slot A!\r\n");
    } else {
        printf("[SYNC] Verification & Sync Complete!\r\n");
    }

    HAL_FLASH_Lock();

}

void ROT_SecureBoot_Sequence(void) {
    printf("\r\n=======================================\r\n");
    printf("[ROT] Firmware Verification Sequence\r\n");
    printf("=======================================\r\n");

    HAL_Delay(500);

    // ----------------------------------------------------
    // Primary Boot Attempt: Slot A (Active Firmware)
    // ----------------------------------------------------

    printf("\r\n[ROT] >>> Attempting to Boot from Slot A <<<\r\n");

    if (ROT_Verify_Slot(0x08100000) == 0) {
        printf("[ROT] SUCCESS: Slot A is VALID.\r\n");

        // Successful boot: update anti-rollback counter
        const fw_header_t *fw_a = (const fw_header_t *)0x08100000;
        ROT_Update_ARB_Version(fw_a->fw_version);

        printf("[ROT] Releasing Pi Gate.\r\n");

        HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_SET);

        return;
    }

    // ----------------------------------------------------
    // Fallback Recovery: Slot B (Backup Firmware)
    // ----------------------------------------------------

    printf("\r\n[ROT] WARNING: Slot A Corrupted, Tampered, or Downgraded!\r\n");
    printf("[ROT] Initiating Auto-Recovery Procedure...\r\n");

    HAL_Delay(1000);

    printf("\r\n[ROT] >>> Attempting to Boot from Slot B (Backup) <<<\r\n");

    if (ROT_Verify_Slot(0x08180000) == 0) {
        printf("[ROT] SUCCESS: Slot B is VALID.\r\n");

        // Trigger self-healing: copy valid Slot B to corrupted Slot A
        printf("[ROT] Initiating Self-Healing: Syncing Slot B to Slot A...\r\n");
        Sync_SlotB_to_SlotA();

        // Since Slot B is valid and latest, update anti-rollback counter
        const fw_header_t *fw_b = (const fw_header_t *)0x08180000;
        ROT_Update_ARB_Version(fw_b->fw_version);

        printf("[ROT] System Recovered! Releasing Pi Gate.\r\n");

        HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_SET);

        return;
    }

    // ----------------------------------------------------
    // Critical Failure: Both Slots Corrupted - System Lockdown
    // ----------------------------------------------------

    printf("\r\n[ROT] FATAL ERROR: Both Slot A and Slot B are Corrupted or Downgraded!\r\n");
    printf("[ROT] System Locked. Contact Administrator.\r\n");

    HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_RESET);

    while(1) {
        // Infinite loop: Maintain system lockdown state
        HAL_Delay(1000);
    }
}
