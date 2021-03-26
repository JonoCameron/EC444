/*
  Implements functions to move a PWM motor in various directions. Code adapted 
  from the Espressif MCPWM Brushed DC Motor Control example project.
  --> https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_brushed_dc_control

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
#include "mcpwm_motor.h"


// Parameters
#define GPIO_PWMA_L_OUT 32  // Set GPIO 32 (pin 32) as PWM0A --> IN1
#define GPIO_PWMB_L_OUT 15  // Set GPIO 15 (pin 15) as PWM0B --> IN2
#define GPIO_PWMA_R_OUT 26  // Set GPIO 26 (pin A0) as PWM1A --> IN1
#define GPIO_PWMB_R_OUT 25  // Set GPIO 25 (pin A1) as PWM1B --> IN2



void mcpwm_motor_initialize()
{
  printf("initializing mcpwm gpio...\n");

  // Initialize left wheel IN1 and IN2 PWM pins
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWMA_L_OUT);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWMB_L_OUT);

  // Initialize right wheel IN1 and IN2 PWM pins
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, GPIO_PWMA_R_OUT);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, GPIO_PWMB_R_OUT);
}



void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle)
{
  mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
  mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
  // Call this each time, if operator was previously in low/high state
  mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
}



void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle)
{
  mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
  mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);

  // Call this each time, if operator was previously in low/high state
  mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
}



void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
  mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
  mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
}



void drive_forward(float duty_cycle_L, float duty_cycle_R)
{
  // Set both wheel motors forward
  brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, duty_cycle_L);
  brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_1, duty_cycle_R);
}



void drive_backward(float duty_cycle_L, float duty_cycle_R)
{
  // Set both wheel motors backwards
  brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, duty_cycle_L);
  brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_1, duty_cycle_R);
}



void turn_left(float duty_cycle)
{
  // Set the right wheel forward
  brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_1, duty_cycle);

  // Stop the left wheel
  brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
}



void turn_right(float duty_cycle)
{
  // Set the left wheel forward
  brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, duty_cycle);

  // Stop the right wheel
  brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_1);
}



void spin_cw(float duty_cycle_L, float duty_cycle_R)
{
  // Set the left wheel forward
  brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, duty_cycle_L);

  // Set the right wheel backwards
  brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_1, duty_cycle_R);
}



void spin_ccw(float duty_cycle_L, float duty_cycle_R)
{
  // Set the left wheel backwards
  brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, duty_cycle_L);

  // Set the right wheel forwards
  brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_1, duty_cycle_R);
}


void stop()
{
  // Stop both wheel motors
  brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
  brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_1);
}


