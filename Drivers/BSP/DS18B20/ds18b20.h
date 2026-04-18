/*
 * ds18b20.h
 *
 *  Created on: Apr 18, 2026
 *      Author: 17224
 */

#ifndef BSP_DS18B20_DS18B20_H_
#define BSP_DS18B20_DS18B20_H_


#include "stm32f1xx_hal.h"

uint8_t DS18B20_Init(void);
uint8_t DS18B20_ReadTemperature(float *temp);



#endif /* BSP_DS18B20_DS18B20_H_ */
