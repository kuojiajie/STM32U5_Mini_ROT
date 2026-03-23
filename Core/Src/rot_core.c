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
    printf("\r\n[ROT] Phase 2: Starting Secure Boot Sequence...\r\n");

    // 1. 保持鎖定
    printf("[ROT] System LOCKED. Verifying Signature...\r\n");
    HAL_Delay(1000); // 給 UART 一點緩衝時間

    // 2. 呼叫 mbedTLS 驗證 (這是關鍵！)
    // 我們傳入 keys_data.h 裡的數據
    int result = ROT_Crypto_VerifySignature(FIRMWARE_HASH, FIRMWARE_SIGNATURE, ROT_PUBKEY);

    // 3. 根據結果決定命運
    if (result == 0) {
        // --- PASS: 驗證成功 ---
        printf("[ROT] PASS! Signature is Valid.\r\n");
        printf("[ROT] Releasing the Gate... Pi Booting.\r\n");

        // 放行 (Open Drain High)
        HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_SET);
    } else {
        // --- FAIL: 驗證失敗 ---
        printf("[ROT] FAIL! Invalid Signature (Error: -0x%04X)\r\n", -result);
        printf("[ROT] SYSTEM HALTED. Pi will NOT boot.\r\n");

        // 永遠鎖死 (保持 Low)
        HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_RESET);

        // 死迴圈警告
        while(1) {
            HAL_Delay(1000);
            printf("[ROT] ALERT: Security Breach!\r\n");
        }
    }

    // 監控模式 (只有 PASS 才會跑到這裡)
    while(1) {
        HAL_Delay(5000);
        printf("[ROT] Status: System Running Securely.\r\n");
    }
}
