/*
  Adapted I2C example code to work with the Adafruit 14-segment Alphanumeric Display. Key notes: MSB!!

  Emily Lam, Sept 2018, Updated Aug 2019
  Edited by Jonathan Cameron, September 2020
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "driver/i2c.h"


#define GPIO32 32
#define GPIO13 13
#define GPIO27 27
#define GPIO15 15
#define GPIO14 14

int flag = 0;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    flag ^=1;
}

// 14-Segment Display
#define SLAVE_ADDR                         0x70 // alphanumeric address
#define OSC                                0x21 // oscillator cmd
#define HT16K33_BLINK_DISPLAYON            0x01 // Display on cmd
#define HT16K33_BLINK_OFF                  0    // Blink off cmd
#define HT16K33_BLINK_CMD                  0x80 // Blink cmd
#define HT16K33_CMD_BRIGHTNESS             0xE0 // Brightness cmd

// Master I2C
#define I2C_EXAMPLE_MASTER_SCL_IO          22   // gpio number for i2c clk
#define I2C_EXAMPLE_MASTER_SDA_IO          23   // gpio number for i2c data
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_0  // i2c port
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000     // i2c master clock freq
#define WRITE_BIT                          I2C_MASTER_WRITE // i2c master write
#define READ_BIT                           I2C_MASTER_READ  // i2c master read
#define ACK_CHECK_EN                       true // i2c master will check ack
#define ACK_CHECK_DIS                      false// i2c master will not check ack
#define ACK_VAL                            0x00 // i2c ack value
#define NACK_VAL                           0xFF // i2c nack value

#define BUF_SIZE                           1024

static void led_driver(){

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
        if(flag == 0){
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
        }
        if(flag == 1){
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
            
            if(bin == 0)
                    bin = 15;
            else
                    bin--;
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

// Function to initiate i2c -- note the MSB declaration!
static void i2c_example_master_init(){
    // Debug
    printf("\n>> i2c Config\n");
    int err;

    // Port configuration
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;

    /// Define I2C configurations
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;                              // Master mode
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;              // Default SDA pin
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;              // Default SCL pin
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;       // CLK frequency
    err = i2c_param_config(i2c_master_port, &conf);           // Configure
    if (err == ESP_OK) {printf("- parameters: ok\n");}

    // Install I2C driver
    err = i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                       I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
    // i2c_set_data_mode(i2c_master_port,I2C_DATA_MODE_LSB_FIRST,I2C_DATA_MODE_LSB_FIRST);
    if (err == ESP_OK) {printf("- initialized: yes\n\n");}

    // Dat in MSB mode
    i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}

// Utility  Functions //////////////////////////////////////////////////////////

// Utility function to test for I2C device address -- not used in deploy
int testConnection(uint8_t devAddr, int32_t timeout) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return err;
}

// Utility function to scan for i2c device
static void i2c_scanner() {
    int32_t scanTimeout = 1000;
    printf("\n>> I2C scanning ..."  "\n");
    uint8_t count = 0;
    for (uint8_t i = 1; i < 127; i++) {
        // printf("0x%X%s",i,"\n");
        if (testConnection(i, scanTimeout) == ESP_OK) {
            printf( "- Device found at address: 0x%X%s", i, "\n");
            count++;
        }
    }
    if (count == 0)
        printf("- No I2C devices found!" "\n");
    printf("\n");
}

// Alphanumeric Functions //////////////////////////////////////////////////////

// Turn on oscillator for alpha display
int alpha_oscillator() {
  int ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, OSC, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  vTaskDelay(200 / portTICK_RATE_MS);
  return ret;
}

// Set blink rate to off
int no_blink() {
  int ret;
  i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
  i2c_master_start(cmd2);
  i2c_master_write_byte(cmd2, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd2, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (HT16K33_BLINK_OFF << 1), ACK_CHECK_EN);
  i2c_master_stop(cmd2);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd2, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd2);
  vTaskDelay(200 / portTICK_RATE_MS);
  return ret;
}

// Set Brightness
int set_brightness_max(uint8_t val) {
  int ret;
  i2c_cmd_handle_t cmd3 = i2c_cmd_link_create();
  i2c_master_start(cmd3);
  i2c_master_write_byte(cmd3, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd3, HT16K33_CMD_BRIGHTNESS | val, ACK_CHECK_EN);
  i2c_master_stop(cmd3);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd3, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd3);
  vTaskDelay(200 / portTICK_RATE_MS);
  return ret;
}

////////////////////////////////////////////////////////////////////////////////

void retChars(uint16_t* displaybuffer, char* buf){

  for(int i = 0; i < 4; i++){
    switch(buf[i]){
      case('!'): displaybuffer[i] = 0b0000000000000110; // !
                 break;
      case('"'): displaybuffer[i] = 0b0000001000100000; // "
                 break;
      case('#'): displaybuffer[i] = 0b0001001011001110; // #
                 break;
      case('$'): displaybuffer[i] = 0b0001001011101101; // $
                 break;
      case('%'): displaybuffer[i] = 0b0000110000100100; // %
                 break;
      case('&'): displaybuffer[i] = 0b0010001101011101; // &
                 break;
      //case(''): displaybuffer[i] = 0b0000010000000000 // '
      //           break;
      case('('): displaybuffer[i] = 0b0010010000000000; // (
                 break;
      case(')'): displaybuffer[i] = 0b0000100100000000; // )
                 break;
      case('*'): displaybuffer[i] = 0b0011111111000000; // *
                 break;
      case('+'): displaybuffer[i] = 0b0001001011000000; // +
                 break;
      case(','): displaybuffer[i] = 0b0000100000000000; // ,
                 break;
      case('-'): displaybuffer[i] = 0b0000000011000000; // -
                 break;
      case('.'): displaybuffer[i] = 0b0000000000000000; // .
                 break;
      case('/'): displaybuffer[i] = 0b0000110000000000; // /
                 break;
      case('0'): displaybuffer[i] = 0b0000110000111111; // 0
                 break;
      case('1'): displaybuffer[i] = 0b0000000000000110; // 1
                 break;
      case('2'): displaybuffer[i] = 0b0000000011011011; // 2
                 break;
      case('3'): displaybuffer[i] = 0b0000000010001111; // 3
                 break;
      case('4'): displaybuffer[i] = 0b0000000011100110; // 4
                 break;
      case('5'): displaybuffer[i] = 0b0010000001101001; // 5
                 break;
      case('6'): displaybuffer[i] = 0b0000000011111101; // 6
                 break;
      case('7'): displaybuffer[i] = 0b0000000000000111; // 7
                 break;
      case('8'): displaybuffer[i] = 0b0000000011111111; // 8
                 break;
      case('9'): displaybuffer[i] = 0b0000000011101111; // 9
                 break;
      case(':'): displaybuffer[i] = 0b0001001000000000; // :
                 break;
      case(';'): displaybuffer[i] = 0b0000101000000000; // ;
                 break;
      case('<'): displaybuffer[i] = 0b0010010000000000; // <
                 break;
      case('='): displaybuffer[i] = 0b0000000011001000; // =
                 break;
      case('>'): displaybuffer[i] = 0b0000100100000000; // >
                 break;
      case('?'): displaybuffer[i] = 0b0001000010000011; // ?
                 break;
      case('@'): displaybuffer[i] = 0b0000001010111011; // @
                 break;
      case('A'): displaybuffer[i] = 0b0000000011110111; // A
                 break;
      case('B'): displaybuffer[i] = 0b0001001010001111; // B
                 break;
      case('C'): displaybuffer[i] = 0b0000000000111001; // C
                 break;
      case('D'): displaybuffer[i] = 0b0001001000001111; // D
                 break;
      case('E'): displaybuffer[i] = 0b0000000011111001; // E
                 break;
      case('F'): displaybuffer[i] = 0b0000000001110001; // F
                 break;
      case('G'): displaybuffer[i] = 0b0000000010111101; // G
                 break;
      case('H'): displaybuffer[i] = 0b0000000011110110; // H
                 break;
      case('I'): displaybuffer[i] = 0b0001001000000000; // I
                 break;
      case('J'): displaybuffer[i] = 0b0000000000011110; // J
                 break;
      case('K'): displaybuffer[i] = 0b0010010001110000; // K
                 break;
      case('L'): displaybuffer[i] = 0b0000000000111000; // L
                 break;
      case('M'): displaybuffer[i] = 0b0000010100110110; // M
                 break;
      case('N'): displaybuffer[i] = 0b0010000100110110; // N
                 break;
      case('O'): displaybuffer[i] = 0b0000000000111111; // O
                 break;
      case('P'): displaybuffer[i] = 0b0000000011110011; // P
                 break;
      case('Q'): displaybuffer[i] = 0b0010000000111111; // Q
                 break;
      case('R'): displaybuffer[i] = 0b0010000011110011; // R
                 break;
      case('S'): displaybuffer[i] = 0b0000000011101101; // S
                 break;
      case('T'): displaybuffer[i] = 0b0001001000000001; // T
                 break;
      case('U'): displaybuffer[i] = 0b0000000000111110; // U
                 break;
      case('V'): displaybuffer[i] = 0b0000110000110000; // V
                 break;
      case('W'): displaybuffer[i] = 0b0010100000110110; // W
                 break;
      case('X'): displaybuffer[i] = 0b0010110100000000; // X
                 break;
      case('Y'): displaybuffer[i] = 0b0001010100000000; // Y
                 break;
      case('Z'): displaybuffer[i] = 0b0000110000001001; // Z
                 break;
      case('['): displaybuffer[i] = 0b0000000000111001; // [
                 break;
      //case(''): displaybuffer[i] = 0b0010000100000000; //
                 //break;
      case(']'): displaybuffer[i] = 0b0000000000001111; // ]
                 break;
      case('^'): displaybuffer[i] = 0b0000110000000011; // ^
                 break;
      //case(''): displaybuffer[i] = 0b0000000000001000; // _
                 //break;
      case('`'): displaybuffer[i] = 0b0000000100000000; // `
                 break;
      case('a'): displaybuffer[i] = 0b0001000001011000; // a
                 break;
      case('b'): displaybuffer[i] = 0b0010000001111000; // b
                 break;
      case('c'): displaybuffer[i] = 0b0000000011011000; // c
                 break;
      case('d'): displaybuffer[i] = 0b0000100010001110; // d
                 break;
      case('e'): displaybuffer[i] = 0b0000100001011000; // e
                 break;
      case('f'): displaybuffer[i] = 0b0000000001110001; // f
                 break;
      case('g'): displaybuffer[i] = 0b0000010010001110; // g
                 break;
      case('h'): displaybuffer[i] = 0b0001000001110000; // h
                 break;
      case('i'): displaybuffer[i] = 0b0001000000000000; // i
                 break;
      case('j'): displaybuffer[i] = 0b0000000000001110; // j
                 break;
      case('k'): displaybuffer[i] = 0b0011011000000000; // k
                 break;
      case('l'): displaybuffer[i] = 0b0000000000110000; // l
                 break;
      case('m'): displaybuffer[i] = 0b0001000011010100; // m
                 break;
      case('n'): displaybuffer[i] = 0b0001000001010000; // n
                 break;
      case('o'): displaybuffer[i] = 0b0000000011011100; // o
                 break;
      case('p'): displaybuffer[i] = 0b0000000101110000; // p
                 break;
      case('q'): displaybuffer[i] = 0b0000010010000110; // q
                 break;
      case('r'): displaybuffer[i] = 0b0000000001010000; // r
                 break;
      case('s'): displaybuffer[i] = 0b0010000010001000; // s
                 break;
      case('t'): displaybuffer[i] = 0b0000000001111000; // t
                 break;
      case('u'): displaybuffer[i] = 0b0000000000011100; // u
                 break;
      case('v'): displaybuffer[i] = 0b0010000000000100; // v
                 break;
      case('w'): displaybuffer[i] = 0b0010100000010100; // w
                 break;
      case('x'): displaybuffer[i] = 0b0010100011000000; // x
                 break;
      case('y'): displaybuffer[i] = 0b0010000000001100; // y
                 break;
      case('z'): displaybuffer[i] = 0b0000100001001000; // z
                 break;
      case('{'): displaybuffer[i] = 0b0000100101001001; // {
                 break;
      case('|'): displaybuffer[i] = 0b0001001000000000; // |
                 break;
      case('}'): displaybuffer[i] = 0b0010010010001001; // }
                 break;
      case('~'): displaybuffer[i] = 0b0000010100100000; // ~
                 break;
      case(' '): displaybuffer[i] = 0b0000000000000000;
                 break;
      //case(''): displaybuffer[i] = 0b0011111111111111;
    }
  }
}


static void test_alpha_display() {
    // Debug
    int ret;
    char buf[BUF_SIZE];
    printf(">> Test Alphanumeric Display: \n");

    // Set up routines
    // Turn on alpha oscillator
    ret = alpha_oscillator();
    if(ret == ESP_OK) {printf("- oscillator: ok \n");}
    // Set display blink off
    ret = no_blink();
    if(ret == ESP_OK) {printf("- blink: off \n");}
    ret = set_brightness_max(0xF);
    if(ret == ESP_OK) {printf("- brightness: max \n");}

    // Write to characters to buffer
    uint16_t displaybuffer[8];

    // Continually writes the same command
    while (1) {
      displaybuffer[0] = 0b0000000000000000;
      displaybuffer[1] = 0b0000000000000000;
      displaybuffer[2] = 0b0000000000000000;
      displaybuffer[3] = 0b0000000000000000; 
      
      if(flag == 0){
          displaybuffer[0] = 0b0000000000111110;
          displaybuffer[1] = 0b0000000101110000;
          displaybuffer[2] = 0b0000000000000000;
          displaybuffer[3] = 0b0000000000000000; 
      }
      if(flag == 1){
          displaybuffer[0] = 0b0001001000001111;
          displaybuffer[1] = 0b0000000011011100;
          displaybuffer[2] = 0b0010100000010100;
          displaybuffer[3] = 0b0001000001010000; 
      }

      // Send commands characters to display over I2C
      i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
      i2c_master_start(cmd4);
      i2c_master_write_byte(cmd4, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
      i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);
      for (uint8_t i=0; i<8; i++) {
        i2c_master_write_byte(cmd4, displaybuffer[i] & 0xFF, ACK_CHECK_EN);
        i2c_master_write_byte(cmd4, displaybuffer[i] >> 8, ACK_CHECK_EN);
      }
      i2c_master_stop(cmd4);
      ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_RATE_MS);
      i2c_cmd_link_delete(cmd4);

      if(ret == ESP_OK) {
      }
      vTaskDelay(50 / portTICK_RATE_MS);

    }
}

void app_main(void)
{
    i2c_example_master_init();
    i2c_scanner();

    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0));
    esp_vfs_dev_uart_use_driver(UART_NUM_0);    
    
    //Configure button
    gpio_reset_pin(GPIO32);
    gpio_set_direction(GPIO32, GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO32, GPIO_INTR_POSEDGE);
    
    printf("Button configured\n");

    gpio_reset_pin(GPIO12);
    gpio_set_direction(GPIO12, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO12, 1);

    gpio_intr_enable(GPIO32);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    gpio_isr_handler_add(GPIO32, gpio_isr_handler, (void*) GPIO32);

    xTaskCreate(led_driver,"led_driver", 4096, NULL, 4, NULL);
    xTaskCreate(test_alpha_display,"test_alpha_display", 4096, NULL, 5, NULL);
}


