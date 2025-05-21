#include "stm32f10x.h"
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 32

char rx_buffer[BUFFER_SIZE];
uint8_t rx_index = 0;

void delay_ms(uint16_t t);

void USART1_IRQHandler()  // Subrutina de interrupción USART1
{
    uint8_t c = USART1->DR;  // Leer dato recibido

    if ((c == 'A') || (c == 'a'))
    {
        GPIOA->ODR |= (1 << 0);  // PA0
        GPIOA->ODR |= (1 << 5);  // PA5
        GPIOA->ODR |= (1 << 1);  // PA1
        GPIOA->ODR |= (1 << 4);  // PA4
        GPIOA->ODR &= ~(1 << 2); // PA2
        GPIOA->ODR &= ~(1 << 3); // PA3
    }
    if ((c == 'R') || (c == 'r'))
    {
        GPIOA->ODR |= (1 << 0);   // PA0
        GPIOA->ODR |= (1 << 5);   // PA5
        GPIOA->ODR &= ~(1 << 1);  // PA1
        GPIOA->ODR &= ~(1 << 4);  // PA4
        GPIOA->ODR |= (1 << 2);   // PA2
        GPIOA->ODR |= (1 << 3);   // PA3
    }
    if ((c == 'S') || (c == 's'))
    {
        GPIOA->ODR &= ~(1 << 0);
        GPIOA->ODR &= ~(1 << 1);
        GPIOA->ODR &= ~(1 << 2);
        GPIOA->ODR &= ~(1 << 3);
        GPIOA->ODR &= ~(1 << 4);
        GPIOA->ODR &= ~(1 << 5);
    }

    if ((c == 'D') || (c == 'd'))
    {
        TIM3->CCR2 = 1000;  // Pulso de 1 ms = 0°
    }
    if ((c == 'I') || (c == 'i'))
    {
        TIM3->CCR2 = 1900;  // Pulso de 2 ms = 180°
    }
		if ((c == 'C') || (c == 'c'))
    {
        TIM3->CCR2 = 1500;  // Pulso de 2 ms = 180°
    }
		if (c == '+') TIM3->CCR1 += 100;
    if (c == '-') TIM3->CCR1 -= 100;
}

void USART3_IRQHandler()  // USART del HC-05 esclavo (PB10/PB11)
{
    if (USART3->SR & USART_SR_RXNE)
    {
        char c = USART3->DR;

        if (c == '\r') return; // Ignora '\r' si viene luego del '\n'

        if (c == '\n')  // Fin de mensaje
        {
            rx_buffer[rx_index] = '\0';  // Termina cadena

            // Separar por coma
            char *token1 = strtok(rx_buffer, ",");
            char *token2 = strtok(NULL, ",");

            if (token1 && token2)
            {
                int valor1 = atoi(token1);
                int valor2 = atoi(token2);

                // ?? Lógica de control del servomotor
                if ((valor1 < 500 && valor2 < 500) || (valor1 >= 1000 && valor2 >= 1000))
                {
                    TIM3->CCR2 = 1500;  // 0°
                }
                else if (valor1 >= 500 && valor2 < 500)
                {
                    TIM3->CCR2 = 1900;  // 180°
                }
                else if (valor1 < 500 && valor2 >= 500)
                {
                    TIM3->CCR2 = 1000;  // Centro
                }
            }

            rx_index = 0;  // Reinicia buffer
        }
        else if (rx_index < BUFFER_SIZE - 1)
        {
            rx_buffer[rx_index++] = c;
        }
    }
}

int main()
{
    // Relojes
    RCC->APB2ENR |= 0xFC | (1 << 14);  // Habilita reloj GPIOs y USART1
    RCC->APB1ENR |= (1 << 1) | (1 << 18);          // Habilita TIM3

    // Configura PA0–PA5 como salidas push-pull (GPIOs)
    GPIOA->CRL &= ~0xFFFFFF;     // Limpia configuración de PA0–PA5
    GPIOA->CRL |= 0x333333;      // PA0–PA5 como salida push-pull 50 MHz

    // Configura PA7 como salida alterna push-pull para PWM (TIM3_CH2)
    GPIOA->CRL &= ~(0xF << 28);  // Limpia configuración de PA7
    GPIOA->CRL |= (0xB << 28);   // PA7 = AF Output Push-Pull 50 MHz

    // PWM TIM3_CH2 (PA7)
    TIM3->PSC = 72 - 1;          // 1 MHz (1 µs por tick)
    TIM3->ARR = 20000 - 1;       // 20 ms periodo = 50 Hz
    TIM3->CCR2 = 1500;           // Pulso inicial = 1.5 ms (centro)

    TIM3->CCMR1 |= (6 << 12);    // OC2M: PWM mode 1
    TIM3->CCMR1 |= (1 << 11);    // OC2PE
    TIM3->CCER |= (1 << 4);      // CC2E: habilita salida CH2
    TIM3->CR1 |= (1 << 7) | (1 << 0);  // ARPE y CEN
    TIM3->EGR |= (1 << 0);       // Evento de actualización
		
		// Configura PA6 como salida alterna push-pull para PWM (TIM3_CH1)
    GPIOA->CRL &= ~(0xF << 24);  // Limpia configuración de PA6
    GPIOA->CRL |= (0xB << 24);   // PA6 = AF Output Push-Pull 50 MHz
		
		// PWM TIM3_CH1 (PA6)
    TIM3->CCR1 = 500;            // Pulso inicial: 50% de 1000
    TIM3->CCMR1 |= (6 << 4);     // OC1M: PWM mode 1
    TIM3->CCMR1 |= (1 << 3);     // OC1PE
    TIM3->CCER |= (1 << 0);      // CC1E: habilita salida CH1

    // USART1 init
    GPIOA->ODR |= (1 << 10);     // Pull-up PA10
    GPIOA->CRH = 0x444448B4;     // PA9 TX (AF), PA10 RX (pull-up)
    USART1->CR1 = 0x2024;        // RX habilitado, interrupciones RX
    USART1->BRR = 7500;          // 72 MHz / 9600 bps
		
		GPIOB->ODR |= (1 << 11);
		GPIOB->CRH = 0x44448B44;
		USART3->CR1 = 0x2024;
		USART3->BRR = 3750;
		
		NVIC_EnableIRQ(USART3_IRQn);
    NVIC_EnableIRQ(USART1_IRQn); // Habilita interrupción USART1

    while (1)
    {
        // Todo el trabajo ocurre en la interrupción
    }
}

void delay_ms(uint16_t t)
{
    for (int i = 0; i < t; i++)
    {
        for (volatile uint16_t a = 0; a < 6000; a++) {}
    }
}