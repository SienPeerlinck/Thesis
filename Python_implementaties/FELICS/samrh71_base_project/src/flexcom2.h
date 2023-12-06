#ifndef INC_FLEXCOM2_H
#define INC_FLEXCOM2_H

#include "FreeRTOS.h"

void init_usart2(void);
int usart2_getchar(TickType_t timeout);
void usart2_putchar(char c, TickType_t timeout);

#endif
