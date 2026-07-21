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

void start_measurement()
{
    echo_waiting = 1;
    echo_done = 0;
    echo_start_ms = ms_counter;

    GPIOA->BRR = (1 << 0);
    delay_us(2);

    GPIOA->BSRR = (1 << 0);
    delay_us(10);

    GPIOA->BRR = (1 << 0);
    // above starts a 10 us TRIG pulse
}