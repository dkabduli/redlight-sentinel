/*
 * TRAFFIC LIGHT MODULE HEADER
 */
#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

// Start the traffic light state machine.
void TrafficLight_init(void);
// Call this again and again from loop().
void TrafficLight_step(void);

#endif
