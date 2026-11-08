/*
*********************************************************************************************************
*
*	模块名称 : 头文件汇总
*	文件名称 : includes.h
*	版    本 : V1.0
*	说    明 : 当前使用头文件汇总
*
*	修改记录 :
*		版本号    日期        作者     说明
*		V1.0    2020-06-04  Eric2013   首次发布
*
*	Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef  __INCLUDES_H__
#define  __INCLUDES_H__

/*
*********************************************************************************************************
*                                         标准库
*********************************************************************************************************
*/
#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_dma.h"

#include "stm32f4xx_ll_exti.h"
/*
*********************************************************************************************************
*                                           OS
*********************************************************************************************************
*/
#include "tx_api.h"
#include "tx_timer.h"
#include "nx_api.h"

/*
*********************************************************************************************************
*                                        APP / BSP
*********************************************************************************************************
*/

#include  <bsp.h>

/*
*********************************************************************************************************
*                                          变量和函数
*********************************************************************************************************
*/

/*************************************************************************/
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart8;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;

#define UART7_DE_nRE_Pin GPIO_PIN_8
#define UART7_DE_nRE_GPIO_Port GPIOF
#define LED_PA3_Pin GPIO_PIN_3
#define LED_PA3_GPIO_Port GPIOA
#define TR_LIGHT_Pin GPIO_PIN_13
#define TR_LIGHT_GPIO_Port GPIOE
#define USART3_DE_nRE_Pin GPIO_PIN_10
#define USART3_DE_nRE_GPIO_Port GPIOD
#define TRIG1_EN_Pin GPIO_PIN_15
#define TRIG1_EN_GPIO_Port GPIOD
#define TRIG2_EN_Pin GPIO_PIN_2
#define TRIG2_EN_GPIO_Port GPIOG
#define TRIG3_EN_Pin GPIO_PIN_3
#define TRIG3_EN_GPIO_Port GPIOG
#define TRIG4_EN_Pin GPIO_PIN_4
#define TRIG4_EN_GPIO_Port GPIOG
#define TRIG5_EN_Pin GPIO_PIN_5
#define TRIG5_EN_GPIO_Port GPIOG
#define TRIG6_EN_Pin GPIO_PIN_6
#define TRIG6_EN_GPIO_Port GPIOG
#define TRIG7_EN_Pin GPIO_PIN_7
#define TRIG7_EN_GPIO_Port GPIOG
#define TRIG8_EN_Pin GPIO_PIN_8
#define TRIG8_EN_GPIO_Port GPIOG
#define USART6_DE_nRE_Pin GPIO_PIN_8
#define USART6_DE_nRE_GPIO_Port GPIOC
#define USART1_DE_nRE_Pin GPIO_PIN_11
#define USART1_DE_nRE_GPIO_Port GPIOA
#define UART4_DE_nRE_Pin GPIO_PIN_15
#define UART4_DE_nRE_GPIO_Port GPIOA
#define UART5_DE_nRE_Pin GPIO_PIN_1
#define UART5_DE_nRE_GPIO_Port GPIOD
#define USART2_DE_nRE_Pin GPIO_PIN_4
#define USART2_DE_nRE_GPIO_Port GPIOD
#define ETH_RESET_Pin GPIO_PIN_12
#define ETH_RESET_GPIO_Port GPIOG
#define SPI1_CS_Pin GPIO_PIN_6
#define SPI1_CS_GPIO_Port GPIOB
#define UART8_DE_nRE_Pin GPIO_PIN_9
#define UART8_DE_nRE_GPIO_Port GPIOB
/***********************************************************/
/* 方便RTOS里面使用 */
extern void SysTick_ISR(void);

#define bsp_ProPer1ms  SysTick_ISR

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
