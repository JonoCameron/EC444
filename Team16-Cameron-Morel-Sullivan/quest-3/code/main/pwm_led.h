/*
  Header file for PWM LED functions. Code adapted from Espressif LEDC example 
  project.
    https://github.com/espressif/esp-idf/tree/0289d1cc81c210b719f28c65f113c45f9afd2c7b/examples/peripherals/ledc

  DJ Morel, Oct 2020
*/
#ifndef PWMLED_H
#define PWMLED_H


#include "driver/ledc.h"
#include "esp_err.h"


/**
  Initializes LED control structs and fade service. Requires caller to initialize memory for the pointers.
  \param ledc_timer_config_t* ledc_timer --> Pointer to LEDC timer struct.
  \param ledc_channel_config_t* ledc_timer --> Pointer to LEDC channel struct.
  \param uint32_t mode --> LEDC speed mode (high or low).
  \param uint32_t timer --> Timer index.
  \param uint32_t channel --> Controller's channel number.
  \param uint32_t gpio --> LED GPIO pin number.
  \return None
**/
void initialize_ledc( ledc_timer_config_t* ledc_timer, ledc_channel_config_t* ledc_channel,
                      uint32_t mode, uint32_t timer, uint32_t channel, uint32_t gpio);

/**
  Fades the LED up to the specified duty.
  \param ledc_channel_config_t* ledc_channel --> Pointer to the targeted LEDC channel struct.
  \param uint32_t duty --> PWM value for max LED intensity during fade.
  \param uint32_t fade_time --> Duration of the fade (in ms).
  \return None
**/
void fade_up(ledc_channel_config_t* ledc_channel, uint32_t duty, uint32_t fade_time);

/**
  Fades the LED to 0 (off).
  \param ledc_channel_config_t* ledc_channel --> Pointer to the targeted LEDC channel struct.
  \param uint32_t fade_time --> Duration of the fade (in ms).
  \return None
**/
void fade_down(ledc_channel_config_t* ledc_channel, uint32_t fade_time);

/**
  Sets the duty (intensity) of the LED to a specified value. Does not fade.
  \param ledc_channel_config_t* ledc_channel --> Pointer to the targeted LEDC channel struct.
  \param uint32_t duty --> PWM for LED intensity.
  \return None
**/
void set_duty(ledc_channel_config_t* ledc_channel, uint32_t duty);


#endif


