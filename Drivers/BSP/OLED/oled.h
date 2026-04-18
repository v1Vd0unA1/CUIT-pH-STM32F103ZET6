/**
 ****************************************************************************************************
 * @file        oled.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2024-01-01
 * @brief       OLED 驱动代码
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

#ifndef OLED_OLED_H_
#define OLED_OLED_H_

#include "stdlib.h"
#include "font.h"
#include "main.h"
/******************************************************************************************/

/* OLED SPI模式相关端口控制函数 定义
 * 注意:OLED_RST/OLED_CS/OLED_RS,这三个是和80并口模式共用的,即80模式也必须实现这3个函数!
 */
#define OLED_RST(x)     do{ x ? \
                                  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET) : \
                                  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); \
                        }while(0)       /* 设置RST引脚 */

#define OLED_CS(x)      do{ x ? \
                                  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET) : \
                                  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET); \
                        }while(0)       /* 设置CS引脚 */
#define OLED_RS(x)      do{ x ? \
                                  HAL_GPIO_WritePin(OLED_RS_GPIO_Port, OLED_RS_Pin, GPIO_PIN_SET) : \
                                  HAL_GPIO_WritePin(OLED_RS_GPIO_Port, OLED_RS_Pin, GPIO_PIN_RESET); \
                        }while(0)       /* 设置RS引脚 */

#define OLED_WR(x)      do{ x ? \
                                  HAL_GPIO_WritePin(OLED_WR_GPIO_Port, OLED_WR_Pin, GPIO_PIN_SET) :  \
                                  HAL_GPIO_WritePin(OLED_WR_GPIO_Port, OLED_WR_Pin, GPIO_PIN_RESET); \
                        } while (0)     /* 设置WR引脚 */

#define OLED_RD(x)      do{ x ? \
                                  HAL_GPIO_WritePin(OLED_RD_GPIO_Port, OLED_RD_Pin, GPIO_PIN_SET) : \
                                  HAL_GPIO_WritePin(OLED_RD_GPIO_Port, OLED_RD_Pin, GPIO_PIN_RESET); \
                        }while(0)       /* 设置RD引脚 */

/* 命令/数据 定义 */
#define OLED_CMD        0       /* 写命令 */
#define OLED_DATA       1       /* 写数据 */
#define OLED_H          64  	/* 水平分辨率 */
#define OLED_W          128 	/* 垂直分辨率 */

/******************************************************************************************/
void oled_init(void);           /* OLED初始化 */
void oled_clear(void);          /* OLED清屏 */
void oled_display_on(void);     /* 开启OLED显示 */
void oled_display_off(void);    /* 关闭OLED显示 */
void oled_refresh_gram(void);   /* 更新显存到OLED */
void oled_draw_point(uint8_t x, uint8_t y, uint8_t dot);    /* OLED画点 */
void oled_fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot);        /* OLED区域填充 */
void oled_show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode); /* OLED显示字符 */
void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);  /* OLED显示数字 */
void oled_show_string(uint8_t x, uint8_t y, const char *p, uint8_t size);           /* OLED显示字符串 */
void oled_draw_bitmap(int16_t x, int16_t y, uint8_t *bitmap, uint16_t p_w, uint16_t p_h, uint8_t mode); /* 显示图片 */
void show_str(uint16_t x, uint16_t y, const char *str, uint8_t f_h, const font_t *font, uint8_t mode);	/* 在指定位置开始显示字符串(支持中英文显示) */
#endif /* OLED_OLED_H_ */
