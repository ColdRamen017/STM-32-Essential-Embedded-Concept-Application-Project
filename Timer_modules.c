void TIM2_IRQHandler()
{
	if (TIM2->SR & 1)
	{ // if the flag is 1
		TIM2->SR = 0;
		sample_tick = 1;
	}
}

volatile uint32_t ms_counter = 0;
void SysTick_Handler()
{ // for 40 ms check for ECHO
	ms_counter++;
}

void delay_us(uint16_t us) // simple hardware delay
{
	uint16_t start = TIM3->CNT;
	while ((uint16_t)(TIM3->CNT - start) < us)
		;
}