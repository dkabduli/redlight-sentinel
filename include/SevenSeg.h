/*
 * SHIFT REGISTER DISPLAY HEADER
 *
 * Note:
 * - This is the ShiftReg API in this project.
 * - The ShiftReg code lives in SevenSeg.cpp.
 *
 * What this header has:
 * - Public functions to init and update the 7-seg display.
 *
 * Why this matters:
 * - Other files can show digits without handling shift bits or latch timing.
 *
 * Why this style:
 * - Include guards stop double include issues.
 * - Details stay inside the .cpp file.
 */
#ifndef SEVENSEG_H
#define SEVENSEG_H

#include <stdint.h>

// Init function for this module.
void SevenSeg_init(void);
// Show one digit from 0 to 9.
void SevenSeg_show_digit(uint8_t d);

#endif
