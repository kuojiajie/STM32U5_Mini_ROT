/**
 * @file rot_core.h
 * @brief Root of Trust Core Interface
 * @description Hardware-level secure boot control and verification API
 */

#ifndef INC_ROT_CORE_H_
#define INC_ROT_CORE_H_

// Initialize system state (hold Pi in reset)
void ROT_System_Init(void);
// Execute secure boot sequence (countdown -> release -> monitor)
void ROT_SecureBoot_Sequence(void);

#endif /* INC_ROT_CORE_H_ */
