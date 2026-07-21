#include "stm32f10x.h"
#include <stdio.h>
#define SAFE_LED    5
#define WARN_LED    6
#define DANGER_LED  7
#define LCD_RS 10
#define LCD_EN 11
#define LCD_D4 12
#define LCD_D5 13
#define LCD_D6 14
#define LCD_D7 15
//Above for easier use

volatile uint8_t sample_tick = 0;
void TIM2_IRQHandler(){
	if (TIM2->SR & 1){//if the flag is 1
		TIM2->SR = 0;
		sample_tick =1;
	}
}

volatile uint32_t ms_counter = 0;
void SysTick_Handler(){//for 40 ms check for ECHO
	ms_counter ++;
}

void delay_us(uint16_t us)//simple hardware delay
{
    uint16_t start = TIM3->CNT;
    while ((uint16_t)(TIM3->CNT - start) < us);
}

volatile uint32_t echo_rise = 0;
volatile uint32_t echo_fall = 0;
volatile uint32_t echo_us = 0;
volatile uint32_t distance_cm = 400;
volatile uint32_t measured_distance = 400;

volatile uint8_t echo_waiting = 0;
volatile uint8_t echo_done = 0;
volatile uint32_t echo_start_ms = 0;

volatile uint16_t adc_value = 0;
volatile uint8_t buzzer_duty_percent = 0;

volatile uint8_t beep_on = 0;
volatile uint32_t beep_counter_ms = 0;

volatile uint8_t dac_code = 0;

uint8_t calculate_dac(uint32_t distance){
	uint8_t clamp_distance;
	if (distance >= 200){
		clamp_distance = 200;
	}
	
	else{
		clamp_distance = distance;
	}
	return (uint8_t)((clamp_distance * 255) / 200);
}

void dac_write(uint8_t code){
	GPIOA->BRR = (1<<11) | (1<<12) | (1<<15);//Reset bits
	GPIOB->BRR = (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7);
	
	if(code & (1<<0)){GPIOA->BSRR = (1<<11);}//PA11
	if(code & (1<<1)){GPIOA->BSRR = (1<<12);}
	if(code & (1<<2)){GPIOA->BSRR = (1<<15);}
	if(code & (1<<3)){GPIOB->BSRR = (1<<3);}
	if(code & (1<<4)){GPIOB->BSRR = (1<<4);}
	if(code & (1<<5)){GPIOB->BSRR = (1<<5);}
	if(code & (1<<6)){GPIOB->BSRR = (1<<6);}
	if(code & (1<<7)){GPIOB->BSRR = (1<<7);}//PB7
	//Assign data to the pins
}

void leds_off(){
  GPIOA->BRR = (1 << SAFE_LED);
  GPIOA->BRR = (1 << WARN_LED);
  GPIOA->BRR = (1 << DANGER_LED);	
}
void EXTI1_IRQHandler()
{
	if (EXTI->PR & (1<<1))
	{EXTI->PR = (1<<1);
		if (!echo_waiting){
			return;}
		
		if (GPIOA->IDR & (1<<1))//rising edge 
		{
			echo_rise = TIM3->CNT;
		}
		else
		{
			echo_fall = TIM3->CNT;//falling edge
			
			echo_us = (uint16_t)(echo_fall - echo_rise);
			
			distance_cm = echo_us /58;
			echo_done = 1;
			echo_waiting = 0;
		}
	}
}

void check_ultrasonic_timeout(void)
{
    if (echo_waiting)
    {
        if ((ms_counter - echo_start_ms) >= 40)
        {
            distance_cm = 400;
            echo_us = 0;
            echo_waiting = 0;
            echo_done = 1;
					
					TIM4->CCR4 = 0;
        }
    }
}

void start_measurement(){
	echo_waiting = 1;
	echo_done = 0;
	echo_start_ms = ms_counter;
	
	GPIOA->BRR = (1<<0);
	delay_us(2);
	
	GPIOA->BSRR = (1<<0);
	delay_us(10);
	
	GPIOA->BRR = (1<<0);
	//above starts a 10 us TRIG pulse	
}
uint16_t read_adc(void){
	ADC1->CR2 |= (1<<0);//start conversion
	
	while(!(ADC1->SR & (1 << 1)));//EOC bit becomes 1
	
	return ADC1->DR;//return result
}


void warning(){
	if (distance_cm > 50){//safe
		leds_off();
		GPIOA->BSRR = (1<<SAFE_LED);
		TIM4->CCR4 = 0;
	}
	
	else if (distance_cm > 20){//warning
		leds_off();
		GPIOA->BSRR = (1 << WARN_LED);
		
		beep_counter_ms += 200;
		
		if (beep_counter_ms >= 250){
			beep_counter_ms = 0;
			beep_on = !beep_on;
			
		}
		
		if (beep_on){
			TIM4->CCR4 = (buzzer_duty_percent * (TIM4->ARR + 1)) / 100;
		}
		else{
			TIM4->CCR4 = 0;
		}
	}	
	else{//danger
			leds_off();
			GPIOA->BSRR = (1 << DANGER_LED);
			TIM4->CCR4 = (buzzer_duty_percent * (TIM4->ARR + 1)) / 100;
		}
	}




