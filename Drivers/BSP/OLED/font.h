/**
 ****************************************************************************************************
 * @file        font.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2024-01-01
 * @brief       汉字相关参数结构体以及全局声明相关字模
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20240101
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef BSP_OLED_FONT_H_
#define BSP_OLED_FONT_H_
#include <stdint.h>

struct _font
{
    uint16_t w;
    uint16_t h;
    const char *index;
    const uint8_t *bitmap;
};

typedef struct
{
    struct _font ch;
} font_t;

/* 汉字字模全局声明 */
extern const font_t font_16x16_mis;
extern const font_t font_24x24_mis;
extern const font_t font_32x32_mis;

/* 图模全局声明 */
extern const uint8_t refresh_32x32_logo[];
extern const uint8_t temp_32x32_logo[];
extern const uint8_t zdyz_32x32_logo[];
extern const uint8_t wendu_32x32_logo[];
#endif /* BSP_OLED_FONT_H_ */
