#include "stm32f10x.h"                  // Device header
#include "stdbool.h"
#include "pre_def.h"
#include "stdio.h"

volatile uint32_t tick_counter = 0;
volatile bool interrupt_flag = false;
bool one_time_flag = true;
uint32_t current_time = 0;
uint32_t debounce = 200;

int main(void){
	systick_config(72000000);
	SystemClock_Config();
	GPIO_init();
	EXTI_config();
	NVIC_config();
	UART_config();
	RTC_config();
	TIM3_config();
	TIM3_start();

	while(1){
		if (input_pin_event() && one_time_flag)
		{
			send_TimeDate();
			one_time_flag = false;
		}
	}
}

//handle interrupt flag and debounce of input pib
bool input_pin_event(void){
  current_time = interrupt_flag?current_time:GetTick();
  if (GPIOA->IDR & GPIO_IDR_IDR1)
  {
	interrupt_flag = false;
	one_time_flag = true;
  }
  return (GetTick()-current_time >= debounce && interrupt_flag);
}

void PINB10_off(void){
	GPIOB->BSRR |= GPIO_BSRR_BS10;
}

void PINB10_on(void){
	GPIOB->BRR |= GPIO_BRR_BR10;
}

//gpio gonfiguration
void GPIO_init(void)
{
	// RCC->APB2ENR &= ~RCC_APB2ENR_IOPBEN | ~RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |=	RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
	GPIOB->CRH 	 |=	GPIO_CRH_MODE10;	
	GPIOB->CRH 	 &=~GPIO_CRH_CNF10;
	GPIOB->ODR 	 |= GPIO_ODR_ODR10;

	GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1);
	GPIOA->CRL |= GPIO_CRL_CNF1_1;
	GPIOA->ODR |= GPIO_ODR_ODR1;	
}
void EXTI1_IRQHandler(void){
	if (EXTI->PR & EXTI_PR_PR1)
	{
		EXTI->PR |= EXTI_PR_PR1;
		interrupt_flag = true;
	}
}

void EXTI_config(void){
	AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI1_PA;
	AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI1_PA;

	EXTI->IMR |= EXTI_IMR_MR1;
	EXTI->FTSR |= EXTI_FTSR_TR1;
}

void NVIC_config(void){
	NVIC_SetPriority(EXTI1_IRQn,1);
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_SetPriority(TIM3_IRQn,0);
	NVIC_EnableIRQ(TIM3_IRQn);

}

void SystemClock_Config(void)
{
	RCC->CR |= RCC_CR_HSEON;
	while(!(RCC->CR & RCC_CR_HSERDY)){}
	FLASH->ACR &= ~(FLASH_ACR_LATENCY);
	FLASH->ACR |= FLASH_ACR_LATENCY_2;
	RCC->CR &= ~RCC_CR_PLLON;
	RCC->CFGR |= RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLXTPRE_HSE | RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PPRE2_DIV1 |
	RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_HPRE_DIV1 | RCC_CFGR_SW_PLL | RCC_CFGR_SWS_PLL;
	RCC->CR |= RCC_CR_PLLON;
	while(!(RCC->CR & RCC_CR_PLLRDY)){}

	RCC->CSR |=RCC_CSR_LSION;
	while(!(RCC->CSR & RCC_CSR_LSIRDY)){}
}

void systick_config(uint32_t systemclock){
	SysTick->LOAD = (systemclock/1000)-1;
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk|SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void){
	tick_counter++;
}

uint32_t GetTick(void){
	return tick_counter;
}

void delay_ms(uint32_t mx){
	// for(int i= 0;i<= mx*4000;i++){}	
	uint32_t start_tick = GetTick();
    while ((GetTick() - start_tick) < mx) {}

}


