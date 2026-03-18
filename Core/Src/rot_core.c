/**
 * @file rot_core.c
 * @brief Root of Trust Core Implementation
 * @description Hardware-level secure boot control and verification logic
 */

#include "rot_core.h"
#include "main.h"
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
    printf("[ROT] Starting Secure Boot Sequence...\r\n");

    // 1. 倒數 5 秒 (模擬驗證時間)
    for(int i=5; i>0; i--) {
        printf("[ROT] Verifying Firmware... (Holding Reset: %d)\r\n", i);

        HAL_Delay(1000);
    }

    // 2. 解鎖 (Release Reset)
    printf("[ROT] Verification PASS. Releasing the Gate!\r\n");

    // Open Drain High = Float = Pi 內部上拉生效 = 開機
    HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_SET);

    // 3. 監控
    printf("[ROT] Target System (Pi) should be booting now.\r\n");

    while(1) {
        HAL_Delay(2000);

        printf("[ROT] Status: Monitoring Target System...\r\n");
    }
}
