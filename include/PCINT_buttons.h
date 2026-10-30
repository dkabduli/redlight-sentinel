/*
 * BUTTON PCINT MODULE HEADER
 */
#ifndef PCINT_BUTTONS_H
#define PCINT_BUTTONS_H

#include <stdint.h>

// Start button interrupts and debounce logic.
void PCINT_buttons_init(void);
// Return 1 for one pause click, then clear it.
uint8_t PCINT_consume_pause_click(void);
// Return 1 for one reset click, then clear it.
uint8_t PCINT_consume_reset_click(void);

#endif
