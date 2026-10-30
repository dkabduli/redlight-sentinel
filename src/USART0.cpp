/*
 * USART0 MODULE (9600 8N1 UART)
 *
 * What this module does:
 * - Sets up AVR USART0 for async serial at 9600 baud.
 * - Provides helpers to print text and uint16 values.
 * - Uses blocking byte send so output order stays correct.
 *
 * Why this module is meaningful in the project:
 * - Serial output is useful for debug and violation logs.
 * - Ordered UART output helps check traffic behavior.
 *
 * Why it is written this way:
 * - Register setup (UBRR/UCSR/UDR) makes behavior clear.
 * - Polling UDRE0 avoids lost bytes.
 * - Local number printing avoids heavy stdio cost.
 * - Small API keeps UART internals hidden.
 */
#include <Arduino.h> // brings AVR register names like UCSR0A UBRR0L UDR0
#include <stdint.h>  // fixed width integer types
#include "USART0.h"  // module function declarations

// --- usart setup ---
// USART0_init_9600 configures hardware uart so other modules can print logs.
void USART0_init_9600(void)
{
    // UBRR = 103 gives about 9600 baud at 16 MHz in normal speed mode
    UBRR0H = 0u;     // high byte of baud divider
    UBRR0L = 103u;   // low byte of baud divider
    // clear status flags and keep normal async mode
    UCSR0A = 0u; // U2X0=0 means normal speed x16 clocking
    // enable transmitter and receiver
    UCSR0B = (uint8_t)((1u << TXEN0) | (1u << RXEN0)); // turn on TX and RX hardware
    // frame format 8 data bits no parity 1 stop bit
    UCSR0C = (uint8_t)((1u << UCSZ01) | (1u << UCSZ00)); // UCSZ01:0 = 11 gives 8 bit data
}

// --- byte level tx helper ---
// USART0_tx_byte sends one byte and is used by all print helpers below.
static void USART0_tx_byte(uint8_t b)
{
    // wait until hardware data register becomes empty
    while ((UCSR0A & (1u << UDRE0)) == 0u) {
        // spin here until previous byte moved into shift register
    }
    // writing UDR0 starts transmission of one byte
    UDR0 = b; // TX line shifts this byte out LSB first in hardware
}

// --- string print API ---
// USART0_print writes a null terminated string over uart.
void USART0_print(const char *s)
{
    // send characters until null terminator
    while (*s != '\0')                  // loop while current character is not end marker
        USART0_tx_byte((uint8_t)*s++);  // send this char then advance pointer
}

// --- integer formatting helper ---
// print_digits converts uint16 to ascii decimal and streams it via uart.
static void print_digits(uint16_t v)
{
    // special case so 0 prints as one digit not empty output
    if (v == 0u) {
        USART0_tx_byte((uint8_t)'0');
        return;
    }
    // reverse buffer then replay backwards for correct order
    char tmp[6];
    uint8_t i = 0u;
    while (v > 0u) {
        // grab least significant decimal digit each loop
        tmp[i++] = (char)('0' + (v % 10u)); // convert number 0..9 to ascii digit
        v /= 10u;                            // remove least significant digit
    }
    // emit from last stored digit back to first
    while (i > 0u)
        USART0_tx_byte((uint8_t)tmp[--i]); // reverse order back to normal reading order
}

// --- typed print API ---
// USART0_print_u16 is public wrapper used by TrafficLight violation logs.
void USART0_print_u16(uint16_t v)
{
    print_digits(v); // wrapper keeps uint16 print API simple for callers
}