void lcd_enable_pulse(void)
{
    GPIOB->BSRR = (1 << LCD_EN);
    delay_us(1);
    GPIOB->BRR = (1 << LCD_EN);
    delay_us(100);
}	
	
void lcd_send_nibble(uint8_t nibble)//4 bit send
{
    GPIOB->BRR = (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7);

    if (nibble & 0x01) GPIOB->BSRR = (1 << LCD_D4);//bit 1
    if (nibble & 0x02) GPIOB->BSRR = (1 << LCD_D5);//bit 2
    if (nibble & 0x04) GPIOB->BSRR = (1 << LCD_D6);//bit 3
    if (nibble & 0x08) GPIOB->BSRR = (1 << LCD_D7);//bit 4

    lcd_enable_pulse();
}	
	
void lcd_send_data(uint8_t data, uint8_t rs){
  if (rs)
        GPIOB->BSRR = (1 << LCD_RS);
  else
      GPIOB->BRR = (1 << LCD_RS);

	lcd_send_nibble(data >> 4);
	lcd_send_nibble(data & 0x0F);
}

void lcd_sendCommand(uint8_t cmd){
	lcd_send_data(cmd, 0);
   if (cmd == 0x01 || cmd == 0x02){
      delay_us(2000);}	
	 else{
		 delay_us(100);}
}

void lcd_data(uint8_t Data){
	lcd_send_data(Data, 1);
}

void lcd_init(void)
{
	GPIOB->CRH &= ~(0xF << 8);
	GPIOB->CRH |=  (0x2 << 8);	//PB 10

	GPIOB->CRH &= ~(0xF << 12);
	GPIOB->CRH |=  (0x2 << 12);//PB11

	GPIOB->CRH &= ~(0xF << 16);
	GPIOB->CRH |=  (0x2 << 16);//PB 12
	
	GPIOB->CRH &= ~(0xF << 20);
	GPIOB->CRH |=  (0x2 << 20);//PB13
	
	GPIOB->CRH &= ~(0xF << 24);
	GPIOB->CRH |=  (0x2 << 24);//PB14
	
	GPIOB->CRH &= ~(0xF << 28);
	GPIOB->CRH |=  (0x2 << 28);//PB15
  delay_us(50000);

  GPIOB->BRR = (1 << LCD_RS);
  GPIOB->BRR = (1 << LCD_EN);

  // 4-bit initialization sequence
  lcd_send_nibble(0x03);
  delay_us(5000);

  lcd_send_nibble(0x03);
  delay_us(5000);

  lcd_send_nibble(0x03);
  delay_us(5000);

  lcd_send_nibble(0x02); // 4-bit mode

  lcd_sendCommand(0x28); // 4-bit, 2 lines, 5x8 font
  lcd_sendCommand(0x0C); // display ON, cursor OFF
  lcd_sendCommand(0x06); // entry mode
  lcd_sendCommand(0x01); // clear display
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t address;

    if (row == 0)
        address = 0x00 + col;
    else
        address = 0x40 + col;

    lcd_sendCommand(0x80 | address);
}

void lcd_print(char *str){
	while(*str){
		lcd_data(*str++);
	}
}

