#include "stm32f0xx.h"

#include "RTTLL.h"

int main(void)
{
	LED_init();
	Timer_init();	
	
	while(1) {
		// ----------- RED_1 AND GREEN_2 ON ----------
		LED_on(RED_1 | GREEN_2);
		delay(GREEN_ON);
		// -------------------------------------------
		
		// --------------- GREEN_2 BLINK -------------
		LED_Blink(GREEN_2, GREEN_BLINK);
		// -------------------------------------------
		
		//   YELLOWS ON AND OFF, RED_1 AND GREEN_2 OFF
		LED_off(GREEN_2);
		LED_on(YELLOW_1 | YELLOW_2);
		delay(YELLOW_ON);
		LED_off(YELLOW_1 | YELLOW_2 | RED_1);
		// -------------------------------------------
		
		// ----------- GREEN_1 AND RED_2 ON  ---------
		LED_on(GREEN_1 | RED_2);
		delay(GREEN_ON);
		// -------------------------------------------
		
		// --------------- GREEN_1 BLINK -------------
		LED_Blink(GREEN_1, GREEN_BLINK);
		// -------------------------------------------
		
		//   YELLOWS ON AND OFF, RED_2 AND GREEN_1 OFF
		LED_off(GREEN_1);
		LED_on(YELLOW_1 | YELLOW_2);
		delay(YELLOW_ON);
		LED_off(YELLOW_1 | YELLOW_2 | RED_2);
		// -------------------------------------------
	}
	
}
