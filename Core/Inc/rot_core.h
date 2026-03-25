/**
 * @file rot_core.h
 * @brief Root of Trust Core Interface
 * @description Hardware-level secure boot control and verification API
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
 */
int ROT_Verify_Slot(uint32_t start_address);

/**
 * @brief Execute A/B dual-image secure boot sequence with auto-recovery
 * @description Implements failsafe boot logic: Slot A -> Slot B -> System Lock
 */
void ROT_SecureBoot_Sequence(void);

#endif /* INC_ROT_CORE_H_ */
