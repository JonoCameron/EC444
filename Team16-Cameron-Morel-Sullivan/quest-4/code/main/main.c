/*
  Main file for Quest 4 - Electronic Voting. Has fobs determine a poll leader 
  and use that poll leader to send fobs votes to a Raspberry Pi server over UDP.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "ir_tx_rx.h"
#include "timer.h"
#include "udp.h"
#include "wifi_station.h"
#include "button_intr.h"



// UART Parameters
#define UART_NUM        UART_NUM_0
#define UART_TXD        (UART_PIN_NO_CHANGE)
#define UART_RXD        (UART_PIN_NO_CHANGE)
#define UART_RTS        (UART_PIN_NO_CHANGE)
#define UART_CTS        (UART_PIN_NO_CHANGE)
#define UART_BAUD_RATE  115200

#define GPIO_INPUT_IO_1  4   // GPIO pin A5 - Button 1 (change color)
#define GPIO_INPUT_IO_2  21  // GPIO pin 21 - Button 2 (toggle signal activation)
#define COLOR 'G'            // Default to GREEN

// Buffer Parameters
#define TASK_STACK_SIZE 2048

// Fob Parameters
#define DEVICE_ID       CONFIG_ESP_MYID

// Timeout Parameters
#define ELECTION_TO     10    // 10 seconds long for the poll leader election
#define LEADER_TO       3600  // Poll leader lasts for 3600 seconds (1 hour) (Can set up to 9999)
#define HEARTBEAT_TO    5     // Must receive heartbeat from poll leader within 5 seconds

#define GPIO_INPUT_PIN_SEL_2    1ULL<<GPIO_INPUT_IO_2



// Create a fobTable
// Note: struct fobNode defined in udp.h
struct fobNode fobTable[] = { { .isLeader = false, .ip     = "192.168.1.23" , .port     = 2323 ,  // "Special fob"
                                                   .ip_pub = "98.171.20.181", .port_pub = 4444  },
                              { .isLeader = false, .ip     = "192.168.1.18" , .port     = 1818 ,
                                                   .ip_pub = "98.171.20.181", .port_pub = 6464  },
                              { .isLeader = false, .ip     = "192.168.1.178", .port     = 1178 ,
                                                   .ip_pub = "86.190.83.206", .port_pub = 5555  },
                              { .isLeader = false, .ip     = "192.168.1.22" , .port     = 2222 ,
                                                   .ip_pub = "98.171.20.181", .port_pub = 3232  },
                              { .isLeader = false, .ip     = "192.168.1.241", .port     = 2241 , 
                                                   .ip_pub = "86.190.83.206", .port_pub = 6666  }
                            };
// Record the number of elements (fobs) in the fobTable
#define MAX_FOBS (sizeof fobTable / sizeof fobTable[0])



struct fobNode rPi_server = { .ip = "10.0.0.217", .port = 3333};



// Global Variables
char start = 0x1B;                  // Start byte for UDP transmission
char heartbeat = 0x21;              // Heartbeat byte for UDP transmission (tells fob citizens leader still alive)
uint8_t* send_data;                 // Buffer to hold data to be sent over UDP
uint8_t* recv_data;                 // Buffer to hold data to be received over UDP
int myID = DEVICE_ID;               // ID of the fob (subtract 1 from this value to index into fobTable)
int minVal = DEVICE_ID;             // Smallest ID known to the current fob
int state = 0;                      // Tracks current fob state (candidate, poll leader, citizen)
int len_out = 4;                    // Specifies length of UART message
char myColor = (char) COLOR;        // Device's current color code
char led_states[] = {'R','B','G'};  // Array of LED states
candidate_e candidates;             // Fob's candidate option (changes by button press and used to set myColor)
bool transmit_flag = false;         // Flag that indicates when to send UART vote payload over IR
bool vote_flag = false;             // Indicates the fob received a vote from another fob over IR NFC
bool recv_vote = false;             // Flag to indicate a UART/IR vote has been received
uint8_t* ir_data;                   // Buffer to hold data received over UART/IR.

// Initialised as NULL because not constants in compilation.
// They are assigned values in "button_task_transmit()" and sent in "register_votes()".
vote vote_data = { .fob_id = NULL, .candidate = NULL };


// Timeout Variables
int count_election = 0;   // Counter variable for election timeout
int count_leader = 0;     // Counter variable for leader timeout
int count_heartbeat = 0;  // Counter variable for leader heartbeat
bool isElection = true;   // Flag that tells if an election is going on or not

// Mutex (for resources)
SemaphoreHandle_t mux = NULL;



// Initializes all tasks
static void init(void)
{
  // Initialize timer-based interrupts
  initialize_TimerQueue();

  // Mutex for current values when sending
  mux = xSemaphoreCreateMutex();

  // Set and config parameters for the UART driver, then install the driver
  uart_config_t uart_config = {.baud_rate = UART_BAUD_RATE,
                               .data_bits = UART_DATA_8_BITS,
                               .parity    = UART_PARITY_DISABLE,
                               .stop_bits = UART_STOP_BITS_1,
                               .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                               .source_clk = UART_SCLK_APB
                              };
  uart_param_config(UART_NUM, &uart_config);
  uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_RTS, UART_CTS);
  uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

  // Initialize the LEDs
  led_init();

  //Initialise IR
  rmt_tx_init();
  uart_init();

  // Initialize alarm using timer API
  alarm_init();

  // Initialise change candidate button and send data button
  gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);  // Install gpio isr service
  button_queue_init(GPIO_INPUT_IO_1);
  button_toggle_init(GPIO_INPUT_IO_2);


  // Allocate space for the data buffers
  send_data = (uint8_t*) malloc(BUF_SIZE);
  recv_data = (uint8_t*) malloc(BUF_SIZE);
  ir_data   = (uint8_t*) malloc(BUF_SIZE);

  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Initialization for UDP client
  ESP_ERROR_CHECK(esp_netif_init());

  // Initialize the WiFi station (no need to call example_connect() for UDP client)
  ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
  wifi_init_sta();
}

static void register_votes(void* pvParameters){
  while(1){  
    if(state != 0 && vote_flag){
      char ip[IP_CHAR_SIZE];  // Holds server fob's IP address
      uint16_t port;          // Holds server fob's port number
      vote_flag = false;
      strcpy(ip, rPi_server.ip);
      port = rPi_server.port;

      send_data[0] = vote_data.fob_id;
      send_data[1] = vote_data.candidate;


      udp_client(ip, port, send_data, recv_data);

      if(recv_data[0] == 1){
        printf("V\n");
      }
    }
    else
    {
      vTaskDelay(10 / portTICK_PERIOD_MS);  // Delay stops watchdog from triggering.
    }
  }
}


void button_task_transmit()
{
  uint32_t io_num;
  while (1)
  {
    if ( xQueueReceive(gpio_evt_queue_transmit, &io_num, portMAX_DELAY) )
    {
      xSemaphoreTake(mux, portMAX_DELAY);

      // Transmit a vote once the fobs already know who the poll leader is
      if (state != 0)
      {
        transmit_flag = true;
        vote_flag = true;
        vote_data.fob_id = (char)myID;
        vote_data.candidate = (char)candidates;
      }
      xSemaphoreGive(mux);
      printf("Tranmit button pressed.\n");
      //register_votes((void*)NULL);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



// UDP Server task. Receives data from UDP clients, and sends a response to them (server fob's ID).
static void udp_server_task(void* pvParameters)
{
  // Pull the server's port number from the fobTable
  uint16_t port = fobTable[myID - 1].port;

  // Configure the data for the server to send
  uint8_t server_data[3];
  server_data[0] = start;
  server_data[1] = (char) myID;
  server_data[2] = '\0';

  // Setup the server configuration object
  // Note: a lot of pass by reference so the UDP server can communicate with main
  server_conf conf;
  conf.port = port;
  conf.data = server_data;
  conf.count_heartbeat = &count_heartbeat;
  conf.count_leader = &count_leader;
  conf.state = &state;
  conf.fobTable = fobTable;
  conf.recv_vote = &recv_vote;
  conf.candidates = &candidates;

  // Wait and listen for clients to communicate with
  udp_server(conf, pvParameters);

  // If we get to this point (we shouldn't), then delete the task
  vTaskDelete(NULL);
}

// UDP Client task. Sends data to and receives data from UDP server.
static void udp_client_task(void* pvParameters)
{
  char ip[IP_CHAR_SIZE];  // Holds server fob's IP address
  uint16_t port;          // Holds server fob's port number
  int fobIndex = 0;       // Tracks next entry in the fobTable to communicate with
  int leaderIndex = -1;

  // Loop through the table
  while (1)
  {
    // Only send UDP packets as a candidate or as the leader
    if (state == 0 || state == 1)
    {
      // Avoid indexing into the fobTable whose element matches this fob's ID
      if ( (myID-1) == fobIndex )
      {
        // Increment the fob table iterator
        if (fobIndex < MAX_FOBS-1)
        {
          fobIndex++;
        }
        else
        {
          fobIndex = 0;
        }
      }

      // Record the server fob's public IP address and port number used for port forwarding
      strcpy(ip, fobTable[fobIndex].ip_pub);
      port = fobTable[fobIndex].port_pub;

      // Send a candidate query if in an election
      if (state == 0)
      {
        // Write to the send_data buffer
        send_data[0] = start;
        send_data[1] = '\0';

        // Attempt to communicate!
        if ( udp_client(ip, port, send_data, recv_data) == 0 )
        {
          int rVal = (int) recv_data[1];

          // Update this fob's knowledge with the received data
          if (rVal < minVal)
          {
            minVal = rVal;  // Set this fob's knowledge of the minVal to the server fob's ID
            state = 2;      // This fob is no longer running as a candidate (return to citizen status)
            fobIndex = 0;   // Reset the fob table index
          }
        }
      }
      else if (state == 1)
      {
        // This fob is the leader, so it knows it is alive
        count_heartbeat = 0;

        // Split the count_leader integer into a 16 bit value with 2 bytes, msb and lsb
        char digit_thousands = '0' + ((count_leader / 1000) % 10);  // Thousand's place digit
        char digit_hundreds  = '0' + ((count_leader / 100 ) % 10);  // Hundred's place digit
        char digit_tens      = '0' + ((count_leader / 10  ) % 10);  // Ten's place digit
        char digit_ones      = '0' + (count_leader % 10);           // One's place digit

        // Write the leader heartbeat to the send_data buffer
        send_data[0] = heartbeat;
        send_data[1] = (char) myID;
        send_data[2] = digit_thousands;
        send_data[3] = digit_hundreds;
        send_data[4] = digit_tens;
        send_data[5] = digit_ones;
        send_data[6] = '\0';

        // Attempt to communicate! (Note: we don't really care about the server's response)
        udp_client(ip, port, send_data, recv_data);
      }

      // Increment the fob table iterator
      if (fobIndex < MAX_FOBS-1)
      {
        fobIndex++;
      }
      else
      {
        // Wrap around zero
        fobIndex = 0;
      }
    }
    else
    {
      // Fob is neither a candidate nor a leader, so just wait and do nothing
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Sending received IR voting data to the Pi server
    if ( ( state != 0 ) && ( recv_vote ) )
    {
      // Find index of poll leader in fobTable
      for (int i = 0; i < MAX_FOBS; i++)
      {
        if (fobTable[i].isLeader == true)
        {
          leaderIndex = i;
          printf("fobTable[i].isLeader == true @ index: %d\n", i);
          break;
        }
      }
      // If I am the leader, send the IR data directly to the Pi
      if ( leaderIndex == ( myID - 1 ) )
      {
        udp_client(PI_SERVER_IP, PI_SERVER_PORT, ir_data, recv_data);

        // Already contacted the pi, so set recv_vote to false
        recv_vote = false;
      }
      // If I am not the poll leader, then send the IR data to them.
      else if (leaderIndex >= 0)
      {
        udp_client(fobTable[leaderIndex].ip_pub, fobTable[leaderIndex].port_pub, ir_data, recv_data);

        // Assume the vote was sent (it should given that udp_client does a few tries
        recv_vote = false;
      }

      // DEBUGGING
      printf("Vote receiver fob's recv_data[0] is %x\n", (int)recv_data[0]);

      // Note: there is a chance that there isn't a poll leader (i.e. there is an ongoing election)
      // If that is the case, avoid sending the vote, but keep recv_vote active.
      // Regardless, rely on UDP server to resolve the recv_vote flag.
    }
  }

  // If we get to this point (we shouldn't), then delete the task
  vTaskDelete(NULL);
}



/**
  LED task to light LED based on current fob status (state).
  \param None
  \return None
**/
void led_task()
{
  while(1)
  {
    // Indicate whether or not the fob is the poll leader
    if (state == 1)
    {
      gpio_set_level(ONBOARD, 1);
    }
    else
    {
      gpio_set_level(ONBOARD, 0);
    }

    // Set the colored LEDs based on the fob's selected candidate
    switch(candidates)
    {
      case 0 : // Candidate red
        gpio_set_level(GREENPIN, 0);
        gpio_set_level(REDPIN, 1);
        gpio_set_level(BLUEPIN, 0);
        break;
      case 1 : // Candidate blue
        gpio_set_level(GREENPIN, 0);
        gpio_set_level(REDPIN, 0);
        gpio_set_level(BLUEPIN, 1);
        break;
      case 2 : // Candidate green
        gpio_set_level(GREENPIN, 1);
        gpio_set_level(REDPIN, 0);
        gpio_set_level(BLUEPIN, 0);
        break;
      default:  // Off
        gpio_set_level(GREENPIN, 0);
        gpio_set_level(REDPIN, 0);
        gpio_set_level(BLUEPIN, 0);
        break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



/**
  Button task that rotates through the LED color options (off, Red, Green, Blue).
  \param None
  \return None
**/
void button_task_candidate()
{
  uint32_t io_num;
  while(1)
  {
    if ( xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY) )
    {
      xSemaphoreTake(mux, portMAX_DELAY);

      // Change the LED state to the next one in sequence
      candidates = (candidates + 1)%3;

      // Set the LED color to the appropriate state
      myColor = led_states[candidates];

      xSemaphoreGive(mux);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



/**
  Handles timer event to raise the send_data. Event flag raised every 2 seconds.
  \param void* arg --> Pointer to arguments.
  \return None
**/
static void timer_event_task(void* arg)
{
  while (1)
  {
    // Create dummy structure to store structure from queue
    timer_event_t evt;

    // Transfer from queue
    xQueueReceive(timer_queue, &evt, portMAX_DELAY);

    // Do something if triggered!
    if (evt.flag == 1)
    {
      // Update the timeout counters based on the current state
      if (state == 0 || isElection)
      {
        count_election++;
      }
      else
      {
        count_leader++;
        count_heartbeat++;
      }

      // Check to see if the timeout flags should be raised
      if (count_election >= ELECTION_TO)
      {
        // Reset counts and flags
        count_election = 0;
        count_leader = 0;
        count_heartbeat = 0;
        isElection = false;

        // Reset the fob's isLeader knowledge of the fobTable
        for (int i = 0; i < MAX_FOBS; i++)
        {
          fobTable[i].isLeader = false;
        }

        // Check if the fob has been elected the leader
        if ( (state == 0) && (myID <= minVal) )
        {
          state = 1;                         // Still in election with smallest ID, so become the leader
          fobTable[myID-1].isLeader = true;  // Record that this fob is the fob leader in its fobTable
        }
        else
        {
          state = 2;  // Set fob to citizen (non-leader)
        }
      }

      // Check the conditions for starting a new election
      if ( (count_leader >= LEADER_TO) || (count_heartbeat >= HEARTBEAT_TO))
      {
        count_leader = 0;     // Reset leader term counter
        count_heartbeat = 0;  // Reset leader heartbeat counter
        count_election = 0;   // Reset election counter
        isElection = true;    // Raise the election flag
        state = 0;            // Change status to candidate for a new election
        minVal = myID;        // Reset the fob's knowledge of the minVal
      }
    }  // End of timer interrupt handling
  }  // End of while loop
}



/**
  Send task that sends a payload in the format of: | Start | myColor | myID | checksum |.
  \param None
  \return None
**/
void send_task()
{
  char data_out[4];

  while (1)
  {
    xSemaphoreTake(mux, portMAX_DELAY);
    data_out[0] = start;
    data_out[1] = (char) myColor;
    data_out[2] = (char) myID;
    data_out[3] = genCheckSum(data_out,len_out-1);

    if (transmit_flag)
    {
      // Make 10 attempts to transmit the UART packet over IR
      for (int i = 0; i < 10; i++)
      {
        //printf("sending signal...\n");
        uart_write_bytes(UART_NUM_1, data_out, len_out);

        ESP_LOG_BUFFER_HEXDUMP(TAG_SYSTEM, data_out, len_out, ESP_LOG_INFO);
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }

      // Reset the transmit flag
      transmit_flag = false;
    }
    else
    {
      // Make a time sacrifice so that the watchdog doesn't get upset and continuously restart the board
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    xSemaphoreGive(mux);
  }
}



/**
  Receive task that looks for Start byte then stores received values. Sets LED color based on received color code.
  \param None
  \return None
**/
void recv_task()
{
  // Buffer for input data
  uint8_t *data_in = (uint8_t *) malloc(BUF_SIZE);

  while (1)
  {
    int len_in = uart_read_bytes(UART_NUM_1, data_in, BUF_SIZE, 20 / portTICK_RATE_MS);
    if (len_in > 0)
    {
      // Find the start byte and use an offset if needed
      if ( (data_in[0] == start) && (len_in >= len_out))
      {
        if (checkCheckSum(data_in,len_out))
        {
          ESP_LOG_BUFFER_HEXDUMP(TAG_SYSTEM, data_in, len_out, ESP_LOG_INFO);
          myColor = data_in[1];
          printf("received\n");

          ir_data[0] = 'V';
          ir_data[1] = data_in[2];  // Send myID
          ir_data[2] = data_in[1];  // Send myColor
          ir_data[3] = '\0';

          recv_vote = true;
        }
      }
      else if ( (data_in[1] == start) && (len_in >= (len_out+1)) )
      {
        if (checkCheckSum(data_in+1,len_out))
        {
          ESP_LOG_BUFFER_HEXDUMP(TAG_SYSTEM, data_in+1, len_out, ESP_LOG_INFO);
          myColor = data_in[2];
          printf("received (offset 1)\n");

          ir_data[0] = 'V';
          ir_data[1] = data_in[2 + 1];  // Send myID
          ir_data[2] = data_in[1 + 1];  // Send myColor
          ir_data[3] = '\0';

          recv_vote = true;
        }
      }
      else if ( (data_in[2] == start) && (len_in >= (len_out+2)) )
      {
        if (checkCheckSum(data_in+2,len_out))
        {
          ESP_LOG_BUFFER_HEXDUMP(TAG_SYSTEM, data_in+2, len_out, ESP_LOG_INFO);
          myColor = data_in[3];
          printf("received (offset 2)\n");

          ir_data[0] = 'V';
          ir_data[1] = data_in[2 + 2];  // Send myID
          ir_data[2] = data_in[1 + 2];  // Send myColor
          ir_data[3] = '\0';

          recv_vote = true;
        }
      }
      else if ( (data_in[3] == start) && (len_in >= (len_out+3)) )
      {
        if (checkCheckSum(data_in+3,len_out))
        {
          ESP_LOG_BUFFER_HEXDUMP(TAG_SYSTEM, data_in+3, len_out, ESP_LOG_INFO);
          myColor = data_in[4];
          printf("received (offset 3)\n");

          ir_data[0] = 'V';
          ir_data[1] = data_in[2 + 3];  // Send myID
          ir_data[2] = data_in[1 + 3];  // Send myColor
          ir_data[3] = '\0';

          recv_vote = true;
        }
      }
      // DEBUG
      else
      {
        printf("Uh oh, got something, but couldn't read it...\n");
      }

      // Adjust the candidate based on the received color
      if (myColor == 'R')
      {
        candidates = RED_CANDIDATE;
      }
      else if (myColor == 'B')
      {
        candidates = BLUE_CANDIDATE;
      }
      else if (myColor == 'G')
      {
        candidates = GREEN_CANDIDATE;
      }
      else
      {
        candidates = NONE_CANDIDATE;
      }
    }
    else
    {
      // printf("Nothing received.\n");
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
  free(data_in);
}



// Runs the show by setting up all initialization and configurations for tasks
void app_main(void)
{
  // Initialize everything
  init();

  // Create task to handle timber-based events
  xTaskCreate(timer_event_task, "timer_event_task", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES, NULL);

  // Create UDP server task (receives leader election data)
  xTaskCreate(udp_server_task, "udp_server_task", TASK_STACK_SIZE*2, (void*)AF_INET, configMAX_PRIORITIES - 1, NULL);

  // Create UDP client task (sends leader election data)
  xTaskCreate(udp_client_task, "udp_client_task", TASK_STACK_SIZE*2, NULL, configMAX_PRIORITIES - 2, NULL);

  // Create UDP client task (sends leader election data)
  xTaskCreate(register_votes, "register_votes", TASK_STACK_SIZE*2, NULL, configMAX_PRIORITIES - 3, NULL);

  // Create LED task to display fob status
  xTaskCreate(led_task, "led_task", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 4, NULL);

  // Button to cycle through candidates
  xTaskCreate(button_task_candidate, "button_task_candidate", 1024*2, NULL, configMAX_PRIORITIES - 5, NULL);

  // Button to transmit
  xTaskCreate(button_task_transmit, "button_task_transmit", 1024*2, NULL, configMAX_PRIORITIES - 6, NULL);

  // Send and receive IR
  xTaskCreate(recv_task, "uart_rx_task", 1024*4, NULL, configMAX_PRIORITIES - 7, NULL);
  xTaskCreate(send_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES - 8, NULL);
}
