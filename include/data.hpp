#pragma once

#include <stdio.h>  
#include <string.h> 
#include <stdlib.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib> 
#include <ctime> 
#include <map>
#include <memory>
#include <stddef.h>
#include <stdint.h>

#include "debug.hpp"

using namespace std;

const unsigned int MAX_PACKET_SIZE = 1472;

// Структура описывюащая заголовок пакета
typedef struct packet_header{
  uint32_t seq_number;
  uint32_t seq_total;
  uint8_t type;
  unsigned char id[8];
} packet_header;

const unsigned int HEADER_SIZE = sizeof(packet_header);
const unsigned int DATA_SIZE = (MAX_PACKET_SIZE-HEADER_SIZE);

// Структура описывюащая пакет
typedef struct data_str{
	packet_header header;
    unsigned char data[DATA_SIZE];
} data_str;

// Структура содержащая id пересыаемого файла
struct f_id{
	std::array<char, 8> id;
    f_id(unsigned char new_id[8])
	{
		for (int i=0; i<8; i++)
		   id[i]=new_id[i];
    }
};
struct fileIdCompare
{
   bool operator() (const f_id& a, const f_id& b) const
   {
		int const min_size = std::min(a.id.size(), b.id.size());
		for (int i = 0; i < min_size; ++i) {
			if (a.id[i] < b.id[i]) {
				return true;
			}
			if (b.id[i] < a.id[i]) {
				return false;
			}
		}
		return b.id.size() > min_size;
   }
};
// типы пакетов
enum PACKET_TYPES {
	TYPE_ACK,
	TYPE_PUT	
};

uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len);

// Класс содержащий буфер с данными и данные о передаваемых пакетах
class Data
{
  protected:
	unsigned char* data_buffer=nullptr;
	unsigned int datasize;

	uint32_t packets_num=0;
	uint32_t packets_processed=0;
	uint8_t *packet_flags=NULL;
	
  private:
   	  
  public:  
	~Data();

	void init(unsigned int p_datasize);
	void init_by_packets(unsigned int p_packets_num);

	void confirm_packet_as_processed(data_str& data_packet);
	int all_packets_processed();
	unsigned int get_packets_processed();
	unsigned int get_data_len();
	void set_data_len(unsigned int p_datasize);
	unsigned char* get_data_buffer();
	unsigned char* get_data_packet(data_str& data_packet);
	void put_data_packet(data_str& data_packet, unsigned int data_len);
	unsigned int get_packets_num();
	void print_stats();
	uint32_t calc_crc();
	static void show_file_id(unsigned char *file_id);
	static void show_file_id(std::array<char, 8> &p_fileID);
};

// Класс с данными о передаваемом файле (используется в клиенте при передаче)
// позволяет загрузить входные данные (файл) во внутренний буфер, формирует в случайном порядке отправляемые пакеты
class DataSource: public Data
{
	protected:
		unsigned char file_id[sizeof(packet_header().id)];
		  
		uint64_t get_random();
		void get_next_packet_num(unsigned int& next_pack_num, unsigned int& packet_len);
     
	public:
		void show_file_id();
		void print_stats();

		void set_file_id(uint64_t random_id);	  
		unsigned char* get_file_id();

		int load_data(const char* datafilename);

		unsigned int get_next_data_packet(data_str& next_packet);
		  
		void dump_data();
};

// структура содержащая буфер данных для каждого принимаемого файла
typedef map <f_id, Data*, fileIdCompare> file_data;

// Класс с данными о принимаемых файлах (используется на сервере при приеме)
// сохраняет полученный пакет данных в буфере - отдельном для каждого передаваемого файла
// при получении последнего пакета из файла - подтверждает его получение высылкой crc
// и очищает буфер с данными файла
class DataStorage
{
	protected:
		file_data fd_list;
     
	public:

		unsigned int register_data_packet(f_id file_id, data_str& data_packet, unsigned int packet_len);
			
		int all_packets_processed(f_id file_id);
		int all_files_processed();
		  
		void delete_file_data(f_id file_id);
		  
		uint32_t calc_crc(f_id file_id);
		
		void dump_data(f_id file_id);
};

