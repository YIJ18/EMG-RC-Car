//Mandar msj desde el bluetooth
#include <stm32f10x.h>
#include <stdio.h>
#include <string.h>

void usart1_init(void);
void usart1_sendByte(unsigned char c);
void usart1_sendStr(char *str);
void delay_ms(uint16_t t);
void delay_us(uint16_t t);

void adc_init(void);
uint16_t read_adc(uint8_t channel);

int main(void)
{
    char msg[50];
    uint16_t adc1_val, adc2_val, prom_adc1 = 0, prom_adc2 = 0;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_ADC1EN | RCC_APB2ENR_USART1EN;
    usart1_init();
    adc_init();

    while (1)
    {
			for(int i = 0; i < 10; i++)
			{
        adc1_val = read_adc(1); // PA1 -> ADC_IN1
        adc2_val = read_adc(2); // PA2 -> ADC_IN2
				prom_adc1 += adc1_val;
				prom_adc2 += adc2_val;
			}
				
				prom_adc1 = prom_adc1 / 10;
				prom_adc2 = prom_adc2 / 10;
        sprintf(msg, "%d,%d\n\r", prom_adc1, prom_adc2);
        usart1_sendStr(msg);
        delay_ms(50); // actualiza ~20 veces por segundo
    }
}

// -------------------- ADC ------------------------

void adc_init(void)
{
    GPIOA->CRL &= ~((0xF << 4) | (0xF << 8)); // PA1 y PA2 como analógicas
    GPIOA->CRL |= (0x0 << 4) | (0x0 << 8);

    ADC1->SMPR2 |= (7 << 3) | (7 << 6); // sample time para canal 1 y 2 (239.5 ciclos)
    ADC1->CR2 |= ADC_CR2_ADON;         // encender ADC
    delay_us(1);
    ADC1->CR2 |= ADC_CR2_ADON;         // segundo ADON inicia calibración
}

uint16_t read_adc(uint8_t channel)
{
    ADC1->SQR3 = channel;                   // Selecciona el canal
    ADC1->CR2 |= ADC_CR2_ADON;             // Start conversion
    while (!(ADC1->SR & ADC_SR_EOC));      // Esperar fin de conversión
    return ADC1->DR;                       // Leer valor convertido
}

// ------------------ USART ------------------------

void usart1_init()
{
    GPIOA->CRH &= ~(0xFF << 4);
    GPIOA->CRH |= (0x0B << 4) | (0x04 << 8); // TX (PA9) AF push-pull, RX (PA10) input pull-up
    GPIOA->ODR |= (1 << 10); // pull-up RX

    USART1->BRR = 7500; // 72MHz / 9600
    USART1->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void usart1_sendByte(unsigned char c)
{
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = c;
}

void usart1_sendStr(char *str)
{
    while (*str)
    {
        usart1_sendByte(*str++);
    }
}

// ------------------- Delay ------------------------

void delay_ms(uint16_t t)
{
    for (int i = 0; i < t; i++)
        for (volatile uint16_t a = 0; a < 6000; a++);
}

void delay_us(uint16_t t)
{
  for (int i = 0; i < t; i++)
		for(volatile uint16_t a = 0; a < 6; a++);
}