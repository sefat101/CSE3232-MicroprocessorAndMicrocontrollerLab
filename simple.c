#define RCC_BASE        0x40021000
#define GPIOC_BASE      0x40011000

#define RCC_APB2ENR     (*(volatile unsigned int *)(RCC_BASE + 0x18))
#define GPIOC_CRH       (*(volatile unsigned int *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR       (*(volatile unsigned int *)(GPIOC_BASE + 0x0C))

void delay_loop(volatile unsigned int t);

int main(void)
{
    RCC_APB2ENR |= (1 << 4);

    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |=  (0x1 << 20);

    while (1)
    {
        GPIOC_ODR &= ~(1 << 13); // LED ON
        delay_loop(800000);

        GPIOC_ODR |= (1 << 13);  // LED OFF
        delay_loop(800000);
    }
}

void delay_loop(volatile unsigned int t)
{
    while(t--);
}
