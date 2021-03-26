/*
  Implements functions for hardware interrupts (button). Code adapted from 
  EC444 traffic-light-ir-example project.
  --> https://github.com/BU-EC444/code-examples/tree/master/traffic-light-ir-example

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Dec. 2020
*/
#include "button_intr.h"



// Button interrupt handler -- add to queue
void IRAM_ATTR gpio_isr_handler_queue(void* arg)
{
  uint32_t gpio_num = (uint32_t) arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// Button interrupt handler -- add to queue
void IRAM_ATTR gpio_isr_handler_toggle(void* arg)
{
  // Activate the enable signal flag
  // Note: relies on the send signal task to reset the flag
  //en_signal = true;

  uint32_t gpio_num = (uint32_t) arg;
  xQueueSendFromISR(gpio_evt_queue_transmit, &gpio_num, NULL);
}



// Button interrupt init
void button_queue_init(uint32_t gpio_pin)//, gpio_config_t* io_conf)
{
  // Create a GPIO configure object
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;       // Interrupt of rising edge
  io_conf.pin_bit_mask = 1ULL << gpio_pin;         // Bit mask of the pins, use GPIO4 here
  io_conf.mode = GPIO_MODE_INPUT;                  // Set as input mode
  io_conf.pull_up_en = 1;                          // Enable pull-up mode

  // Send the GPIO info for configuration
  gpio_config(&io_conf);
  gpio_intr_enable(gpio_pin);

  // Rely on caller to install the gpio isr service
  //gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);  // Install gpio isr service

  // Hook isr handler for specific GPIO pin
  gpio_isr_handler_add(gpio_pin, gpio_isr_handler_queue, (void*) gpio_pin);

  // Create a queue to handle GPIO event from isr then start GPIO task
  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
}



// Button interrupt init
void button_toggle_init(uint32_t gpio_pin)//, gpio_config_t* io_conf)
{
  // Initialize the enable signal to false
  en_signal = false;

  // Create a GPIO configure object
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;       // Interrupt of rising edge
  io_conf.pin_bit_mask = 1ULL << gpio_pin;         // Bit mask of the pins, use GPIO36 here
  io_conf.mode = GPIO_MODE_INPUT;                  // Set as input mode
  io_conf.pull_up_en = 1;                          // Enable pull-up mode

  // Send the GPIO info for configuration
  gpio_config(&io_conf);
  gpio_intr_enable(gpio_pin);

  // Rely on caller to install the gpio isr service
  //gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);  // Install gpio isr service

  // Hook isr handler for specific GPIO pin
  gpio_isr_handler_add(gpio_pin, gpio_isr_handler_toggle, (void*) gpio_pin);

  gpio_evt_queue_transmit = xQueueCreate(10, sizeof(uint32_t));
}
