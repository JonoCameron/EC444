/*
  Adapted I2C example code to work with the Adafruit ADXL343 accelerometer. 
  Ported and referenced a lot of code from the Adafruit_ADXL343 driver code.
  --> https://www.adafruit.com/product/4097
  Emily Lam, Aug 2019 for BU EC444

  Adapted to work with Garmin LIDAR-Lite v4 LED LIDAR and alphanumeric I2C display.
  Jonathan Cameron, DJ Morel, Ryan Sullivan, Dec. 2020
*/
#include "i2c.h"



// ASCII Lookup Table
// 15-segment in order of: 0 DP N M L K J H G2 G1 F E D C B A
// Note that the MSB (0) bit doesn't matter since the I2C display only has 15 segments, rather than 16
uint16_t ASCIILookupTable[] = { 0b0000000000000000,  /*SPACE*/
                                0b0100000000000110,  /*  !  */
                                0b0000001000000010,  /*  "  */
                                0b0001001011001110,  /*  #  */
                                0b0001001011101101,  /*  $  */
                                0b0011111111100100,  /*  %  */
                                0b0010001101011001,  /*  &  */
                                0b0000001000000000,  /*  '  */
                                0b0010010000000000,  /*  (  */
                                0b0000100100000000,  /*  )  */
                                0b0011111111000000,  /*  *  */
                                0b0001001011000000,  /*  +  */
                                0b0000100000000000,  /*  ,  */
                                0b0000000011000000,  /*  -  */
                                0b0100000000000000,  /*  .  */
                                0b0000110000000000,  /*  /  */
                                0b0000110000111111,  /*  0  */
                                0b0000010000000110,  /*  1  */
                                0b0000000011011011,  /*  2  */
                                0b0000000010001111,  /*  3  */
                                0b0000000011100110,  /*  4  */
                                0b0010000001101001,  /*  5  */
                                0b0000000011111101,  /*  6  */
                                0b0000000000000111,  /*  7  */
                                0b0000000011111111,  /*  8  */
                                0b0000000011101111,  /*  9  */
                                0b0001001000000000,  /*  :  */
                                0b0000101000000000,  /*  ;  */
                                0b0010010001000000,  /*  <  */
                                0b0000000011001000,  /*  =  */
                                0b0000100110000000,  /*  >  */
                                0b0101000010000011,  /*  ?  */
                                0b0000001010111011,  /*  @  */
                                0b0000000011110111,  /*  A  */
                                0b0001001010001111,  /*  B  */
                                0b0000000000111001,  /*  C  */
                                0b0001001000001111,  /*  D  */
                                0b0000000001111001,  /*  E  */
                                0b0000000001110001,  /*  F  */
                                0b0000000010111101,  /*  G  */
                                0b0000000011110110,  /*  H  */
                                0b0001001000001001,  /*  I  */
                                0b0000000000011110,  /*  J  */
                                0b0010010001110000,  /*  K  */
                                0b0000000000111000,  /*  L  */
                                0b0000010100110110,  /*  M  */
                                0b0010000100110110,  /*  N  */
                                0b0000000000111111,  /*  O  */
                                0b0000000011110011,  /*  P  */
                                0b0010000000111111,  /*  Q  */
                                0b0010000011110011,  /*  R  */
                                0b0000000011101101,  /*  S  */
                                0b0001001000000001,  /*  T  */
                                0b0000000000111110,  /*  U  */
                                0b0000110000110000,  /*  V  */
                                0b0010100000110110,  /*  W  */
                                0b0010110100000000,  /*  X  */
                                0b0001000011100010,  /*  Y  */
                                0b0000110000001001,  /*  Z  */
                                0b0000000000111001,  /*  [  */
                                0b0010000100000000,  /*  \  */
                                0b0000000000001111,  /*  ]  */
                                0b0010100000000000,  /*  ^  */
                                0b0000000000001000,  /*  _  */
                                0b0000000100000000,  /*  `  */
                                0b0001000001011000,  /*  a  */
                                0b0010000001111000,  /*  b  */
                                0b0000000011011000,  /*  c  */
                                0b0000100010001110,  /*  d  */
                                0b0000100001011000,  /*  e  */
                                0b0001010011000000,  /*  f  */
                                0b0000010010001110,  /*  g  */
                                0b0001000001110000,  /*  h  */
                                0b0001000000000000,  /*  i  */
                                0b0000101000010000,  /*  j  */
                                0b0011011000000000,  /*  k  */
                                0b0000000000110000,  /*  l  */
                                0b0001000011010100,  /*  m  */
                                0b0001000001010000,  /*  n  */
                                0b0000000011011100,  /*  o  */
                                0b0000000101110000,  /*  p  */
                                0b0000010010000110,  /*  q  */
                                0b0000000001010000,  /*  r  */
                                0b0010000010001000,  /*  s  */
                                0b0000000001111000,  /*  t  */
                                0b0000000000011100,  /*  u  */
                                0b0000100000010000,  /*  v  */
                                0b0010100000010100,  /*  w  */
                                0b0010110100000000,  /*  x  */
                                0b0000001010001110,  /*  y  */
                                0b0000100001001000,  /*  z  */
                                0b0000100101001001,  /*  {  */
                                0b0001001000000000,  /*  |  */
                                0b0010010010001001,  /*  }  */
                                0b0000110011000000   /*  ~  */
                              };



