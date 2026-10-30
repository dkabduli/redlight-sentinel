/*
 * SHIFT REGISTER DISPLAY MODULE (74HC595 + 7-segment)
 *
 * Note:
 * - Shift-register logic is inside this SevenSeg module.
 *
 * What this module does:
 * - Shifts one byte into 74HC595 using SER/CLK, then latches output.
 * - Maps digits 0..9 to segment patterns for one 7-seg display.
 * - Hides shift/latch timing so callers just send a digit.
 *
 * Why this module is meaningful in the project:
 * - Gives more output using very few MCU pins.
 * - Shows quick numeric info like countdown.
 * - Keeps bit timing code in one place.
 *
 * Why it is written this way:
 * - Helpers (`pulse_clk`, `latch`, `sr_send`) split the steps clearly.
 * - MSB-first shifting keeps bit order predictable.
 * - Small delays keep clock/latch edges clean.
 * - Digit clamp avoids out-of-range table access.
 */
#include <Arduino.h>    // direct access to AVR port registers
#include <util/delay.h> // tiny pulse timing helpers
#include <stdint.h>     // uint8_t types
#include "SevenSeg.h"   // public seven segment API
#include "PinMap.h"     // pin mapping macros for SER CLK RCK

// --- shift register clock helper ---
// pulse_clk generates one SR clock pulse for each data bit shifted.
static void pulse_clk(void)
{
    // rising edge clocks current SER bit into the shift register
    SR_CLK_PORT |= (uint8_t)(1u << SR_CLK_BIT); // set clock high
    _delay_us(1);                               // hold high long enough for 74HC595
    // bring clock low so next bit can be prepared
    SR_CLK_PORT &= (uint8_t) ~(1u << SR_CLK_BIT); // finish one full clock pulse
}

// --- output latch helper ---
// latch copies internal shift register bits to visible segment outputs.
static void latch(void)
{
    // latch edge moves shifted bits to output pins together
    SR_RCK_PORT |= (uint8_t)(1u << SR_RCK_BIT); // raise latch pin
    _delay_us(1);                               // short hold time
    SR_RCK_PORT &= (uint8_t) ~(1u << SR_RCK_BIT); // outputs update on edge
}

// --- byte shift helper ---
// sr_send streams one byte into the shift register over SER and CLK.
static void sr_send(uint8_t b)
{
    // send 8 bits MSB first because we test top bit then shift left
    for (uint8_t i = 0u; i < 8u; i++) {
        // write current bit onto SER data pin
        if ((b & 0x80u) != 0u)
            SR_SER_PORT |= (uint8_t)(1u << SR_SER_BIT); // put logic 1 on serial data line
        else
            SR_SER_PORT &= (uint8_t) ~(1u << SR_SER_BIT); // put logic 0 on serial data line
        // next bit becomes MSB for next loop
        b = (uint8_t)(b << 1); // shift left so next bit moves into bit7
        pulse_clk();           // clock this bit into shift register storage
    }
}

// --- module init ---
// SevenSeg_init is kept for symmetry; GPIO_init already sets needed pins.
void SevenSeg_init(void)
{
    // pins are configured in GPIO_init so no extra setup needed here
}

// --- display API ---
// SevenSeg_show_digit selects segment pattern and updates display via shift reg.
void SevenSeg_show_digit(uint8_t d)
{
    // common cathode segment map: gfedcba with dp off
    static const uint8_t map[10] = {0x3Fu, 0x06u, 0x5Bu, 0x4Fu, 0x66u,
                                    0x6Du, 0x7Du, 0x07u, 0x7Fu, 0x6Fu};
    // clamp so index never goes outside table
    if (d > 9u)
        d = 9u; // fallback to 9 if caller sends invalid value
    // shift pattern in then latch to update display
    sr_send(map[d]); // stream 8 segment bits into shift register
    latch();         // copy streamed bits to output pins at once
}
