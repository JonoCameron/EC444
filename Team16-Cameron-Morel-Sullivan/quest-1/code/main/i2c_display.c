/*
  Adapted I2C example code to work with the Adafruit 14-segment Alphanumeric Display. Key notes: MSB!!
  Added I/O functionality to enable users to enter in alphanumeric characters, and have their inputs
  be displayed on the I2C display.

  DJ Morel, Sept 2020
  Emily Lam, Sept 2018, Updated Aug 2019
*/
#include "i2c_display.h"



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
void i2c_example_master_init()
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


// Utility function to scan for i2c device
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
    printf("- No I2C devices found!" "\n");
  printf("\n");
}

////////////////////////////////////////////////////////////////////////////////



// Alphanumeric Functions //////////////////////////////////////////////////////

// Turn on oscillator for alpha display
int alpha_oscillator()
{
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
int no_blink()
{
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
int set_brightness_max(uint8_t val)
{
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
    i2c_master_write_byte(cmd4, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
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
    i2c_master_write_byte(cmd4, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
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
    i2c_master_write_byte(cmd4, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
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


