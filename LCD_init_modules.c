void lcd_enable_pulse(void)
{
    GPIOB->BSRR = (1 << LCD_EN);
    delay_us(1);
    GPIOB->BRR = (1 << LCD_EN);
    delay_us(100);
}

void lcd_send_nibble(uint8_t nibble) // 4 bit send
{
    GPIOB->BRR = (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7);

    if (nibble & 0x01)
        GPIOB->BSRR = (1 << LCD_D4); // bit 1
    if (nibble & 0x02)
        GPIOB->BSRR = (1 << LCD_D5); // bit 2
    if (nibble & 0x04)
        GPIOB->BSRR = (1 << LCD_D6); // bit 3
    if (nibble & 0x08)
        GPIOB->BSRR = (1 << LCD_D7); // bit 4

    lcd_enable_pulse();
}

void lcd_send_data(uint8_t data, uint8_t rs)
{
    if (rs)
        GPIOB->BSRR = (1 << LCD_RS);
    else
        GPIOB->BRR = (1 << LCD_RS);

    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);
}

void lcd_sendCommand(uint8_t cmd)
{
    lcd_send_data(cmd, 0);
    if (cmd == 0x01 || cmd == 0x02)
    {
        delay_us(2000);
    }
    else
    {
        delay_us(100);
    }
}

void lcd_data(uint8_t Data)
{
    lcd_send_data(Data, 1);
}

void lcd_init(void)
{
    GPIOB->CRH &= ~(0xF << 8);
    GPIOB->CRH |= (0x2 << 8); // PB 10

    GPIOB->CRH &= ~(0xF << 12);
    GPIOB->CRH |= (0x2 << 12); // PB11

    GPIOB->CRH &= ~(0xF << 16);
    GPIOB->CRH |= (0x2 << 16); // PB 12

    GPIOB->CRH &= ~(0xF << 20);
    GPIOB->CRH |= (0x2 << 20); // PB13

    GPIOB->CRH &= ~(0xF << 24);
    GPIOB->CRH |= (0x2 << 24); // PB14

    GPIOB->CRH &= ~(0xF << 28);
    GPIOB->CRH |= (0x2 << 28); // PB15
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

void lcd_print(char *str)
{
    while (*str)
    {
        lcd_data(*str++);
    }
}

void lcd_update(void)
{
    char line1[17];
    char line2[17];

    sprintf(line1, "d=%3lucm Ts=%3dms", (unsigned long)distance_cm, 200);
    sprintf(line2, "DAC=%3d A=%2d%%", dac_code, buzzer_duty_percent);

    lcd_set_cursor(0, 0);
    lcd_print(line1);

    lcd_set_cursor(1, 0);
    lcd_print(line2);
}
