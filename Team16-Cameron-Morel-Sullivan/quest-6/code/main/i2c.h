/*
  Header file to define I2C functions for accelerometer, LIDAR, and
  alphanumeric I2C display.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Dec. 2020
*/
#ifndef I2C_H
#define I2C_H

#include <stdio.h>
#include <math.h>
#include "driver/i2c.h"
#include "ADXL343.h"


// 14-Segment Display
#define ALPHANUM_ADDR                      0x70  // alphanumeric address
#define OSC                                0x21  // oscillator cmd
#define HT16K33_BLINK_DISPLAYON            0x01  // Display on cmd
#define HT16K33_BLINK_OFF                  0     // Blink off cmd
#define HT16K33_BLINK_CMD                  0x80  // Blink cmd
#define HT16K33_CMD_BRIGHTNESS             0xE0  // Brightness cmd

// LIDAR
#define LIDARV4_ADDRESS                    (0x62)
#define EN_ACQUISITION                     0x00

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



/**
  Function to initiate I2C. Note: Uses MSB declaration.
  \param None
  \return None
**/
void i2c_master_init();

// Utility functions //////////////////////////////////////////////////////////

/**
  Utility function to test for I2C device address -- not used in deploy.
  \param uint8_t devAddr --> Address to search for the I2C device.
  \param int32_t timeout --> How long to wait to receive a response.
  \return ESP_OK if successfully found an I2C device, or another int value if not.
**/
int testConnection(uint8_t devAddr, int32_t timeout);

/**
  Utility function to scan for I2C device. Calls testConnection().
  \param None
  \return None
**/
void i2c_scanner();

/**
  Returns the value of the 0x00 register, which is obsolete since the LIDAR doesn't have an inherent ID register.
  \param uint8_t *data --> Pointer to data.
  \param uint8_t devAddr --> Address of the I2C device.
  \return Value of the 0x00 register.
**/
int getDeviceID(uint8_t *data, uint8_t devAddr);

/**
  Writes one byte to a specified register belonging to a given I2C device.
  \param uint8_t reg --> Register to write to.
  \param uint8_t data --> Data to write.
  \param uint8_t devAddr --> Address of the I2C device.
  \return Status of the command link execution.
**/
int writeRegister(uint8_t reg, uint8_t data, uint8_t devAddr);

/**
  Reads a byte from a specified register belonging to a given I2C device.
  \param uint8_t reg --> Register to read from.
  \param uint8_t devAddr --> Address of the I2C device.
  \return Byte stored in the input argument register as an unsigned integer.
**/
uint8_t readRegister(uint8_t reg, uint8_t devAddr);

/**
  Primes the LLv4 so that it can take a measurement reading. Priming involves putting 0x04
  register 0x00 and waiting for the status register (0x01) to go be ready, indicated by the
  LSB turning to 0.
**/
int primeLLv4();

/**
  Reads 2 bytes (16 bits) from a specified register belonging to a given I2C device. Calls readRegister().
  \param uint8_t reg --> Register to read from.
  \param uint8_t devAddr --> Address of the I2C device.
  \return 2 byte value stored in the input argument register as a signed integer.
**/
int16_t read16(uint8_t reg, uint8_t devAddr);

///////////////////////////////////////////////////////////////////////////////


// LIDAR Functions ////////////////////////////////////////////////////////////

/**
  Measures the distance from the LLv4 by calling primeLLv4() and read16(0x10).
  \param None
  \return Distance as a 16-bit integer.
**/
int16_t distanceLLv4();

/**
  Continuously polls the LIDAR and outputs distance. Runs as a task. Calls
  readRegister(), writeRegister() and read16().
**/
void LLv4();

///////////////////////////////////////////////////////////////////////////////


// Accelerometer Functions ////////////////////////////////////////////////////

/**
  Reads from the accelerometer, and stores the tri-axis acceleration readings into the passed pointers.
  \param float* xp --> Data pointer for the x-axis acceleration.
  \param float* yp --> Data pointer for the y-axis acceleration.
  \param float* zp --> Data pointer for the z-axis acceleration.
  \return None
**/
void getAccel(float * xp, float *yp, float *zp);

/**
  Prints the accelerometer's roll and pitch values (in degrees).
  \param float x --> x-axis acceleration.
  \param float y --> y-axis acceleration.
  \param float z --> z-axis acceleration.
  \param float* roll --> Data pointer for the roll calculation.
  \param float* pitch --> Data pointer for the pitch calculation.
  \return None
**/
void calcRP(float x, float y, float z, float* roll, float* pitch);

/**
  Simple task that tests the accelerometer.
  \param None
  \return None
**/
void test_adxl343();

/**
  Gets the accelerometer's data range by reading from the ADXL343 data format register.
  \param None
  \return Accelerometer's data format range.
**/
range_t getRange(void);

/**
  Gets the accelerometer's data rate by reading from the ADXL343 bandwidth rate register.
  \param None
  \return Accelerometer's data rate.
**/
dataRate_t getDataRate(void);

/**
  Sets the data range for the accelerometer by writing to the ADXL343 data format register.
  \param range_t range --> Range to use.
  \return None
**/
void setRange(range_t range);

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
  \param unsigned int* count --> Pointer to the remaining time to display.
  \return None
**/
void displayCount_HoursMinutes(unsigned int* count);

/**
  Displays the passed count value on the I2C display in minutes and seconds.
  \param unsigned int* count --> Pointer to the remaining time to display.
  \return None
**/
void displayCount_MinutesSeconds(unsigned int* count);

/**
  Displays the passed distance value on the I2C display as a absolute value in centimeters.
  \param double* distance --> Pointer to the distance value (centimeters) to display.
  \return None
**/
void displayDistance(double* distance);

/**
  Displays the left and right wheel's tick counters.
  \param uint32_t* tickCount_L --> Pointer to the left wheel's tick counter.
  \param uint32_t* tickCount_R --> Pointer to the right wheel's tick counter.
  \return None
**/
void displayTickCounts(uint32_t* tickCount_L, uint32_t* tickCount_R);

/**
  Displays the current wheel speed on the I2C display as m/s.
  \param double* speed --> Pointer to the current speed value.
  \return None
**/
void displaySpeed(double* speed);

///////////////////////////////////////////////////////////////////////////////

#endif
