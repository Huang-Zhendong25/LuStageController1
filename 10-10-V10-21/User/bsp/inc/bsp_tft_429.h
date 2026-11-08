/*
*********************************************************************************************************
*
*	模块名称 : STM32F4驱动液晶
*	文件名称 : bsp_tft_F4.h
*	版    本 : V2.0
*	说    明 : 头文件
*
*	Copyright (C), 2020-2030 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_TFT_F4_H
#define _BSP_TFT_F4_H

#define LCD_LAYER_1     0x0000		/* 顶层 */
#define LCD_LAYER_2		0x0001		/* 第2层 */

/* 可供外部模块调用的函数 */
void LCDF4_InitHard(void);
void LCDF4_GetChipDescribe(char *_str);
void LCDF4_DispOn(void);
void LCDF4_DispOff(void);
void LCDF4_ClrScr(uint16_t _usColor);
void LCDF4_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor);
uint16_t LCDF4_GetPixel(uint16_t _usX, uint16_t _usY);
void LCDF4_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor);
void LCDF4_DrawPoints(uint16_t *x, uint16_t *y, uint16_t _usSize, uint16_t _usColor);
void LCDF4_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);
void LCDF4_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);
void LCDF4_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr);
void LCDF4_FillRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);

void LCDF4_SetDirection(uint8_t _dir);

void LCDF4_SetLayer(uint8_t _ucLayer);

void LCDF4_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth);
void LCDF4_QuitWinMode(void);
void _LCD_DrawCamera16bpp(int x, int y, uint16_t * p, int xSize, int ySize, int SrcOffLine);
#endif


