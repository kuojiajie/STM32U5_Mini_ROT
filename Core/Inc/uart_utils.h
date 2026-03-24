/**
 * @file uart_utils.h
 * @brief UART Communication Utilities Interface
 * @description Printf redirection API for debug output via UART
 */

#ifndef INC_UART_UTILS_H_
#define INC_UART_UTILS_H_

#include <stdio.h>

// Write function prototype for printf redirection
int _write(int file, char *ptr, int len);

#endif /* INC_UART_UTILS_H_ */
