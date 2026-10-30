/*
 * BOARD PIN MAP
 *
 * Register cheat sheet:
 * - DDRx  : sets pin direction (1 output, 0 input)
 * - PORTx : writes output value or turns on pull-up
 * - PINx  : reads input value
 */
#ifndef PINMAP_H
#define PINMAP_H

#include <stdint.h>

// GREEN LED -> Arduino A2 (PC2)
#define LED_GRN_DDR DDRC
#define LED_GRN_PORT PORTC
#define LED_GRN_BIT 2u

// AMBER LED -> Arduino A3 (PC3)
#define LED_AMB_DDR DDRC
#define LED_AMB_PORT PORTC
#define LED_AMB_BIT 3u

// RED LED -> Arduino A4 (PC4)
#define LED_RED_DDR DDRC
#define LED_RED_PORT PORTC
#define LED_RED_BIT 4u

// Pause button input -> Arduino D10 (PB2)
// BTN_PAUSE_PORT can enable the internal pull-up.
#define BTN_PAUSE_DDR DDRB
#define BTN_PAUSE_PIN PINB
#define BTN_PAUSE_PORT PORTB
#define BTN_PAUSE_BIT 2u

// Reset button input -> Arduino A0 (PC0)
// BTN_RESET_PORT can enable the internal pull-up.
#define BTN_RESET_DDR DDRC
#define BTN_RESET_PIN PINC
#define BTN_RESET_PORT PORTC
#define BTN_RESET_BIT 0u

// Buzzer output -> Arduino D3 (PD3)
#define BUZ_DDR DDRD
#define BUZ_PORT PORTD
#define BUZ_BIT 3u

// Ultrasonic trigger output -> Arduino A5 (PC5)
#define SONIC_TRIG_DDR DDRC
#define SONIC_TRIG_PORT PORTC
#define SONIC_TRIG_BIT 5u

// Ultrasonic echo input -> Arduino D2 (PD2)
// Read SONIC_ECHO_PIN to measure echo pulse width.
#define SONIC_ECHO_DDR DDRD
#define SONIC_ECHO_PIN PIND
#define SONIC_ECHO_PORT PORTD
#define SONIC_ECHO_BIT 2u

// Shift register SER (serial data) -> Arduino D11 (PB3)
#define SR_SER_DDR DDRB
#define SR_SER_PORT PORTB
#define SR_SER_BIT 3u

// Shift register SRCLK (shift clock) -> Arduino D12 (PB4)
#define SR_CLK_DDR DDRB
#define SR_CLK_PORT PORTB
#define SR_CLK_BIT 4u

// Shift register RCLK (latch clock) -> Arduino D13 (PB5)
#define SR_RCK_DDR DDRB
#define SR_RCK_PORT PORTB
#define SR_RCK_BIT 5u

// LCD RS (register select) -> Arduino D8 (PB0)
#define LCD_RS_DDR DDRB
#define LCD_RS_PORT PORTB
#define LCD_RS_BIT 0u

// LCD EN (enable strobe) -> Arduino D9 (PB1)
#define LCD_EN_DDR DDRB
#define LCD_EN_PORT PORTB
#define LCD_EN_BIT 1u

// LCD data D4 -> Arduino D4 (PD4)
#define LCD_D4_DDR DDRD
#define LCD_D4_PORT PORTD
#define LCD_D4_BIT 4u

// LCD data D5 -> Arduino D5 (PD5)
#define LCD_D5_DDR DDRD
#define LCD_D5_PORT PORTD
#define LCD_D5_BIT 5u

// LCD data D6 -> Arduino D6 (PD6)
#define LCD_D6_DDR DDRD
#define LCD_D6_PORT PORTD
#define LCD_D6_BIT 6u

// LCD data D7 -> Arduino D7 (PD7)
#define LCD_D7_DDR DDRD
#define LCD_D7_PORT PORTD
#define LCD_D7_BIT 7u

#endif
