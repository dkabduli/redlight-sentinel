/*
 * LCD MODULE (HD44780, 4-bit mode)
 *
 * What this module does:
 * - Drives a 16x2 HD44780 LCD in 4-bit mode.
 * - Gives a small API for init, clear, and line writes.
 * - Sends each 8-bit value as two 4-bit writes.
 *
 * Why this module is meaningful in the project:
 * - LCD is the main local feedback display.
 * - Reliable text helps users see system state quickly.
 * - Full-line writes avoid old/stale characters on screen.
 *
 * Why it is written this way:
 * - 4-bit mode saves GPIO pins.
 * - Direct port writes are fast and predictable.
 * - Helper functions keep low-level logic organized.
 * - Init follows HD44780 startup sequence.
 * - Line writes pad to 16 chars to clear leftovers.
 */
#include <Arduino.h>    // AVR register names for port writes
#include <util/delay.h> // blocking microsecond millisecond delays
#include <stdint.h>     // uint8_t types for byte operations
#include "LCD.h"        // module API declarations
#include "PinMap.h"     // pin mapping macros for lcd lines

// --- nibble write primitive ---
// lcd_nibble_write drives D4..D7 and pulses EN for one 4 bit transfer.
// called by lcd_byte_write for both command bytes and character data.
static void lcd_nibble_write(uint8_t nibble, uint8_t rs)
{
    // RS selects command mode or data mode
    if (rs != 0u)
        LCD_RS_PORT |= (uint8_t)(1u << LCD_RS_BIT); // RS high means write character data
    else
        LCD_RS_PORT &= (uint8_t) ~(1u << LCD_RS_BIT); // RS low means write command byte

    // clear each data bit and write new bit from nibble
    LCD_D4_PORT = (LCD_D4_PORT & (uint8_t) ~(1u << LCD_D4_BIT)) | // clear old D4 then
                  (uint8_t)(((nibble >> 0) & 1u) << LCD_D4_BIT);   // write bit0 of nibble to D4
    LCD_D5_PORT = (LCD_D5_PORT & (uint8_t) ~(1u << LCD_D5_BIT)) | // clear old D5 then
                  (uint8_t)(((nibble >> 1) & 1u) << LCD_D5_BIT);   // write bit1 of nibble to D5
    LCD_D6_PORT = (LCD_D6_PORT & (uint8_t) ~(1u << LCD_D6_BIT)) | // clear old D6 then
                  (uint8_t)(((nibble >> 2) & 1u) << LCD_D6_BIT);   // write bit2 of nibble to D6
    LCD_D7_PORT = (LCD_D7_PORT & (uint8_t) ~(1u << LCD_D7_BIT)) | // clear old D7 then
                  (uint8_t)(((nibble >> 3) & 1u) << LCD_D7_BIT);   // write bit3 of nibble to D7
    // EN pulse tells controller to sample pins
    LCD_EN_PORT |= (uint8_t)(1u << LCD_EN_BIT); // EN high begins strobe
    _delay_us(1);                               // keep high minimum pulse width
    LCD_EN_PORT &= (uint8_t) ~(1u << LCD_EN_BIT); // EN low completes write latch
    // short settle wait keeps writes stable
    _delay_us(100);
}

// --- byte write primitive ---
// lcd_byte_write sends an 8 bit value as two nibbles in 4 bit bus mode.
static void lcd_byte_write(uint8_t value, uint8_t rs)
{
    // send high nibble first then low nibble as HD44780 expects
    lcd_nibble_write((uint8_t)(value >> 4), rs);     // upper 4 bits first
    lcd_nibble_write((uint8_t)(value & 0x0Fu), rs);  // lower 4 bits second
    _delay_us(100);                                   // instruction execution guard time
}

// --- module init ---
// LCD_init runs HD44780 startup sequence and default display config.
void LCD_init(void)
{
    // power up wait before first command
    _delay_ms(50); // wait after power up so lcd internal reset completes
    // init sequence enters 4 bit mode from unknown reset state
    lcd_nibble_write(0x03u, 0u); // wake sequence step 1
    _delay_ms(5);                // required gap from datasheet
    lcd_nibble_write(0x03u, 0u); // wake sequence step 2
    _delay_us(200);              // short gap before final steps
    lcd_nibble_write(0x03u, 0u); // wake sequence step 3
    lcd_nibble_write(0x02u, 0u); // switch bus mode to 4 bit transfers
    // function set: 4 bit, 2 lines, 5x8 font
    lcd_byte_write(0x28u, 0u); // 00101000 function set
    // display on, cursor off, blink off
    lcd_byte_write(0x0Cu, 0u); // 00001100 display control
    // entry mode: move cursor right after each char
    lcd_byte_write(0x06u, 0u); // 00000110 entry mode set
    LCD_clear();               // clear any random startup characters
}

// --- clear API ---
// LCD_clear sends clear command then waits until controller finishes.
void LCD_clear(void)
{
    // clear display command needs longer wait
    lcd_byte_write(0x01u, 0u); // clear display instruction
    _delay_ms(2);              // this command is slower than normal writes
}

// --- char helper ---
// lcd_putc writes one ascii character at current lcd cursor position.
static void lcd_putc(char c)
{
    lcd_byte_write((uint8_t)c, 1u); // send ascii byte as data
}

// --- cursor helper ---
// lcd_goto selects row and column using lcd DDRAM addressing.
static void lcd_goto(uint8_t row, uint8_t col)
{
    // DDRAM start addresses for line 1 and line 2
    static const uint8_t row_off[2] = {0x80u, 0xC0u}; // DDRAM base addresses
    lcd_byte_write((uint8_t)(row_off[row] + col), 0u); // command to set cursor address
}

// --- row writer API ---
// LCD_line1 writes text to first row and space pads to 16 characters.
void LCD_line1(const char *text)
{
    // always write full 16 chars so old chars do not remain
    lcd_goto(0, 0); // move cursor to line1 column0
    uint8_t n = 0u; // character counter for fixed width line
    while (n < 16u && text[n] != '\0')
        lcd_putc(text[n++]); // write input text until end or 16 chars
    while (n < 16u) {
        lcd_putc(' '); // pad with spaces to overwrite old leftovers
        n++;           // track padded columns
    } // pad to 16  cols
}

// LCD_line2 writes text to second row and space pads to 16 characters.
void LCD_line2(const char *text)
{
    // same strategy for second row
    lcd_goto(1, 0); // move cursor to line2 column0
    uint8_t n = 0u; // character counter for fixed width line
    while (n < 16u && text[n] != '\0')
        lcd_putc(text[n++]); // write input text until end or 16 chars
    while (n < 16u) {
        lcd_putc(' '); // pad remainder of row
        n++;           // move to next column position
    }
}
