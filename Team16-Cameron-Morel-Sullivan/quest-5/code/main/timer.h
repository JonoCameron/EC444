/*
  Header file for timer functions. Code adapted from EC444 
  traffic-light-ir-example project.
  --> https://github.com/BU-EC444/code-examples/tree/master/traffic-light-ir-example

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
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


#define TIMER_DIVIDER         16                                // Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // Converts timer clock to seconds
#define TIMER_SCALE_MS        (TIMER_SCALE / 1000)              // Converts timer clock to milliseconds
#define TIMER_INTERVAL_SEC    (1)                               // 1 second timer interval
#define TIMER_INTERVAL_2_SEC  (2)                               // 2 second timer interval
#define TIMER_INTERVAL_10_SEC (10)                              // 10 second timer interval
#define TIMER_INTERVAL_100_MS (100)                             // 100ms timer interval
#define TEST_WITH_RELOAD      1                                 // Testing will be done with auto reload


// A simple structure to pass "events" to main task
typedef struct
{
  int flag;  // Flag for enabling stuff in main code
} timer_event_t;


// Queue handler for timer-based events
xQueueHandle timer_queue;


/**
  Initializes queue handler for timer-based events.
  \param None
  \return None
**/
void initialize_TimerQueue();

/**
  ISR handler. Currently sets timer interrupts to occur every 10 seconds.
  \param void* para --> Pointer to parameters.
  \return None
**/
void IRAM_ATTR timer_group0_isr(void* para);

/**
  Initializes timer 0 in group 0 for 1 sec alarm interval and auto reload.
  \param None
  \return None
**/
void alarm_init();


#endif

