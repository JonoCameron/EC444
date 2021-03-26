/**
 *
 * ESP-IDF driver for ultrasonic range meters, e.g. HC-SR04, HY-SRF05 and the like
 *
 * Ported from esp-open-rtos
 *
 * Copyright (C) 2016, 2018 Ruslan V. Uss <unclerus@gmail.com>
 *
 * BSD Licensed as described in the file LICENSE
 * 
 * Modified by Jonathan Cameron October 2020.
 */
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "i2c_display.h"
#include <esp_timer.h>


#define TRIGGER_LOW_DELAY 4
#define TRIGGER_HIGH_DELAY 10
#define PING_TIMEOUT 6000
#define ROUNDTRIP 58

#define TRIGGER 13
#define ECHO 32

double distance = 0;

esp_err_t ultrasonic_init()
{

    //Reset the pins for this purpose. This was important.
    gpio_reset_pin(TRIGGER);
    gpio_reset_pin(ECHO);

    //Set the pin I/O directions TRIGGER is out and ECHO is in.
    gpio_set_direction(TRIGGER, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO, GPIO_MODE_INPUT);

    gpio_set_level(TRIGGER, 0);
    printf("Ultrasonic initialised\n");

    return 0;
}

static void ultrasonic_measure_m()
{
    int64_t time_t = 0;
    int64_t start_t = 0;

    int echo;
    while(1){
        // Ping: Low for 2..4 us, then high 10 us
        //This is a 60uS ultrasonic pulse that TRIGGER bounces off the object infront of the sensor, to be detected by ECHO 
        gpio_set_level(TRIGGER, 0);
        ets_delay_us(TRIGGER_LOW_DELAY);
        gpio_set_level(TRIGGER, 1);
        ets_delay_us(TRIGGER_HIGH_DELAY);
        gpio_set_level(TRIGGER, 0);

        //Read ECHO, waiting return signal
        echo = gpio_get_level(ECHO);
        
        while (echo == 0)
        {
            //Reset start_t to this loop, and re-read ECHO (until !0) to break the loop.
            start_t = esp_timer_get_time();
            echo = gpio_get_level(ECHO);
        }

        while (echo == 1)
        {
            //Wait until echo == 0 again to take final time 
            time_t = esp_timer_get_time();
            echo = gpio_get_level(ECHO);
        }

        //Distance is the total time * speed of sound / 2 to account for the pulse doing twice the distance to get back to ECHO. 
        //Divide by 10000 to make it metres.
        
        distance = (double) (time_t - start_t)*340 / 2 / 10000;
        printf("Distance: %fm\n", distance);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void app_main(void){
    ultrasonic_init();  
    
    xTaskCreate(ultrasonic_measure_m, "ultrasonic_measure_m", 4096, NULL, 4, NULL);
}