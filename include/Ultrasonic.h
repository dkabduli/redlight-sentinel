/*
 * ULTRASONIC MODULE HEADER
 */
#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

// Module init hook.
void Ultrasound_init(void);
// Return distance in cm. 0 means timeout or bad read.
uint16_t Ultrasound_read_cm(void);

#endif
