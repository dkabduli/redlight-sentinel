/*
 * USART0 MODULE HEADER
 *
 * What this header defines:
 * - Public UART API for init and print.
 * - Used for debug logs and event logs.
 *
 * Why this interface matters:
 * - Other modules can print without touching UART registers.
 * - One shared print path keeps logs consistent.
 *
 * Why structured this way:
 * - Include guards stop double include issues.
 * - Small API keeps UART internals hidden.
 */
#ifndef USART0_H
#define USART0_H

#include <stdint.h>

// Initialize UART at 9600 baud.
void USART0_init_9600(void);
// Print a null-terminated string.
void USART0_print(const char *s);
// Print uint16_t as decimal text.
void USART0_print_u16(uint16_t v);

#endif
