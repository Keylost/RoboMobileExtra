#include "stm32f0xx.h"
	
	

// ----- Pins Defines -----
#define GREEN_1	 GPIO_Pin_0
#define YELLOW_1 GPIO_Pin_1
#define RED_1 	 GPIO_Pin_2

#define GREEN_2	 GPIO_Pin_3
#define YELLOW_2 GPIO_Pin_4
#define RED_2 	 GPIO_Pin_5
// ------------------------



// ----- Time Defines -----
#define RED_ON	  	10000
#define YELLOW_ON 	3600
#define GREEN_BLINK	3600
#define GREEN_ON		(RED_ON - GREEN_BLINK)
// ------------------------



// ------ Sub Defines -----
#define NUMBER_OF_BLINKS 3
// ------------------------



// --- Global variables ---
	uint16_t Timer = 0;
// ------------------------



// ------- Functions ------
void SysTick_Handler(void)
{
	if (Timer > 0)
		--Timer;
}



void Timer_init(void)
{
	SysTick_Config(SystemCoreClock / 1000);
}



void delay(uint16_t time)
{
	Timer = time;
	
	while (Timer)
	{}
		
}



void LED_init(void)
{
	GPIO_InitTypeDef GPIO_Init_LED;
	
	RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOAEN, ENABLE);
	
	GPIO_Init_LED.GPIO_Pin   = RED_1 | YELLOW_1 | GREEN_1 | RED_2 | YELLOW_2 | GREEN_2;
	GPIO_Init_LED.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_Init_LED.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init_LED.GPIO_OType = GPIO_OType_PP;
	GPIO_Init_LED.GPIO_PuPd  = GPIO_PuPd_DOWN;
	
	GPIO_Init(GPIOA, &GPIO_Init_LED);
}



void LED_on(uint16_t LED)
{
	GPIO_SetBits(GPIOA, LED);
}



void LED_off(uint16_t LED)
{
	GPIO_ResetBits(GPIOA, LED);
}



void LED_Blink(uint16_t LED, uint16_t blinkTime)
{
	uint16_t i, delayTime = blinkTime / (2 * NUMBER_OF_BLINKS);
	for (i = 0; i < NUMBER_OF_BLINKS; ++i) {
		LED_off(LED);
		delay(delayTime);
		LED_on(LED);
		delay(delayTime);
	}
}
// ------------------------
