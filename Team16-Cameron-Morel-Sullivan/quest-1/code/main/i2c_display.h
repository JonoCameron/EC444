/*
  Header for I2C Display functions.

  DJ Morel, Sept 2020
  Emily Lam, Sept 2018, Updated 2019
*/
#ifndef I2CDISPLAY_H
#define I2CDISPLAY_H


#include <stdio.h>
#include "driver/i2c.h"

// 14-Segment Display
#define SLAVE_ADDR                         0x70  // alphanumeric address
#define OSC                                0x21  // oscillator cmd
#define HT16K33_BLINK_DISPLAYON            0x01  // Display on cmd
#define HT16K33_BLINK_OFF                  0     // Blink off cmd
#define HT16K33_BLINK_CMD                  0x80  // Blink cmd
#define HT16K33_CMD_BRIGHTNESS             0xE0  // Brightness cmd

// Master I2C
#define I2C_EXAMPLE_MASTER_SCL_IO          22                // gpio number for i2c clk
#define I2C_EXAMPLE_MASTER_SDA_IO          23                // gpio number for i2c data
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_0         // i2c port
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0                 // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0                 // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000            // i2c master clock freq
#define WRITE_BIT                          I2C_MASTER_WRITE  // i2c master write
#define READ_BIT                           I2C_MASTER_READ   // i2c master read
#define ACK_CHECK_EN                       true              // i2c master will check ack
#define ACK_CHECK_DIS                      false             // i2c master will not check ack
#define ACK_VAL                            0x00              // i2c ack value
#define NACK_VAL                           0xFF              // i2c nack value


/**
  Function to initiate I2C. Note: Uses MSB declaration.
  \param None
  \return None
**/
void i2c_example_master_init();



// Utility functions //////////////////////////////////////////////////////////

/**
  Utility function to test for I2C device address -- not used in deploy.
  \param uint8_t devAddr --> Address to search for the I2C device.
  \param int32_t timeout --> How long to wait to receive a response.
  \return ESP_OK if successfully found an I2C device, or another int value if not.
**/
int testConnection(uint8_t devAddr, int32_t timeout);

/**
  Utility function to scan for I2C device.
  \param None
  \return None
**/
void i2c_scanner();

///////////////////////////////////////////////////////////////////////////////



// Alphanumeric Functions /////////////////////////////////////////////////////

/**
  Turn on oscillator for alpha display.
  \param None
  \return ESP_OK if successfully turned on oscillator (value comes from i2c_master_cmd_begin() call).
**/
int alpha_oscillator();

/**
  Set blink rate to off.
  \param None
  \return ESP_OK if successfully disabled blink rate (value comes from i2c_master_cmd_begin() call).
**/
int no_blink();

/**
  Set Brightness.
  \param uint8_t val --> Value to bitwise OR with HT16K33_CMD_BRIGHTNESS.
  \return ESP_OK if successfully set brightness to max (value comes from i2c_master_cmd_begin() call).
**/
int set_brightness_max(uint8_t val);

///////////////////////////////////////////////////////////////////////////////



/**
  Initializes the I2C display by configuring the alpha oscillator, display blink, and display brightness.
  \param None
  \return ESP_OK on success.
**/
int initialize_I2C();

/**
  Takes a character input, and determines its I2C 15 segment design using the ASCII Lookup Table.
  \param char input --> Character to get an I2C 15 segment pattern for.
  \return ASCII Lookup Table's index for the character, or -1 if index doesn't exist (i.e. invalid character).
**/
int getIndex(char input);

/**
  Sets the I2C display to show the current counting direction (UP or DOWN). Requires calling initialize_I2C() first!
  \param bool* countUp --> Pointer to the direction boolean (True to display UP, or False to display DOWN).
  \return None
**/
void displayDirection(bool* countUp);

/**
  Displays the passed count value on the I2C display in hours and minutes.
  \param unsigned int count --> Remaining time to display.
  \return None
**/
void displayCount_HoursMinutes(unsigned int* count);

/**
  Displays the passed count value on the I2C display in minutes and seconds.
  \param unsigned int count --> Remaining time to display.
  \return
**/
void displayCount_MinutesSeconds(unsigned int* count);

#endif