void lcd_update(void)
{
    char line1[17];
    char line2[17];

    sprintf(line1, "d=%3lucm Ts=%3dms",(unsigned long) distance_cm, 200);
    sprintf(line2, "DAC=%3d A=%2d%%", dac_code, buzzer_duty_percent);

    lcd_set_cursor(0, 0);
    lcd_print(line1);

    lcd_set_cursor(1, 0);
    lcd_print(line2);
}



	int main(){
	RCC->APB2ENR |= (0xFC | (1<<0) | (1<<9));//GPIO clocks and the AFIO and the ADC1
	RCC->APB1ENR |= (1<<0);//TIM2 clock enabled
	RCC->APB1ENR |= (1 << 1);//TIM 3 clock enabled   
	RCC->APB1ENR |= (1 << 2);//TIM4
	
	AFIO->MAPR |= (2 << 24);//disable JTAG for DAC pins
		
	TIM2->SMCR = 0x0000; /* TIM2_CH1 as internal clock */
	TIM2->PSC = 7200-1;//gives 10kHz
	TIM2->ARR = 2000-1;//To make period 200ms,arbitrary choice, 2000ticks at 10 kHz
	TIM2->SR = 0;//clear flag
	
	TIM2->DIER |= 1;//update interrupt enable
	NVIC_EnableIRQ(TIM2_IRQn); //enable TIM2 interrupt

	TIM2->CR1 = 1;//counts up
	//With the above, the timer should be initalized
	TIM3->PSC = 72-1;
	TIM3->ARR = 0xFFFF;//TIM3 is a 16-bit counter
	TIM3->CR1 = 1;
	
	TIM4->PSC = 72-1;
	TIM4->ARR = 500-1;//2 kHz
	
	TIM4->CCMR2 &= ~(0xFF << 8); 
	TIM4->CCMR2 |= (6<<12);//PWM mode on channel 4
	TIM4->CCMR2 |= (1<<11);//ENABLE preload
	TIM4->CCER &= ~(1 << 13);  
	TIM4->CCER |= (1<<12);//enable CH4 outpur
	
	TIM4->CCR4 = 0;
	TIM4->EGR |= 1; 
	TIM4->CR1 |= (1<<7); //ARPE 1
	TIM4->CR1 |= (1<<0);//enable counter
	
	GPIOA->CRL &= ~(0xF << (0 * 4));
	GPIOA->CRL |= (0x2 << (0 * 4));//TRIG output psuh-pull PA0
	
	GPIOA->CRL &= ~(0xF << (1 * 4));
	GPIOA->CRL |=  (0x8 << (1 * 4));//ECHO input on PA1
	GPIOA->ODR &= ~(1 << 1);//pull down
	
	GPIOB->CRH &= ~(0xF << 4); 
	GPIOB->CRH |= (0xB << 4);//PB9 analog input mode 00, cnf 00
	
	GPIOA->CRL &= ~(0xF << (2 * 4));// PA2 ADC input
		
	GPIOA->CRL &= ~(0xF << (5 * 4));
	GPIOA->CRL |=  (0x2 << (5 * 4));//PA5 safe led

	GPIOA->CRL &= ~(0xF << (6 * 4));
	GPIOA->CRL |=  (0x2 << (6 * 4));//PA6warn led

	GPIOA->CRL &= ~(0xF << (7 * 4));
	GPIOA->CRL |=  (0x2 << (7 * 4));//PA7 danger led
//Above push-pull for LEDs

	GPIOB->CRH &= ~(0xF << 8);
	GPIOB->CRH |=  (0x2 << 8);	//PB 10

	GPIOB->CRH &= ~(0xF << 12);
	GPIOB->CRH |=  (0x2 << (12));//PB11

	GPIOB->CRH &= ~(0xF << 16);
	GPIOB->CRH |=  (0x2 << 16);//PB 12
	
	GPIOB->CRH &= ~(0xF << 20);
	GPIOB->CRH |=  (0x2 << 20);//PB13
	
	GPIOB->CRH &= ~(0xF << 24);
	GPIOB->CRH |=  (0x2 << 24);//PB14
	
	GPIOB->CRH &= ~(0xF << 28);
	GPIOB->CRH |=  (0x2 << 28);//PB15
//Above for LCD data pins

	GPIOB->CRL &= ~(0xF << (7 * 4));
	GPIOB->CRL |=  (0x2 << (7 * 4));//PB7

	GPIOB->CRL &= ~(0xF << (6 * 4));
	GPIOB->CRL |=  (0x2 << (6 * 4));//PB6

	GPIOB->CRL &= ~(0xF << (5 * 4));
	GPIOB->CRL |=  (0x2 << (5 * 4));//PB5

	GPIOB->CRL &= ~(0xF << (4 * 4));
	GPIOB->CRL |=  (0x2 << (4 * 4));//PB4

	GPIOB->CRL &= ~(0xF << (3 * 4));
	GPIOB->CRL |=  (0x2 << (3 * 4));//PB3

	GPIOA->CRH &= ~(0xF << 28);
	GPIOA->CRH |=  (0x2 << 28);//PA15

	GPIOA->CRH &= ~(0xF << 16);
	GPIOA->CRH |=  (0x2 << 16);//PA12
	
	GPIOA->CRH &= ~(0xF << 12);
	GPIOA->CRH |=  (0x2 << 12);//PA11
//Above are the 8 bits for the DAC

	AFIO->EXTICR[0] &= ~(0xF << 4);//select PA1 for line 1
	EXTI->FTSR |= (1<<1);
	EXTI->RTSR |= (1<<1);
	EXTI->IMR = (1<<1);
	EXTI->PR = (1 << 1);
	NVIC_EnableIRQ(EXTI1_IRQn);
	
	SysTick->LOAD = 72000-1;//for 40ms a 1 ms systick counter
	SysTick->VAL = 0;
	SysTick->CTRL = 0x07;//enable 1, tickint 1, clksource 1
	
	ADC1->SQR1 = 0;
	ADC1 ->SQR3 = 2;
	ADC1 ->SMPR2 |= (7<<(3*2));//SMP2 71.5 ADC 
	
	ADC1->CR2 |= (1<<0);//turn on
	delay_us(10);
	
	ADC1->CR2 |= (1 << 3);//reset calibration
	while(ADC1->CR2 & (1 << 3));
	
	ADC1->CR2 |= (1<<2);//calibration
	while(ADC1->CR2 & (1 << 2));//for hardware matchup and safety
	
	
	while (1){
		check_ultrasonic_timeout();
		
		if(sample_tick){
			sample_tick = 0;
			
			if (!echo_waiting){
				start_measurement();
			}
		adc_value = read_adc();
			
		buzzer_duty_percent = 10 + (80*adc_value) / 4095;//12-bit
		
		dac_code = calculate_dac(distance_cm);
		dac_write(dac_code);
			
		warning();
		
		lcd_init();

		lcd_update();
		}
	}
}

































