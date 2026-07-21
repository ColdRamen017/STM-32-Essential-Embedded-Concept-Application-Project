int main()
{
	RCC->APB2ENR |= (0xFC | (1 << 0) | (1 << 9)); // GPIO clocks and the AFIO and the ADC1
	RCC->APB1ENR |= (1 << 0);					  // TIM2 clock enabled
	RCC->APB1ENR |= (1 << 1);					  // TIM 3 clock enabled
	RCC->APB1ENR |= (1 << 2);					  // TIM4

	AFIO->MAPR |= (2 << 24); // disable JTAG for DAC pins

	TIM2->SMCR = 0x0000;  /* TIM2_CH1 as internal clock */
	TIM2->PSC = 7200 - 1; // gives 10kHz
	TIM2->ARR = 2000 - 1; // To make period 200ms,arbitrary choice, 2000ticks at 10 kHz
	TIM2->SR = 0;		  // clear flag

	TIM2->DIER |= 1;		   // update interrupt enable
	NVIC_EnableIRQ(TIM2_IRQn); // enable TIM2 interrupt

	TIM2->CR1 = 1; // counts up
	// With the above, the timer should be initalized
	TIM3->PSC = 72 - 1;
	TIM3->ARR = 0xFFFF; // TIM3 is a 16-bit counter
	TIM3->CR1 = 1;

	TIM4->PSC = 72 - 1;
	TIM4->ARR = 500 - 1; // 2 kHz

	TIM4->CCMR2 &= ~(0xFF << 8);
	TIM4->CCMR2 |= (6 << 12); // PWM mode on channel 4
	TIM4->CCMR2 |= (1 << 11); // ENABLE preload
	TIM4->CCER &= ~(1 << 13);
	TIM4->CCER |= (1 << 12); // enable CH4 outpur

	TIM4->CCR4 = 0;
	TIM4->EGR |= 1;
	TIM4->CR1 |= (1 << 7); // ARPE 1
	TIM4->CR1 |= (1 << 0); // enable counter

	GPIOA->CRL &= ~(0xF << (0 * 4));
	GPIOA->CRL |= (0x2 << (0 * 4)); // TRIG output psuh-pull PA0

	GPIOA->CRL &= ~(0xF << (1 * 4));
	GPIOA->CRL |= (0x8 << (1 * 4)); // ECHO input on PA1
	GPIOA->ODR &= ~(1 << 1);		// pull down

	GPIOB->CRH &= ~(0xF << 4);
	GPIOB->CRH |= (0xB << 4); // PB9 analog input mode 00, cnf 00

	GPIOA->CRL &= ~(0xF << (2 * 4)); // PA2 ADC input

	GPIOA->CRL &= ~(0xF << (5 * 4));
	GPIOA->CRL |= (0x2 << (5 * 4)); // PA5 safe led

	GPIOA->CRL &= ~(0xF << (6 * 4));
	GPIOA->CRL |= (0x2 << (6 * 4)); // PA6warn led

	GPIOA->CRL &= ~(0xF << (7 * 4));
	GPIOA->CRL |= (0x2 << (7 * 4)); // PA7 danger led
	// Above push-pull for LEDs

	GPIOB->CRH &= ~(0xF << 8);
	GPIOB->CRH |= (0x2 << 8); // PB 10

	GPIOB->CRH &= ~(0xF << 12);
	GPIOB->CRH |= (0x2 << (12)); // PB11

	GPIOB->CRH &= ~(0xF << 16);
	GPIOB->CRH |= (0x2 << 16); // PB 12

	GPIOB->CRH &= ~(0xF << 20);
	GPIOB->CRH |= (0x2 << 20); // PB13

	GPIOB->CRH &= ~(0xF << 24);
	GPIOB->CRH |= (0x2 << 24); // PB14

	GPIOB->CRH &= ~(0xF << 28);
	GPIOB->CRH |= (0x2 << 28); // PB15
	// Above for LCD data pins

	GPIOB->CRL &= ~(0xF << (7 * 4));
	GPIOB->CRL |= (0x2 << (7 * 4)); // PB7

	GPIOB->CRL &= ~(0xF << (6 * 4));
	GPIOB->CRL |= (0x2 << (6 * 4)); // PB6

	GPIOB->CRL &= ~(0xF << (5 * 4));
	GPIOB->CRL |= (0x2 << (5 * 4)); // PB5

	GPIOB->CRL &= ~(0xF << (4 * 4));
	GPIOB->CRL |= (0x2 << (4 * 4)); // PB4

	GPIOB->CRL &= ~(0xF << (3 * 4));
	GPIOB->CRL |= (0x2 << (3 * 4)); // PB3

	GPIOA->CRH &= ~(0xF << 28);
	GPIOA->CRH |= (0x2 << 28); // PA15

	GPIOA->CRH &= ~(0xF << 16);
	GPIOA->CRH |= (0x2 << 16); // PA12

	GPIOA->CRH &= ~(0xF << 12);
	GPIOA->CRH |= (0x2 << 12); // PA11
	// Above are the 8 bits for the DAC

	AFIO->EXTICR[0] &= ~(0xF << 4); // select PA1 for line 1
	EXTI->FTSR |= (1 << 1);
	EXTI->RTSR |= (1 << 1);
	EXTI->IMR = (1 << 1);
	EXTI->PR = (1 << 1);
	NVIC_EnableIRQ(EXTI1_IRQn);

	SysTick->LOAD = 72000 - 1; // for 40ms a 1 ms systick counter
	SysTick->VAL = 0;
	SysTick->CTRL = 0x07; // enable 1, tickint 1, clksource 1

	ADC1->SQR1 = 0;
	ADC1->SQR3 = 2;
	ADC1->SMPR2 |= (7 << (3 * 2)); // SMP2 71.5 ADC

	ADC1->CR2 |= (1 << 0); // turn on
	delay_us(10);

	ADC1->CR2 |= (1 << 3); // reset calibration
	while (ADC1->CR2 & (1 << 3))
		;

	ADC1->CR2 |= (1 << 2); // calibration
	while (ADC1->CR2 & (1 << 2))
		; // for hardware matchup and safety

	while (1)
	{
		check_ultrasonic_timeout();

		if (sample_tick)
		{
			sample_tick = 0;

			if (!echo_waiting)
			{
				start_measurement();
			}
			adc_value = read_adc();

			buzzer_duty_percent = 10 + (80 * adc_value) / 4095; // 12-bit

			dac_code = calculate_dac(distance_cm);
			dac_write(dac_code);

			warning();

			lcd_init();

			lcd_update();
		}
	}
}