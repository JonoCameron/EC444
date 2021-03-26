/*
  Implements steering and motor control of the Buggy.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/


#include "buggy_motor.h"

// GPIO initialization function //

void buggy_mcpwm_init()
{

    mcpwm_config_t pwm_config;
    pwm_config.frequency = 100;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOTOR_GPIO);    //Set GPIO 12 as PWM0A, to which servo is connected
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, STEERING_GPIO);    //Set GPIO 18 as PWM0A, to which servo is connected
}


// ESC functions //

// calibrates the ESC //
void calibrateESC() {
    vTaskDelay(4000 / portTICK_PERIOD_MS);  // Give yourself time to turn on crawler
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, SERVO_MED_PULSEWIDTH);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    printf("Calibrated........\n");
}

// Tests ESC functions //
void testESC(){
    printf("FORWARD:\n");
    forwardESC(200);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("REVERSE:\n");
    printf("\tpulse width: %dus\n", SERVO_MIN_PULSEWIDTH);
    reverseESC(200);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("NEUTRAL:\n");
    printf("\tpulse width: %dus\n", SERVO_MED_PULSEWIDTH);
    brakeESC();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

// Due to the forward brake reverse style of the ESC the buggy must first be going FORWARD before receiving a REVERSE signal before going back to NEUTRAL and then to REVERSE //
void reverseESC(int speed){
    if((SERVO_MED_PULSEWIDTH - speed) < SERVO_MIN_PULSEWIDTH){
        return;
    }
    //forwardESC(speed);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, SERVO_MED_PULSEWIDTH - speed);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, SERVO_MED_PULSEWIDTH);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, SERVO_MED_PULSEWIDTH - speed);
}

// sends FORWARD signal to ESC //
void forwardESC(int speed){
    if((speed + SERVO_MED_PULSEWIDTH) > SERVO_MAX_PULSEWIDTH){
        return;
    }
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, SERVO_MED_PULSEWIDTH + speed);
}

// sends BRAKE signal to ESC //
void brakeESC(){
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, SERVO_MED_PULSEWIDTH - 100);
}


// Steering functions //

// sends LEFT signal to steering servo //
void turnLeft(int angle){
    mcpwm_set_duty_in_us(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, SERVO_MID - angle);
}

// sends LEFT signal to steering servo //
void turnRight(int angle){
    mcpwm_set_duty_in_us(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, SERVO_MID + angle);
}

// sends LEFT signal to steering servo //
void goStraight(){
    mcpwm_set_duty_in_us(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, SERVO_MID);
}

// tests the steering range for the Buggy
void testSteering(){
    printf("LEFT\n");
    turnLeft(500);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("RIGHT\n");
    turnRight(500);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("MIDDLE\n");
    goStraight();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}
