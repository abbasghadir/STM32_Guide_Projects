/**Project 1
 * The aim of Project 1 is to implement a basic and guide project on STM32 microcontroller.
 * In this project the following peripherals are implemented:
 * 1.Timer
 * 2.RTC
 * 3.GPIO
 * 4.DMA
 * 5.UART
 * And we can also explore the interrupt operations.
 */

#include "stm32f103xb.h"
#include "main.h"
#include "string.h"
#include "stdbool.h"
#include "pre_def.h"
#include "stdio.h"

RTC_HandleTypeDef hrtc;
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;
RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};

bool interrupt_flag = false;
bool one_time_flag = true;
uint32_t current_time = 0;
uint32_t debounce = 200;
uint8_t RTC_buffer[36];

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  RTC_time_init();
  
  while (1)
  {
    if (interrupt_routine() && one_time_flag)
    {
      get_time();
      get_date();
      buff_UART_send();
      one_time_flag = false;
    }
  }
}

//Changing data format and sending via uart DMA
void buff_UART_send(void){
  sprintf((char*)RTC_buffer,"Time: %02d:%02d:%02d  Date: %02d-%02d-%02d"
  ,sTime.Hours,sTime.Minutes,sTime.Seconds,sDate.Year,sDate.Month,sDate.Date);
  if (HAL_UART_Transmit_DMA(&huart1,RTC_buffer, strlen((char*)RTC_buffer)) != HAL_OK) {
        // Error_Handler();
    }
}

//get last time
void get_time(void){
  if(HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BIN) !=HAL_OK){Error_Handler();}
}
//get last date
void get_date(void){
  if(HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN) !=HAL_OK){Error_Handler();}
}

void led_on(void){
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
}

void led_off(void){
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
}

//handle interrupt routine and software debouncing
bool interrupt_routine(void){
  current_time = interrupt_flag?current_time:HAL_GetTick();
  interrupt_flag = (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1) == GPIO_PIN_SET)?false:true;
  return (HAL_GetTick()-current_time >= debounce && interrupt_flag);
}
//interrupt callback
void HAL_GPIO_EXTI_Callback(uint16_t input_callback){
  if (input_callback==GPIO_PIN_1 && !interrupt_flag)
  {
    interrupt_flag = true;
    one_time_flag = true;

  }  
}
//uart successful send callback
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UART){
  if (UART->Instance==USART1)
  {
    led_on();
    HAL_TIM_Base_Start_IT(&htim2);
  }
}
//end of timer operation
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  if (htim->Instance == TIM2)
  {
    HAL_TIM_Base_Stop_IT(&htim2);
    led_off();
  }
  
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_RTC_Init(void)
{
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 18000;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 3999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_DMA_Init(void)
{

  __HAL_RCC_DMA1_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}
void RTC_time_init(void){
  
  sTime.Hours = 17;
  sTime.Minutes = 44;
  sTime.Seconds = 30;
  HAL_RTC_SetTime(&hrtc,&sTime,RTC_FORMAT_BIN);

  
  sDate.Year = 25;
  sDate.Month = 05;
  sDate.Date = 29;
  HAL_RTC_SetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif
