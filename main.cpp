/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mbed.h"

// Time constants in seconds
#define HOUR   60 * 60
#define MINUTE 60

// Globals
DigitalOut alarm_out(D2, 0);
DigitalOut alarm_led(LED_RED, 1);
DigitalOut hour_led(LED_GREEN, 1);
DigitalOut min_led(LED_BLUE, 1);

InterruptIn inc_time(BUTTON1);
InterruptIn sel(BUTTON2);

LowPowerTicker alarm_event;

volatile uint64_t delay        = 0;
volatile uint8_t  hour_count   = 0;
volatile uint8_t  min_count    = 0;
volatile uint8_t  select_state = 0;

// Timer Callbacks
void inc_select(void) {
    if (select_state < 2) {
        select_state++;
    } else {
        // Use select button to disable alarm
        alarm_out = 0;
        alarm_led = 1;
    }
}

void set_time_leds(void) {
    if (select_state == 0) {
        hour_led = !hour_led;
    } else {
        min_led = !min_led;
    }
}

void inc_delay(void) {
    if (select_state == 0) {
        delay += HOUR;
        hour_count++;
        hour_led = !hour_led;
    } else {
        delay += MINUTE;
        min_count++;
        min_led = !min_led;
    }
}

void trigger_alarm_out(void) {
    alarm_out = 1;
    alarm_led = 0;
}

/* Use buttons to select alarm time. Cycle through hours in an incrementing
 * fashion using button1, hit select and increment through minutes. Hit
 * select one more time to begin the alarm timer.
 *
 * The Time LEDs will blink in time with the button inputs to show the
 * currently selected alarm time. Once select is hit a second time to begin
 * the timer, the LEDs will blink out the configured delay in hours and
 * minutes before going into a low power sleep mode.
 *
 * Once the alarm fires, hitting the select button will turn the alarm off
 * until the next time it fires.
 *__________________________________________________________________________
 * You may also use the RTC (hardware or software through the Time API) to
 * set a real world time and set an alarm for a specific timestamp rather
 * than on a delay. This would require manually setting the time on each
 * reset however, or an internet connection to automatically collect the
 * time.
 */
// Main thread
int main() {
    // Configure interrupt-in pins (button controls)
    sel.rise(inc_select);
    inc_time.fall(set_time_leds);
    inc_time.rise(inc_delay);

    // Sleep while waiting for user input to set the desired delay
    while (select_state < 2) { wait_ms(10); }

    // Once the delay has been input, blink back the configured hours and
    // minutes selected
    for (uint8_t i = 0; i < hour_count * 2; i++) {
        hour_led = !hour_led;
        wait(0.25f);
    }

    for (uint8_t i = 0; i < min_count * 2; i++) {
        min_led = !min_led;
        wait(0.25f);
    }

    // Attach the low power ticker with the configured alarm delay
    alarm_event.attach(&trigger_alarm_out, delay);

    // Sleep in the main thread
    while (1) { wait(100000); }
}
