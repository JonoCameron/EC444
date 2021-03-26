/*
  Header file for IR transmitter/transceiver functions. Code adapted from 
  EC444 traffic-light-ir-example project.
  --> https://github.com/BU-EC444/code-examples/tree/master/traffic-light-ir-example

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
#ifndef IRTXRX_H
#define IRTXRX_H


#include "driver/rmt.h"
#include "soc/rmt_reg.h"
#include "driver/uart.h"
#include "driver/periph_ctrl.h"


// RMT definitions
#define RMT_TX_CHANNEL    1                              // RMT channel for transmitter
#define RMT_TX_GPIO_NUM   25                             // GPIO number for transmitter signal -- A1
#define RMT_CLK_DIV       100                            // RMT counter clock divider
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)  // RMT counter value for 10 us.(Source clock is APB clock)
#define rmt_item32_tIMEOUT_US   9500                     // RMT receiver timeout value(us)

// UART definitions
#define UART_TX_GPIO_NUM 26  // A0
#define UART_RX_GPIO_NUM 34  // A2
#define BUF_SIZE (1024)

// LED Output pins definitions
#define BLUEPIN   14
#define GREENPIN  32
#define REDPIN    15
#define ONBOARD   13


// System tags
const char *TAG_SYSTEM;  // For debug logs


// Utility Functions ///////////////////////////////////////////////////////////

/**
  Generates checksum byte for UART transmission.
  \param char* p --> Pointer to UART output data (characters).
  \param int len --> Length of UART output data (in bytes).
  \return Character (byte) representing the checksum calculation.
**/
char genCheckSum(char* p, int len);

/**
  Checks the checksum byte from UART transmission.
  \param uint8_t* p --> Pointer to UART input data (bytes/characters).
  \param int len --> Length of UART input data (in bytes).
  \return True if computed checksum matches the passed UART checksum byte, or False if not (corrupt transmission).
**/
bool checkCheckSum(uint8_t* p, int len);

////////////////////////////////////////////////////////////////////////////////

// Init Functions //////////////////////////////////////////////////////////////

/**
  Initializes RMT tx.
  \param None
  \return None
**/
void rmt_tx_init();

/**
  Configures UART.
  \param None
  \return None
**/
void uart_init();

/**
  Initializes LED GPIO.
  \param None
  \return None
**/
void led_init();

////////////////////////////////////////////////////////////////////////////////


#endif

