/*
  Header file for PWM motor control functions. Code adapted from the Espressif 
  MCPWM Brushed DC Motor Control example project.
  --> https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_brushed_dc_control

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
#ifndef MCPWMMOTOR_H
#define MCPWMMOTOR_H

#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"


// Parameters
#define GPIO_PWMA_L_OUT 32  // Set GPIO 32 (pin 32) as PWM0A --> IN1
#define GPIO_PWMB_L_OUT 15  // Set GPIO 15 (pin 15) as PWM0B --> IN2
#define GPIO_PWMA_R_OUT 26  // Set GPIO 26 (pin A0) as PWM1A --> IN1
#define GPIO_PWMB_R_OUT 25  // Set GPIO 25 (pin A1) as PWM1B --> IN2


/**
  Initializes DC motors to support PWM.
  \param None
  \return None
**/
void mcpwm_motor_initialize();

/**
  Sets a motor to move in the forwards direction with a given duty cycle.
  \param mcpwm_unit_t mcpwm_num --> MCPWM unit to reference.
  \param mcpwm_timer_t timer_num --> MCPWM timer to reference.
  \param float duty_cycle --> Duty cycle to set the movement (as a percentage).
  \return None
**/
void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle);

/**
  Sets a motor to move in the backwards direction with a given duty cycle.
  \param mcpwm_unit_t mcpwm_num --> MCPWM unit to reference.
  \param mcpwm_timer_t timer_num --> MCPWM timer to reference.
  \param float duty_cycle --> Duty cycle to set the movement (as a percentage).
  \return None
**/
void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle);

/**
  Stops a motor from moving.
  \param mcpwm_unit_t mcpwm_num --> MCPWM unit to reference.
  \param mcpwm_timer_t timer_num --> MCPWM timer to reference.
  \return None
**/
void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num);

/**
  Sets both of a car's wheels to move forwards with a given duty cycle.
  \param float duty_cycle_L --> Duty cycle for the left motor (as a percentage).
  \param float duty_cycle_R --> Duty cycle for the right motor (as a percentage).
  \return None
**/
void drive_forward(float duty_cycle_L, float duty_cycle_R);

/**
  Sets both of a car's wheels to move backwards with a given duty cycle.
  \param float duty_cycle_L --> Duty cycle for the left motor (as a percentage).
  \param float duty_cycle_R --> Duty cycle for the right motor (as a percentage).
  \return None
**/
void drive_backward(float duty_cycle_L, float duty_cycle_R);

/**
  Turns a car left by only moving its right wheel forward with a given duty cycle.
  \param float duty_cycle --> Duty cycle for the right motor (as a percentage).
  \return None
**/
void turn_left(float duty_cycle);

/**
  Turns a car right by only moving its left wheel forward with a given duty cycle.
  \param float duty_cycle --> Duty cycle for the left motor (as a percentage).
  \return None
**/
void turn_right(float duty_cycle);

/**
  Spins a car in place clockwise by moving its left wheel forwards & right wheel backwards with a given duty cycle.
  \param float duty_cycle_L --> Duty cycle for the left motor (as a percentage).
  \param float duty_cycle_R --> Duty cycle for the right motor (as a percentage).
  \return None
**/
void spin_cw(float duty_cycle_L, float duty_cycle_R);

/**
  Spins a car in place counter-clockwise by moving its right wheel forwards & left wheel backwards with a given duty cycle.
  \param float duty_cycle_L --> Duty cycle for the left motor (as a percentage).
  \param float duty_cycle_R --> Duty cycle for the right motor (as a percentage).
  \return None
**/
void spin_ccw(float duty_cycle_L, float duty_cycle_R);

/**
  Stops a car by setting both of its wheel motors off.
  \param None
  \return None
**/
void stop();


#endif


