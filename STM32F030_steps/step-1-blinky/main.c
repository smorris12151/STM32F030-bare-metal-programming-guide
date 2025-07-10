// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include <inttypes.h>
#include <stdbool.h>

#define BIT(x) (1UL << (x))
#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)

struct rcc {
  volatile uint32_t CR, CFGR, CIR, APB2RSTR, APB1RSTR,
  AHBENR, APB2ENR, APB1ENR, BCDR, CSR, AHBRSTR, CFGR2,
  CFGR3, CR2;
};
#define RCC ((struct rcc *) 0x40021000)

struct gpio {
  volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFRL, AFRH, BRR;
};
#define GPIO(bank) ((struct gpio *) (0x48000000 + 0x400 * (bank))) //Changed gpio mem address to match stm32f030

// Enum values are per datasheet: 0, 1, 2, 3
enum { GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG };

static inline void gpio_set_mode(uint16_t pin, uint8_t mode) {
  struct gpio *gpio = GPIO(PINBANK(pin));  // GPIO bank
  int n = PINNO(pin);                      // Pin number
  gpio->MODER &= ~(3U << (n * 2));         // Clear existing setting
  gpio->MODER |= (mode & 3U) << (n * 2);   // Set new mode
}

static inline void gpio_write(uint16_t pin, bool val) {
  struct gpio *gpio = GPIO(PINBANK(pin));
  gpio->BSRR = (1U << PINNO(pin)) << (val ? 0 : 16);
}

static inline void spin(volatile uint32_t count) {
  while (count--) asm("nop");
}

int main(void) {
  uint16_t L2 = PIN('A', 5);            // User LED L2
  RCC->AHBENR |= BIT(17); // Enable GPIO clock for GPIOA, which is bit 17 of AHBENR for STM32F030
  gpio_set_mode(L2, GPIO_MODE_OUTPUT);  // Set L2 to output mode
  for (;;) {
    gpio_write(L2, true);
    spin(999999);
    gpio_write(L2, false);
    spin(999999);
  }
  return 0;
}

// Startup code
__attribute__((naked, noreturn)) void _reset(void) {          //naked=don't generate prologue/epilogue  //noreturn=it will never exit
  // memset .bss to zero, and copy .data section to RAM region
  extern long _sbss, _ebss, _sdata, _edata, _sidata;          //linker symbols declared in the .ld file
  for (long *dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
  for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

  main();             // Call main()
  for (;;) (void) 0;  // Infinite loop in the case if main() returns
}

extern void _estack(void);  // Defined in link.ld

// 16 standard and 32 STM32-specific handlers
__attribute__((section(".vectors"))) void (*const tab[16 + 32])(void) = { // defines the interrupt vector table and dumps it into .vectors via the linker script
    _estack, _reset};
