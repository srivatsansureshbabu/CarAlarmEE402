/*
----------------------------------------------------------------------------------------------------------------------------
| File:     main.c                                                                                                          | 
| Date:     04-03-2017                                                                                                      |
| Compiler: V6.6                                                                                                            |
| Hardware: Discovery kit STM32L476G-DISCO                                                                                  | 
| Program Objectives: To enable a utrasonic distance sensor to measure distance            																|		
| Display distance on LCD                                                                                                   |
----------------------------------------------------------------------------------------------------------------------------
*/ 

#include "stm32l476xx.h"
#include "I2C.h"
#include "LED.h"
#include "ssd1306.h"
#include "SysTimer.h"
#include "UART.h"
#include "Button.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "DAC.h"
#include <stdlib.h> // Memory allocation, exit(), etc.


// ultrasonic sensor declarations
volatile int overflow = 0;
volatile int current = 0;
volatile int last = 0;
volatile int time = 0;
//volatile uint16_t waveIndex = 0;

void itoa(unsigned int n, char *s);
void System_Clock_Init(void);
void TIM4_CH1_Init(void);
void TIM3_CH2_Init(void);
void TIM4_IRQHandler(void);
void Display_Centimeters(void);
void Display_Inches(void);
void OLED_DisplayString(char *str);


// DAC Speaker Declarations
//void TIM4_IRQHandler(void);
//void TIM4_CH1_Init(void);


//void InitDAC(void);
//void playNote(uint16_t counts, uint16_t ms);
//void TIM6_Init(uint16_t arr);

// DAC Global variables
// Macros and defines
#define SINE_TABLE_SIZE	100	//??
#define	AMPLITUDE				4095.0
#define	OFFSET					4095.0/2
#define	PI							3.1415926
#define	CLOCK_FREQ			80000000	//??		// 80MHz

// Macro to convert Hz to timer counts for a given clock and sine table size
#define HZ_TO_COUNTS(f)	(CLOCK_FREQ / f / SINE_TABLE_SIZE)

//uint16_t data[SINE_TABLE_SIZE];	// Sine table - stored in RAM
//uint8_t bKillNote;							// Flag indicating to kill the current note at the end of the sine wave cycle
//uint16_t delayVariable =1;

// Frequencies for given pitches
#define	NOTE_A					HZ_TO_COUNTS(440)
#define	NOTE_B					HZ_TO_COUNTS(494)
#define	NOTE_C					HZ_TO_COUNTS(523)
#define	NOTE_D					HZ_TO_COUNTS(587)
#define	NOTE_E					HZ_TO_COUNTS(659)
#define	NOTE_F					HZ_TO_COUNTS(698)
#define	NOTE_G					HZ_TO_COUNTS(784)

// Duration
#define	HALF_NOTE_MS		200
#define	QTR_NOTE_MS			150


