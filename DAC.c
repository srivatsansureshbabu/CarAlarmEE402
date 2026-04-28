#include <stdio.h>
#include "stm32l476xx.h"
#include <math.h>
#include "DAC.h"
#include "SysClock.h"
#include "SysTimer.h"
#define SINE_TABLE_SIZE	100	//??
#define	AMPLITUDE				4095.0
#define	OFFSET					4095.0/2
#define	PI							3.1415926
#define	CLOCK_FREQ			80000000	//??		// 80MHz

// Macro to convert Hz to timer counts for a given clock and sine table size
#define HZ_TO_COUNTS(f)	(CLOCK_FREQ / f / SINE_TABLE_SIZE)

uint16_t data[SINE_TABLE_SIZE];	// Sine table - stored in RAM
uint8_t bKillNote;							// Flag indicating to kill the current note at the end of the sine wave cycle
uint16_t delayVariable =1;
volatile uint16_t waveIndex = 0;


// Generates a signal in the form of a sine wave, which is required for the DAC. 
void generateSignal(void){
		uint16_t i;

			for(i = 0; i < SINE_TABLE_SIZE;i++){
					data[i] = ( (sin( (float) i/SINE_TABLE_SIZE* (2.0*PI))*AMPLITUDE)/2.0 )+OFFSET;
//						printf("%d ",data[i]);
			}
				
}


void TIM6_Init(uint16_t arr) {
	
	
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;

    TIM6->PSC = 79;          // 80MHz / 80 = 1 MHz
    TIM6->ARR = arr;

		TIM6->DIER |= TIM_DIER_UIE;          // enable update interrupt

    // TRGO on update event
    TIM6->CR2 &= ~TIM_CR2_MMS;
    TIM6->CR2 |= (2 << 4);

    TIM6->CR1 |= TIM_CR1_CEN;
	
		NVIC_SetPriority(TIM6_DAC_IRQn, 0);
		NVIC_EnableIRQ(TIM6_DAC_IRQn);
	
}


void TIM6_DAC_IRQHandler(void) {
		

    if (TIM6->SR & TIM_SR_UIF) {
        TIM6->SR &= ~TIM_SR_UIF;

        if (!bKillNote) {
            DAC->DHR12R1 = data[waveIndex];
            waveIndex = (waveIndex + 1) % SINE_TABLE_SIZE;
        } else {
            DAC->DHR12R1 = 2048; // silence (midpoint)
        }
    }
}

void playNote(uint16_t counts, uint16_t ms)
{
	bKillNote = 0;									// Clear bKillNote flag to enable playing note
	TIM6->ARR = counts;							// Update the ARR with the note's rollover count
	TIM6->CR1 |= TIM_CR1_CEN;				// Enable the counter
	delay(ms);											// Give the note some playing time
	

	bKillNote = 1;									// Set bKillNote flag to silence note
	
	delay(ms);	// Give the note some off time
	//delayVariable = delayVariable+10;

}

void InitDAC(void) {
	// Enable DAC clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_DAC1EN;
	
	// Disable DAC
	DAC->CR &= ~( DAC_CR_EN1 | DAC_CR_EN2 );
	
	// Configure both CH1 and CH2 for mode 0 (connected to external pin with buffer enabled)
	DAC->MCR = 0;
	
	// Enable trigger for CH1
	DAC->CR |= DAC_CR_TEN1;
	
	// Enable software trigger for CH1
	//DAC->CR |= DAC_CR_TSEL1;
	DAC->CR &= ~DAC_CR_TSEL1; // select TIM6 TRGO (000)
	// Enable DAC CH1
	DAC->CR |= DAC_CR_EN1;

	// Enable the clock of GPIO port A
	RCC-> AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
	// Set I/O mode as analog (11) for PA4 (CH1)
	GPIOA->MODER |= GPIO_MODER_MODER4_0 | GPIO_MODER_MODER4_1;
}




