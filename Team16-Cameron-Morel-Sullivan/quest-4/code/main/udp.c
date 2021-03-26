/*
  Implements UDP client and server functions. Code adapted from esp-idf 
  udp_client and udp_server example projects.
  --> https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_client
  --> https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_server

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
#include "udp.h"


static const char *UDP_TAG_CLIENT = "ESP UDP Client";  // UDP message tag
static const char *UDP_TAG_SERVER = "ESP UDP Server";  // UDP message tag


void udp_server(server_conf conf, void* pvParameters)
{
  char rx_buffer[128];
  char addr_str[128];
  int addr_family = (int)pvParameters;
  int ip_protocol = 0;
  struct sockaddr_in6 dest_addr;
  uint8_t* vote_to_pi = (uint8_t *) malloc(16);
  uint8_t* recv_from_pi = (uint8_t *) malloc(16);
  bool forwardVote = false;  // Flag to forward vote to Pi Server (done if fob is the poll leader)
  int voterIndex = -1;  // Holds the voter fob's index for receipts

  while (1)
  {
    if (addr_family == AF_INET)
    {
      struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
      dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
      dest_addr_ip4->sin_family = AF_INET;
      dest_addr_ip4->sin_port = htons(conf.port);
      ip_protocol = IPPROTO_IP;
    }
    else if (addr_family == AF_INET6)
    {
      bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
      dest_addr.sin6_family = AF_INET6;
      dest_addr.sin6_port = htons(conf.port);
      ip_protocol = IPPROTO_IPV6;
    }

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0)
    {
      ESP_LOGE(UDP_TAG_SERVER, "Unable to create socket: errno %d", errno);
      break;
    }
    ESP_LOGI(UDP_TAG_SERVER, "Socket created");

    #if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    if (addr_family == AF_INET6)
    {
      // Note that by default IPV6 binds to both protocols, it is must be disabled
      // if both protocols used at the same time (used in CI)
      int opt = 1;
      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
      setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
    }
    #endif

    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
    {
      ESP_LOGE(UDP_TAG_SERVER, "Socket unable to bind: errno %d", errno);
    }
    ESP_LOGI(UDP_TAG_SERVER, "Socket bound, port %d", conf.port);

    while (1)
    {
      ESP_LOGI(UDP_TAG_SERVER, "Waiting for data");
      struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
      socklen_t socklen = sizeof(source_addr);
      int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

      // Error occurred during receiving
      if (len < 0)
      {
        ESP_LOGE(UDP_TAG_SERVER, "recvfrom failed: errno %d", errno);
        break;
      }
      // Data received
      else
      {
        // Get the sender's ip address as string
        if (source_addr.sin6_family == PF_INET)
        {
          inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        }
        else if (source_addr.sin6_family == PF_INET6)
        {
          inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        }

        rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
        ESP_LOGI(UDP_TAG_SERVER, "Received %d bytes from %s:", len, addr_str);
        ESP_LOG_BUFFER_HEXDUMP(UDP_TAG_SERVER, rx_buffer, len, ESP_LOG_INFO);

        // Determine what to do based on the first byte of the UDP message
        if (rx_buffer[0] == '!')  // Sync with the poll leader's heartbeat
        {
          // DEBUG
          printf("Got a ! byte!\n");

          // If the fob is currently a candidate, switch it to be a citizen regardless of its current ID.
          //   This helps ensure fobs that enter a system late (after a poll leader has been selected)
          //   become a citizen automatically. Note that the fob has a change to become a poll leader
          //   during the next leader election.
          *(conf.state) = 2;  // Set status to citizen

          // Record the poll leader in the fob's fobTable
          conf.fobTable[rx_buffer[1]-1].isLeader = true;

          // Reset leader heartbeat counter
          *(conf.count_heartbeat) = 0;

          // Reconstruct the poll leader's count_leader value
          int digit_thousands = (int)(rx_buffer[2] - '0');
          int digit_hundreds  = (int)(rx_buffer[3] - '0');
          int digit_tens      = (int)(rx_buffer[4] - '0');
          int digit_ones      = (int)(rx_buffer[5] - '0');

          // Sync with the poll leader's leader_count
          int recovered = (digit_thousands * 1000) + (digit_hundreds * 100) + (digit_tens * 10) + digit_ones;
          *(conf.count_leader) = recovered;  // Sync leader counter
        }
        else if (rx_buffer[0] == 'P')  // Fob received a vote receipt from the poll leader
        {
          // DEBUG
          printf("Got a P byte!\n");

          // Got the vote receipt, so indicate via turning off the LEDs
          *(conf.candidates) = NONE_CANDIDATE;
        }
        else if (rx_buffer[0] == 'V')  // Fob (poll leader) received a vote from a fob IR receiver
        {
          // DEBUG
          printf("Got a V byte!\n");

          // Prepare to forward the vote to the Raspberry Pi server
          vote_to_pi[0] = rx_buffer[0];  // Pass along 'V'
          vote_to_pi[1] = rx_buffer[1];  // Pass along the sender's device ID
          vote_to_pi[2] = rx_buffer[2];  // Pass along the sender's candidate vote
          vote_to_pi[3] = '\0';          // Terminate the message with a NULL byte
          forwardVote = true;            // Raise the forward vote flag

          // Record the voter's index (will be used by the poll leader to sends vote receipts)
          voterIndex = (int)rx_buffer[1] - 1;
          printf("Voter index is %d\n", voterIndex);

          // Set the poll leader's LED color to the vote choice
          if (rx_buffer[2] == 'R')
          {
            *(conf.candidates) = RED_CANDIDATE;
          }
          else if (rx_buffer[2] == 'G')
          {
            *(conf.candidates) = GREEN_CANDIDATE;
          }
          else if (rx_buffer[2] == 'B')
          {
            *(conf.candidates) = BLUE_CANDIDATE;
          }
          else
          {
            *(conf.candidates) = NONE_CANDIDATE;
          }
        }

        // If the forwardVote flag is raised, send the current vote data to the Raspberry Pi server
        // Note: need to keep the flag in case a UDP message is lost along the way
        if (forwardVote)
        {
          // Forward the vote to the pi
          if ( udp_client(PI_SERVER_IP, PI_SERVER_PORT, vote_to_pi, recv_from_pi) == 0 )
          {
            printf("recv_from_pi[0] = %x\n", recv_from_pi[0]);

            // Got a response back from the pi server, and the vote was successfully counted!
            // Give the fob voter a receipt, which will turn off its LEDs
            forwardVote = false;

            uint8_t vote_receipt[2] = {'P', '\0'};
            uint8_t ret_buf[10];

            // DEBUG
            printf("Hey we got a response from the pi server!\n");

            // Send a vote receipt to the fob voter
            if ( *(conf.state) == 1 && voterIndex >= 0 )
            {
              printf("Fob leader attempting to communicate with index %d...\n", voterIndex);

              udp_client(conf.fobTable[voterIndex].ip_pub, conf.fobTable[voterIndex].port_pub, vote_receipt, ret_buf);
            }

            // Turn off the poll leader's LEDs to indicate a successful receipt
            *(conf.candidates) = NONE_CANDIDATE;
          }
        }

        // Send the data response to the client (server fob's ID)
        int err = sendto( sock, conf.data, strlen((const char*) conf.data), 0, 
                          (struct sockaddr *)&source_addr, sizeof(source_addr) );

        // Error check the sent UDP package
        if (err < 0)
        {
          ESP_LOGE(UDP_TAG_SERVER, "Error occurred during sending: errno %d", errno);
          break;
        }
        else
        {
          ESP_LOGI(UDP_TAG_SERVER, "Sent response to %s:", addr_str);
          ESP_LOG_BUFFER_HEXDUMP(UDP_TAG_SERVER, conf.data, strlen((const char*) conf.data), ESP_LOG_INFO);
        }
      }
    }  // End of listening while loop

    if (sock != -1)
    {
      ESP_LOGE(UDP_TAG_SERVER, "Shutting down socket and restarting...");
      shutdown(sock, 0);
      close(sock);
    }
  }
}



int udp_client(const char* ip, uint16_t port, uint8_t* send_data, uint8_t* recv_data)
{
  int ret = -1;  // Set default return value to failure
  int attempt_count = 0;  // Tracks how many connection attempts have been made
  int addr_family = 0;
  int ip_protocol = 0;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 500000;  // Timeout every 500ms

  // Attempt connection 5 times or until receive successful response (whichever one happens first)
  while ( (attempt_count < 5) && (ret < 0) )
  {
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0)
    {
      ESP_LOGE(UDP_TAG_CLIENT, "Unable to create socket: errno %d", errno);
      break;
    }
    ESP_LOGI(UDP_TAG_CLIENT, "Socket created, sending to %s:%d", ip, port);

    // Set a timeout for the socket and error check
    if ( setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0 )
    {
      ESP_LOGE(UDP_TAG_CLIENT, "Unable to create socket timeout: errno %d", errno);
      break;
    }

    // While loop for UDP client-server communication
    while (1)
    {
      // Increment the connection count
      attempt_count++;

      // Send data to the server
      int err = sendto( sock, (const char*) send_data, strlen((const char*) send_data), 0, 
                        (struct sockaddr *)&dest_addr, sizeof(dest_addr) );
      if (err < 0)
      {
        ESP_LOGE(UDP_TAG_CLIENT, "Error occurred during sending: errno %d", errno);
        break;
      }
      ESP_LOGI(UDP_TAG_CLIENT, "Message sent");

      // Receive data from the server
      struct sockaddr_in source_addr; // Large enough for both IPv4 or IPv6
      socklen_t socklen = sizeof(source_addr);
      int len = recvfrom(sock, recv_data, sizeof(recv_data) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

      // Error occurred during receiving
      if (len < 0)
      {
        ESP_LOGE(UDP_TAG_CLIENT, "recvfrom failed: errno %d", errno);
        break;
      }
      // Data received from the server so handle it
      else
      {
        recv_data[len] = 0; // Null-terminate whatever we received and treat like a string
        ESP_LOGI(UDP_TAG_CLIENT, "Received %d bytes from %s:", len, ip);
        ESP_LOGI(UDP_TAG_CLIENT, "%s", recv_data);
        ESP_LOG_BUFFER_HEXDUMP(UDP_TAG_SERVER, recv_data, len, ESP_LOG_INFO);

        // UDP client case where poll leader attempted to communicate with the pi server
        if ( (send_data[0] == 'V') && (recv_data[0] == 'R' || recv_data[0] == 'A') )
        {
          // Poll leader successfully got a response from the pi server
          ret = 0;
          break;
        }
        // All other successful UDP communications between fobs
        else if ( (recv_data[0] == 0x1b) && isdigit(recv_data[1] + '0') )
        {
          // Fob successfully communicated with the server fob
          ret = 0;
          break;  // Break out of this while loop, and we'll exit out of the larger while loop
        }

        // Add a slight delay
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    }

    if (sock != -1)
    {
      if (ret == 0)
      {
        ESP_LOGI(UDP_TAG_CLIENT, "Received server message. Shutting down socket...");
      }
      else
      {
        ESP_LOGE(UDP_TAG_CLIENT, "Shutting down socket and restarting...");
      }
      shutdown(sock, 0);
      close(sock);
    }
  }

  return ret;
}




