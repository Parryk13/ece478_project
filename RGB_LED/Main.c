#include "stm32l1xx.h"
#include "core_cm3.h"
#define led 8
#define kd 0
#define kp 1
#define ki 0
#define desired 50
void HSI(void);
void GPIOC_Setup(void);
void uno(void);
void cero(void);
void Delay(int x);
int* ColorArray(int Green, int Red, int Blue);
int Wheel(int Green, int Red, int Blue);
void Light(void);
void ColorSame(int Green, int Red, int Blue);
void Cycle(int Green, int Red, int Blue);
int color(uint8_t color[led][3]);
void place_color(uint8_t color[led][3],int leds,uint8_t green,uint8_t red,uint8_t blue);
void ADC1_IRQHandler(void);
void adc_setup(void);
int result;

int main(){
	uint8_t color_set[led][3];
	HSI();
	GPIOC_Setup();
	place_color(color_set,0x0,0x1,0x1,0x1);
	place_color(color_set,0x1,1,1,1);
	place_color(color_set,2,0x1,0x0,0x1);
	place_color(color_set,3,0x1,0x0,0x1);
	place_color(color_set,4,0x3,0x1,0x8);
	place_color(color_set,5,0x5,0x0,0x1);
	place_color(color_set,6,1,0,7);
	place_color(color_set,7,1,0,0);
	color(color_set);



	/*while(1){
		Light();
		//ColorSame(16,30,0);
		while(GPIOA->IDR & 0x1){
			Wheel(0, 15, 0);
		}
	}*/
}
int color(uint8_t color[led][3]){
	int i,j,k;
	for(i=0;i<led;i++){
		for(k=0;k<3;k++){
			for(j=7;j>=0;j--){
					if(((color[i][k]>>j)&0x1)==1) uno();
					else cero();
			}
		}
	}
	return 0;
}
void place_color(uint8_t color[led][3],int leds,uint8_t green,uint8_t red,uint8_t blue){
	color[leds][0]=green;
	color[leds][1]=red;
	color[leds][2]=blue;

}

