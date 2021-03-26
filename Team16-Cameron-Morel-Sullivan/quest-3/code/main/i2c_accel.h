/*
  Header file for ADXL343 accelerometer I2C functions. Adapted from EC444 
  i2c-accel example project.  

  DJ Morel, Oct 2020
  Emily Lam, Aug 2019 for BU EC444
*/
#ifndef I2CACCEL_H
#define I2CACCEL_H


#include "driver/i2c.h"
#include "./ADXL343.h"


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

// ADXL343
#define SLAVE_ADDR                         ADXL343_ADDRESS // 0x53



/**
  Initializes the I2C accelerometer. Calls all necessary functions to intialize I2C and accelerometer.
  \param None
  \return None
**/
void initialize_accelerometer();

/**
  Function to initiate I2C. Uses MSB declaration.
  \param None
  \return None
**/
void i2c_master_init();


// Utility  Functions //////////////////////////////////////////////////////////

/**
  Utility function to test for I2C device address. Not used in deploy.
  \param uint8_t devAddr --> I2C device address.
  \param int32_t timeout --> Timeout to cancel the test connection (not used).
  \return Status of the command link execution.
**/
int testConnection(uint8_t devAddr, int32_t timeout);

/**
  Utility function to scan for I2C device.
  \param None
  \return None
**/
void i2c_scanner();

////////////////////////////////////////////////////////////////////////////////


// ADXL343 Functions ///////////////////////////////////////////////////////////

/**
  Gets the device ID.
  \param uint8_t* data --> Location to store the device ID.
  \return Status of the command link execution.
**/
int getDeviceID(uint8_t* data);

/**
  Writes one byte to a specified register.
  \param uint8_t reg --> Register to write to.
  \param uint8_t data --> Data to write.
  \return Status of the command link execution.
**/
int writeRegister(uint8_t reg, uint8_t data);

/**
  Reads a byte from a specified register.
  \param uint8_t reg --> Register to read from.
  \return Byte stored in the input argument register as an unsigned integer.
**/
uint8_t readRegister(uint8_t reg);

/**
  Reads 2 bytes (16 bits) from a specified register.
  \param uint8_t reg --> Register to read from.
  \return 2 byte value stored in the input argument register as a signed integer.
**/
int16_t read16(uint8_t reg);

/**
  Sets the data range for the accelerometer by writing to the ADXL343 data format register.
  \param range_t range --> Range to use.
  \return None
**/
void setRange(range_t range);

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

////////////////////////////////////////////////////////////////////////////////


/**
  Reads from the accelerometer, and stores the tri-axis acceleration readings into the passed pointers.
  \param float* xp --> Data pointer for the x-axis acceleration.
  \param float* yp --> Data pointer for the y-axis acceleration.
  \param float* zp --> Data pointer for the z-axis acceleration.
  \return None
**/
void getAccel(float* xp, float* yp, float* zp);

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



#endif


