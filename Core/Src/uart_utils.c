/**
 * @file uart_utils.c
 * @brief UART Communication Utilities
 * @description Printf redirection for debug output via UART
 */

#include "uart_utils.h"
#include "main.h"

// External UART handle reference (defined in main.c by STM32CubeMX)
extern UART_HandleTypeDef huart1;

int _write(int file, char *ptr, int len) {
    // Redirect printf output to UART1
    // Timeout: 100ms to prevent system deadlock
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, 100);

    return len;
}
