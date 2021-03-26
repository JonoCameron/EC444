#ifndef I2C_H
#define I2C_H

#include <stdio.h>
#include <math.h>
#include "driver/i2c.h"
#include "ADXL343.h"

#define LIDARV4_ADDRESS                 (0xC4)

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

// LIDAR
#define SLAVE_ADDR                         LIDARV4_ADDRESS // 0x62
#define EN_ACQUISITION                     0x00


// Initiate I2C 
void i2c_master_init();

//Test connection to determine which address LIDAR is on
int testConnection(uint8_t devAddr, int32_t timeout);

// Scans through I2C addresses to find LIDAR. Calls testConnection()
void i2c_scanner();

// Returns the value of the 0x00 register, which is obsolete since the LIDAR 
// doesn't have an inherent ID register
int getDeviceID(uint8_t *data, uint8_t devAddr);

// Writes a single byte, data to register, reg
int writeRegister(uint8_t reg, uint8_t data, uint8_t devAddr);

// Reads single byte from register and returns as uint8_t
uint8_t readRegister(uint8_t reg, uint8_t devAddr);

// Primes the LLv4 so that it can take a measurement reading. Priming involves putting 0x04
// register 0x00 and waiting for the status register (0x01) to go be ready, indicated by the
// LSB turning to 0.
int primeLLv4();

// Reads a bit from one register, reg, then reads the next register, reg + 1,
// and returns the result as a uint16_t. Used for reading distance from LIDAR.
// Calls readRegister().
int16_t read16(uint8_t reg, uint8_t devAddr);

// Take the distance from the LLv4 by calling primeLLv4() and read16(0x10).
int16_t distanceLLv4();

// Continuously polls the LIDAR and outputs distance. Runs as a task. Calls
// readRegister(), writeRegister() and read16().
void LLv4();

void getAccel(float * xp, float *yp, float *zp);

void calcRP(float x, float y, float z);

void test_adxl343();

range_t getRange(void);

dataRate_t getDataRate(void);

void setRange(range_t range);
#endif