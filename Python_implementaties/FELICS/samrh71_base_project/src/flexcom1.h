#ifndef INC_FLEXCOM1_H
#define INC_FLEXCOM1_H

#include "FreeRTOS.h"

void init_usart1(void);
int usart1_getchar(TickType_t timeout);
void usart1_putchar(char c, TickType_t timeout);

extern unsigned int cntr_tx_ok;
extern unsigned int cntr_tx_q_empty;
extern unsigned int cntr_rx_q_full;
extern unsigned int cntr_rx_ok;

#endif
