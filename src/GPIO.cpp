// ddr and pullups for leds buzzer shift reg lcd sonic trig echo and buttons
#include <Arduino.h> // AVR register access
#include <stdint.h>  // uint8_t type
#include "GPIO.h"    // GPIO_init declaration
#include "PinMap.h"  // all pin mapping macros

// --- local pin helpers ---
// out1 configures one pin as output and drives it low as safe default.
// used by GPIO_init for leds buzzer trigger shift reg and lcd lines.
static void out1(volatile uint8_t *ddr, volatile uint8_t *port, uint8_t bit)
{
    *ddr |= (uint8_t)(1u << bit); // set pin direction to output
    *port &= (uint8_t) ~(1u << bit); // drive output low as safe default
}

// in_pullup configures one pin as input with internal pullup enabled.
// used for buttons and ultrasonic echo so floating inputs are avoided.
static void in_pullup(volatile uint8_t *ddr, volatile uint8_t *port, uint8_t bit)
{
    *ddr &= (uint8_t) ~(1u << bit); // set pin direction to input
    *port |= (uint8_t)(1u << bit); // enable internal pullup resistor
}

// --- global gpio bootstrap ---
// GPIO_init sets direction and safe startup levels for all connected devices.
// every other module depends on this being called first in setup().
void GPIO_init(void)
{
    // traffic leds output
    out1(&LED_GRN_DDR, &LED_GRN_PORT, LED_GRN_BIT); // green led pin output low
    out1(&LED_AMB_DDR, &LED_AMB_PORT, LED_AMB_BIT); // amber led pin output low
    out1(&LED_RED_DDR, &LED_RED_PORT, LED_RED_BIT); // red led pin output low

    // buzzer pin out and start quiet
    out1(&BUZ_DDR, &BUZ_PORT, BUZ_BIT); // buzzer pin output low
    BUZ_PORT &= (uint8_t) ~(1u << BUZ_BIT);
    // trig fires pulse echo is input w pullup
    out1(&SONIC_TRIG_DDR, &SONIC_TRIG_PORT, SONIC_TRIG_BIT); // trigger output
    SONIC_TRIG_PORT &= (uint8_t) ~(1u << SONIC_TRIG_BIT); // keep trigger low
    in_pullup(&SONIC_ECHO_DDR, &SONIC_ECHO_PORT, SONIC_ECHO_BIT); // echo input pullup
    out1(&SR_SER_DDR, &SR_SER_PORT, SR_SER_BIT); // shift reg serial data output
    out1(&SR_CLK_DDR, &SR_CLK_PORT, SR_CLK_BIT); // shift reg clock output
    out1(&SR_RCK_DDR, &SR_RCK_PORT, SR_RCK_BIT); // shift reg latch output
    SR_SER_PORT &= (uint8_t) ~(1u << SR_SER_BIT); // start serial line low
    SR_CLK_PORT &= (uint8_t) ~(1u << SR_CLK_BIT); // start clock line low
    SR_RCK_PORT &= (uint8_t) ~(1u << SR_RCK_BIT); // start latch line low
    // pause and reset buttons otherwise they float
    in_pullup(&BTN_PAUSE_DDR, &BTN_PAUSE_PORT, BTN_PAUSE_BIT); // pause button input pullup
    in_pullup(&BTN_RESET_DDR, &BTN_RESET_PORT, BTN_RESET_BIT); // reset button input pullup
    out1(&LCD_RS_DDR, &LCD_RS_PORT, LCD_RS_BIT); // lcd RS output
    out1(&LCD_EN_DDR, &LCD_EN_PORT, LCD_EN_BIT); // lcd EN output
    out1(&LCD_D4_DDR, &LCD_D4_PORT, LCD_D4_BIT); // lcd data bit 4 output
    out1(&LCD_D5_DDR, &LCD_D5_PORT, LCD_D5_BIT); // lcd data bit 5 output
    out1(&LCD_D6_DDR, &LCD_D6_PORT, LCD_D6_BIT); // lcd data bit 6 output
    out1(&LCD_D7_DDR, &LCD_D7_PORT, LCD_D7_BIT); // lcd data bit 7 output
}