int main(void) {
	System_Clock_Init();
	SysTick_Init();
	I2C_GPIO_init();
	I2C_Initialization(I2C1);
	UART2_Init();
	Button_Init();
	InitDAC();
	TIM4_CH1_Init();
	TIM6_Init(NOTE_E);
	LED_Init();
//	generateSignal();

	uint16_t i;
	uint16_t count;
	//ssd1306_Init();
	//OLED_DisplayString("** Lab 8 **");
	delay(1000);

	TIM3_CH2_Init(); // Triger signal
	//TIM4_CH1_Init(); // Timer input capture
	//	for(i = 0; i < SINE_TABLE_SIZE;i++){
	//	data[i] = ( (sin( (float) i/SINE_TABLE_SIZE* (2.0*PI))*AMPLITUDE)/2.0 )+OFFSET;
	
	//	printf("%d ",data[i]);

//	}
		
	
	while(1) {
		//Display_Centimeters();  // Display distance in cm.
		//playNote(NOTE_E, HALF_NOTE_MS);
		Display_Inches();     // Displayed distance in inches.
		delay(1000);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// OLED_DisplayString(char *str): Display string on OLED display
// Timer 4: 
//     * input clock = 80 MHz
//     * prescaler = 79
//     
void OLED_DisplayString(char *str) {
	ssd1306_Fill(Black);
  ssd1306_SetCursor(2, 18);
  ssd1306_WriteString(str, Font_11x18, White);
  ssd1306_UpdateScreen();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PB.6 (TIM4_CH1): Input capture for the sensor echo
// Timer 4: 
//     * input clock = 80 MHz
//     * prescaler = 79
//     
void TIM4_CH1_Init(void) {
	
	// Set PB.6 as alternate function 2
	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOBEN;

	// MODE: 00: Input mode,              01: General purpose output mode
  //       10: Alternate function mode, 11: Analog mode (reset state)	
	GPIOB->MODER &= ~(0x3<<12);    	// Clear bit 13 and bit 12
  GPIOB->MODER |= (0x2<<12);			// Set to Alternate Function Mode
	
	GPIOB->OSPEEDR &= ~(0x3<<12);	  // Clear bits 12 and 13 to set Output speed of the pin
	GPIOB->OSPEEDR |= (0x3<<12);		// Set output speed of the pin to 40MHz (Highspeed = 0b11)
		
	GPIOB->PUPDR &= ~(0x3 << 12);

	GPIOB->OTYPER &= ~(1<<6);

	GPIOB->AFR[0] &= ~GPIO_AFRL_AFRL6; 	// Clear pin 6 for alternate function 
	GPIOB->AFR[0] |= (0x2<<24);					// Set pin 6 to alternate function 2 (enables TIM4)
	
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN; 	// Enable the clock of timer 4
	
	// Set TIM4 Channel 1 as input capture
	TIM4->PSC = 79;		// Set up an appropriate prescaler to slow down the clock or timer counter

	// Counting direction: 0 = up-counting, 1 = down-counting
	TIM4->CR1 &= ~TIM_CR1_DIR; 
		
	// Set the direction as input and select the active input
	// CC1S[1:0] for channel 1;
	// 00 = output
	// 01 = input, CC1 is mapped on timer Input 1
	// 10 = input, CC1 is mapped on timer Input 2
	// 11 = input, CC1 is mapped on slave timer
	TIM4->CCMR1 &= ~TIM_CCMR1_CC1S;
	TIM4->CCMR1 |= 0x1; 
											
	// Disable digital filtering by clearing IC1F[3:0] bits
	// because we want to capture every event
	TIM4->CCMR1 &= ~TIM_CCMR1_IC1F;
	
	// Select the edge of the active transition
	// Detect both rising and falling edges in this example
	// CC1NP:CC1P bits
	// 0-0 = rising edge,
	// 0-1 = falling edge,
	// 1-0 = reserved,
	// 1-1 = both edges
	TIM4->CCER |= (1<<1 | 1<<3);		// Both rising and falling edges.
	
	// Program the input prescaler
	// To capture each valid transition, set the input prescaler to zero;
	// IC1PSC[1:0] bits (input capture 1 prescaler)
	TIM4->CCMR1 &= ~(TIM_CCMR1_IC1PSC); // Clear filtering because we need to capture every event
	
	// Enable Capture/compare output enable for channel 1
	TIM4->CCER |= TIM_CCER_CC1E;    
	
	// Enable related interrupts
	TIM4->DIER |= TIM_DIER_CC1IE;		// Enable Capture/Compare interrupts for channel 1
	TIM4->DIER |= TIM_DIER_UIE;			// Enable update interrupts
	
	TIM4->CR1 |= TIM_CR1_CEN;				// Enable the counter
	
	NVIC_SetPriority(TIM4_IRQn, 1); // Set priority to 1

	NVIC_EnableIRQ(TIM4_IRQn);      // Enable TIM4 interrupt in NVIC
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PB.5 (TIM3_CH2): Trigger signal to the sensor,  pulse width = 10 us
// Timer 1: input clock 80 Mhz
//   * prescaler = ??? 
//   * counter clock frequency = 80 MHz / (??? + 1) = 0.1 MHz
//   * CCR = 1
//   * pulse width = CCR * 1/0.1MHz = 10 us
//   * ARR = 0xFFFF (max: 65535)
//   * period = (ARR + 1) * 1/0.1MHz = 0.6 s
//
void TIM3_CH2_Init(void) {

	// Set PB.5 as alternate function 1
	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOBEN;

	// MODE: 00: Input mode,              01: General purpose output mode
  //       10: Alternate function mode, 11: Analog mode (reset state)	
	GPIOB->MODER &= ~(0x3<<(2*5)); // Clear mode bits
	GPIOB->MODER |=  (0x2<<(2*5)); // Set to Alternate Function Mode  
	
	GPIOB->AFR[0] &= ~(0xF<<(4*5));	// Clear alternate function 
	GPIOB->AFR[0] |=  (0x2<<(4*5)); // alternate function 2 (enables TIM3)
 
	// Configure PullUp/PullDown to No Pull-Up, No Pull-Down 
	// Set pin 5 to push-pull output type (0) 
	GPIOB->PUPDR &= ~(0x3 << (2*5));

	// Set TIM1 Channel 2 as PWM output
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;	// Enable the clock of TIM3
	
	TIM3->PSC = 79;				// Set Prescaler  
	
	TIM3->ARR = 0xFFFF; 		// Set auto-reload register to 65535

	// Counting direction: 0 = up-counting, 1 = down-counting
	TIM3->CR1 &= ~TIM_CR1_DIR; 
	
	TIM3->CCMR1 &= ~(TIM_CCMR1_OC2M); // Clear OC2M (Channel 2) 
	TIM3->CCMR1 |= (0x6 << 12); 			// Enable PWM Mode 1, upcounting, on Channel 2 
	TIM3->CCMR1 |= (TIM_CCMR1_OC2PE); // Enable output preload bit for channel 2 
	
	TIM3->CR1  |= (TIM_CR1_ARPE); 	// Set Auto-Reload Preload Enable 
	TIM3->CCER |= TIM_CCER_CC2E; 		// Set CC2E Bit 
	TIM3->CCER |= TIM_CCER_CC2NE; 	// Set CC2NE Bit 
	
	// Set Main Output Enable (MOE) bit
	// Set Off-State Selection for Run mode (OSSR) bit
	// Set Off-State Selection for Idle mode (OSSI) bit
	TIM3->BDTR |= TIM_BDTR_MOE | TIM_BDTR_OSSR | TIM_BDTR_OSSI; 
	
	TIM3->CCR2 &= ~(TIM_CCR2_CCR2); 	// Clear CCR2 (Channel 2) 
	TIM3->CCR2 |= 0x1; 							  // Load the register 
	
	TIM3->CR1 |= TIM_CR1_CEN; 	// Enable the counter
}

//void TIM6_CH1_Init(void) {

//	// Set PB.5 as alternate function 1
//	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOBEN;

//	// MODE: 00: Input mode,              01: General purpose output mode
//  //       10: Alternate function mode, 11: Analog mode (reset state)	
//	GPIOB->MODER &= ~(0x3<<(2*5)); // Clear mode bits
//	GPIOB->MODER |=  (0x2<<(2*5)); // Set to Alternate Function Mode  
//	
//	GPIOB->AFR[0] &= ~(0xF<<(4*5));	// Clear alternate function 
//	GPIOB->AFR[0] |=  (0x2<<(4*5)); // alternate function 2 (enables TIM3)
// 
//	// Configure PullUp/PullDown to No Pull-Up, No Pull-Down 
//	// Set pin 5 to push-pull output type (0) 
//	GPIOB->PUPDR &= ~(0x3 << (2*5));

//	// Set TIM1 Channel 2 as PWM output
//	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;	// Enable the clock of TIM3
//	
//	TIM3->PSC = 79;				// Set Prescaler  
//	
//	TIM3->ARR = 0xFFFF; 		// Set auto-reload register to 65535

//	// Counting direction: 0 = up-counting, 1 = down-counting
//	TIM3->CR1 &= ~TIM_CR1_DIR; 
//	
//	TIM3->CCMR1 &= ~(TIM_CCMR1_OC2M); // Clear OC2M (Channel 2) 
//	TIM3->CCMR1 |= (0x6 << 12); 			// Enable PWM Mode 1, upcounting, on Channel 2 
//	TIM3->CCMR1 |= (TIM_CCMR1_OC2PE); // Enable output preload bit for channel 2 
//	
//	TIM3->CR1  |= (TIM_CR1_ARPE); 	// Set Auto-Reload Preload Enable 
//	TIM3->CCER |= TIM_CCER_CC2E; 		// Set CC2E Bit 
//	TIM3->CCER |= TIM_CCER_CC2NE; 	// Set CC2NE Bit 
//	
//	// Set Main Output Enable (MOE) bit
//	// Set Off-State Selection for Run mode (OSSR) bit
//	// Set Off-State Selection for Idle mode (OSSI) bit
//	TIM3->BDTR |= TIM_BDTR_MOE | TIM_BDTR_OSSR | TIM_BDTR_OSSI; 
//	
//	TIM3->CCR2 &= ~(TIM_CCR2_CCR2); 	// Clear CCR2 (Channel 2) 
//	TIM3->CCR2 |= 0x1; 							  // Load the register 
//	
//	TIM3->CR1 |= TIM_CR1_CEN; 	// Enable the counter
//}

void TIM4_IRQHandler(void) { 
	if((TIM4->SR & TIM_SR_UIF) != 0) {	// Check if overflow has taken place 
		overflow++;												// If overflow occurred, increment counter 
		TIM4->SR &= ~TIM_SR_UIF; 					// Clear the UIF Flag
	}
	
	// Captures events with consideration of overflows
	if((TIM4->SR & TIM_SR_CC1IF) != 0) { 
		current = TIM4->CCR1;  // Reading CCR1 clears CC1IF
		time = (current - last) + (overflow*65536);
		last = current; 
		overflow = 0; 
	}
}

void Display_Centimeters(void) {
	// Display distance in centimeters on LCD
	char str[6];			// x = measurement
	float distance;
	
	distance = (time/58.0); 
	sprintf(str, "%.2f", distance);
	OLED_DisplayString(str);		// Display measurement on LCD
}

void Display_Inches(void) {
	// Display distance in inches on LCD
	char str[6];			// x = measurement
	float distance;
	
	distance = (time/148.0); 
	//sprintf(str, "%.2f", distance);

	if(distance < 5){
		printf("Enemy Spotted!");
		playNote(NOTE_E, HALF_NOTE_MS);
		LED_On();
	}
	else{
		LED_Off();
	}
	printf("Distance: %.2f: ",distance);
	//OLED_DisplayString(str);		// Display measurement on LCD	
}
