/*
  Implements servo functions. Adapted from Espressif mcpwm_servo_control example project.  
  --> https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_servo_control  

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Dec. 2020
*/
#include "servo.h"



uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
{
  uint32_t cal_pulsewidth = 0;
  cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + ( ((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) 
                                             / (SERVO_MAX_DEGREE)) );
  return cal_pulsewidth;
}



void initialize_servo(uint32_t servo_gpio)
{
  // MCPWM GPIO Initialization
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, servo_gpio);
}



int servo_rotateTo(uint32_t degree, uint32_t delay_ms)
{
  uint32_t angle;

  // Assumes servo GPIO already configured from initialize_servo()

  // Initial MCPWM Configuration
  mcpwm_config_t pwm_config;
  pwm_config.frequency = 50;  // Frequency = 50Hz (i.e. for every servo motor time period should be 20ms)
  pwm_config.cmpr_a = 0;      // Duty cycle of PWMxA = 0
  pwm_config.cmpr_b = 0;      // Duty cycle of PWMxb = 0
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);  //Configure PWM0A & PWM0B with above settings

  // Check that the passed degree argument is within the servo's range of motion
  if (degree > SERVO_MAX_DEGREE)
  {
    // Out of bounds, so return an error
    return -1;
  }

  // Set servo to the specified degree location
  angle = servo_per_degree_init(degree);
  mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);

  // Add some delay
  vTaskDelay(delay_ms);  // Recommended delay of 100ms per 60degrees at 5V (requires caller to know previous angle)

  // Turn the servo off
  mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);

  // Successfully moved the servo
  return 0;
}


