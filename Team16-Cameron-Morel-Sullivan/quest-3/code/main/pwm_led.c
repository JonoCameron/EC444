/*
  Implements PWM LED functions. Code adapted from Espressif LEDC example project.
    https://github.com/espressif/esp-idf/tree/0289d1cc81c210b719f28c65f113c45f9afd2c7b/examples/peripherals/ledc

  DJ Morel, Oct 2020
*/
#include "pwm_led.h"



void initialize_ledc( ledc_timer_config_t* ledc_timer, ledc_channel_config_t* ledc_channel, 
                      uint32_t mode, uint32_t timer, uint32_t channel, uint32_t gpio)
{
  // Configure the timer
  ledc_timer->duty_resolution = LEDC_TIMER_13_BIT;  // Resolution of PWM duty
  ledc_timer->freq_hz         = 1000;               // Frequency of PWM signal (in Hz)
  ledc_timer->speed_mode      = mode;               // Timer mode
  ledc_timer->timer_num       = timer;              // Timer index
  ledc_timer->clk_cfg         = LEDC_AUTO_CLK;      // Auto select source clock based on given resolution & duty

  // Set configuration of timer0 for low speed channels
  ledc_timer_config(ledc_timer);

  // Prepare the configuration for the LED controller channel
  ledc_channel->channel    = channel;  // Controller's channel number
  ledc_channel->duty       = 0;        // Output duty cycle
  ledc_channel->gpio_num   = gpio;     // LED GPIO pin number
  ledc_channel->speed_mode = mode;     // Speed mode (either high or low)
  ledc_channel->hpoint     = 0;        // LEDC hpoint value (max 0xfffff)
  ledc_channel->timer_sel  = timer;    // Timer servicing selected channel

  // Set LED controller with previously prepared configuration
  ledc_channel_config(ledc_channel);

  // Initialize fade service
  ledc_fade_func_install(0);
}



void fade_up(ledc_channel_config_t* ledc_channel, uint32_t duty, uint32_t fade_time)
{
  ledc_set_fade_with_time(ledc_channel->speed_mode, ledc_channel->channel, duty, fade_time);
  ledc_fade_start(ledc_channel->speed_mode, ledc_channel->channel, LEDC_FADE_NO_WAIT);
}



void fade_down(ledc_channel_config_t* ledc_channel, uint32_t fade_time)
{
  ledc_set_fade_with_time(ledc_channel->speed_mode, ledc_channel->channel, 0, fade_time);
  ledc_fade_start(ledc_channel->speed_mode, ledc_channel->channel, LEDC_FADE_NO_WAIT);
}



void set_duty(ledc_channel_config_t* ledc_channel, uint32_t duty)
{
  ledc_set_duty(ledc_channel->speed_mode, ledc_channel->channel, duty);
  ledc_update_duty(ledc_channel->speed_mode, ledc_channel->channel);
}


