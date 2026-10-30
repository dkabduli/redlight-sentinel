/*
 * TRAFFIC CONTROLLER MODULE
 */
// traffic controller state machine and red light violation logic
// zone far and near values are tuned thresholds for motion trigger
#include <Arduino.h>       // AVR register and ISR related symbols
#include <stdint.h>        // fixed width integer types
#include "TrafficLight.h"  // public API for this state machine
#include "PinMap.h"        // hardware pin bit macros
#include "LCD.h"           // lcd output helpers
#include "SevenSeg.h"      // seven segment display helpers
#include "Ultrasonic.h"    // distance sensor readings
#include "Buzzer.h"        // buzzer alert
#include "USART0.h"        // serial logging output
#include "PCINT_buttons.h" // debounced button click consumers
#include "Timer1Tick.h"    // one second tick source

typedef enum { PH_GREEN = 0, PH_AMBER, PH_RED } phase_t; // traffic phases in order

// shared state for current phase, countdown, pause, and lcd override timing
static phase_t s_phase;                // current active phase
static uint8_t s_sec_left;             // uint8_t: compact 0..255 countdown value
static uint8_t s_running;              // uint8_t flag (1 running, 0 paused)
static uint8_t s_violation_lcd_ticks;  // seconds to keep violation banner

// crossing detection window in centimeters
// uint16_t is used because distance can be more than 255.
static const uint16_t ZONE_FAR_CM = 45u;  // considered far from stop line
static const uint16_t ZONE_NEAR_CM = 18u; // considered crossing near stop line

// previous distance and one shot arm flag for edge detection
static uint16_t s_prev_cm; // previous ultrasonic sample for edge detection
static uint8_t s_armed;    // one shot gate for violation trigger

// --- LED output helpers ---
// leds_only_green applies hardware outputs for green state.
static void leds_only_green(void)
{
    // set green bit and clear amber and red bits
    LED_GRN_PORT |= (uint8_t)(1u << LED_GRN_BIT);   // set green output high
    LED_AMB_PORT &= (uint8_t) ~(1u << LED_AMB_BIT); // force amber output low
    LED_RED_PORT &= (uint8_t) ~(1u << LED_RED_BIT); // force red output low
}

// leds_only_amber applies hardware outputs for amber state.
static void leds_only_amber(void)
{
    // set amber bit and clear green and red bits
    LED_GRN_PORT &= (uint8_t) ~(1u << LED_GRN_BIT); // force green output low
    LED_AMB_PORT |= (uint8_t)(1u << LED_AMB_BIT);   // set amber output high
    LED_RED_PORT &= (uint8_t) ~(1u << LED_RED_BIT); // force red output low
}

// leds_only_red applies hardware outputs for red state.
static void leds_only_red(void)
{
    // set red bit and clear green and amber bits
    LED_GRN_PORT &= (uint8_t) ~(1u << LED_GRN_BIT); // force green output low
    LED_AMB_PORT &= (uint8_t) ~(1u << LED_AMB_BIT); // force amber output low
    LED_RED_PORT |= (uint8_t)(1u << LED_RED_BIT);   // set red output high
}

// --- LCD text policy ---
// lcd_normal prints regular phase text when no violation override is active.
static void lcd_normal(void)
{
    // normal two line text for each traffic phase
    if (s_phase == PH_GREEN) {
        LCD_line1("Green: You are  "); // row1 phase text
        LCD_line2("free to go      "); // row2 instruction text
    } else if (s_phase == PH_AMBER) {
        LCD_line1("Yellow: stop soo"); // intentionally clipped phrase to fit style
        LCD_line2("                "); // clear second line
    } else {
        LCD_line1("Red: stop       "); // red phase warning
        LCD_line2("                "); // keep second row blank
    }
}

// --- public init ---
// TrafficLight_init sets startup state and synchronizes LED sevenseg and lcd.
void TrafficLight_init(void)
{
    // boot starts at green with full countdown and running enabled
    s_phase = PH_GREEN;          // start cycle at green
    s_sec_left = 10u;            // green duration seconds
    s_running = 1u;              // run immediately after boot
    // no violation banner active at boot
    s_violation_lcd_ticks = 0u;  // no temporary violation banner active
    // large startup value prevents fake far to near jump
    s_prev_cm = 999u;            // sentinel larger than far threshold
    // allow first valid crossing to trigger once in red phase
    s_armed = 1u;                // allow first red crossing detection
    leds_only_green();           // apply physical LED outputs
    SevenSeg_show_digit(s_sec_left); // show initial countdown value
    lcd_normal();                // draw initial lcd message
}

