#ifndef INC_TC_TM_H
#define INC_TC_TM_H

#include "FreeRTOS.h"

void init_telecommand(void);
void tc_task(void *pvParameters);

#endif
