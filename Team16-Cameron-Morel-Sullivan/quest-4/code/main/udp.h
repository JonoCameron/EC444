/*
  Header file for UDP client and server functions. Code adapted from esp-idf 
  udp_client and udp_server example projects.
  --> https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_client
  --> https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_server

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
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

// Pi server public IP address and port numbering
#define PI_SERVER_IP "98.171.20.181"
#define PI_SERVER_PORT 3141

// Structure to contain fobNodes
struct fobNode
{
  bool isLeader;              // Tells if this current fob entry is the poll leader (true) or not (false)
  char ip[IP_CHAR_SIZE];      // Holds IP address for the fob server (private IP)
  uint16_t port;              // Holds the port number for the fob server (private port)
  char ip_pub[IP_CHAR_SIZE];  // Holds IP address for the fob server (public IP - router)
  uint16_t port_pub;          // Holds the port number for the fob server (public port used in port forwarding)
};

typedef struct vote_data
{
  char fob_id;
  char candidate;
} vote;

// Defines names for the candidate options
typedef enum
{
  RED_CANDIDATE,
  BLUE_CANDIDATE,
  GREEN_CANDIDATE,
  NONE_CANDIDATE
} candidate_e;

// Configuration object for the server
// Used so that we only need to pass one parameter to the udp_server function instead of a billion
typedef struct server_object
{
  uint16_t port;             // Port number for the server to listen on
  uint8_t* data;             // Pointer to data to receive
  int* count_heartbeat;      // Pointer to the server fob's leader heartbeat counter
  int* count_leader;         // Pointer to the fob's leader term counter
  int* state;                // Pointer to the fob's current state
  struct fobNode* fobTable;  // Address of the fob server's fobTable
  bool* recv_vote;           // Pointer to the fob's recv_vote flag (toggles off when gets ACK from poll leader)
  candidate_e* candidates;   // Pointer to the fob's candidates variable
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

