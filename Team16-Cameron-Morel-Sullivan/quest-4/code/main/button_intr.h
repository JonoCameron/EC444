/*
  Header file for hardware interrupts (button). Code adapted from EC444 
  traffic-light-ir-example project.
  --> https://github.com/BU-EC444/code-examples/tree/master/traffic-light-ir-example

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
#ifndef BUTTONINTR_H
#define BUTTONINTR_H


#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"


// Hardware interrupt definitions
#define ESP_INTR_FLAG_DEFAULT 0


// Global Variables
xQueueHandle gpio_evt_queue;           // Button event queue for LED color changes
xQueueHandle gpio_evt_queue_transmit;  // Button event queue for IR transmissions
bool en_signal;                        // Flag to tell when to send IR signals (see main.c's send_task)


/**
  Button interrupt handler. Adds a hardware interrupt to the queue.
  \param void* arg --> Pointer to arguments. Configured in button_queue_init() to specify GPIO pin.
  \return None
**/
void IRAM_ATTR gpio_isr_handler_queue(void* arg);

/**
  Button interrupt handler. Sets en_signal flag to true. Puts flag resetting responsibility on main's send_task().
  \param void* arg --> Pointer to arguments. Note: not currently used.
  \return None
**/
void IRAM_ATTR gpio_isr_handler_toggle(void* arg);

/**
  Initializes button interrupts that are sent to the GPIO event queue. Use for the color button. Note: rely on caller to set gpio_install_isr_service().
  \param uint32_t gpio_pin --> Button pin number.
  \return None
**/
void button_queue_init(uint32_t gpio_pin);

/**
  Initializes button interrupts that toggle flags. Use for the send signal button. Note: rely on caller to set gpio_install_isr_service().
  \param uint32_t gpio_pin --> Button pin number.
  \return None
**/
void button_toggle_init(uint32_t gpio_pin);

#endif  
