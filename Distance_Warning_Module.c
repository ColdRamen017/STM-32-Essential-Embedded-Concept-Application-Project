void warning()
{
	if (distance_cm > 50)
	{ // safe
		leds_off();
		GPIOA->BSRR = (1 << SAFE_LED);
		TIM4->CCR4 = 0;
	}

	else if (distance_cm > 20)
	{ // warning
		leds_off();
		GPIOA->BSRR = (1 << WARN_LED);

		beep_counter_ms += 200;

		if (beep_counter_ms >= 250)
		{
			beep_counter_ms = 0;
			beep_on = !beep_on;
		}

		if (beep_on)
		{
			TIM4->CCR4 = (buzzer_duty_percent * (TIM4->ARR + 1)) / 100;
		}
		else
		{
			TIM4->CCR4 = 0;
		}
	}
	else
	{ // danger
		leds_off();
		GPIOA->BSRR = (1 << DANGER_LED);
		TIM4->CCR4 = (buzzer_duty_percent * (TIM4->ARR + 1)) / 100;
	}
}