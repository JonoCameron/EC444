/*
  Implements timer functions. Code adapted from EC444 traffic-light-ir-example 
  project.
  --> https://github.com/BU-EC444/code-examples/tree/master/traffic-light-ir-example

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
#include "timer.h"



// Initialize queue handler for timer-based events
void initialize_TimerQueue()
{
  // Create a FIFO queue for timer-based events
  timer_queue = xQueueCreate(10, sizeof(timer_event_t));
}



// ISR handler
void IRAM_ATTR timer_group0_isr(void *para)
{
  // Prepare basic event data, aka set flag
  timer_event_t evt;
  evt.flag = 1;

  // Clear the interrupt, Timer 0 in group 0
  TIMERG0.int_clr_timers.t0 = 1;

  // After the alarm triggers, we need to re-enable it to trigger it next time
  TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;

  // Send the event data back to the main program task
  xQueueSendFromISR(timer_queue, &evt, NULL);
}



// Initialize timer 0 in group 0 for 1 sec alarm interval and auto reload
void alarm_init() 
{
  // Select and initialize basic parameters of the timer
  timer_config_t config;
  config.divider = TIMER_DIVIDER;
  config.counter_dir = TIMER_COUNT_UP;
  config.counter_en = TIMER_PAUSE;
  config.alarm_en = TIMER_ALARM_EN;
  config.intr_type = TIMER_INTR_LEVEL;
  config.auto_reload = TEST_WITH_RELOAD;
  timer_init(TIMER_GROUP_0, TIMER_0, &config);

  // Timer's counter will initially start from value below
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);

  // Configure the alarm value and the interrupt on alarm (use 100ms)
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_100_MS * TIMER_SCALE_MS);
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_register( TIMER_GROUP_0, TIMER_0, timer_group0_isr, (void *) TIMER_0, ESP_INTR_FLAG_IRAM, NULL);

  // Start timer
  timer_start(TIMER_GROUP_0, TIMER_0);
}


