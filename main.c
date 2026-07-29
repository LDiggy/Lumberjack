#include <stdint.h>

// Declare the memory location as a 32 bit int, then add in the offset before setting it
// Declare funcitons and positions of bitshifts

// Register addresses
#define RCC_AHB1ENR (*(volatile uint32_t *)0x40023830)
#define GPIOA_MODER (*(volatile uint32_t *)0x40020000)
#define GPIOA_ODR   (*(volatile uint32_t *)0x40020014)
#define GPIOA_AFRL  (*(volatile uint32_t *)0x40020020)
#define RCC_APB1ENR (*(volatile uint32_t *)0x40023840)
#define USART2_BRR  (*(volatile uint32_t *)0x40004408)
#define USART2_CR1  (*(volatile uint32_t *)0x4000440C)
#define USART2_SR   (*(volatile uint32_t *)0x40004400)
#define USART2_DR   (*(volatile uint32_t *)0x40004404)

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

// RCC: USART2 clock enable
#define APB1_USART2EN_POS 17

// Define baud rate values, USART2 baud: 16 MHz / 115200 = 138.89
// Mantissa 138
#define BRR_MANTISSA 138
#define BRR_MANTISSA_POS 4
// Fraction 0.89*16 ≈ 14
#define BRR_FRACTION 14

// Bit positions to enable USART and Transmission
#define USART2_EN_POS 13
#define USART2_TX_EN_POS 3

// Mask for the TXE bit (bit 7 in USART2_SR)
#define USART2_TXE_POS_MASK 0x80

void SystemInit(void) {}

int main(void) {
    // Enabled the clock for the LED
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

    // Enabling clock for USART2
    RCC_APB1ENR |= (1 << APB1_USART2EN_POS);

    // Setting clock divisor for USART2's baud rate
    // Note: BRR_FRACTION has no position because I'm ORing it with the mantissa
    //       position value which starts at 4, leaving my entire bottom half of
    //       the register combined, the last 4 bits being the fraction
    USART2_BRR = ((BRR_MANTISSA << BRR_MANTISSA_POS) | BRR_FRACTION);

    // Set control register to enable USART and USART_TX
    USART2_CR1 |= ((1 << USART2_EN_POS) | (1 << USART2_TX_EN_POS));

    // Loop indefinitely so main() doesn't exit
    // Kept this because a heartbeat is nice
    while(1){
        // Commented out the "Count to a million then invert the LED"
        //GPIOA_ODR ^= (1 << LED_PIN);
        //for (int i = 1; i <= 1000000; i++){};

        // This loop's purpose is to get stuck here and do nothing until the write is ready,
        // and then it will exit once it is ready and let the line after this loop run.
        while ( (USART2_SR & USART2_TXE_POS_MASK) != USART2_TXE_POS_MASK){
            // do nothing
        }

        USART2_DR = 'U';
        for (int i = 1; i <= 1000; i++){};
    }
}