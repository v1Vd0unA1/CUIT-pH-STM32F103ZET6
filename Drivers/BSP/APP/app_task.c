/*
 * app_task.c
 *
 *  Created on: Apr 18, 2026
 *      Author: 17224
 */

#include "app_task.h"
#include "delay_us.h"
#include "ds18b20.h"
#include "ph_meter.h"
#include "app_beijing_time.h"
#include "oled.h"
#include "adc.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

#define APP_UART_CMD_LEN 40
#define APP_UART_RX_BUF_LEN 128

static volatile uint8_t g_uart_rx_buf[APP_UART_RX_BUF_LEN];
static volatile uint16_t g_uart_rx_head = 0;
static volatile uint16_t g_uart_rx_tail = 0;
static volatile uint8_t g_uart_rx_overflow = 0;
static volatile uint8_t g_uart_hw_overrun = 0;
static char g_cmd_buf[APP_UART_CMD_LEN];
static uint8_t g_cmd_len = 0;
static uint8_t g_cmd_discard_until_eol = 0;

static void APP_OLED_ClearRows(void)
{
    static const char blank_row[] = "                     ";

    oled_show_string(0, 0, blank_row, 12);
    oled_show_string(0, 16, blank_row, 12);
    oled_show_string(0, 32, blank_row, 12);
    oled_show_string(0, 48, blank_row, 12);
}

static uint8_t APP_IsDigit(char ch)
{
    return (ch >= '0') && (ch <= '9');
}

static uint8_t APP_Parse2Digits(const char *s, uint8_t *value)
{
    if ((!APP_IsDigit(s[0])) || (!APP_IsDigit(s[1])))
        return 0;

    *value = (uint8_t)((s[0] - '0') * 10U + (s[1] - '0'));
    return 1;
}

static uint8_t APP_Parse4Digits(const char *s, uint16_t *value)
{
    if ((!APP_IsDigit(s[0])) || (!APP_IsDigit(s[1])) ||
        (!APP_IsDigit(s[2])) || (!APP_IsDigit(s[3])))
        return 0;

    *value = (uint16_t)((s[0] - '0') * 1000U +
                        (s[1] - '0') * 100U +
                        (s[2] - '0') * 10U +
                        (s[3] - '0'));
    return 1;
}

static uint8_t APP_IsLeapYear(uint16_t year)
{
    return ((year % 4U == 0U && year % 100U != 0U) || (year % 400U == 0U));
}

static uint8_t APP_DaysInMonth(uint16_t year, uint8_t month)
{
    static const uint8_t days_tbl[12] =
    {
        31,28,31,30,31,30,31,31,30,31,30,31
    };

    if ((month == 0U) || (month > 12U))
        return 0;

    if (month == 2U)
        return (uint8_t)(days_tbl[1] + APP_IsLeapYear(year));

    return days_tbl[month - 1U];
}

static uint8_t APP_TimeIsValid(const APP_Time_t *t)
{
    uint8_t days = APP_DaysInMonth(t->year, t->month);

    if ((t->year < 2000U) || (t->year > 2099U))
        return 0;
    if ((t->month < 1U) || (t->month > 12U))
        return 0;
    if ((t->day < 1U) || (t->day > days))
        return 0;
    if (t->hour > 23U)
        return 0;
    if (t->min > 59U)
        return 0;
    if (t->sec > 59U)
        return 0;

    return 1;
}

static uint8_t APP_ParseSetTimeCommand(const char *cmd, APP_Time_t *t)
{
    const char *p = cmd;

    if (strncmp(p, "SETTIME ", 8) != 0)
        return 0;

    p += 8;

    if ((!APP_Parse4Digits(&p[0], &t->year)) ||
        (p[4] != '-') ||
        (!APP_Parse2Digits(&p[5], &t->month)) ||
        (p[7] != '-') ||
        (!APP_Parse2Digits(&p[8], &t->day)) ||
        (p[10] != ' ') ||
        (!APP_Parse2Digits(&p[11], &t->hour)) ||
        (p[13] != ':') ||
        (!APP_Parse2Digits(&p[14], &t->min)) ||
        (p[16] != ':') ||
        (!APP_Parse2Digits(&p[17], &t->sec)) ||
        (p[19] != '\0'))
    {
        return 0;
    }

    return APP_TimeIsValid(t);
}

static void APP_HandleCommand(const char *cmd)
{
    APP_Time_t t;

    if (APP_ParseSetTimeCommand(cmd, &t))
    {
        APP_Time_Set(&t);
        printf("OK:TIME SET TO %04u-%02u-%02u %02u:%02u:%02u\r\n",
               t.year, t.month, t.day, t.hour, t.min, t.sec);
    }
    else
    {
        printf("ERR:USE SETTIME YYYY-MM-DD HH:MM:SS\r\n");
    }
}

static void APP_ResetCommandParser(void)
{
    g_cmd_len = 0U;
    g_cmd_discard_until_eol = 0U;
}

