/*
 * ds18b20.c
 *
 *  Created on: Apr 18, 2026
 *      Author: 17224
 */

#include "ds18b20.h"
#include "delay_us.h"

#define DS18B20_GPIO_PORT GPIOG
#define DS18B20_GPIO_PIN  GPIO_PIN_11

static void DS18B20_Pin_Output_OD(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DS18B20_GPIO_PORT, &GPIO_InitStruct);
}

static void DS18B20_Pin_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DS18B20_GPIO_PORT, &GPIO_InitStruct);
}

static void DS18B20_WritePin(GPIO_PinState state)
{
    HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, state);
}

static GPIO_PinState DS18B20_ReadPin(void)
{
    return HAL_GPIO_ReadPin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN);
}

static uint8_t DS18B20_Reset(void)
{
    uint8_t presence = 0;

    DS18B20_Pin_Output_OD();
    DS18B20_WritePin(GPIO_PIN_RESET);
    delay_us(500);

    DS18B20_Pin_Input();
    delay_us(70);

    presence = (DS18B20_ReadPin() == GPIO_PIN_RESET) ? 1 : 0;
    delay_us(430);

    return presence;
}

static void DS18B20_WriteBit(uint8_t bit)
{
    DS18B20_Pin_Output_OD();
    DS18B20_WritePin(GPIO_PIN_RESET);

    if (bit)
    {
        delay_us(3);
        DS18B20_Pin_Input();
        delay_us(60);
    }
    else
    {
        delay_us(60);
        DS18B20_Pin_Input();
        delay_us(3);
    }
}

static uint8_t DS18B20_ReadBit(void)
{
    uint8_t bit = 0;

    DS18B20_Pin_Output_OD();
    DS18B20_WritePin(GPIO_PIN_RESET);
    delay_us(3);

    DS18B20_Pin_Input();
    delay_us(10);

    bit = (DS18B20_ReadPin() == GPIO_PIN_SET) ? 1 : 0;
    delay_us(50);

    return bit;
}

static void DS18B20_WriteByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        DS18B20_WriteBit(data & 0x01);
        data >>= 1;
    }
}

static uint8_t DS18B20_ReadByte(void)
{
    uint8_t data = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        if (DS18B20_ReadBit())
        {
            data |= (1U << i);
        }
    }
    return data;
}

static uint8_t DS18B20_CRC8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;

    for (uint8_t i = 0; i < len; i++)
    {
        uint8_t inbyte = data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix)
            {
                crc ^= 0x8C;
            }
            inbyte >>= 1;
        }
    }
    return crc;
}

uint8_t DS18B20_Init(void)
{
    __HAL_RCC_GPIOG_CLK_ENABLE();
    return DS18B20_Reset();
}

uint8_t DS18B20_ReadTemperature(float *temp)
{
    uint8_t buf[9];
    int16_t raw;

    if (temp == NULL)
        return 0;

    if (!DS18B20_Reset())
        return 0;

    DS18B20_WriteByte(0xCC);
    DS18B20_WriteByte(0x44);
    HAL_Delay(750);

    if (!DS18B20_Reset())
        return 0;

    DS18B20_WriteByte(0xCC);
    DS18B20_WriteByte(0xBE);

    for (uint8_t i = 0; i < 9; i++)
    {
        buf[i] = DS18B20_ReadByte();
    }

    if (DS18B20_CRC8(buf, 8) != buf[8])
        return 0;

    raw = (int16_t)((buf[1] << 8) | buf[0]);
    *temp = raw / 16.0f;

    return 1;
}
