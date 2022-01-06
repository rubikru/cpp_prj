
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

#include <common.hpp>  
#include <udp.hpp>  
#include <data.hpp>  
#include <debug.hpp>  

using namespace std;

/*
 * Класс DataSender - отправляет пакеты данных входного файла на сервер через объект класса UDPDataConnector,
 * берет данные из объекта класса DataSource.
 * При при завершении передачи всего объема данных(всего файла),
 * позволяет сверить crc входного файла с полученным в пакете подтверждения с сервера.
 * 
 */
 
class DataSender
{
  protected:
	std::shared_ptr<DataSource> FileData;
	std::shared_ptr<UDPDataConnector> Connector;
	
	data_str curr_packet;
	unsigned int curr_packet_len=0;
  	
  public:

  void init(std::shared_ptr<UDPDataConnector> p_Connector, std::shared_ptr<DataSource> p_FileData)
  {	  
	  Connector = p_Connector;

	  FileData = p_FileData;
      srand(time(0));
  }
  
  int all_packets_sent()
  {
	  return FileData->all_packets_processed();
  }

  void dump_data()
  {
	  FileData->dump_data();
  }

  int sending_is_ok()
  {
	  uint32_t file_crc_from_server, calculated_crc;

	  calculated_crc = FileData->calc_crc();
	  memcpy((unsigned char*)&file_crc_from_server, curr_packet.data, sizeof(uint32_t));
	  
	  return (file_crc_from_server == calculated_crc);
  }
  
  void show_file_id()
  {
		FileData->show_file_id();
  }
  int send_next_packet()
  {  
	int ret;  
	if (all_packets_sent())
	  return -1;
	
	unsigned int data_len;
	
	curr_packet_len = FileData->get_next_data_packet(curr_packet);

	if (ret = Connector->send_packet((unsigned char*)&curr_packet, curr_packet_len))
	{
		if (ret = Connector->get_packet((unsigned char*)&curr_packet, sizeof(curr_packet)))
		{
			if (curr_packet.header.type == TYPE_ACK)
			{
			   FileData->confirm_packet_as_processed(curr_packet);
		   }
		}
	}
	return ret;
  }
  
  unsigned int packets_sent()
  {
	  return FileData->get_packets_processed();
  }
  void show_packet_data()
  {
	  printf("%02u. Packet #%02u of %02u sent!\n",  FileData->get_packets_processed(), curr_packet.header.seq_number, curr_packet.header.seq_total);
  }
  void show_fileid()
  {
	  FileData->show_file_id();
  }
};

void stop(const char* msg)
{
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[])
{
  struct sockaddr_in si_other;
  unsigned int port = PORT;
  const char* server=SERVER;
  socklen_t slen=sizeof(si_other);
  const char* datafilename;
  char* data;

  // Инициализация
  if(argc < 2) 
  {
     printf("\n Usage: %s <filename> [<ip of server> [<port>]]\n",argv[0]);
     return 1;
  }
  datafilename = argv[1];
  if(argc >= 3) 
  {
    server = argv[2];
  } 
  if(argc >= 4) 
  {
    port = atoi(argv[3]);
  }
  
  std::shared_ptr<DataSource> FileData = std::make_shared<DataSource>();

  if (!FileData->load_data(datafilename))
	{
		stop("Input file error");
	}

  std::shared_ptr<UDPDataConnector> Connector =  std::make_shared<UDPDataConnector>();
  
  std::shared_ptr<DataSender> sender = std::make_shared<DataSender>();
  
  if (Connector->init(CLIENT_SOCKET_TYPE, port, server))
  { 
	  sender->init(Connector, FileData);
	  
	  printf("Start transfer file '%s', ", datafilename);
	  sender->show_fileid(); puts("");
	  
	  // отправка пакетов данных пока не будут все отосланы
	  while(!sender->all_packets_sent())
	  {	 
		 if (!sender->send_next_packet())
		   {
				puts("Timeout error!");
				break;
		   }
		 if (sender->packets_sent()<=20)
		   {
				sender->show_packet_data();
		   }
	  }

	  if (sender->all_packets_sent())
	     {
			 sender->show_file_id();
			 if (sender->sending_is_ok())
			 {
				puts("CRC is OK!"); 
			 }
			 else
			 {
				puts("CRC is FAILED!"); 				 
			 }
		 }
  }
  else
  {
	stop("Open socket error");
  } 
  
  return 0;
}
