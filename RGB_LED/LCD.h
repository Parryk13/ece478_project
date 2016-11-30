/*LCD.h : Put init functions here: LCD_Init(); LCD_Display(char* str);
//LCD.c : Put code into here:      void LCD_Init(){};
// NVIC should be >>5
// LSR r2, r0, #5
 
//Main .s 
    //export __main
    //import LCD_Init
    //import LCD_Display
//__main
//BL LCD_Init

//#define bool _bool

To pass arguments: (r0 -> First Argument, r1 -> Second Argument, etc.)
	AREA MyDATA
	str DCB "Hello",\0
	LDR   r0, =str
	BL    LCD_Display
*/
#include "stm32l1xx.h"
#include "core_cm3.h"
#define bool _Bool
	
#ifndef LCD
#define LCD

void LCD_Clock_Init();
void LCD_Pin_Init();
void LCD_Configure();
static void LCD_Conv_Char_Seg(uint8_t*, bool, bool, uint8_t*);
void LCD_WriteChar(uint8_t*, bool, bool, uint8_t);
void LCD_Display_String(uint8_t);
void To_String(uint8_t, int);
void ADC1_IRQHandler(void);
void Delay();
#endif