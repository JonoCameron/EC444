/*
Written using the ir traffic light example on the BU-EC444 code-examples repository
Jonathan Cameron, November 2020
*/

#ifndef TIMER_H
#define TIMER_H

#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

// LED Output pins definitions
#define BLUEPIN   14
#define GREENPIN  32
#define REDPIN    15
#define ONBOARD   13

#define TIMER_DIVIDER         16    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_2_SEC  (2)
#define TIMER_INTERVAL_10_SEC (10)
#define TEST_WITH_RELOAD      1     // Testing will be done with auto reload
#define TIMER_INTERVAL_100_MS (100)    
#define TIMER_SCALE_MS        (TIMER_SCALE / 1000)  

// A simple structure to pass "events" to main task
typedef struct {
    int flag;     // flag for enabling stuff in timer task
} timer_event_t;

xQueueHandle timer_queue;

void timerQueue_init();

// ISR handler
void IRAM_ATTR timer_group0_isr(void *para);

// Configure timer
void alarm_init();

#endif