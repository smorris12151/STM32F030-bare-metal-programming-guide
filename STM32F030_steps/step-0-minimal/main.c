// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

int main(void) {
  return 0;
}

// Startup code
__attribute__((naked, noreturn)) void _reset(void) {
   // memset .bss to zero, and copy .data section to RAM region
  extern long _sbss, _ebss, _sdata, _edata, _sidata;
  for (long *dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
  for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

  main();             // Call main()
  for (;;) (void) 0;  // Infinite loop in the case if main() returns
}

extern void _estack(void);  // Defined in link.ld

// 16 standard and 32 STM32-specific handlers
//NOTE: changed from 91 to 32 STM handlers to match simplified STM32F030 Architecture
__attribute__((section(".vectors"))) void (*const tab[16 + 32])(void) = {
  _estack, _reset
};
