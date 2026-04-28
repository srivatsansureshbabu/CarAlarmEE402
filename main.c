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
#include "UltrasonicSensor.h"
#include "SysClock.h"
//// ultrasonic sensor declarations
//volatile int overflow = 0;
//volatile int current = 0;
//volatile int last = 0;
//volatile int time = 0;
//volatile uint16_t waveIndex = 0;

//void itoa(unsigned int n, char *s);
//void System_Clock_Init(void);
//void TIM4_CH1_Init(void);
//void TIM3_CH2_Init(void);
//void TIM4_IRQHandler(void);
//void Display_Centimeters(void);
//void Display_Inches(void);
//void OLED_DisplayString(char *str);


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
//#define	NOTE_A					HZ_TO_COUNTS(440)
//#define	NOTE_B					HZ_TO_COUNTS(494)
//#define	NOTE_C					HZ_TO_COUNTS(523)
//#define	NOTE_D					HZ_TO_COUNTS(587)
#define	NOTE_E					HZ_TO_COUNTS(659)
//#define	NOTE_F					HZ_TO_COUNTS(698)
//#define	NOTE_G					HZ_TO_COUNTS(784)

//// Duration
//#define	HALF_NOTE_MS		200
//#define	QTR_NOTE_MS			150


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

	uint16_t i;
	uint16_t count;
	
	delay(1000);

	TIM3_CH2_Init(); // Triger signal
		
	
	while(1) {
		Display_Inches();     // Displayed distance in inches.
		delay(1000);
	}
}


