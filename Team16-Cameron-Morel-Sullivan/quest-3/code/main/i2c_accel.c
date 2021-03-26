/*
  Implements ADXL343 accelerometor I2C functions. Adapted from EC444 i2c-accel 
  example project.

  EC444 example code originally adapted I2C example code to work with the 
  Adafruit ADXL343 accelerometer. Ported and referenced a lot of code from the 
  Adafruit_ADXL343 driver code.  
    ----> https://www.adafruit.com/product/4097  

  DJ Morel, Oct 2020
  Emily Lam, Aug 2019 for BU EC444
*/
#include <stdio.h>
#include <math.h>
#include "i2c_accel.h"



void initialize_accelerometer()
{
  // Routine
  i2c_master_init();
  i2c_scanner();

  // Check for ADXL343
  uint8_t deviceID;
  getDeviceID(&deviceID);
  if (deviceID == 0xE5)
  {
    printf("\n>> Found ADXL343\n");
  }

  // Disable interrupts
  writeRegister(ADXL343_REG_INT_ENABLE, 0);

  // Set range
  setRange(ADXL343_RANGE_16_G);
  // Display range
  printf  ("- Range:         +/- ");
  switch(getRange())
  {
    case ADXL343_RANGE_16_G:
      printf  ("16 ");
      break;
    case ADXL343_RANGE_8_G:
      printf  ("8 ");
      break;
    case ADXL343_RANGE_4_G:
      printf  ("4 ");
      break;
    case ADXL343_RANGE_2_G:
      printf  ("2 ");
      break;
    default:
      printf  ("?? ");
      break;
  }
  printf(" g\n");

  // Display data rate
  printf ("- Data Rate:    ");
  switch(getDataRate())
  {
    case ADXL343_DATARATE_3200_HZ:
      printf  ("3200 ");
      break;
    case ADXL343_DATARATE_1600_HZ:
      printf  ("1600 ");
      break;
    case ADXL343_DATARATE_800_HZ:
      printf  ("800 ");
      break;
    case ADXL343_DATARATE_400_HZ:
      printf  ("400 ");
      break;
    case ADXL343_DATARATE_200_HZ:
      printf  ("200 ");
      break;
    case ADXL343_DATARATE_100_HZ:
      printf  ("100 ");
      break;
    case ADXL343_DATARATE_50_HZ:
      printf  ("50 ");
      break;
    case ADXL343_DATARATE_25_HZ:
      printf  ("25 ");
      break;
    case ADXL343_DATARATE_12_5_HZ:
      printf  ("12.5 ");
      break;
    case ADXL343_DATARATE_6_25HZ:
      printf  ("6.25 ");
      break;
    case ADXL343_DATARATE_3_13_HZ:
      printf  ("3.13 ");
      break;
    case ADXL343_DATARATE_1_56_HZ:
      printf  ("1.56 ");
      break;
    case ADXL343_DATARATE_0_78_HZ:
      printf  ("0.78 ");
      break;
    case ADXL343_DATARATE_0_39_HZ:
      printf  ("0.39 ");
      break;
    case ADXL343_DATARATE_0_20_HZ:
      printf  ("0.20 ");
      break;
    case ADXL343_DATARATE_0_10_HZ:
      printf  ("0.10 ");
      break;
    default:
      printf  ("???? ");
      break;
  }
  printf(" Hz\n\n");

  // Enable measurements
  writeRegister(ADXL343_REG_POWER_CTL, 0x08);
}



void i2c_master_init()
{
  // Debug
  printf("\n>> i2c Config\n");
  int err;

  // Port configuration
  int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;

  // Define I2C configurations
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;                              // Master mode
  conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;              // Default SDA pin
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
  conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;              // Default SCL pin
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
  conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;       // CLK frequency
  err = i2c_param_config(i2c_master_port, &conf);           // Configure
  if (err == ESP_OK)
  {
    printf("- parameters: ok\n");
  }

  // Install I2C driver
  err = i2c_driver_install(i2c_master_port, conf.mode,
                           I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                           I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
  if (err == ESP_OK)
  {
    printf("- initialized: yes\n");
  }

  // Data in MSB mode
  i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}



// Utility  Functions //////////////////////////////////////////////////////////

int testConnection(uint8_t devAddr, int32_t timeout)
{
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return err;
}



void i2c_scanner()
{
  int32_t scanTimeout = 1000;
  printf("\n>> I2C scanning ..."  "\n");
  uint8_t count = 0;
  for (uint8_t i = 1; i < 127; i++)
  {
    // printf("0x%X%s",i,"\n");
    if (testConnection(i, scanTimeout) == ESP_OK)
    {
      printf( "- Device found at address: 0x%X%s", i, "\n");
      count++;
    }
  }
  if (count == 0)
  {
    printf("- No I2C devices found!" "\n");
  }
}



////////////////////////////////////////////////////////////////////////////////

// ADXL343 Functions ///////////////////////////////////////////////////////////

int getDeviceID(uint8_t* data)
{
  int ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, ADXL343_REG_DEVID, ACK_CHECK_EN);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | READ_BIT, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, data, ACK_CHECK_DIS);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}



