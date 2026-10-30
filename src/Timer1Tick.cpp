//timer1 ~1 sec for countdown — not using delay() in loop for that
#include <Arduino.h>       // gives core register definitions
#include <avr/interrupt.h> // gives ISR macro and cli function
#include <stdint.h>        // gives uint8_t type
#include "Timer1Tick.h"    // this module public function declarations

// --- timer flag shared with main loop ---
static volatile uint8_t s_pending_1s;

// ISR sets a one second pending flag.
// main code reads and clears this flag via Timer1_consume_1s_tick().
ISR(TIMER1_COMPA_vect)
{
    s_pending_1s = 1u; // mark one tick pending for foreground code
}

// Timer1_init_1Hz_tick configures timer1 in CTC mode at 1 Hz.
// with 16 MHz clock and 1024 prescaler, OCR1A=15624 gives one second.
void Timer1_init_1Hz_tick(void)
{
    TCCR1A = 0u; // normal compare output behavior
    TCCR1B = (uint8_t)((1u << WGM12) | (1u << CS12) | (1u << CS10)); // CTC mode and 1024 prescaler
    OCR1A = 15624u; // compare match value for one second period
    TIMSK1 = (uint8_t)(1u << OCIE1A); // enable compare A interrupt
    TCNT1 = 0u; // clear counter so timing starts from zero
}

// Timer1_consume_1s_tick atomically reads and clears pending flag.
// called from TrafficLight_step to drive phase countdown timing.
uint8_t Timer1_consume_1s_tick(void)
{
    uint8_t oldSREG = SREG; // save interrupt enable state
    cli(); // dont let isr clobber flag while u read
    uint8_t v = s_pending_1s; // copy pending flag to local value
    s_pending_1s = 0u; // clear flag so tick is consumed once
    SREG = oldSREG; // restore previous interrupt state
    return v; // return one if a tick was pending else zero
}
