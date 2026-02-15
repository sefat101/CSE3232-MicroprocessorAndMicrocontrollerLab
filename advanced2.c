/* Register Base Addresses */
#define RCC_BASE            (0x40021000)
#define GPIOC_BASE          (0x40011000)
#define TIM2_BASE           (0x40000000)

/* Clock Control Registers */
#define RCC_CR              (*(volatile unsigned int *)(RCC_BASE + 0x00))
#define RCC_CFGR            (*(volatile unsigned int *)(RCC_BASE + 0x04))
#define RCC_APB2ENR         (*(volatile unsigned int *)(RCC_BASE + 0x18))
#define RCC_APB1ENR         (*(volatile unsigned int *)(RCC_BASE + 0x1C))

/* Timer 2 Registers */
#define TIM2_CR1            (*(volatile unsigned int *)(TIM2_BASE + 0x00))
#define TIM2_SR             (*(volatile unsigned int *)(TIM2_BASE + 0x10))
#define TIM2_PSC            (*(volatile unsigned int *)(TIM2_BASE + 0x28))
#define TIM2_ARR            (*(volatile unsigned int *)(TIM2_BASE + 0x2C))

/* GPIO Port C Registers */
#define GPIOC_CRH           (*(volatile unsigned int *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR           (*(volatile unsigned int *)(GPIOC_BASE + 0x0C))

void delay(int s);

int main(void) {
    // 1. Turn off PLL to change settings safely
    RCC_CR &= ~(1 << 24); 

    // 2. Select PLL as System Clock source (SW bits)
    RCC_CFGR &= ~(3 << 0);  // Clear SW bits
    RCC_CFGR |= (2 << 0);   // Set SW to 10 (Select PLL as System Clock)

    // 3. Configure PLL Multiplier (PLLMUL)
    // Clear bits 18-21, then set to x4 (0010)
    // Result: (HSI 8MHz / 2) * 4 = 16MHz System Clock
    RCC_CFGR &= ~(15 << 18);
    RCC_CFGR |= (2 << 18);  // 2 in binary is 0010 (x4 multiplier)

    // 4. Turn PLL back ON
    RCC_CR |= (1 << 24);

    // WAIT LOOP 1 - Wait for the PLL engine to stabilize (Lock)
    // We check Bit 25 (PLLRDY). 1 = Ready.
    while(!(RCC_CR & (1 << 25))); 

    // WAIT LOOP 2 - Wait for the CPU to actually switch to the PLL
    // We check Bits 3:2 (SWS). 10 = PLL is now the active system clock.
    while(((RCC_CFGR >> 2) & 0x03) != 0x02);

    // 5. Power up the Peripherals (Enable Clocks)
    RCC_APB2ENR |= (1 << 4); // Enable Port C clock
    RCC_APB1ENR |= (1 << 0); // Enable Timer 2 clock

    // 6. Setup PC13 as Output
    GPIOC_CRH &= ~(0xF << 20); // Clear Pin 13 config (4 bits: 20,21,22,23)
    GPIOC_CRH |= (1 << 21);    // Mode: Output 10MHz, Config: Push-Pull

    while(1) {
        GPIOC_ODR |= (1 << 13);  // LED OFF (PC13 is usually active-low)
        delay(1000); 

        GPIOC_ODR &= ~(1 << 13); // LED ON
        delay(1000); 
    }
}

/**
 * Uses Hardware Timer 2 to create a precise millisecond delay
 */
void delay(int s) {
    // Prescaler: 16MHz / 16000 = 1000 ticks per second (1ms per tick)
    TIM2_PSC = 16000 - 1; 
    TIM2_ARR = s - 1;      // Set the "goal" (e.g. 1000 for 1 second)

    TIM2_SR &= ~(0x01);    // Clear the Update Interrupt Flag (UIF)
    TIM2_CR1 |= (1 << 0);  // Start the Counter (CEN bit)

    // Wait until the hardware sets the UIF flag (Bit 0) indicating time is up
    while(!(TIM2_SR & 0x01));

    TIM2_CR1 &= ~(1 << 0); // Stop the Counter to save power
}
