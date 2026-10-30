/*
 * LCD MODULE HEADER
 *
 * What this header defines:
 * - Public LCD API: init, clear, line1, line2.
 * - Keeps app code separate from LCD pin details.
 *
 * Why this interface matters:
 * - Other modules can write text without low-level LCD commands.
 * - Small API helps keep behavior consistent.
 *
 * Why structured this way:
 * - Include guards work across older AVR toolchains.
 * - Helper details stay private in the .cpp file.
 */
#ifndef LCD_H
#define LCD_H

// Initialize LCD startup settings.
void LCD_init(void);
// Clear LCD and reset cursor.
void LCD_clear(void);
// Write text to line 1.
void LCD_line1(const char *text);
// Write text to line 2.
void LCD_line2(const char *text);

#endif
