#include <stdint.h>

// Declare the memory location as a 32 bit int, then add in the offset before setting it
// Declare funcitons and positions of bitshifts

// Register addresses
#define RCC_AHB1ENR (*(volatile uint32_t *)0x40023830)
#define GPIOA_MODER (*(volatile uint32_t *)0x40020000)
#define GPIOA_ODR   (*(volatile uint32_t *)0x40020014)
#define GPIOA_AFRL  (*(volatile uint32_t *)0x40020020)

// MODER field encodings (pin-agnostic)
#define MODER_MASK 0x3
#define MODER_OUTPUT 1
#define MODER_AF 2

// RCC: GPIOA clock enable
#define RCC_GPIOAEN_POS 0

// PA5 — user LED
#define PA5_MODER_POS 10
#define LED_PIN 5

// PA2 — USART2 TX
#define PA2_MODER_POS 4
#define AFRL_PA2_POS 8
#define AFRL_PA2_USART2 7
#define AFRL_PA2_USART2_MASK 0xF

void SystemInit(void) {}

int main(void) {
    // Enabled the clock for the GPIO I want
    RCC_AHB1ENR |= (1 << RCC_GPIOAEN_POS);

    // Set the mode for the LED pin (output)
    GPIOA_MODER &= ~(MODER_MASK << PA5_MODER_POS);
    GPIOA_MODER |= (MODER_OUTPUT << PA5_MODER_POS);

    // Set the value of the LED GPIO to 1
    GPIOA_ODR |= (1 << LED_PIN);

    // Set PA2 to alternate function mode
    GPIOA_MODER &= ~(MODER_MASK << PA2_MODER_POS);
    GPIOA_MODER |= (MODER_AF << PA2_MODER_POS);

    // Setting up USART2 for TX on pin PA2
    GPIOA_AFRL &= ~(AFRL_PA2_USART2_MASK << AFRL_PA2_POS);
    GPIOA_AFRL |= (AFRL_PA2_USART2 << AFRL_PA2_POS);

    // Loop indefinitely so main() doesn't exit
    // Kept this because a heartbeat is nice
    while(1){
        GPIOA_ODR ^= (1 << LED_PIN);
        for (int i = 1; i <= 1000000; i++){};
    }
}