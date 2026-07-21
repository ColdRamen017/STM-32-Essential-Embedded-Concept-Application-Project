void EXTI1_IRQHandler()
{
	if (EXTI->PR & (1 << 1))
	{
		EXTI->PR = (1 << 1);
		if (!echo_waiting)
		{
			return;
		}

		if (GPIOA->IDR & (1 << 1)) // rising edge
		{
			echo_rise = TIM3->CNT;
		}
		else
		{
			echo_fall = TIM3->CNT; // falling edge

			echo_us = (uint16_t)(echo_fall - echo_rise);

			distance_cm = echo_us / 58;
			echo_done = 1;
			echo_waiting = 0;
		}
	}
}