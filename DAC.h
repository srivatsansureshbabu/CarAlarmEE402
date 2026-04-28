#ifndef __STM32L476G_DAC_H
#define __STM32L476G_DAC_H

#include "stm32l476xx.h"

//#define SINE_TABLE_SIZE	100	//??
//#define	AMPLITUDE				4095.0
//#define	OFFSET					4095.0/2
//#define	PI							3.1415926
//#define	CLOCK_FREQ			80000000	//??		// 80MHz

//// Macro to convert Hz to timer counts for a given clock and sine table size
//#define HZ_TO_COUNTS(f)	(CLOCK_FREQ / f / SINE_TABLE_SIZE)

//uint16_t data[SINE_TABLE_SIZE];	// Sine table - stored in RAM
//uint8_t bKillNote;							// Flag indicating to kill the current note at the end of the sine wave cycle
//uint16_t delayVariable =1;
//volatile uint16_t waveIndex = 0;


void InitDAC(void);
void playNote(uint16_t counts, uint16_t ms);
void TIM6_Init(uint16_t arr);

#endif