// --- phase transition helper ---
// next_phase advances green->amber->red->green and resets per phase state.
static void next_phase(void)
{
    // green goes to amber, amber goes to red, red goes back to green
    if (s_phase == PH_GREEN) {
        s_phase = PH_AMBER;    // next phase after green
        s_sec_left = 5u;       // amber duration
        leds_only_amber();     // update led outputs
    } else if (s_phase == PH_AMBER) {
        s_phase = PH_RED;      // next phase after amber
        s_sec_left = 10u;      // red duration
        leds_only_red();       // update led outputs
        // rearm distance detector when entering red
        s_prev_cm = 999u;      // reset previous sample for clean red detection
        s_armed = 1u;          // rearm trigger on new red window
    } else {
        s_phase = PH_GREEN;    // cycle loops back to green
        s_sec_left = 10u;      // restore green duration
        leds_only_green();     // update led outputs
    }
    SevenSeg_show_digit(s_sec_left); // reflect new countdown start
    lcd_normal();                    // refresh lcd for new phase
}

// --- one second scheduler helper ---
// tick_one_second applies one countdown tick or transitions to next phase.
static void tick_one_second(void)
{
    // pause state freezes countdown
    if (s_running == 0u)
        return; // paused means ignore tick
    // count down until 1, then phase switch handles reload
    if (s_sec_left > 1u) {
        s_sec_left--;                   // consume one second from countdown
        SevenSeg_show_digit(s_sec_left); // update seven segment immediately
        return;                          // done for this tick
    }
    next_phase(); // if timer reached end, switch phase and reload
}

// --- violation response helper ---
// handle_violation raises buzzer and lcd alert and emits serial log.
static void handle_violation(uint16_t cm)
{
    // visual and audio alert and serial log
    Buzzer_beep_once();             // short alert sound
    LCD_line1("Dude u ran it!  "); // show violation message line1
    LCD_line2("                "); // clear line2 during alert
    // keep alert text for two one second ticks
    s_violation_lcd_ticks = 2u;     // keep alert text for next 2 tick events
    USART0_print("VIOLATION dist="); // serial prefix
    USART0_print_u16(cm);           // measured distance value
    USART0_print(" cm\r\n");        // units plus newline
}

// --- red phase sensing helper ---
// poll_red_camera watches ultrasonic movement and detects illegal crossing.
static void poll_red_camera(void)
{
    // only check crossing during red and while system running
    if (s_phase != PH_RED || s_running == 0u)
        return; // only enforce crossing logic during active red phase

    uint16_t cm = Ultrasound_read_cm(); // sample distance from ultrasonic sensor
    // 0 means sensor timeout or invalid pulse
    if (cm == 0u)
        return; // ignore invalid timeout reads

    // trigger when object moves from far zone to near zone
    if (s_armed != 0u && s_prev_cm >= ZONE_FAR_CM && cm <= ZONE_NEAR_CM) {
        handle_violation(cm); // violation: object crossed from far to near while red
        // disarm until object leaves far zone again
        s_armed = 0u; // one shot lockout until object moves away again
    }
    if (cm >= ZONE_FAR_CM)
        // rearm when lane is clear or object moved back
        s_armed = 1u; // object moved back far enough, allow next trigger
    // save sample for next edge comparison
    s_prev_cm = cm; // store current reading for next comparison
}

// --- public step ---
// TrafficLight_step is called from loop and coordinates buttons timer and sensor.
void TrafficLight_step(void)
{
    // pause button toggles run state
    if (PCINT_consume_pause_click() != 0u)
        s_running = (uint8_t)(1u - s_running); // toggle between 1 and 0

    // reset button returns system to initial green phase
    if (PCINT_consume_reset_click() != 0u) {
        s_phase = PH_GREEN;            // reset phase to startup state
        s_sec_left = 10u;              // reset countdown
        s_running = 1u;                // ensure running resumes
        leds_only_green();             // reset LED outputs
        SevenSeg_show_digit(s_sec_left); // reset displayed digit
        lcd_normal();                  // reset lcd text
        s_prev_cm = 999u;              // reset edge detector memory
        s_armed = 1u;                  // rearm violation detector
    }
    // one second heartbeat drives countdown and lcd timeout
    if (Timer1_consume_1s_tick() != 0u) {
        if (s_violation_lcd_ticks > 0u) {
            s_violation_lcd_ticks--; // consume one second of violation banner time
            if (s_violation_lcd_ticks == 0u)
                // restore normal phase text after alert window ends
                lcd_normal(); // return to standard phase message
        } else
            tick_one_second(); // no override active so normal countdown progression
    }
    // sample ultrasonic every loop for quicker crossing detection
    poll_red_camera(); // run crossing check every loop for responsiveness
}
