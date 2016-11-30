#include "stm32l1xx.h"
#include "core_cm3.h"
#include "LCD.h"

/* Constant table for cap characters 'A' --> 'Z' */
const uint16_t CapLetterMap[26] = {
        /* A      B      C      D      E      F      G      H      I  */
        0xFE00,0x6714,0x1d00,0x4714,0x9d00,0x9c00,0x3f00,0xfa00,0x0014,
        /* J      K      L      M      N      O      P      Q      R  */
        0x5300,0x9841,0x1900,0x5a48,0x5a09,0x5f00,0xFC00,0x5F01,0xFC01,
        /* S      T      U      V      W      X      Y      Z  */
        0xAF00,0x0414,0x5b00,0x18c0,0x5a81,0x00c9,0x0058,0x05c0
};
/* Constant table for number '0' --> '9' */
const uint16_t NumberMap[10] = {
        /* 0      1      2      3      4      5      6      7      8      9  */
        0x5F00,0x4200,0xF500,0x6700,0xEa00,0xAF00,0xBF00,0x04600,0xFF00,0xEF00
};
int Result, Result_1, Result_2, Result_3, Result_4;

void LCD_Pin_Init(void){
		//Initialize LCD Pins
		
		RCC->AHBENR    |= 0x00000007;  					//Enable Clocks for GPIOX (A,B,C)
		
		GPIOA->MODER   &= 0x3FC0FF00; 
		GPIOA->MODER   |= 0x802A00A8; 					//Also sets PA.0 as Digital Input
		
		GPIOA->AFR[0]  &= 0xFFFF000F;
		GPIOA->AFR[0]  |= 0x0000BBB0;
		GPIOA->AFR[1]  &= 0x0FFFF000;
		GPIOA->AFR[1]  |= 0xB0000BBB;
	
		GPIOA->OTYPER  &= ~(0x1);								//Set pin 0 as push-pull
	  GPIOA->OSPEEDR &= ~(0x03);							//Speed mask
	  GPIOA->OSPEEDR |= 0x01;
	  GPIOA->PUPDR   &= ~(0x03);
		
		GPIOB->MODER   &= 0x0000F03F;
		GPIOB->MODER   |= 0xAAAA0A80;
		
		GPIOB->AFR[0]  &= 0xFF000FFF;
		GPIOB->AFR[0]  |= 0x00BBB000;
		GPIOB->AFR[1]  &= 0x00000000;
		GPIOB->AFR[1]  |= 0xBBBBB1BB;
		
		GPIOC->MODER   &= 0xFF000F00;
		GPIOC->MODER   |= 0x00AAA0AA;
		
		GPIOC->AFR[0]  &= 0x00FF0000;
		GPIOC->AFR[0]  |= 0xBB00BBBB;
		GPIOC->AFR[1]  &= 0xFFFF0000;
		GPIOC->AFR[1]  |= 0x0000BBBB;
}
void LCD_Clock_Init(void){
		//Enable the LCD Clock
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
		PWR->CR 		 |= PWR_CR_DBP;
		RCC->CSR 		 |= RCC_CSR_RTCSEL_LSI;
		RCC->CSR 		 |= RCC_CSR_RTCEN;
		
		RTC->WPR = 0xCA;
		RTC->WPR = 0x53;
		
		while(RCC->CSR & RCC_CR_MSIRDY == 0);
		
		RCC->APB1ENR |= RCC_APB1ENR_LCDEN;
		
		RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
		
		RCC->CSR |= RCC_CSR_LSION;
		while((RCC->CSR & RCC_CSR_LSIRDY) == 0);
		
		RCC->CSR &= ~RCC_CSR_RTCSEL_LSI;
		RCC->CSR |= RCC_CSR_RTCSEL_LSI;
		RCC->CSR |= RCC_CSR_RTCEN;
}
void LCD_Configure(void){
	LCD_Clock_Init();
	LCD_Pin_Init();
	
	LCD->CR  &= ~0xFF;
	LCD->CR  |= 0x4C; 												//Set Bias: 1/3 and Duty: 1/4
	LCD->FCR |= 0x1000;												//Set Contrast Control to VLCD4; 
	LCD->CR  |= 0xCC;													//Enabe MUX Segment
	LCD->FCR |= 0x7 << 4;
	//PON[2:0] = 0x111;
	
	while((LCD->SR & LCD_SR_FCRSR) == 0); 		//Wait for FCRSR to be set
	
	LCD->CR  |= 0xCD;		//Enable LCD
	
	while((LCD->CR & LCD_CR_LCDEN) == 0);			//Wait for LCD to be enabled
	while((LCD->SR & LCD_SR_RDY) == 0);				//Wait for RDY bit to be set
}
static void LCD_Conv_Char_Seg(uint8_t* c, bool point, bool column, uint8_t* digit) {
	// Converts an ascii char to the a LCD digit
  uint16_t ch = 0 ;
  uint8_t i,j;
 
  switch (*c) {
    case ' ' : 
      ch = 0x00;
      break;
 
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':			
      ch = NumberMap[*c-0x30];		
      break;
 
    default:
      /* The character c is one letter in upper case*/
      if ( (*c < 0x5b) && (*c > 0x40) ) {
        ch = CapLetterMap[*c - 'A'];
      }
      /* The character c is one letter in lower case*/
      if ( (*c <0x7b) && ( *c> 0x60) ) {
        ch = CapLetterMap[*c - 'a'];
      }
      break;
  }
 
  /* Set the digital point can be displayed if the point is on */
  if (point) {
    ch |= 0x0002;
  }
 
  /* Set the "COL" segment in the character that can be displayed if the column is on */
  if (column) {
    ch |= 0x0020;
  }		
 
  for (i = 12,j=0; j<4; i-=4,j++) {
    digit[j] = (ch >> i) & 0x0f; //To isolate the less signifiant digit
  }
}
void LCD_WriteChar(uint8_t* ch, bool point, bool colon, uint8_t position) {
  uint8_t digit[4];     /* Digit frame buffer */
	
	// Convert displayed character in segment in array digit
	LCD_Conv_Char_Seg(ch, point, colon, digit);
 
  // Wait until LCD Ready */  
  while ((LCD->SR & LCD_SR_UDR) != 0); // Wait for Update Display Request Bit
 
  switch (position) {
    /* Position 1 on LCD (Digit 1)*/
    case 1:
      LCD->RAM[0] &= 0xcffffffc;
      LCD->RAM[2] &= 0xcffffffc;
      LCD->RAM[4] &= 0xcffffffc;
      LCD->RAM[6] &= 0xcffffffc;
 
      LCD->RAM[0] |= ((digit[0]& 0x0c) << 26 ) | (digit[0]& 0x03) ; // 1G 1B 1M 1E	    
      LCD->RAM[2] |= ((digit[1]& 0x0c) << 26 ) | (digit[1]& 0x03) ; // 1F 1A 1C 1D 
      LCD->RAM[4] |= ((digit[2]& 0x0c) << 26 ) | (digit[2]& 0x03) ; // 1Q 1K 1Col 1P                                                                                                                                    
      LCD->RAM[6] |= ((digit[3]& 0x0c) << 26 ) | (digit[3]& 0x03) ; // 1H 1J 1DP 1N
 
      break;
 
    /* Position 2 on LCD (Digit 2)*/
    case 2:
      LCD->RAM[0] &= 0xf3ffff03;
      LCD->RAM[2] &= 0xf3ffff03;      
      LCD->RAM[4] &= 0xf3ffff03;
      LCD->RAM[6] &= 0xf3ffff03;
 
      LCD->RAM[0] |= ((digit[0]& 0x0c) << 24 )|((digit[0]& 0x02) << 6 )|((digit[0]& 0x01) << 2 ) ; // 2G 2B 2M 2E	  
      LCD->RAM[2] |= ((digit[1]& 0x0c) << 24 )|((digit[1]& 0x02) << 6 )|((digit[1]& 0x01) << 2 ) ; // 2F 2A 2C 2D
      LCD->RAM[4] |= ((digit[2]& 0x0c) << 24 )|((digit[2]& 0x02) << 6 )|((digit[2]& 0x01) << 2 ) ; // 2Q 2K 2Col 2P
      LCD->RAM[6] |= ((digit[3]& 0x0c) << 24 )|((digit[3]& 0x02) << 6 )|((digit[3]& 0x01) << 2 ) ; // 2H 2J 2DP 2N
 
      break;
 
    /* Position 3 on LCD (Digit 3)*/
    case 3:
      LCD->RAM[0] &= 0xfcfffcff;
      LCD->RAM[2] &= 0xfcfffcff;
      LCD->RAM[4] &= 0xfcfffcff;
      LCD->RAM[6] &= 0xfcfffcff;
 
      LCD->RAM[0] |= ((digit[0]& 0x0c) << 22 ) | ((digit[0]& 0x03) << 8 ) ; // 3G 3B 3M 3E	
      LCD->RAM[2] |= ((digit[1]& 0x0c) << 22 ) | ((digit[1]& 0x03) << 8 ) ; // 3F 3A 3C 3D
      LCD->RAM[4] |= ((digit[2]& 0x0c) << 22 ) | ((digit[2]& 0x03) << 8 ) ; // 3Q 3K 3Col 3P
      LCD->RAM[6] |= ((digit[3]& 0x0c) << 22 ) | ((digit[3]& 0x03) << 8 ) ; // 3H 3J 3DP 3N
 
      break;
 
    /* Position 4 on LCD (Digit 4)*/
    case 4:
      LCD->RAM[0] &= 0xffcff3ff;
      LCD->RAM[2] &= 0xffcff3ff;
      LCD->RAM[4] &= 0xffcff3ff;
      LCD->RAM[6] &= 0xffcff3ff;
 
      LCD->RAM[0] |= ((digit[0]& 0x0c) << 18 ) | ((digit[0]& 0x03) << 10 ) ; // 4G 4B 4M 4E	
      LCD->RAM[2] |= ((digit[1]& 0x0c) << 18 ) | ((digit[1]& 0x03) << 10 ) ; // 4F 4A 4C 4D
      LCD->RAM[4] |= ((digit[2]& 0x0c) << 18 ) | ((digit[2]& 0x03) << 10 ) ; // 4Q 4K 4Col 4P
      LCD->RAM[6] |= ((digit[3]& 0x0c) << 18 ) | ((digit[3]& 0x03) << 10 ) ; // 4H 4J 4DP 4N
 
      break;
 
     default:
      break;
  }
  LCD->SR |= LCD_SR_UDR; 
  while ((LCD->SR & LCD_SR_UDD) == 0);
}
void LCD_Display_String(uint8_t C[7]){
	char i;
	for(i = 1; i < 6; i++)
	{
		LCD_WriteChar(C, 0, 0, i);
		C += 1;
	}
}
void To_String(uint8_t str[], int Num)					
{
		int i, Rem, Len = 0, N;
		N = Num;

    while (N != 0)
		{
			Len++;
			N /= 10;
		}
		for (i = 0; i < Len; i++)
		{
			Rem = Num % 10;
			Num = Num / 10;
			str[Len - (i + 1)] = Rem + '0';				
		}
		str[Len] = '\0';
}
void ADC1_IRQHandler(void){
	if(ADC1->SR & ADC_SR_EOC){
		Result = ADC1->DR;
		//Result = (Result / 4095) * 3;
	}
	else
	if(ADC1->SR & ADC_SR_JEOC){
		Result_1 = ADC1->JDR1;
		Result_2 = ADC1->JDR2;
		Result_3 = ADC1->JDR3;
		Result_4 = ADC1->JDR4;
		ADC1->SR &= ~(ADC_SR_JEOC);
	}
}
void Delay(){
	int i;
	for(i = 0; i < 750000; i++);
}