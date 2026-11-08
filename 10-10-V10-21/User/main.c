#include "includes.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "dma.h"
#include "bsp_flash.h"
#include "application.h"
/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
	/* HAL库，MPU，Cache，时钟等系统初始化 */
	System_Init();	
	/* 内核开启前关闭HAL的时间基准 */
//	HAL_SuspendTick();
		
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_SPI1_Init();
	MX_UART4_Init();
	MX_UART5_Init();
	MX_UART7_Init();
	MX_UART8_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();
	MX_USART6_UART_Init();	
	Frame_UART_Start_Receive();
    /* 进入ThreadX内核 */
    tx_kernel_enter();

	while(1);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}