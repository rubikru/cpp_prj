#include <udp.hpp>  
#include <debug.hpp>  

  int UDPDataSocket::open_socket(int p_port, const char* p_server)
  { 
	if ((udp_socket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		return -1;
	}

	struct timeval timeout;      
	timeout.tv_sec = SENDING_TIMEOUT_SECONDS;
	timeout.tv_usec = 0;

	setsockopt (udp_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
	setsockopt (udp_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(p_port);

	if (inet_aton(p_server , &si_other.sin_addr) == 0) 
	{
		return 0;
	}  
	return 1; 
  }

  int UDPDataSocket::bind_socket()
  {
	if ( bind(udp_socket, (const struct sockaddr *)&si_other, sizeof(si_other)) < 0 )
	{
		return -1;
	}
	return 1; 
  }  

  int UDPDataSocket::init_socket(SOCKET_TYPE socket_type, int p_port, const char* p_server)
  {
	int ret = open_socket( p_port, p_server);
	if (ret <=0) 
	  {
		return ret;
	  }
	if (socket_type == SERVER_SOCKET_TYPE)
	 {
		ret = bind_socket();
	 }
	return ret;
  }
  
  int UDPDataConnector::init(SOCKET_TYPE socket_type,unsigned int p_port, const char* p_server)
  {  	  	  
	return UDPDataSocket::init_socket(socket_type, p_port, p_server);
  }
  
  int UDPDataConnector::send_packet(unsigned char *p_curr_packet, unsigned int p_packet_len)
  {  
	int repeats=REPEATS; 
	
    while (repeats--)
    {   int ret;
		if ((ret = sendto(udp_socket, p_curr_packet, p_packet_len, 0 , (struct sockaddr *) &si_other, slen)) > 0)
		{
			return ret;
		}
	}
	return 0;
  }

  int UDPDataConnector::send_packet_with_ack(unsigned char *p_curr_packet, unsigned int p_packet_len)
  {  
    if (send_packet(p_curr_packet, p_packet_len))
    {
		memset(p_curr_packet, '\0', p_packet_len);
		if (recvfrom(udp_socket, p_curr_packet, p_packet_len, 0, (struct sockaddr *) &si_other, &slen) > 0)
		{
			return 1;
		}	  
	}
	return 0;
  }
  
  int UDPDataConnector::get_packet(unsigned char *p_curr_packet, unsigned int p_packet_len)
  { int ret; 

	if ((ret = recvfrom(udp_socket, p_curr_packet, p_packet_len, 0, (struct sockaddr *) &si_other, &slen)) > 0)
	{
		return ret;
	}	  
	return 0;
  }