static void APP_ProcessRxByte(uint8_t ch)
{
    if ((ch == '\r') || (ch == '\n'))
    {
        if (g_cmd_discard_until_eol)
        {
            APP_ResetCommandParser();
            return;
        }

        if (g_cmd_len > 0U)
        {
            g_cmd_buf[g_cmd_len] = '\0';
            APP_HandleCommand(g_cmd_buf);
            APP_ResetCommandParser();
        }
        return;
    }

    if (g_cmd_discard_until_eol)
    {
        return;
    }

    if (g_cmd_len >= (APP_UART_CMD_LEN - 1U))
    {
        g_cmd_len = 0U;
        g_cmd_discard_until_eol = 1U;
        printf("ERR:CMD TOO LONG\r\n");
        return;
    }

    g_cmd_buf[g_cmd_len++] = (char)ch;
}

static void format_fixed(char *buf, size_t len, float value, uint8_t frac_digits)
{
    int32_t scale = 1;
    for (uint8_t i = 0; i < frac_digits; i++) scale *= 10;

    int negative = 0;
    if (value < 0)
    {
        negative = 1;
        value = -value;
    }

    int32_t iv = (int32_t)value;
    int32_t fv = (int32_t)((value - iv) * scale + 0.5f);

    if (fv >= scale)
    {
        iv++;
        fv = 0;
    }

    if (negative)
        snprintf(buf, len, "-%ld.%0*ld", (long)iv, frac_digits, (long)fv);
    else
        snprintf(buf, len, "%ld.%0*ld", (long)iv, frac_digits, (long)fv);
}

void APP_UserInit(void)
{
    delay_us_init();
    HAL_ADCEx_Calibration_Start(&hadc1);

    DS18B20_Init();
    PH_Init();
    oled_init();
    oled_clear();
    APP_Time_InitDefault();

    oled_show_string(0, 0, "PH Meter Start", 12);
    oled_refresh_gram();
    HAL_Delay(800);
    oled_clear();
    oled_refresh_gram();

    printf("UART CMD: SETTIME YYYY-MM-DD HH:MM:SS\r\n");
}

void APP_UART_IRQHandler(void)
{
    uint32_t sr = huart1.Instance->SR;

    if ((sr & (USART_SR_RXNE | USART_SR_ORE)) != 0U)
    {
        uint8_t ch = (uint8_t)(huart1.Instance->DR & 0x00FFU);
        uint16_t next_head = (uint16_t)((g_uart_rx_head + 1U) % APP_UART_RX_BUF_LEN);

        if ((sr & USART_SR_ORE) != 0U)
        {
            g_uart_hw_overrun = 1U;
        }

        if ((sr & USART_SR_RXNE) != 0U)
        {
            if (next_head == g_uart_rx_tail)
            {
                g_uart_rx_overflow = 1U;
            }
            else
            {
                g_uart_rx_buf[g_uart_rx_head] = ch;
                g_uart_rx_head = next_head;
            }
        }
    }
}

void APP_TaskPoll(void)
{
    if (g_uart_rx_overflow)
    {
        g_uart_rx_overflow = 0U;
        g_cmd_len = 0U;
        g_cmd_discard_until_eol = 1U;
        printf("ERR:UART RX BUF FULL\r\n");
    }

    if (g_uart_hw_overrun)
    {
        g_uart_hw_overrun = 0U;
        g_cmd_len = 0U;
        g_cmd_discard_until_eol = 1U;
        printf("ERR:UART ORE\r\n");
    }

    while (g_uart_rx_tail != g_uart_rx_head)
    {
        uint8_t ch = g_uart_rx_buf[g_uart_rx_tail];
        g_uart_rx_tail = (uint16_t)((g_uart_rx_tail + 1U) % APP_UART_RX_BUF_LEN);
        APP_ProcessRxByte(ch);
    }
}

void APP_Task1s(void)
{
    float temp = 0.0f;
    float voltage = 0.0f;
    float ph = 7.00f;
    uint8_t temp_valid = 0;

    char time_str[32];
    char ph_str[16];
    char temp_str[16];
    char volt_str[16];
    char line[32];

    if (DS18B20_ReadTemperature(&temp))
    {
        temp_valid = 1;
    }

    if (temp_valid)
    {
        ph = PH_ReadPH(temp, &voltage);
        format_fixed(ph_str, sizeof(ph_str), ph, 2);
        format_fixed(temp_str, sizeof(temp_str), temp, 2);
    }
    else
    {
        voltage = PH_ReadVoltage();
        snprintf(ph_str, sizeof(ph_str), "ERR");
        snprintf(temp_str, sizeof(temp_str), "ERR");
    }

    format_fixed(volt_str, sizeof(volt_str), voltage, 3);
    APP_Time_GetString(time_str, sizeof(time_str));

    APP_OLED_ClearRows();

    snprintf(line, sizeof(line), "PH:%s", ph_str);
    oled_show_string(0, 0, line, 12);

    if (temp_valid)
        snprintf(line, sizeof(line), "T:%sC", temp_str);
    else
        snprintf(line, sizeof(line), "T:%s", temp_str);
    oled_show_string(0, 16, line, 12);

    snprintf(line, sizeof(line), "V:%sV", volt_str);
    oled_show_string(0, 32, line, 12);

    oled_show_string(0, 48, time_str, 12);
    oled_refresh_gram();

    if (temp_valid)
        printf("%s,PH=%s,T=%sC,V=%sV\r\n", time_str, ph_str, temp_str, volt_str);
    else
        printf("%s,PH=%s,T=%s,V=%sV\r\n", time_str, ph_str, temp_str, volt_str);
}
