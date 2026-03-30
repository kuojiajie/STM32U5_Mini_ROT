/**
 * @file rot_core.h
 * @brief Root of Trust Core Interface (Phase 8: Anti-Rollback)
 * @description Hardware-level secure boot control and verification API with anti-rollback protection
 */

#ifndef INC_ROT_CORE_H_
#define INC_ROT_CORE_H_

/**
 * @brief Initialize Root of Trust system and secure the target device
 * @description Configures GPIO to hold target system in reset state during verification
 */
void ROT_System_Init(void);

/**
 * @brief Verify firmware integrity at specified flash memory slot
 * @param start_address Base address of firmware slot in flash memory
 * @return 0 on verification success, non-zero on verification failure
 * @description Performs cryptographic verification including anti-rollback check
 */
int ROT_Verify_Slot(uint32_t);

/**
 * @brief Update Anti-Rollback minimum version counter in Flash
 * @param current_boot_version Firmware version that successfully booted
 * @description Updates the minimum allowed version stored in Flash to prevent rollback attacks
 */
void ROT_Update_ARB_Version(uint32_t);

/**
 * @brief Synchronize valid firmware from Slot B to corrupted Slot A
 * @description Self-healing mechanism using STM32 HAL Flash APIs for recovery
 */
void Sync_SlotB_to_SlotA(void);

/**
 * @brief Execute A/B dual-image secure boot sequence with auto-recovery
 * @description Implements failsafe boot logic: Slot A -> Slot B -> System Lock
 * @description Includes anti-rollback protection and automatic version counter updates
 */
void ROT_SecureBoot_Sequence(void);

#endif /* INC_ROT_CORE_H_ */
