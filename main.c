#include <stdint.h>
#include "stm32f411xe.h"

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

// PA5 — User LED
#define PA5_MODER_POS 10
#define LED_PIN 5

// PA2 — USART2 TX
#define PA2_MODER_POS 4
// AFRL means "alternate function register, low" because only bits 0-7 are
// present, while 8-15 are present in the ALFH register (the "high" register)
#define PA2_AFRL_POS 8
// I called this pin USART2 because AF7 = USART2 TX, which is 0b0111, or 7
#define PA2_AFRL_USART2 7
#define PA2_AFRL_USART2_MASK 0xF

// PA3 - USART2 RX
#define PA3_MODER_POS 6
#define PA3_AFRL_POS 12
#define PA3_AFRL_USART2 7
#define PA3_AFRL_USART2_MASK 0xF

// RCC: USART2 clock enable
#define APB1_USART2EN_POS 17

// Define baud rate values
// USART2 baud: BRR = PCLK1 / (16 x baud) x 16 ... simplified below
// USARTDIV = 16,000,000 / (16 x 115200) = 8.68
// Mantissa = 8 (integer part)
#define BRR_MANTISSA 8
#define BRR_MANTISSA_POS 4
// Fraction = 0.68 x 16 = 10.9 ≈ 11
#define BRR_FRACTION 11

// Bit positions to enable USART, Transmission, and Receiver
#define USART2_EN_POS 13
#define USART2_TX_EN_POS 3
#define USART2_RX_EN_POS 2

// Mask for the TXE bit (bit 7 in USART2_SR)
#define USART2_TXE_POS_MASK 0x80
// Mask for the RXNE bit (bit 5 in USART2_SR)
#define USART2_RXNE_POS_MASK 0x20

// Bit positions for RXNEIE and ORE interrupts
#define USART2_RXNEIE_POS 5
#define USART2_ORE_POS 3

// Masks for RXNEIE and ORE bits (trying new convention instead of raw number)
#define USART2_RXNEIE_POS_MASK (1 << USART2_RXNEIE_POS)
#define USART2_ORE_POS_MASK (1 << USART2_ORE_POS)

// Bit position of USART interrupt enable in ISER
#define USART2_ISER_POS 6

// Set the buffer size used for the ring buffer
#define BUFFER_SIZE 64

// These will be my global variables

// All of my buffer items are volatile so the compiler knows to read them
//    every time, not optimize reading them from a cpu cache
// Set up ring buffer
volatile uint8_t ring_buffer[BUFFER_SIZE];
// Set head and tail to zero
volatile uint8_t head = 0;
volatile uint8_t tail = 0;


void SystemInit(void) {}

// Handles the IRQ request for USART2
void USART2_IRQHandler(void) {
    // Read SR and DR unconditionally to clear flags that might hang ISR
    uint32_t status_register = USART2_SR;
    uint8_t data_register = USART2_DR;
    // Check if the RXNE flag has been raised, if so put the data in the buffer
    if ( ( status_register & USART2_RXNE_POS_MASK ) == USART2_RXNE_POS_MASK ) {
        // // Initial DR read to clear the ORE flag if it's been raised
        // uint8_t data_register = USART2_DR;

        // This used to be in the conditional, but I made it a variable so it doesn't need
        //    to be recomputed...also if I change it I change one spot, not multiple. Also
        //    it's way more readable in all places
        uint8_t next_head = ( head + 1 ) % BUFFER_SIZE;
        // This bit drops the next byte if the buffer is 1 away from being full
        if ( next_head != tail ) {
            ring_buffer[head] = data_register;
            head = next_head;
        }
    }
}

// Initializes USART2
void usart2_init(void) {
    // Enabled the clock for GPIOA (enables USART2 and LED pins)
    RCC_AHB1ENR |= (1 << RCC_GPIOAEN_POS);

    // Set the mode for the LED pin (output)
    GPIOA_MODER &= ~(MODER_MASK << PA5_MODER_POS);
    GPIOA_MODER |= (MODER_OUTPUT << PA5_MODER_POS);

    // Set the value of the LED GPIO to 1
    GPIOA_ODR |= (1 << LED_PIN);

    // Set PA2 (TXE Pin) to alternate function mode
    GPIOA_MODER &= ~(MODER_MASK << PA2_MODER_POS);
    GPIOA_MODER |= (MODER_AF << PA2_MODER_POS);

    // Set PA3 (RX Pin) to alternate function mode
    GPIOA_MODER &= ~(MODER_MASK << PA3_MODER_POS);
    GPIOA_MODER |= (MODER_AF << PA3_MODER_POS);

    // Setting up USART2 for TX on pin PA2 by setting it to
    // alternate function mode 7 (TX)
    GPIOA_AFRL &= ~(PA2_AFRL_USART2_MASK << PA2_AFRL_POS);
    GPIOA_AFRL |= (PA2_AFRL_USART2 << PA2_AFRL_POS);

    // Setting up USART2 for RX on pin PA3 with AF7 (RX)
    GPIOA_AFRL &= ~(PA3_AFRL_USART2_MASK << PA3_AFRL_POS);
    GPIOA_AFRL |= (PA3_AFRL_USART2 << PA3_AFRL_POS);

    // Enabling clock for USART2
    RCC_APB1ENR |= (1 << APB1_USART2EN_POS);

    // Setting clock divisor for USART2's baud rate
    // Note: BRR_FRACTION has no position because I'm ORing it with the mantissa
    //       position value which starts at 4, leaving my entire bottom half of
    //       the register combined, the last 4 bits being the fraction
    USART2_BRR = ((BRR_MANTISSA << BRR_MANTISSA_POS) | BRR_FRACTION);

    // Set control register to enable USART, USART_TX, USART_RX, and USART_RXNEIE
    USART2_CR1 |= ((1 << USART2_EN_POS) | (1 << USART2_TX_EN_POS) | (1 << USART2_RX_EN_POS) | (1 << USART2_RXNEIE_POS));

    // Enable USART interrupts in the NVIC (Nested Vectored Interrupt Controller)
    NVIC->ISER[1] = (1 << USART2_ISER_POS);
}

int main(void) {
    // Initialize USART2 with all of the register writes in the function above
    usart2_init();

    // Loop indefinitely so main() doesn't exit
    // Kept the heartbeat code just in case
    while(1){
        // If nothing in the buffer, wait
        while ( head == tail ) {}
        // If transmission not ready, wait
        while ( ( USART2_SR & USART2_TXE_POS_MASK ) != USART2_TXE_POS_MASK ) {}
        // Store data
        USART2_DR = ring_buffer[tail];
        // This basically says "add 1 to the tail, divide by the buffer size, leave the remainder"
        //    i.e. if the remainder is 0, it loops back to zero because it got to the end of the buffer
        tail = ( tail + 1 ) % BUFFER_SIZE;
        

        // Heartbeat: Count to a million then invert the LED
        // GPIOA_ODR ^= (1 << LED_PIN);
        // for (int i = 1; i <= 1000000; i++){};
    }
}