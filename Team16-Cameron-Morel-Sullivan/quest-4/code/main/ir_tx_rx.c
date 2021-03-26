/*
  Implements IR transmitter and transceiver functions. Code adapted from EC444 
  traffic-light-ir-example project.
  --> https://github.com/BU-EC444/code-examples/tree/master/traffic-light-ir-example

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
#include "ir_tx_rx.h"
#include <stdio.h>
#include <string.h>



// System tags
const char *TAG_SYSTEM = "system";  // For debug logs



// Checksum
char genCheckSum(char *p, int len)
{
  char temp = 0;
  for (int i = 0; i < len; i++)
  {
    temp = temp^p[i];
  }
  // printf("%X\n",temp);

  return temp;
}



bool checkCheckSum(uint8_t *p, int len)
{
  char temp = (char) 0;
  bool isValid;
  for (int i = 0; i < len-1; i++)
  {
    temp = temp^p[i];
  }
  // printf("Check: %02X ", temp);
  if (temp == p[len-1])
  {
    isValid = true;
  }
  else
  {
    isValid = false;
  }
  return isValid;
}



// RMT tx init
void rmt_tx_init()
{
  rmt_config_t rmt_tx;
  rmt_tx.channel = RMT_TX_CHANNEL;
  rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
  rmt_tx.mem_block_num = 1;
  rmt_tx.clk_div = RMT_CLK_DIV;
  rmt_tx.tx_config.loop_en = false;
  rmt_tx.tx_config.carrier_duty_percent = 50;

  // Carrier Frequency of the IR receiver
  rmt_tx.tx_config.carrier_freq_hz = 38000;
  rmt_tx.tx_config.carrier_level = 1;
  rmt_tx.tx_config.carrier_en = 1;

  // Never idle -> aka ontinuous TX of 38kHz pulses
  rmt_tx.tx_config.idle_level = 1;
  rmt_tx.tx_config.idle_output_en = true;
  rmt_tx.rmt_mode = 0;
  rmt_config(&rmt_tx);
  rmt_driver_install(rmt_tx.channel, 0, 0);
}



// Configure UART
void uart_init()
{
  // Basic configs
  uart_config_t uart_config = {
    .baud_rate = 1200, // Slow BAUD rate
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  uart_param_config(UART_NUM_1, &uart_config);

  // Set UART pins using UART0 default pins
  uart_set_pin(UART_NUM_1, UART_TX_GPIO_NUM, UART_RX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  // Reverse receive logic line
  uart_set_line_inverse(UART_NUM_1,UART_SIGNAL_RXD_INV);

  // Install UART driver
  uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
}



// GPIO init for LEDs
void led_init()
{
  gpio_pad_select_gpio(BLUEPIN);
  gpio_pad_select_gpio(GREENPIN);
  gpio_pad_select_gpio(REDPIN);
  gpio_pad_select_gpio(ONBOARD);
  gpio_set_direction(BLUEPIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(GREENPIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(REDPIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(ONBOARD, GPIO_MODE_OUTPUT);
}

