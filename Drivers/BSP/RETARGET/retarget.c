/*
 * retarget.c
 *
 *  Created on: Apr 18, 2026
 *      Author: 17224
 */

#include "usart.h"
#include <stdio.h>

#ifdef __GNUC__
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif
{
    uint8_t c = (uint8_t)ch;
    HAL_UART_Transmit(&huart1, &c, 1, 10);
    return ch;
}
