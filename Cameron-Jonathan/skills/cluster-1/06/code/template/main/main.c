/*
 * Jonathan 2020-09-07
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include <string.h>
#include "driver/uart.h"
#include "esp_vfs_dev.h"

#define BLINK_GPIO 13
#define BUF_SIZE 1024

int change_mode(int mode, char *buf);
int blonk(int LED, char *buf);

void app_main(){
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0,
      256, 0, 0, NULL, 0) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);

    /* Set up the GPIO as output */
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    char buf[BUF_SIZE];
    int mode = 0;
    int LED = 0;
    int num = 0;

    while(1) {

        switch(mode){
            case 0: printf("toggle mode\n");

                    while(mode == 0){
                        printf("Read: ");
                        gets(buf);

                        mode = change_mode(mode, buf);
                        LED = blonk(LED, buf);
                        
                        vTaskDelay(50 / portTICK_RATE_MS);
                    }

                    break;
            case 1: printf("echo mode\n");

                    while(mode == 1){
                        printf("echo: ");
                        gets(buf);
                        printf("%s\n", buf);

                        mode = change_mode(mode, buf);

                        vTaskDelay(50 / portTICK_RATE_MS);
                    }

                    break;
            case 2: printf("echo dec to hex mode\n");

                    while(mode == 2){
                        printf("Enter an interger: ");
                        gets(buf);
                        
                        num = atoi(buf);

                        mode = change_mode(mode, buf);
                        
                        printf("Ox%X\n", num);
                        vTaskDelay(50 / portTICK_RATE_MS);
                    }
                    break;
        }

        vTaskDelay(50 / portTICK_RATE_MS);
    }
}

int change_mode(int mode, char *buf){
    if (buf[0] == 's' && buf[1] == '\0'){
        mode++;
        if(mode == 3)
            mode = 0;
    }
    return mode;
}

int blonk(int LED, char* buf){
    if(buf[0] == 't' && buf[1] == '\0'){
        if(LED == 0){
            gpio_set_level(BLINK_GPIO, 1);
            printf("%s\n", buf);
            return LED = 1;
        }
        else if(LED == 1){
            gpio_set_level(BLINK_GPIO, 0);
            printf("%s\n", buf);
            return LED = 0;
        }
    }
    printf("%s\n", buf);
    return LED;
}

/*
#define ECHO_TEST_TXD (1)
#define ECHO_TEST_RXD (3)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      (0)
#define ECHO_UART_BAUD_RATE     (115200)
#define ECHO_TASK_STACK_SIZE    (2048)

#define BUF_SIZE (1024)

static void echo_task(void *arg)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    int LED = 0;

     Configure parameters of an UART driver,
       communication pins and install the driver 
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        if(data[0] == 't' && LED == 0){
            gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
            gpio_set_level(BLINK_GPIO, 1);
            LED = 1;
        }
        else if(data[0] == 't' && LED == 1){
            gpio_reset_pin(BLINK_GPIO);
            LED = 0;
        }
        else
        {
            printf(".");
        }
        

        // Write data back to the UART
        uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) data, len);
    }
}

void app_main(void)
{
    xTaskCreate(echo_task, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
}

#define BLINK_GPIO 13

void app_main(void)
{
    
    gpio_reset_pin(BLINK_GPIO);

    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        // Blink off (output low) 
    	printf("Turning off the LED\n");
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // Blink on (output high)
	    printf("Turning on the LED\n");
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
*/