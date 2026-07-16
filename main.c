#include <stdint.h>

// declare the memory location as a 32 bit int, then add in the offset before setting it
#define RCC_AHB1ENR (*(volatile uint32_t *)0x40023830)
#define GPIOA_MODER (*(volatile uint32_t *)0x40020000)
#define GPIOA_ODR (*(volatile uint32_t *)0x40020014)

void SystemInit(void) {}

int main(void) {
    // Enabled the clock for the GPIO I want
    RCC_AHB1ENR |= (1 << 0);

    // Set the mode for the pin (output)
    GPIOA_MODER &= ~(0x3 << 10);
    GPIOA_MODER |= (0x01 << 10);
    
    // Set the value of the GPIO to 1
    GPIOA_ODR |= (0 << 5);

    // Loop indefinitely so main() doesn't exit
    while(1){
        GPIOA_ODR ^= (1 << 5);
        for (int i = 1; i <= 1000000; i++){};
    }
}