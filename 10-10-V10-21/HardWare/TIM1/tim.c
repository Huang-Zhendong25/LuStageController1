#include "tim.h"

/* TIM1 init function */
void MX_TIM1_Init(void)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
  LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
  NVIC_SetPriority(TIM1_CC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(TIM1_CC_IRQn);
  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  TIM_InitStruct.Prescaler = 17999;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 999;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM1, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM1);
  
  LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
  
  LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH3);
  
  TIM_OC_InitStruct.OCMode 		= LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState 	= LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState 	= LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue= 500;
  TIM_OC_InitStruct.OCPolarity 	= LL_TIM_OCPOLARITY_LOW;
  TIM_OC_InitStruct.OCNPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
  TIM_OC_InitStruct.OCNIdleState= LL_TIM_OCIDLESTATE_LOW;
  LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH3, &TIM_OC_InitStruct);
  
  LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH3);
  
  LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_UPDATE);
  
  LL_TIM_EnableMasterSlaveMode(TIM1);
  
  LL_TIM_OC_DisablePreload(TIM1, LL_TIM_CHANNEL_CH3);
  
  TIM_BDTRInitStruct.OSSRState 		 = LL_TIM_OSSR_DISABLE;
  TIM_BDTRInitStruct.OSSIState 		 = LL_TIM_OSSI_DISABLE;
  TIM_BDTRInitStruct.LockLevel 		 = LL_TIM_LOCKLEVEL_OFF;
  TIM_BDTRInitStruct.DeadTime 		 = 0;
  TIM_BDTRInitStruct.BreakState 	 = LL_TIM_BREAK_DISABLE;
  TIM_BDTRInitStruct.BreakPolarity 	 = LL_TIM_BREAK_POLARITY_HIGH;
  TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_DISABLE;
  LL_TIM_BDTR_Init(TIM1, &TIM_BDTRInitStruct);
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
    /**TIM1 GPIO Configuration
    PE13     ------> TIM1_CH3
    */
  GPIO_InitStruct.Pin 			= LL_GPIO_PIN_13;
  GPIO_InitStruct.Mode 			= LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed 		= LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType 	= LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull 			= LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate 	= LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

}
/* TIM4 init function */
void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

  /* USER CODE BEGIN TIM4_Init 1 */
  NVIC_SetPriority(TIM4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(TIM4_IRQn);
  /* USER CODE END TIM4_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM4, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM4);
  LL_TIM_SetTriggerInput(TIM4, LL_TIM_TS_ITR0);
  LL_TIM_SetClockSource(TIM4, LL_TIM_CLOCKSOURCE_EXT_MODE1);
  LL_TIM_DisableIT_TRIG(TIM4);
  LL_TIM_DisableDMAReq_TRIG(TIM4);
  LL_TIM_SetTriggerOutput(TIM4, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM4);
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