int writeRegister(uint8_t reg, uint8_t data)
{
  // Create a command link to eventually populate it with a series of data to be sent to the slave
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  // Start bit
  i2c_master_start(cmd);

  // Send the slave address (indicating write) and register to be written
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);

  // Send register to write to
  i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);

  // Send the data to write
  i2c_master_write_byte(cmd, data, ACK_CHECK_EN);

  // Stop bit
  i2c_master_stop(cmd);

  // Trigger the execution of the command link by the I2C controller
  int ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);

  // Commands are transmitted, so release the resource used by the command link
  i2c_cmd_link_delete(cmd);
  
  // Return the result of the command link
  return ret;
}



uint8_t readRegister(uint8_t reg)
{
  // Allocate memory for the data pointer
  uint8_t* data_ptr = malloc(sizeof(uint8_t));

  // Create a command link (will populate it with data to be sent to the slave)
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  // Start bit
  i2c_master_start(cmd);

  // Send the slave address (indiciating write)
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);

  // Send the register to read from
  i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);

  // Send a repeated start bit
  i2c_master_start(cmd);

  // Send the slave address (indicating read)
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | READ_BIT, ACK_CHECK_EN);

  // Send the location to read the data into
  i2c_master_read_byte(cmd, data_ptr, ACK_CHECK_DIS);

  // Stop bit
  i2c_master_stop(cmd);

  // Trigger the execution of the command link by the I2C controller
  int ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);

  // Commands are transmitted, so release the resource used by the command link
  i2c_cmd_link_delete(cmd);

  // Dereference the data pointer, and free the allocated memory
  uint8_t data = *data_ptr;
  free(data_ptr);

  // Return the register's data
  return data;
}



int16_t read16(uint8_t reg)
{
  // Initialize return value to 0
  int16_t data = 0;

  // Read the first byte (LSB)
  uint16_t lsb = (uint16_t) readRegister(reg);

  // Read the second byte (MSB)
  uint16_t msb = (uint16_t) readRegister(reg + sizeof(uint8_t));

  // Left shift the MSB by 8
  msb = msb << 8;

  // OR the LSB and MSB into data
  data = (int16_t) (msb | lsb);

  return data;
}



void setRange(range_t range)
{
  // Read the data format register to preserve bits
  uint8_t format = readRegister(ADXL343_REG_DATA_FORMAT);

  // Update the data rate
  format &= ~0x0F;
  format |= range;

  // Make sure that the FULL-RES bit is enabled for range scaling
  format |= 0x08;

  // Write the register back to the IC
  writeRegister(ADXL343_REG_DATA_FORMAT, format);
}



range_t getRange(void)
{
  // Read the data format register to preserve bits
  return (range_t)(readRegister(ADXL343_REG_DATA_FORMAT) & 0x03);
}



dataRate_t getDataRate(void)
{
  return (dataRate_t)(readRegister(ADXL343_REG_BW_RATE) & 0x0F);
}

////////////////////////////////////////////////////////////////////////////////



void getAccel(float* xp, float* yp, float* zp)
{
  *xp = read16(ADXL343_REG_DATAX0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
  *yp = read16(ADXL343_REG_DATAY0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
  *zp = read16(ADXL343_REG_DATAZ0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
  //printf("X: %.2f \t Y: %.2f \t Z: %.2f\n", *xp, *yp, *zp);
}



void calcRP(float x, float y, float z, float* roll, float* pitch)
{
  // Calculate the roll (radians) and convert to degrees
  *roll = atan2(y, z) * 57.3;

  // Calculate the pitch (radians) and convert to degrees
  *pitch = atan2((-x), sqrt(y*y + z*z)) * 57.3;

  //printf("roll: %.2f \t pitch: %.2f \n", *roll, *pitch);
}


