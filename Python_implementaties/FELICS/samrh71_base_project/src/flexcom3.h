#ifndef INC_FLEXCOM3_H
#define INC_FLEXCOM3_H

#include "FreeRTOS.h"

void init_spi3(void);
void spi3_transmit (char* buf, int n);
int spi3_tx_remaining_bytes(void);

#endif
