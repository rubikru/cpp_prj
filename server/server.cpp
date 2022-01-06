#include <stdio.h>  
#include <string.h> 
#include <stdlib.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib> 
#include <ctime> 
#include <map>

#include <debug.hpp>  
#include <common.hpp>  
#include <udp.hpp>  
#include <data.hpp>  

using namespace std;

#define FILE_PROCESSED -1

/*
 * Класс DataReceiver - принимает пакеты данных на сервере через объект класса UDPDataConnector,
 * помещает данные в объект класса DataStorage.
 * При получении очередного пакета, при завершении передачи всего объема данных(всего файла),
 * очищает буфер с данными данного файла и сообщает о завершении передачи файла.
 * 
 */
class DataReceiver
{
  protected:
  data_str curr_packet;
  unsigned int packet_size;
  std::shared_ptr<UDPDataConnector> Connector;
  std::shared_ptr<DataStorage> RcvdData;
  unsigned int packets_num=0, max_packet_num;
    
  private:
   
  public:
    
  void init(std::shared_ptr<UDPDataConnector> p_Connector, std::shared_ptr<DataStorage> p_RcvdData)
  {	  
	  Connector = p_Connector;
	  RcvdData = p_RcvdData;
  }

  /* Метод transmission_finished:
   * - вовращает 1, весь пул принимаемых файлов, к которым был получен хотя бы один пакет, был принят, 
   *                и больше нет незаконченных передач
   * иначе - возвращает 0.
   */ 
  int transmission_finished()
  {
	  return RcvdData->all_files_processed();
  }
  
  /* Метод get_next_packet:
   * - получает очередной пакет, 
   * - подготоваливает подтверждающий пакет, при этом:
   *       если он последний в передаваемом пуле пакетов (относящихся к одному файлу), 
   *       то отправляет crc полученных данных для подтверждения корректности полученных данных клиенту
   *       и очищает буфер полученных данных, возвращает флаг о завершении передачи файла 
   * 	   (возможно - одного из многих передаваемых)
   */ 
  int get_next_packet()
  {  
	int ret = 0, file_processed=0;
	if (ret = Connector->get_packet((unsigned char*)&curr_packet, sizeof(curr_packet)))
	{
		max_packet_num = curr_packet.header.seq_total;
		f_id file_id(curr_packet.header.id);
		   
	    RcvdData->register_data_packet(file_id, curr_packet, ret);
	    
	    file_processed = RcvdData->all_packets_processed(file_id);
	   		
		curr_packet.header.type = TYPE_ACK;
		
		if (file_processed)
		{
			uint32_t file_crc=0;
			file_crc = RcvdData->calc_crc(file_id);
			memcpy(curr_packet.data, (unsigned char*)&file_crc, sizeof(uint32_t));			
		}
		
		ret = Connector->send_packet((unsigned char*)&curr_packet, sizeof(curr_packet));

		if (file_processed)
		{
		   RcvdData->delete_file_data(file_id);
		   return FILE_PROCESSED;
	   }
	}	  
	return ret;
  }
};

#define TIMEOUT_SEC  5
class Timer
{
	protected:
		time_t init_time, timeout=0;
	public:
		Timer(time_t p_timeout)
		{
			timeout = p_timeout;
			reset();
		}
		void reset()
		{
			init_time = time(0);
		}
		int time_is_out()
		{
			return (time(0)>(init_time+timeout));
		}
};

void stop(const char* msg)
{
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[])
{
  int s, i, port = PORT, ip;
  const char* server=SERVER;

  if(argc==1) 
  {
     printf("\n Usage: %s [[<port>] [<server>]]\n",argv[0]);
  }
  if(argc > 1) 
  {
    port = atoi(argv[1]);
  }
  if(argc > 2) 
  {
    server = argv[2];
  } 

  std::shared_ptr<DataStorage> RcvdData = std::make_shared<DataStorage>();
	 
  std::shared_ptr<UDPDataConnector> Connector =  std::make_shared<UDPDataConnector>();
  
  if (Connector->init(SERVER_SOCKET_TYPE, port, server))
  {
      printf("Listening on port %u (ip %s)\n",port, server);
	  std::shared_ptr<DataReceiver> receiver =  std::make_shared<DataReceiver>();

	  // Инициализация объекта принимающего данные
	  receiver->init(Connector, RcvdData);
	  Timer timer(TIMEOUT_SEC);
	  do {
		 // принять и сохранить очередной пакет, 
		 // подтвердить получение пакета и - если завершена передача - файла
		 int result = receiver->get_next_packet();
		 if (result == FILE_PROCESSED)
		   {
			 // если все передаваемые файлы получены - завершить работу сервера
			 if (receiver->transmission_finished())
		         break;
	       }
	     if (result>0) 
	       {
				timer.reset(); // сброс таймера
		   }
	  } while(!timer.time_is_out()); // выход по таймауту
  }
  else  
  {
	stop("open socket error");
  }
     
  return 0;
}