void GPIOC_Setup(){
	RCC->AHBENR    |= RCC_AHBENR_GPIOCEN;      //Enable Clock of GPIOC
	GPIOC->MODER   |= GPIO_MODER_MODER13_0;    //Set Mode as Output
	GPIOC->OTYPER  &= ~GPIO_OTYPER_OT_13;		   //Set as Push-Pull
	GPIOC->PUPDR 	 &= ~GPIO_PUPDR_PUPDR13;     //Set as No PUPD
	GPIOC->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR13; //Set Output Speed

	RCC->AHBENR    |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER   &= ~GPIO_MODER_MODER0_1;
	GPIOA->OTYPER  &= ~GPIO_OTYPER_OT_0;
	GPIOA->PUPDR   &= ~GPIO_PUPDR_PUPDR0;
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0;
}
void HSI(void){
	RCC->CR |= RCC_CR_HSION;                  // Turn on HSI (16MHz)
	while( (RCC->CR & RCC_CR_HSIRDY) == 0);   // Wait until HSI is Ready
	RCC->CFGR &= ~RCC_CFGR_SW_HSI;            // Select HSI as System Clock
	RCC->CFGR |= RCC_CFGR_SW_HSI;
	while( (RCC->CFGR & RCC_CFGR_SWS)!= RCC_CFGR_SWS_HSI ); // Wait till HSI
}
void uno(void){
	//Output One to LED's
	GPIOC->ODR |= 1<<13;
	GPIOC->ODR |= 1<<13;
	GPIOC->ODR &= ~(1<<13);
}
void cero(void){
	//Output Zero to LED's
	GPIOC->ODR |= 1<<13;
	GPIOC->ODR &= ~(1<<13);
	GPIOC->ODR &= ~(1<<13);
}
void Delay(int x){
	int i;
	for(i = 0; i < x; i++);
}
void ADC1_IRQHandler(void){
	if(ADC1->SR & ADC_SR_EOC){
		result = ADC1->DR;
		//Result = (Result / 4095) * 3;
	}
	else
	if(ADC1->SR & ADC_SR_JEOC){

		ADC1->SR &= ~(ADC_SR_JEOC);
	}
}
void adc_setup(void){
	/* ADC STUFFFFFFFFFFFFFFFFFFF */
	RCC->CR |= RCC_CR_HSION;	// Turn on HSI (16 MHz)
	while ((RCC->CR & RCC_CR_HSIRDY) == 0);	// Wait until HIS is ready
	RCC->CFGR &= ~RCC_CFGR_SW_HSI;	// Select HSI as system clock
	RCC->CFGR |= RCC_CFGR_SW_HSI;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);	// Wait for HSI ready

	RCC->AHBENR |= (RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN);	// Enables clock for GPIO port C and D
	GPIOB->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER15);	// Configures PB.6 as output and PB.15 as analog mode
	GPIOC->MODER |= GPIO_MODER_MODER0;		// Configures PC.0 as analog mode
	GPIOD->MODER |= GPIO_MODER_MODER2_0;	// Configures PD.2 as output


	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;	// Enables ADC clock

	ADC1->CR2 &= ~(ADC_CR2_ADON);	// Clears ADON bit of CR, turning off ADC conversion

	ADC1->SQR1 &= ~(ADC_SQR1_L);	// Clears bits of L in SQR1 register
	ADC1->SQR5 = 0;	// Sets channel 10 or 21 as the 1st conversion in regular sequence
					//ADC1->SQR5 |= 10;
	ADC1->SQR5 |= 21;
	//ADC1->SMPR2 |= ADC_SMPR2_SMP10_1;	// Configures the sample time register for channel 10

	ADC1->SMPR1 |= ADC_SMPR1_SMP21_1;	// Configures the sample time register for channel 21
	ADC1->CR1 |= ADC_CR1_EOCIE;	// Enables end of conversion interrupt
	ADC1->CR2 |= ADC_CR2_CONT;	// Enables continuous conversion mode
	ADC1->CR2 |= ADC_CR2_DELS_0;	// Configures delay selection as delayed until the converted data has been read

	NVIC_EnableIRQ(ADC1_IRQn);	// Enables the interrupt of ADC1_IRQn in NVIC
	NVIC_SetPriority(ADC1_IRQn, 1);	// Configures the interrupt priority of ADC1_IRQn

									//GPIOD->ODR |= GPIO_OTYPER_ODR_2;	// Enables output of PD.2, turning on infrared emitter

	ADC1->CR2 |= ADC_CR2_ADON;	// Turns on ADC conversion

	ADC1->CR2 |= ADC_CR2_SWSTART;	// Starts the conversion
	
	/* OLD SETUP
	RCC->APB2ENR|=RCC_APB2ENR_ADC1EN; //enable adc clock
	RCC->ICSCR |= RCC_ICSCR_MSIRANGE_6;     //Set Clock for 4.194 MHz
	ADC1->CR2&=~ADC_CR2_ADON; //turn off adc
	ADC1->SQR1&=~ADC_SQR1_L; //set rcs to 1
	ADC1->SQR5&=~ADC_SQR5_SQ1; //set channel 10 as 1st conversion
	ADC1->SQR5|=0x0A;
	ADC1->SMPR2&=~ADC_SMPR2_SMP10;
	ADC1->SMPR2|=~ADC_SMPR2_SMP10_1;
	ADC1->CR1|=ADC_CR1_EOCIE;
	NVIC->ISER[0]|= 1<<18;
	NVIC->IP[ADC1_IRQn] = 0;
	ADC1->CR2|=ADC_CR2_ADON;
	ADC1->CR2|=ADC_CR2_SWSTART;
	*/

}
uint8_t PID_SET(int adc_input){ //error[0] is current error, error[1] is current Derror
                                      //error[2] is current Ierror,error[3] is previous error
                                      //error[4] is previous Ierror
    static float error[5] = {0,0,0,0,0};
    static int16_t input =0;
		uint8_t result;
    error[0]=(desired-adc_input);
    error[1]=((error[0]-error[3])/.0004);
    error[2]=(error[0]+(error[4]*.0004));
    input = (int)(error[0]*kp+error[1]*kd+error[2]*ki);
    error[3]=error[0];
    error[4]=error[2];
		input=input+128;
		if(input>255) input=255;
		else if(input<0) input =0;
		result = (uint8_t)input;
		return result;
}
