#include <stdint.h>

/* ================== BASE ADDRESSES ================== */
#define RCC_BASE        0x40021000
#define GPIOC_BASE      0x40011000
#define FLASH_BASE      0x40022000
#define SYSTICK_BASE    0xE000E010

/* ================== RCC REGISTERS ================== */
#define RCC_CR          (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_CFGR        (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))

/* ================== FLASH ================== */
#define FLASH_ACR       (*(volatile uint32_t *)(FLASH_BASE + 0x00))

/* ================== GPIO ================== */
#define GPIOC_CRH       (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR       (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))

/* ================== SYSTICK ================== */
#define SYST_CSR        (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define SYST_RVR        (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define SYST_CVR        (*(volatile uint32_t *)(SYSTICK_BASE + 0x08))

/* ================== GLOBAL ================== */
volatile uint32_t msTicks = 0;

/* ================== SYSTICK HANDLER ================== */
void SysTick_Handler(void) {
    msTicks++;
}

/* ================== DELAY ================== */
void delay_ms(uint32_t ms) {
    uint32_t start = msTicks;
    while ((msTicks - start) < ms);
}

/* ================== CLOCK INIT (72 MHz) ================== */
void clock_init(void) {

    // 1. Enable HSE
    RCC_CR |= (1 << 16);
    while(!(RCC_CR & (1 << 17))); // Wait HSERDY

    // 2. Flash latency = 2 wait states
    FLASH_ACR |= (1 << 4);   // Prefetch enable
    FLASH_ACR &= ~(0x7);
    FLASH_ACR |= (0x2);

    // 3. Configure PLL: HSE * 9 = 72 MHz
    RCC_CFGR |= (1 << 16);   // PLLSRC = HSE
    RCC_CFGR &= ~(1 << 17);  // No division

    RCC_CFGR &= ~(15 << 18);
    RCC_CFGR |= (7 << 18);   // x9

    // APB1 = /2 (max 36MHz)
    RCC_CFGR |= (4 << 8);

    // 4. Enable PLL
    RCC_CR |= (1 << 24);
    while(!(RCC_CR & (1 << 25)));

    // 5. Switch system clock to PLL
    RCC_CFGR &= ~(3 << 0);
    RCC_CFGR |= (2 << 0);

    while(((RCC_CFGR >> 2) & 0x3) != 0x2);
}

/* ================== SYSTICK INIT ================== */
void systick_init(void) {
    // 72 MHz / 1000 = 72000 → 1 ms tick
    SYST_RVR = 72000 - 1;

    SYST_CVR = 0; // reset current value

    // Enable SysTick:
    // Bit 2 = CLKSOURCE (CPU clock)
    // Bit 1 = TICKINT (interrupt enable)
    // Bit 0 = ENABLE
    SYST_CSR = (1 << 2) | (1 << 1) | (1 << 0);
}

/* ================== MAIN ================== */
int main(void) {

    clock_init();

    // Enable GPIOC
    RCC_APB2ENR |= (1 << 4);

    // Configure PC13 as output
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x1 << 20);

    systick_init();

    while (1) {
        GPIOC_ODR ^= (1 << 13); // Toggle LED
        delay_ms(1000);
    }
}