// Function to initiate i2c -- note the MSB declaration!
void i2c_master_init(){
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
  if (err == ESP_OK) {printf("- initialized: yes\n");}

  // Data in MSB mode
  i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);

  // Disable interrupts
  writeRegister(ADXL343_REG_INT_ENABLE, 0, ADXL343_ADDRESS);

  // Set range
  setRange(ADXL343_RANGE_16_G);
  // Display range
  printf  ("- Range:         +/- ");
  switch(getRange()) {
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
  switch(getDataRate()) {
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
  writeRegister(ADXL343_REG_POWER_CTL, 0x08, ADXL343_ADDRESS);
}


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
void i2c_scanner() {
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
  if (count == 0) {printf("- No I2C devices found!" "\n");}
}


// Get Device ID
int getDeviceID(uint8_t *data, uint8_t devAddr) {
  int ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( devAddr << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( devAddr << 1 ) | READ_BIT, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, data, ACK_CHECK_DIS);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}


// Write one byte to register
int writeRegister(uint8_t reg, uint8_t data, uint8_t devAddr) {
  uint8_t* wr_data = (uint8_t*) malloc(sizeof(uint8_t));
  *wr_data = data;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( devAddr << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, *wr_data, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  int ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  free(wr_data);
  return ret;
}


// Read register
uint8_t readRegister(uint8_t reg, uint8_t devAddr) {
  uint8_t *data = (uint8_t*) malloc(sizeof(uint8_t));
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( devAddr << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( devAddr << 1 ) | READ_BIT, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, data, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  uint8_t regVal = *data;
  free(data);
  return regVal;
}

int primeLLv4() {
  uint8_t ready = 1;

  // Enable target acquisition
  writeRegister(EN_ACQUISITION, 0x04, LIDARV4_ADDRESS);

  while ( ( ready % 2 ) == 1 ){
    ready = readRegister(0x01, LIDARV4_ADDRESS);
  }

  return ready;
}

// read 16 bits (2 bytes)
int16_t read16(uint8_t reg, uint8_t devAddr) {
  uint8_t lsb = readRegister(reg, devAddr);
  uint8_t msb = readRegister(reg + 1, devAddr);
  
  int16_t distance = (msb << 8) | lsb;

  return distance;
}

int16_t distanceLLv4() {
  primeLLv4();
  int16_t dist = read16(0x10, LIDARV4_ADDRESS);

  if (dist < 0) {
    //printf("error: distance measured as negative number\n");
    return -1;
  }

  return dist;
}

void LLv4() {
  printf("\n>> Polling Garmin LiDAR v4\n");
  int count = 0;
  while (1) {

    int16_t dist = distanceLLv4();
    count++;
    printf("count: %d\t\tdistance: %dcm\n", count, dist);

    vTaskDelay(500 / portTICK_RATE_MS);
  }
}

// function to get acceleration
void getAccel(float * xp, float *yp, float *zp) {
  *xp = read16(ADXL343_REG_DATAX0, ADXL343_ADDRESS) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
  *yp = read16(ADXL343_REG_DATAY0, ADXL343_ADDRESS) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
  *zp = read16(ADXL343_REG_DATAZ0, ADXL343_ADDRESS) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
}

// function to print roll and pitch
void calcRP(float x, float y, float z, float* roll, float* pitch){
  // Calculate the roll (radians) and convert to degrees
  *roll = atan2(y, z) * 57.3;

  // Calculate the pitch (radians) and convert to degrees
  *pitch = atan2(-x, sqrt(y*y + z*z)) * 57.3;
}

// Task to continuously poll acceleration and calculate roll and pitch
void test_adxl343() {
  printf("\n>> Polling ADAXL343\n");
  while (1) {
    float xVal, yVal, zVal;
    float roll, pitch;
    getAccel(&xVal, &yVal, &zVal);
    calcRP(xVal, yVal, zVal, &roll, &pitch);
    uint8_t data;
    
    if(getDeviceID(&data, 0xE5) == 0){
      printf("Device ID retrieved\n");
    }
    printf("Device ID: %d\n", data);
    vTaskDelay(500 / portTICK_RATE_MS);
  }
}

range_t getRange(void) {
  /* Read the data format register to preserve bits */
  return (range_t)(readRegister(ADXL343_REG_DATA_FORMAT, ADXL343_ADDRESS) & 0x03);
}

dataRate_t getDataRate(void) {
  return (dataRate_t)(readRegister(ADXL343_REG_BW_RATE, ADXL343_ADDRESS) & 0x0F);
}

void setRange(range_t range) {
  /* Read the data format register to preserve bits */
  uint8_t format = readRegister(ADXL343_REG_DATA_FORMAT, ADXL343_ADDRESS);

  /* Update the data rate */
  format &= ~0x0F;
  format |= range;

  /* Make sure that the FULL-RES bit is enabled for range scaling */
  format |= 0x08;

  /* Write the register back to the IC */
  writeRegister(ADXL343_REG_DATA_FORMAT, format, ADXL343_ADDRESS);
}



// Turn on oscillator for alpha display
int alpha_oscillator()
{
  int ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( ALPHANUM_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, OSC, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  vTaskDelay(200 / portTICK_RATE_MS);
  return ret;
}


// Set blink rate to off
int no_blink()
{
  int ret;
  i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
  i2c_master_start(cmd2);
  i2c_master_write_byte(cmd2, ( ALPHANUM_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd2, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (HT16K33_BLINK_OFF << 1), ACK_CHECK_EN);
  i2c_master_stop(cmd2);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd2, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd2);
  vTaskDelay(200 / portTICK_RATE_MS);
  return ret;
}


// Set Brightness
int set_brightness_max(uint8_t val)
{
  int ret;
  i2c_cmd_handle_t cmd3 = i2c_cmd_link_create();
  i2c_master_start(cmd3);
  i2c_master_write_byte(cmd3, ( ALPHANUM_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd3, HT16K33_CMD_BRIGHTNESS | val, ACK_CHECK_EN);
  i2c_master_stop(cmd3);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd3, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd3);
  vTaskDelay(200 / portTICK_RATE_MS);
  return ret;
}



int initialize_I2C()
{
  // Debug
  int ret;

  // Set up routines
  // Turn on alpha oscillator
  ret = alpha_oscillator();
  if (ret == ESP_OK)
  {
    printf("- oscillator: ok \n");
  }

  // Set display blink off
  ret = no_blink();
  if (ret == ESP_OK)
  {
    printf("- blink: off \n");
  }

  // Set display brightness
  ret = set_brightness_max(0xF);
  if (ret == ESP_OK)
  {
    printf("- brightness: max \n");
  }

  return ret;
}



int getIndex(char input)
{
  // Check if the character exceeds the bounds of valid characters to display
  if (input < ' ' || input > '~')
  {
    return -1;
  }

  // We can represent the character, so convert the character to an int
  int index = (int)input;

  // Subtract 0x20 from the index (first element is SPACE, which is ASCII 0x20)
  index -= 0x20;

  // Return the ASCII Lookup Table index
  return index;
}



void displayDirection(bool* countUp)
{
  // Debugging
  int ret;

  // Write the 2 possible directions to the display buffer
  // Note that displaybuffer has 8 elements, the first 4 display " UP " and the last 4 display "DOWN"
  uint16_t displaybuffer[8];
  unsigned int buff_start = 0;  // Tracks the base of the display buffer to render (either 0 or 4)

  // First 4 elements are the front buffer
  displaybuffer[0] = ASCIILookupTable[getIndex(' ')];
  displaybuffer[1] = ASCIILookupTable[getIndex('U')];
  displaybuffer[2] = ASCIILookupTable[getIndex('P')];
  displaybuffer[3] = ASCIILookupTable[getIndex(' ')];

  // Last 4 elements are the back buffer by default (back buffer ALWAYS initialized to spaces)
  displaybuffer[4] = ASCIILookupTable[getIndex('D')];
  displaybuffer[5] = ASCIILookupTable[getIndex('O')];
  displaybuffer[6] = ASCIILookupTable[getIndex('W')];
  displaybuffer[7] = ASCIILookupTable[getIndex('N')];

  // Continually writes the same command
  while (1)
  {
    // Send commands characters to display over I2C
    i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
    i2c_master_start(cmd4);
    i2c_master_write_byte(cmd4, ( ALPHANUM_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);

    // Dereference the direction boolean pointer to determine what to display
    if (*countUp)
    {
      // We are counting UP
      buff_start = 0;
    }
    else
    {
      // We are counting DOWN
      buff_start = 4;
    }

    // Render the appropriate buffer
    for (uint8_t i = 0; i < 4; i++)
    {
      i2c_master_write_byte(cmd4, displaybuffer[buff_start + i] & 0xFF, ACK_CHECK_EN);
      i2c_master_write_byte(cmd4, displaybuffer[buff_start + i] >> 8, ACK_CHECK_EN);
    }

    i2c_master_stop(cmd4);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd4);

    if (ret == ESP_OK)
    {
      // Do something for debugging
    }
  }  // End of while loop

}



void displayCount_HoursMinutes(unsigned int* count)
{
  // Debugging
  int ret;

  // Write a default number (0) to the display buffer
  uint16_t displaybuffer[4];
  displaybuffer[0] = ASCIILookupTable[getIndex('0')];
  displaybuffer[1] = ASCIILookupTable[getIndex('0')];
  displaybuffer[2] = ASCIILookupTable[getIndex('0')];
  displaybuffer[3] = ASCIILookupTable[getIndex('0')];

  // Continually writes the current time
  while(1)
  {
    // Determine what number to display using some math
    displaybuffer[0] = ASCIILookupTable[getIndex('0') + ((*count / 36000) % 10)];
    displaybuffer[1] = ASCIILookupTable[getIndex('0') + ((*count / 3600 ) % 10)];
    displaybuffer[2] = ASCIILookupTable[getIndex('0') + ((*count / 600  ) % 6)];
    displaybuffer[3] = ASCIILookupTable[getIndex('0') + ((*count / 60   ) % 10)];

    // Send commands characters to display over I2C
    i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
    i2c_master_start(cmd4);
    i2c_master_write_byte(cmd4, ( ALPHANUM_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);

    // Render the buffer
    for (uint8_t i = 0; i < 4; i++)
    {
      i2c_master_write_byte(cmd4, displaybuffer[i] & 0xFF, ACK_CHECK_EN);
      i2c_master_write_byte(cmd4, displaybuffer[i] >> 8, ACK_CHECK_EN);
    }

    i2c_master_stop(cmd4);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd4);

    if (ret == ESP_OK)
    {
      // Do something for debugging
    }
  }  // End of while loop

}



void displayCount_MinutesSeconds(unsigned int* count)
{
  // Debugging
  int ret;

  // Write a default number (0) to the display buffer
  uint16_t displaybuffer[4];
  displaybuffer[0] = ASCIILookupTable[getIndex('0')];
  displaybuffer[1] = ASCIILookupTable[getIndex('0')];
  displaybuffer[2] = ASCIILookupTable[getIndex('0')];
  displaybuffer[3] = ASCIILookupTable[getIndex('0')];

  // Continually writes the current time
  while(1)
  {
    // Determine what number to display using some math
    displaybuffer[0] = ASCIILookupTable[getIndex('0') + ((*count / 600) % 10)];
    displaybuffer[1] = ASCIILookupTable[getIndex('0') + ((*count / 60 ) % 10)];
    displaybuffer[2] = ASCIILookupTable[getIndex('0') + ((*count / 10 ) %  6)];
    displaybuffer[3] = ASCIILookupTable[getIndex('0') +         (*count % 10)];

    // Send commands characters to display over I2C
    i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
    i2c_master_start(cmd4);
    i2c_master_write_byte(cmd4, ( ALPHANUM_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);

    // Render the buffer
    for (uint8_t i = 0; i < 4; i++)
    {
      i2c_master_write_byte(cmd4, displaybuffer[i] & 0xFF, ACK_CHECK_EN);
      i2c_master_write_byte(cmd4, displaybuffer[i] >> 8, ACK_CHECK_EN);
    }

    i2c_master_stop(cmd4);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd4);

    if (ret == ESP_OK)
    {
      // Do something for debugging
    }
  }  // End of while loop

}



void displayDistance(double* distance)
{
  // Debugging
  int ret;

  // Write a default number (0) to the display buffer
  uint16_t displaybuffer[4];
  displaybuffer[0] = ASCIILookupTable[getIndex('0')];
  displaybuffer[1] = ASCIILookupTable[getIndex('0')];
  displaybuffer[2] = ASCIILookupTable[getIndex('0')];
  displaybuffer[3] = ASCIILookupTable[getIndex('0')];

  // Continually writes the current time
  while(1)
  {
    // Convert the current distance to a positive integer
    int temp = (int)(*distance);
    if (temp < 0)
    {
      temp = -temp;
    }

    // Display digits from the thousands place to the ones place
    displaybuffer[0] = ASCIILookupTable[getIndex('0') + ((temp / 1000) % 10)];
    displaybuffer[1] = ASCIILookupTable[getIndex('0') + ((temp / 100 ) % 10)];
    displaybuffer[2] = ASCIILookupTable[getIndex('0') + ((temp / 10  ) % 10)];
    displaybuffer[3] = ASCIILookupTable[getIndex('0') +          (temp % 10)];

    // Send commands characters to display over I2C
    i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
    i2c_master_start(cmd4);
    i2c_master_write_byte(cmd4, ( ALPHANUM_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);

    // Render the buffer
    for (uint8_t i = 0; i < 4; i++)
    {
      i2c_master_write_byte(cmd4, displaybuffer[i] & 0xFF, ACK_CHECK_EN);
      i2c_master_write_byte(cmd4, displaybuffer[i] >> 8, ACK_CHECK_EN);
    }

    i2c_master_stop(cmd4);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd4);

    if (ret == ESP_OK)
    {
      // Do something for debugging
    }
  }  // End of while loop
}



void displayTickCounts(uint32_t* tickCount_L, uint32_t* tickCount_R)
{
  // Debugging
  int ret;

  // Write a default number (0) to the display buffer
  uint16_t displaybuffer[4];
  displaybuffer[0] = ASCIILookupTable[getIndex('0')];
  displaybuffer[1] = ASCIILookupTable[getIndex('0')];
  displaybuffer[2] = ASCIILookupTable[getIndex('0')];
  displaybuffer[3] = ASCIILookupTable[getIndex('0')];

  // Continually writes the current time
  while(1)
  {
    // Dereference the pointers
    uint32_t ticks_L = (*tickCount_L);
    uint32_t ticks_R = (*tickCount_R);

    // Display digits from the thousands place to the ones place
    displaybuffer[0] = ASCIILookupTable[getIndex('0') + ((ticks_L / 10) % 10)];
    displaybuffer[1] = ASCIILookupTable[getIndex('0') +        (ticks_L % 10)];
    displaybuffer[2] = ASCIILookupTable[getIndex('0') + ((ticks_R / 10) % 10)];
    displaybuffer[3] = ASCIILookupTable[getIndex('0') +        (ticks_R % 10)];

    // Send commands characters to display over I2C
    i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
    i2c_master_start(cmd4);
    i2c_master_write_byte(cmd4, ( ALPHANUM_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);

    // Render the buffer
    for (uint8_t i = 0; i < 4; i++)
    {
      i2c_master_write_byte(cmd4, displaybuffer[i] & 0xFF, ACK_CHECK_EN);
      i2c_master_write_byte(cmd4, displaybuffer[i] >> 8, ACK_CHECK_EN);
    }

    i2c_master_stop(cmd4);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd4);

    if (ret == ESP_OK)
    {
      // Do something for debugging
    }
  }  // End of while loop
}



void displaySpeed(double* speed)
{
  // Debugging
  int ret;

  // Write a default number (0) to the display buffer
  uint16_t displaybuffer[4];
  displaybuffer[0] = ASCIILookupTable[getIndex('0')];
  displaybuffer[1] = ASCIILookupTable[getIndex('0')];
  displaybuffer[2] = ASCIILookupTable[getIndex('0')];
  displaybuffer[3] = ASCIILookupTable[getIndex('0')];

  // Continually writes the current time
  while(1)
  {
    // Dereference the pointer
    double spd = (*speed);
    int temp = 0;
    int decimal = 0;

    // Determine the whole number part
    if (spd >= 1000)
    {
      // Note that the last digit contains the decimal point
      decimal = 3;
      temp = (int) spd;
    }
    else if (spd >= 100)
    {
      decimal = 2;
      temp = (int) (spd * 10);  // Left shift by 1 to get the tenth place
    }
    else if (spd >= 10)
    {
      decimal = 1;
      temp = (int) (spd * 100);  // Left shift by 2 to get up to the hundredth place
    }
    else
    {
      decimal = 0;
      temp = (int) (spd * 1000);  // Left shift by 3 to get up to the thousandth place
    }

    // Display digits
    displaybuffer[0] = ASCIILookupTable[getIndex('0') + ((temp / 1000) % 10)];
    displaybuffer[1] = ASCIILookupTable[getIndex('0') + ((temp / 100 ) % 10)];
    displaybuffer[2] = ASCIILookupTable[getIndex('0') + ((temp / 10  ) % 10)];
    displaybuffer[3] = ASCIILookupTable[getIndex('0') +          (temp % 10)];

    // Apply the decimal point to the lucky index (with the help of bitwise OR)
    displaybuffer[decimal] = displaybuffer[decimal] | ASCIILookupTable[getIndex('.')];

    // Send commands characters to display over I2C
    i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
    i2c_master_start(cmd4);
    i2c_master_write_byte(cmd4, ( ALPHANUM_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);

    // Render the buffer
    for (uint8_t i = 0; i < 4; i++)
    {
      i2c_master_write_byte(cmd4, displaybuffer[i] & 0xFF, ACK_CHECK_EN);
      i2c_master_write_byte(cmd4, displaybuffer[i] >> 8, ACK_CHECK_EN);
    }

    i2c_master_stop(cmd4);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd4);

    if (ret == ESP_OK)
    {
      // Do something for debugging
    }
  }  // End of while loop
}

