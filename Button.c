#include "Button.h"

//******************************************************************************************
// User Button = B1 = PC.13
//******************************************************************************************
#define BUTTON_PIN 13

void Button_Init(void){
	
	// Enable periph config clock
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	// Enable the peripheral clock of GPIO Port	
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;	

	// GPIO Mode: Input(00), Output(01), AlterFunc(10), Analog(11, reset)
	GPIOC->MODER &= ~(3U<<(2*BUTTON_PIN));		// Input
		
	// GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOC->PUPDR   &= ~(3U<<(2*BUTTON_PIN));  // No pull-up, no pull-down
	
	// Disable periph config clock
	RCC->APB2ENR &= ~RCC_APB2ENR_SYSCFGEN;
}

//******************************************************************************************
// getButton 
//******************************************************************************************
uint8_t getButton(void) {
	
	uint8_t bRet;

	bRet = !(GPIOC->IDR & (1U<<BUTTON_PIN));
	
	return bRet;
}