//timer 3 setting
void TIM3_config(void){
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->PSC = 3599;
	TIM3->ARR = 9999;
	TIM3 ->DIER |= TIM_DIER_UIE;
	TIM3->CR1 |= TIM_CR1_OPM;
}
void TIM3_start(void){
	PINB10_on();
	TIM3->CNT = 0;
	TIM3->CR1 |=TIM_CR1_CEN;
}
void TIM3_IRQHandler(void){
	if (TIM3->SR & TIM_SR_UIF)
	{
		TIM3->SR &= ~TIM_SR_UIF;
		PINB10_off();
		TIM3->CR1 &= ~TIM_CR1_CEN;
	}
}

//RTC configuration
void RTC_config(void){
	RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
	PWR->CR |= PWR_CR_DBP;

	RCC->CSR |= RCC_CSR_LSION;
	while (!(RCC->CSR & RCC_CSR_LSIRDY)) {}

	RCC->BDCR |= RCC_BDCR_RTCSEL_1;
	RCC->BDCR |= RCC_BDCR_RTCEN;

	RTC->CRL |= RTC_CRL_CNF;
	while (!(RTC->CRL & RTC_CRL_RSF)) {}

	RTC->PRLH = 0;
	RTC->PRLL = 32767;

    uint8_t hours = 17;
    uint8_t minutes = 29;
    uint8_t seconds = 0;
	uint32_t time_bcd = ((hours / 10) << 20) | ((hours % 10) << 16) |
                        ((minutes / 10) << 12) | ((minutes % 10) << 8) |
                        ((seconds / 10) << 4) | (seconds % 10);
	
	RTC->CNTH = (time_bcd >> 16);
	RTC->CNTL = (time_bcd & 0xFFFF);

	uint8_t year = 25;
    uint8_t month = 5;
    uint8_t date = 30;
    uint8_t weekday = 5;
	uint32_t date_bcd = ((year / 10) << 20) | ((year % 10) << 16) |
                        ((month / 10) << 12) | ((month % 10) << 8) |
                        ((date / 10) << 4) | (date % 10) |
                        (weekday << 13);

	RTC->ALRH = (date_bcd >> 16);
	RTC->ALRL = (date_bcd & 0xFFFF);

	RTC->CRL &= ~RTC_CRL_CNF;
	
	RTC->CRL &= ~RTC_CRL_RSF;
	while (!(RTC->CRL & RTC_CRL_RSF)) {}
	
	while (!(RTC->CRL & RTC_CRL_RTOFF)) {}
	PWR->CR &= ~PWR_CR_DBP;
}

void UART_config(void) {
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;

	GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9 | GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOA->CRH |= (GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1);
    GPIOA->CRH |= GPIO_CRH_CNF10_1;
    USART1->BRR = 72000000 / 115200;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void UART_Transmit(char *buffer) {
    while (*buffer) {
        while (!(USART1->SR & USART_SR_TXE)) {}
        USART1->DR = *buffer++;
    }
	TIM3_start();
}

void send_TimeDate(void) {
    char buffer[32];
    uint32_t time_bcd = (RTC->CNTH << 16) | RTC->CNTL;
    uint32_t date_bcd = (RTC->ALRH << 16) | RTC->ALRL;

    uint8_t hours = ((time_bcd >> 20) & 0x3) * 10 + ((time_bcd >> 16) & 0xF);
    uint8_t minutes = ((time_bcd >> 12) & 0x7) * 10 + ((time_bcd >> 8) & 0xF);
    uint8_t seconds = ((time_bcd >> 4) & 0x7) * 10 + (time_bcd & 0xF);
    uint8_t date = ((date_bcd >> 4) & 0x3) * 10 + (date_bcd & 0xF);
    uint8_t month = ((date_bcd >> 12) & 0x1) * 10 + ((date_bcd >> 8) & 0xF);
    uint8_t year = ((date_bcd >> 20) & 0xF) * 10 + ((date_bcd >> 16) & 0xF);

    snprintf(buffer, sizeof(buffer), "Time: %02d:%02d:%02d Date: %02d/%02d/%02d\r\n",
             hours, minutes, seconds, date, month, year);

    UART_Transmit(buffer);
}