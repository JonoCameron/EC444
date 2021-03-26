/*
  Header file for timer.c functions.

  DJ Morel, Sept. 2020
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


#define TIMER_DIVIDER         16    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_SEC   (1)    // Sample test interval for the first timer
#define TEST_WITH_RELOAD      1     // Testing will be done with auto reload


// A simple structure to pass "events" to main task
typedef struct
{
  int flag;  // Flag for enabling stuff in main code
} timer_event_t;


// Queue handler for timer-based events
xQueueHandle timer_queue;


// Initialize queue handler for timer-based events
void initialize_TimerQueue();


// ISR handler
void IRAM_ATTR timer_group0_isr(void *para);


// Initialize timer 0 in group 0 for 1 sec alarm interval and auto reload
void alarm_init();


#endif

