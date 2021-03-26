/*
    Adapted I2C example code to work with the Adafruit ADXL343 accelerometer. Ported and referenced a lot of code from the Adafruit_ADXL343 driver code.
  ----> https://www.adafruit.com/product/4097
  Emily Lam, Aug 2019 for BU EC444

  Adapted to work with Garmin LIDAR-Lite v4 LED LIDAR.
  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/

#include "./lidar.h"

// Function to initiate i2c -- note the MSB declaration!
void i2c_master_init_lidar(){
  // Debug
  printf("\n>> i2c Config\n");
  int err;

  // Port configuration
  int i2c_master_port = I2C_LIDAR_MASTER_NUM;

  /// Define I2C configurations
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;                              // Master mode
  conf.sda_io_num = I2C_LIDAR_MASTER_SDA_IO;                // Default SDA pin
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
  conf.scl_io_num = I2C_LIDAR_MASTER_SCL_IO;                // Default SCL pin
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
  conf.master.clk_speed = I2C_LIDAR_MASTER_FREQ_HZ;       // CLK frequency
  err = i2c_param_config(i2c_master_port, &conf);           // Configure
  if (err == ESP_OK) {printf("- parameters: ok\n");}

  // Install I2C driver
  err = i2c_driver_install(i2c_master_port, conf.mode,
                     I2C_LIDAR_MASTER_RX_BUF_DISABLE,
                     I2C_LIDAR_MASTER_TX_BUF_DISABLE, 0);
  if (err == ESP_OK) {printf("- initialized: yes\n");}

  // Data in MSB mode
  i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}


int testConnection_lidar(uint8_t devAddr, int32_t timeout) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  int err = i2c_master_cmd_begin(I2C_LIDAR_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return err;
}

// Utility function to scan for i2c device
void i2c_scanner_lidar() {
  int32_t scanTimeout = 1000;
  printf("\n>> I2C scanning ..."  "\n");
  uint8_t count = 0;
  for (uint8_t i = 1; i < 127; i++) {
    // printf("0x%X%s",i,"\n");
    if (testConnection_lidar(i, scanTimeout) == ESP_OK) {
      printf( "- Device found at address: 0x%X%s", i, "\n");
      count++;
    }
  }
  if (count == 0) {printf("- No I2C devices found!" "\n");}
}


// Get Device ID
int getDeviceID(uint8_t *data) {
  int ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR_LIDAR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR_LIDAR << 1 ) | READ_BIT, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, data, ACK_CHECK_DIS);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_LIDAR_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}


// Write one byte to register
int writeRegister(uint8_t reg, uint8_t data) {
  uint8_t* wr_data = (uint8_t*) malloc(sizeof(uint8_t));
  *wr_data = data;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, 0xC4 | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, *wr_data, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  //int ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
  int ret = i2c_master_cmd_begin(I2C_LIDAR_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  free(wr_data);
  return ret;
}


// Read register
uint8_t readRegister(uint8_t reg) {
  uint8_t *data = (uint8_t*) malloc(sizeof(uint8_t));
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, 0xC4 | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, 0xC4 | READ_BIT, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, data, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  //i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_master_cmd_begin(I2C_LIDAR_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  uint8_t regVal = *data;
  free(data);
  return regVal;
}

int primeLLv4() {
  uint8_t ready = 1;

  // Enable target acquisition
  writeRegister(EN_ACQUISITION, 0x04);

  while ( ( ready % 2 ) == 1 ){
    ready = readRegister(0x01);
  }

  return ready;
}

// read 16 bits (2 bytes)
int16_t read16(uint8_t reg) {
  uint8_t lsb = readRegister(reg);
  uint8_t msb = readRegister(reg + 1);
  
  int16_t distance = (msb << 8) | lsb;

  return distance;
}

int16_t distanceLLv4() {
  primeLLv4();
  int16_t dist = read16(0x10);

  if (dist < 0) {
    printf("error: distance measured as negative number\n");
    return -1;
  }
  return dist;
}

void LLv4() {
  printf("\n>> Polling Garmin LiDAR v4\n");
  
  while (1) {
    // uint8_t data;

    int16_t dist = distanceLLv4();
    printf("distance: %dcm\n", dist);
    

    // if(getDeviceID(&data) == 0){
    //   printf("Device ID retrieved\n");
    // }
    // printf("Device ID: %d\n", data);
    vTaskDelay(500 / portTICK_RATE_MS);
  }
}


