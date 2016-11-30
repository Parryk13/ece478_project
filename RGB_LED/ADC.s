;********************************************************************************************************
; file:     main.s
; author:   Chris Santos, christopher.santos@maine.edu
; date:     03/22/2015
; version:  1.0
; compiler: MDK 5.12
; hardware: Discovery kit with STM32L152RCT6 MCU
; description: ADC
;********************************************************************************************************

		INCLUDE stm32l1xx_constants.s       ; Load Constant Definitions
		INCLUDE stm32l1xx_tim_constants.s   ; TIM Constants

;********************************************************************************************************
;                                      CODE AREA
;********************************************************************************************************
		AREA main, CODE, READONLY
		EXPORT __main                       ; make __main visible to linker
		ENTRY


;********************************************************************************************************
;                                      MAIN FUNCTION
; Description  : xxxxxx
; C prototype  : void main (void)
; Note         : Has a dead loop and never exit
; Argument(s)  : none
; Return value : none
;********************************************************************************************************
__main	PROC
		import ADC1_IRQHandler
		import Delay
		extern Result
	
		BL    HSI
	    BL    GPIO_Setup
		BL    ADC_Setup
		BL    Timer4_Setup
		LDR   r4, =Result
		LDR   r7, =TIM4_BASE
		LDR   r2, =TIM_CCR1_CCR1
		LDR   r3, [r7, #TIM_CCR1]
		LDR   r6, =4090
		
while   LDR   r5, [r4]
		CMP   r5, r6
		BGT   Next
		BIC   r3, r3, r2
		ORR   r3, r3, r5
		STR   r3, [r7, #TIM_CCR1]
		B     while
		
Next    BIC   r3, r3, r2
		STR   r3, [r7, #TIM_CCR1]
		BL    Delay
		
		B     while

		ENDP
		ALIGN
			
GPIO_Setup PROC
		
		;**********GPIOB Enable/Setup***************
		; Enable clock for Timer and GPIOB
		LDR   r7, =RCC_BASE
		LDR   r0, [r7, #RCC_AHBENR]
		BIC   r0, r0, #RCC_AHBENR_GPIOBEN
		ORR   r0, r0, #RCC_AHBENR_GPIOBEN
		STR   r0, [r7, #RCC_AHBENR]
		; Set GPIOB MODER, 6: Output
		LDR   r7, =GPIOB_BASE
		LDR   r0, [r7, #GPIO_MODER]		
		BIC   r0, r0, #0x00003000
		ORR   r0, r0, #0x00001000
		STR   r0, [r7, #GPIO_MODER]
		; Set as Push-Pull
		LDR   r0, [r7, #GPIO_OTYPER]
		BIC   r0, r0, #GPIO_OTYPER_OT_6
		STR   r0, [r7, #GPIO_OTYPER]
		; Set Output Speed
		LDR   r0, [r7, #GPIO_OSPEEDR]
		ORR   r0, r0, #GPIO_OSPEEDER_OSPEEDR6
		STR   r0, [r7, #GPIO_OSPEEDR]
		;********************************************
		
		; Set GPIOB MODER
		LDR   r7, =GPIOB_BASE
		LDR   r0, [r7, #GPIO_MODER]		
		BIC   r0, r0, #0x0000F000
		ORR   r0, r0, #0x0000A000
		STR   r0, [r7, #GPIO_MODER]
		; Set GPIOB Function
		LDR   r0, [r7, #GPIO_AFRL]
		BIC   r0, r0, #0xFF000000
		ORR   r0, r0, #0x22000000
		STR   r0, [r7, #GPIO_AFRL]
		; Set GPIOB OTYPER
		LDR   r0, [r7, #GPIO_OTYPER]
		BIC   r0, r0, #0x00C0
		ORR   r0, r0, #0x0000
		STR   r0, [r7, #GPIO_OTYPER]
		; Set GPIOB PUPDR
		LDR   r0, [r7, #GPIO_PUPDR]
		BIC   r0, r0, #0x000000C0
		ORR   r0, r0, #0x00000000
		STR   r0, [r7, #GPIO_PUPDR]
		
		;**********GPIOC Enable/Setup****************
		LDR   r7, =RCC_BASE
		LDR   r0, [r7, #RCC_AHBENR]
		BIC   r0, r0, #RCC_AHBENR_GPIOCEN
		ORR   r0, r0, #RCC_AHBENR_GPIOCEN
		STR   r0, [r7, #RCC_AHBENR]
		; Set GPIOC MODER, 0: Analog
		LDR   r7, =GPIOC_BASE
		LDR   r0, [r7, #GPIO_MODER]
		BIC   r0, r0, #0x00000003
		ORR   r0, r0, #0x00000003
		STR   r0, [r7, #GPIO_MODER]
		;********************************************
		
		BX   LR
		ENDP
		;********************************************
			
HSI     PROC
		;**************HSI Enable/Select*************
		LDR   r0, =RCC_BASE
		
		; Turn on HSI Oscillator
		LDR   r1, [r0, #RCC_CR]
		ORR   r1, r1, #RCC_CR_HSION
		STR   r1, [r0, #RCC_CR]
		
		; Select HSI as System Clock
		LDR   r1, [r0, #RCC_CFGR]
		BIC   r1, r1, #RCC_CFGR_SW
		ORR   r1, r1, #RCC_CFGR_SW_HSI
		STR   r1, [r0, #RCC_CFGR]
		
		; Wait for HSI Stable
WaitHSI LDR   r1, [r0, #RCC_CR]
		AND   r1, r1, #RCC_CR_HSIRDY
		CMP   r1, #0
		BEQ   WaitHSI
		BX    LR
		ENDP
		;********************************************
	
ADC_Setup PROC
		;**************ADC Enable/Select*************
		LDR   r7, =RCC_BASE
		LDR   r0, [r7, #RCC_APB2ENR]
		ORR   r0, r0, #RCC_APB2ENR_ADC1EN
		STR   r0, [r7, #RCC_APB2ENR]
		; Turn Off ADC Conversion
		LDR   r7, =ADC1_BASE
		LDR   r0, [r7, #ADC_CR2]
		BIC   r0, r0, #ADC_CR2_ADON
		STR   r0, [r7, #ADC_CR2]
		; Set Length of RCS to 1
		LDR   r0, [r7, #ADC_SQR1]
		BIC   r0, r0, #ADC_SQR1_L
		STR   r0, [r7, #ADC_SQR1]
		; Set Channel 10 as 1st Conversion
		LDR   r0, [r7, #ADC_SQR5]
		BIC   r0, r0, #ADC_SQR5_SQ1
		ORR   r0, r0, #0x0A
		STR   r0, [r7, #ADC_SQR5]
		; Set Sample TIme Register
		LDR   r0, [r7, #ADC_SMPR2]
		BIC   r0, r0, #ADC_SMPR2_SMP10
		ORR   r0, r0, #ADC_SMPR2_SMP10_1
		STR   r0, [r7, #ADC_SMPR2]
		; Enable End of COnverson Interrupt
		LDR   r0, [r7, #ADC_CR1]
		ORR   r0, r0, #ADC_CR1_EOCIE
		STR   r0, [r7, #ADC_CR1]
		; Enable Continuous Conversion Mode
		LDR   r0, [r7, #ADC_CR2]
		ORR   r0, r0, #ADC_CR2_CONT
		STR   r0, [r7, #ADC_CR2]
		; Configure Delay Selection
		LDR   r0, [r7, #ADC_CR2]
		BIC   r0, r0, #ADC_CR2_DELS
		ORR   r0, r0, #ADC_CR2_DELS_0
		STR   r0, [r7, #ADC_CR2]
		; Set ADC Interrupt Priority
		LDR   r0, =ADC1_IRQn
		LDR   r1, =1
		LSLS  r2, r1, #4
		LDR   r3, =NVIC_BASE
		LDR   r4, =NVIC_IPR0
		ADD   r3, r3, r4
		STRB  r2, [r3, r0]
		; Enable ADC Interrupt
		LDR   r3, =NVIC_BASE
		LDR   r4, [r3, #NVIC_ISER0]
		ORR   r4, r4, #(0x01<<18)
		STR   r4, [r3, #NVIC_ISER0]
		; Enable ADC Conversion
		LDR   r0, [r7, #ADC_CR2]
		ORR   r0, r0, #ADC_CR2_ADON
		STR   r0, [r7, #ADC_CR2]
		; Start Conversion
		LDR   r0, [r7, #ADC_CR2]
		ORR   r0, r0, #ADC_CR2_SWSTART
		STR   r0, [r7, #ADC_CR2]
		
		BX    LR
		ENDP	
			
;**********Timer Enable/Setup***************
Timer4_Setup PROC
		LDR   r6, =0xFF
		; Enable clock on timer 4
		LDR   r7, =RCC_BASE
		LDR   r0, [r7, #RCC_APB1ENR]
		BIC   r0, r0, #RCC_APB1ENR_TIM4EN
		ORR   r0, r0, #RCC_APB1ENR_TIM4EN
		STR   r0, [r7, #RCC_APB1ENR]
		LDR   r7, =TIM4_BASE
		; Set prescaler
		LDR   r0, [r7, #TIM_PSC]
		LDR   r1, =TIM_PSC_PSC
		BIC   r0, r0, r1
		STR   r0, [r7, #TIM_PSC]
		; Set auto-reload value
		LDR   r0, [r7, #TIM_ARR]
		LDR   r1, = TIM_ARR_ARR
		BIC   r0, r0, r1
		ORR   r0, r0, r6
		STR   r0, [r7, #TIM_ARR]
		; Set PWM mode on channel 1
		LDR   r0, [r7, #TIM_CCMR1]
		BIC   r0, r0, #TIM_CCMR1_OC1M
		ORR   r0, r0, #TIM_CCMR1_OC1M_2
		ORR   r0, r0, #TIM_CCMR1_OC1M_1
		; Enable output preload on channel 1
		BIC   r0, r0, #TIM_CCMR1_OC1PE
		ORR   r0, r0, #TIM_CCMR1_OC1PE
		STR   r0, [r7, #TIM_CCMR1]
		; Enable auto-reload preload on channel 1
		LDR   r0, [r7, #TIM_CR1]
		BIC   r0, r0, #TIM_CR1_ARPE
		ORR   r0, r0, #TIM_CCER_CC1E
		STR   r0, [r7, #TIM_CR1]
		; Enable ouput on channel 1
		LDR   r0, [r7, #TIM_CCER]
		ORR   r0, r0, #TIM_CCER_CC1E
		STR   r0, [r7, #TIM_CCER]
		; Enable output compare register for channel 1
		LDR   r0, [r7, #TIM_CCR1]
		LDR   r1, =TIM_CCR1_CCR1
		BIC   r0, r0, r1
		ORR   r0, r0, #0xFF
		STR   r0, [r7, #TIM_CCR1]
		; Enable counter on channel 1
		LDR   r0, [r7, #TIM_CR1]
		ORR   r0, r0, #TIM_CR1_CEN
		STR   r0, [r7, #TIM_CR1]
		
		BX    LR
		ENDP
;********************************************
			
;********************************************************************************************************
;                                      DATA AREA
;********************************************************************************************************
		AREA myData, DATA, READWRITE
		ALIGN
C 		   DCB   0,0,0,0,0,0,0

;********************************************************************************************************
;                                      ASSEMBLY FILE END
;********************************************************************************************************
		END
