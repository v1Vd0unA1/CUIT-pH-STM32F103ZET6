/*
 * ph_meter.h
 *
 *  Created on: Apr 18, 2026
 *      Author: 17224
 */

#ifndef BSP_PH_PH_METER_H_
#define BSP_PH_PH_METER_H_


#include "stm32f1xx_hal.h"

typedef struct
{
    float v4;
    float v686;
    float v918;
} PH_Calib_t;

void  PH_Init(void);
void  PH_SetCalibration(PH_Calib_t calib);
float PH_ReadVoltage(void);
float PH_ReadPH(float temp_c, float *voltage_out);



#endif /* BSP_PH_PH_METER_H_ */
