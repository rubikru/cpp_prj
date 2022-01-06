#pragma once

#include <stdio.h>  
#include <string.h> 
#include <stdlib.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib> 
#include <ctime> 
#include <map>

using namespace std;
  
#define REPEATS 12
#define SENDING_TIMEOUT_SECONDS 5
  
// Тип сокета
enum SOCKET_TYPE {
	SERVER_SOCKET_TYPE,
	CLIENT_SOCKET_TYPE
};

/*
 * UDPDataSocket -  обеспечивает открытие сокета серверного или клиентского ( SOCKET_TYPE )
 */
 
class UDPDataSocket
{
  protected:
	  int udp_socket;
	  struct sockaddr_in si_other;
	  socklen_t slen=sizeof(si_other);
	  int open_socket(int p_port, const char* p_server);
	  int bind_socket();
	  int init_socket(SOCKET_TYPE socket_type, int p_port, const char* p_server);
	  
  private:  
  
  public:
};

/*
 * UDPDataConnector -  обеспечивает передачу данных пакетами UDP через открытый сокет 
 */
 
class UDPDataConnector: public UDPDataSocket
{
  protected:
  
  private:  
	  int open_socket(int p_port, const char* p_server);
	  int bind_socket();
  
  public:
	  int init(SOCKET_TYPE socket_type,unsigned int p_port, const char* p_server);
	  int send_packet(unsigned char *p_curr_packet, unsigned int p_packet_len);
	  int send_packet_with_ack(unsigned char *p_curr_packet, unsigned int p_packet_len);
	  int get_packet(unsigned char *p_curr_packet, unsigned int p_packet_len);
};
