/*
 * Jonathan Cameron 2020-09-08
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define GPIO13 13
#define GPIO27 27
#define GPIO15 15
#define GPIO14 14

void app_main(void)
{
    
    gpio_reset_pin(GPIO13);
    gpio_reset_pin(GPIO27);
    gpio_reset_pin(GPIO15);
    gpio_reset_pin(GPIO14);

    gpio_set_direction(GPIO13, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO27, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO15, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO14, GPIO_MODE_OUTPUT);

    int bin = 0;

    while(1) {
        
        int temp = bin;
        printf("%d\n", bin);

        if((temp - 8) >= 0){
            gpio_set_level(GPIO14, 1);
            temp = temp - 8;
        }
        else
            gpio_set_level(GPIO14, 0);

        if((temp - 4) >= 0){
            gpio_set_level(GPIO15, 1);
            temp = temp - 4;
        }
        else
            gpio_set_level(GPIO15, 0);

        if((temp - 2) >= 0){
            gpio_set_level(GPIO27, 1);
            temp = temp - 2;
        }
        else
            gpio_set_level(GPIO27, 0);

        
        if(temp == 0)
                gpio_set_level(GPIO13, 0);
        else
                gpio_set_level(GPIO13, 1);
        
        if(bin == 15)
                bin = 0;
        else
                bin++;

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

