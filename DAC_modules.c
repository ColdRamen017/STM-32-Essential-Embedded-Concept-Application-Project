uint8_t calculate_dac(uint32_t distance)
{
	uint8_t clamp_distance;
	if (distance >= 200)
	{
		clamp_distance = 200;
	}

	else
	{
		clamp_distance = distance;
	}
	return (uint8_t)((clamp_distance * 255) / 200);
}

void dac_write(uint8_t code)
{
	GPIOA->BRR = (1 << 11) | (1 << 12) | (1 << 15); // Reset bits
	GPIOB->BRR = (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);

	if (code & (1 << 0))
	{
		GPIOA->BSRR = (1 << 11);
	} // PA11
	if (code & (1 << 1))
	{
		GPIOA->BSRR = (1 << 12);
	}
	if (code & (1 << 2))
	{
		GPIOA->BSRR = (1 << 15);
	}
	if (code & (1 << 3))
	{
		GPIOB->BSRR = (1 << 3);
	}
	if (code & (1 << 4))
	{
		GPIOB->BSRR = (1 << 4);
	}
	if (code & (1 << 5))
	{
		GPIOB->BSRR = (1 << 5);
	}
	if (code & (1 << 6))
	{
		GPIOB->BSRR = (1 << 6);
	}
	if (code & (1 << 7))
	{
		GPIOB->BSRR = (1 << 7);
	} // PB7
	// Assign data to the pins
}