// ==========================================================
// 🔴 BARE METAL STM32F103 CODE (NO HEADER FILES)
// ==========================================================

// ----------------- MEMORY ADDRESSES -----------------
#define RCC_APB2ENR   (*(volatile unsigned int*)0x40021018)
#define GPIOC_CRH     (*(volatile unsigned int*)0x40011004)
#define GPIOC_ODR     (*(volatile unsigned int*)0x4001100C)

// ----------------- STACK POINTER -----------------
#define STACK_TOP 0x20005000   // Adjust if needed

// ----------------- FUNCTION DECLARATIONS -----------------
void Reset_Handler(void);
void Default_Handler(void);
int main(void);

// ----------------- VECTOR TABLE -----------------
__attribute__((section(".isr_vector")))
void (* const vector_table[])(void) = {
    (void (*)(void))STACK_TOP,  // Initial Stack Pointer
    Reset_Handler,              // Reset Handler
    Default_Handler,            // NMI
    Default_Handler,            // HardFault
    Default_Handler,            // MemManage
    Default_Handler,            // BusFault
    Default_Handler,            // UsageFault
    0, 0, 0, 0,                 // Reserved
    Default_Handler,            // SVCall
    Default_Handler,            // Debug Monitor
    0,                          // Reserved
    Default_Handler,            // PendSV
    Default_Handler             // SysTick
};

// ----------------- DEFAULT HANDLER -----------------
void Default_Handler(void) {
    while (1);
}

// ----------------- RESET HANDLER -----------------
void Reset_Handler(void) {
    main();
}

// ----------------- DELAY -----------------
void delay() {
    for (volatile int i = 0; i < 300000; i++);
}

// ----------------- FLAG CHECK -----------------
void check_flags(unsigned int psr) {

    // Bits: N=31, Z=30, C=29, V=28, Q=27

    for (int i = 31; i >= 27; i--) {

        if (psr & (1U << i)) {

            GPIOC_ODR &= ~(1 << 13); // LED ON
            delay();

            GPIOC_ODR |= (1 << 13);  // LED OFF
            delay();
        }
    }

    delay();
    delay();
}

// ----------------- MAIN PROGRAM -----------------
int main(void) {

    int a, b, result;
    unsigned int psr;

    // Enable GPIOC clock
    RCC_APB2ENR |= (1 << 4);

    // Configure PC13 as output (Push-Pull, 2MHz)
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |=  (0x2 << 20);

    GPIOC_ODR |= (1 << 13); // LED OFF

    while (1) {

        // ==================================================
        // ADD → NEGATIVE FLAG
        // ==================================================
        __asm volatile ("MSR APSR_nzcvq, %0" : : "r"(0) : "cc");

        a = 5; b = -6;

        __asm volatile (
            "ADDS %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
            : "cc"
        );

        __asm volatile ("MRS %0, xPSR" : "=r"(psr));
        check_flags(psr);


        // ==================================================
        // ZERO + CARRY
        // ==================================================
        __asm volatile ("MSR APSR_nzcvq, %0" : : "r"(0) : "cc");

        a = 5; b = -5;

        __asm volatile (
            "ADDS %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
            : "cc"
        );

        __asm volatile ("MRS %0, xPSR" : "=r"(psr));
        check_flags(psr);


        // ==================================================
        // OVERFLOW
        // ==================================================
        __asm volatile ("MSR APSR_nzcvq, %0" : : "r"(0) : "cc");

        a = 2147483647; b = 1;

        __asm volatile (
            "ADDS %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
            : "cc"
        );

        __asm volatile ("MRS %0, xPSR" : "=r"(psr));
        check_flags(psr);


        // ==================================================
        // Q FLAG (SATURATION)
        // ==================================================
        __asm volatile ("MSR APSR_nzcvq, %0" : : "r"(0) : "cc");

        a = 1000;

        __asm volatile (
            "SSAT %0, #8, %1"
            : "=r"(result)
            : "r"(a)
            : "cc"
        );

        __asm volatile ("MRS %0, xPSR" : "=r"(psr));
        check_flags(psr);


        // ==================================================
        // LOGICAL (AND)
        // ==================================================
        __asm volatile ("MSR APSR_nzcvq, %0" : : "r"(0) : "cc");

        a = 0x0F; b = 0xF0;

        __asm volatile (
            "ANDS %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
            : "cc"
        );

        __asm volatile ("MRS %0, xPSR" : "=r"(psr));
        check_flags(psr);


        // ==================================================
        // MULTIPLICATION
        // ==================================================
        __asm volatile ("MSR APSR_nzcvq, %0" : : "r"(0) : "cc");

        a = -5; b = 3;

        __asm volatile (
            "MULS %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
            : "cc"
        );

        __asm volatile ("MRS %0, xPSR" : "=r"(psr));
        check_flags(psr);


        // ==================================================
        // DIVISION
        // ==================================================
        __asm volatile ("MSR APSR_nzcvq, %0" : : "r"(0) : "cc");

        a = -10; b = 2;

        __asm volatile (
            "SDIV %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
        );

        // Update flags manually
        __asm volatile (
            "CMP %0, #0"
            :
            : "r"(result)
            : "cc"
        );

        __asm volatile ("MRS %0, xPSR" : "=r"(psr));
        check_flags(psr);
    }
}
