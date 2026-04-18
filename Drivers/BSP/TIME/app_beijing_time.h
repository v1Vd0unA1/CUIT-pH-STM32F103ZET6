#ifndef BSP_TIME_APP_BEIJING_TIME_H_
#define BSP_TIME_APP_BEIJING_TIME_H_

#include "stm32f1xx_hal.h"
#include <stddef.h>

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} APP_Time_t;

void APP_Time_InitDefault(void);
void APP_Time_Set(const APP_Time_t *t);
void APP_Time_Get(APP_Time_t *t);
void APP_Time_GetString(char *buf, size_t len);

#endif /* BSP_TIME_APP_BEIJING_TIME_H_ */
