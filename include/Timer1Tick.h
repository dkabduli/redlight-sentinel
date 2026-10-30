/*
 * TIMER1 TICK MODULE HEADER
 */
#ifndef TIMER1TICK_H
#define TIMER1TICK_H

#include <stdint.h>

// Start a 1-second timer tick.
void Timer1_init_1Hz_tick(void);
// Return 1 if a tick happened, then clear it.
uint8_t Timer1_consume_1s_tick(void);

#endif
