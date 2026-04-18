#include "app_beijing_time.h"
#include "stm32f1xx_hal.h"
#include "rtc.h"
#include <stdio.h>

#define APP_TIME_BKP_REG   RTC_BKP_DR1
#define APP_TIME_BKP_MAGIC 0x5AA5U

static void APP_Time_EnableBackupAccess(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
}

static uint8_t is_leap_year(uint16_t year)
{
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

static uint8_t days_in_month(uint16_t year, uint8_t month)
{
    static const uint8_t days_tbl[12] =
    {
        31,28,31,30,31,30,31,31,30,31,30,31
    };

    if (month == 2)
        return days_tbl[1] + is_leap_year(year);

    return days_tbl[month - 1];
}

static uint32_t datetime_to_seconds(const APP_Time_t *t)
{
    uint32_t days = 0;

    for (uint16_t y = 2000; y < t->year; y++)
    {
        days += is_leap_year(y) ? 366U : 365U;
    }

    for (uint8_t m = 1; m < t->month; m++)
    {
        days += days_in_month(t->year, m);
    }

    days += (uint32_t)(t->day - 1U);

    return days * 86400UL
         + (uint32_t)t->hour * 3600UL
         + (uint32_t)t->min  * 60UL
         + (uint32_t)t->sec;
}

static void seconds_to_datetime(uint32_t seconds, APP_Time_t *t)
{
    uint32_t days = seconds / 86400UL;
    uint32_t rem  = seconds % 86400UL;

    t->hour = rem / 3600UL;
    rem %= 3600UL;
    t->min  = rem / 60UL;
    t->sec  = rem % 60UL;

    t->year = 2000;
    while (1)
    {
        uint16_t dy = is_leap_year(t->year) ? 366U : 365U;
        if (days >= dy)
        {
            days -= dy;
            t->year++;
        }
        else
        {
            break;
        }
    }

    t->month = 1;
    while (1)
    {
        uint8_t dm = days_in_month(t->year, t->month);
        if (days >= dm)
        {
            days -= dm;
            t->month++;
        }
        else
        {
            break;
        }
    }

    t->day = (uint8_t)days + 1U;
}

/* 等待 RTC 写操作完成 */
static void RTC_WaitForLastTask_Custom(void)
{
    while ((RTC->CRL & RTC_CRL_RTOFF) == 0U)
    {
    }
}

/* 设置 RTC 32 位计数器 */
static void RTC_SetCounter_Custom(uint32_t cnt)
{
    APP_Time_EnableBackupAccess();

    RTC_WaitForLastTask_Custom();
    RTC->CRL |= RTC_CRL_CNF;

    RTC->CNTH = (uint16_t)(cnt >> 16);
    RTC->CNTL = (uint16_t)(cnt & 0xFFFFU);

    RTC->CRL &= (uint16_t)~RTC_CRL_CNF;
    RTC_WaitForLastTask_Custom();
}

/* 读取 RTC 32 位计数器 */
static uint32_t RTC_GetCounter_Custom(void)
{
    uint16_t high1 = 0, low = 0, high2 = 0;

    do
    {
        high1 = RTC->CNTH;
        low   = RTC->CNTL;
        high2 = RTC->CNTH;
    } while (high1 != high2);

    return (((uint32_t)high1 << 16) | low);
}

void APP_Time_InitDefault(void)
{
    APP_Time_EnableBackupAccess();

    if (HAL_RTCEx_BKUPRead(&hrtc, APP_TIME_BKP_REG) == APP_TIME_BKP_MAGIC)
    {
        return;
    }

    APP_Time_t t =
    {
        .year  = 2026,
        .month = 4,
        .day   = 17,
        .hour  = 20,
        .min   = 30,
        .sec   = 0
    };

    RTC_SetCounter_Custom(datetime_to_seconds(&t));
    HAL_RTCEx_BKUPWrite(&hrtc, APP_TIME_BKP_REG, APP_TIME_BKP_MAGIC);
}

void APP_Time_Set(const APP_Time_t *t)
{
    if (t == NULL)
        return;

    APP_Time_EnableBackupAccess();
    RTC_SetCounter_Custom(datetime_to_seconds(t));
    HAL_RTCEx_BKUPWrite(&hrtc, APP_TIME_BKP_REG, APP_TIME_BKP_MAGIC);
}

void APP_Time_Get(APP_Time_t *t)
{
    if (t == NULL)
        return;

    uint32_t sec = RTC_GetCounter_Custom();
    seconds_to_datetime(sec, t);
}

void APP_Time_GetString(char *buf, size_t len)
{
    APP_Time_t t;
    APP_Time_Get(&t);

    snprintf(buf, len, "%04u-%02u-%02u %02u:%02u:%02u",
             t.year, t.month, t.day, t.hour, t.min, t.sec);
}
