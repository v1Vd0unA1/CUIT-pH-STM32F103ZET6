/*
 * app_task.h
 *
 *  Created on: Apr 18, 2026
 *      Author: 17224
 */

#ifndef BSP_APP_APP_TASK_H_
#define BSP_APP_APP_TASK_H_


#include "stm32f1xx_hal.h"

void APP_UserInit(void);
void APP_TaskPoll(void);
void APP_Task1s(void);
void APP_UART_IRQHandler(void);



#endif /* BSP_APP_APP_TASK_H_ */
