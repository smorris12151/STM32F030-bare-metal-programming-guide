// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

int main(void) {
  return 0;
}

// Startup code
__attribute__((naked, noreturn)) void _reset(void) {
  for (;;) (void) 0;  // Infinite loop
}

extern void _estack(void);  // Defined in link.ld

// 16 standard and 32 STM32-specific handlers
//NOTE: changed from 91 to 32 STM handlers to match simplified STM32F030 Architecture
__attribute__((section(".vectors"))) void (*const tab[16 + 32])(void) = {
  _estack, _reset
};
