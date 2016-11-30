				INCLUDE stm32l1xx_constants.s       ; Load Constant Definitions
				INCLUDE stm32l1xx_tim_constants.s   ; TIM Constants

				AREA ISR, CODE, READONLY
				ENTRY
;******************IRQ Handler***********************
ADC1_IRQHandler PROC
				EXPORT ADC1_IRQHandler
				LDR   r10, =Result
				
				LDR   r7, =ADC1_BASE
				LDR   r0, [r7, #ADC_SR]
				AND   r1, r0, #ADC_SR_EOC
				CMP   r1, #0
				BEQ   Exit
				LDR   r2, [r7, #ADC_DR]
				LDR   r2, [r2]
				STR   r10, [r2]
Exit								
				ENDP
;****************************************************
;*******************Variables************************
				AREA variables, DATA, READWRITE
				ALIGN
Result	        DCD   0; 
Result_1        DCD   0; 
Result_2        DCD   0; 
Result_3        DCD   0;
Result_4        DCD   0;
;****************************************************

				END
				