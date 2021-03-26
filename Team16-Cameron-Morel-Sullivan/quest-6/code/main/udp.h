/*
  Header file for UDP client and server functions. Code adapted from esp-idf 
  udp_client and udp_server example projects.
  --> https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_client
  --> https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_server

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Dec. 2020
*/
#ifndef UDP_H
#define UDP_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


// Parameters
#define IP_CHAR_SIZE    16  // 16 characters for an IP address (###.###.###.###\0)


// Configuration object for the server
// Used so that we only need to pass one parameter to the udp_server function instead of a billion
typedef struct server_object
{
  uint16_t port;     // Port number for the server to listen on
  uint8_t* data;     // Pointer to data to receive
  bool* dish_active; // Pointer to the ESP's dish_active flag (disables radar movement)
  bool* pending;     // Pointer to the pending flag that the RC uses to light pending LED
} server_conf;


/**
  Establishes a UDP server to receive data.
  \param server_conf conf --> Configuration object containing fob server info (see above server_conf struct).
  \param void* pvParameters --> Specifies type of addr_family (AF_INET or AF_INET6).
  \return None
**/
void udp_server(server_conf conf, void* pvParameters);


/**
  Establishes a UDP client to send data.
  \param const char* ip --> IP address to connect to (string form of ###.###.###.###).
  \param uint16_t port --> Port number to connect to.
  \param uint8_t* send_data --> Pointer to data to send.
  \param uint8_t* recv_data --> Pointer to data to receive.
  \return 0 if successfully communicated with the server, or -1 if failure (i.e. could not connect).
**/
int udp_client(const char* ip, uint16_t port, uint8_t* send_data, uint8_t* recv_data);


#endif

