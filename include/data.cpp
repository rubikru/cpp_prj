#include "data.hpp"


uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len)
{
    int k;
    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        for (k = 0; k < 8; k++)
        crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
    }
    return ~crc;
}

  Data::~Data()
	{
	  if (data_buffer!=NULL)
	    {
			delete data_buffer;
	    }
	  if (packet_flags!=NULL)
	    {
			delete packet_flags;
	    }	    
	}
  
  void Data::init(unsigned int p_datasize)
    {
		datasize = p_datasize;
		data_buffer = (unsigned char*) new char[datasize];
		memset(data_buffer, 0, datasize); 
		
		packets_processed = 0;
		packets_num = get_packets_num();
		packet_flags = new uint8_t[packets_num];
		memset((char *)packet_flags, 0, packets_num);			 
    }

  void Data::init_by_packets(unsigned int p_packets_num)
    {
		init(p_packets_num * DATA_SIZE);
    }

  void Data::confirm_packet_as_processed(data_str& data_packet)
	{
		 packet_flags[data_packet.header.seq_number] = 1;
		 packets_processed++;	
	}
  
  int Data::all_packets_processed()
	{
		return (get_packets_num() == packets_processed);
	}
	
  unsigned int Data::get_packets_processed()
	{
		return (unsigned int) packets_processed;
	}	

  unsigned int Data::get_data_len()
	{
		return (unsigned int) datasize;
	}	

  void Data::set_data_len(unsigned int p_datasize)
	{
		datasize = p_datasize;
	}	

  unsigned char* Data::get_data_buffer()
    {
		return data_buffer;
    }
	
  unsigned char* Data::get_data_packet(data_str& data_packet)
    {
		return data_buffer + (data_packet.header.seq_number * DATA_SIZE);
    }

  void Data::put_data_packet(data_str& data_packet, unsigned int data_len)
    {
		memcpy( data_buffer + (data_packet.header.seq_number * DATA_SIZE), data_packet.data, data_len);
		if (data_packet.header.seq_number == (data_packet.header.seq_total-1))
		  {   // если это последний пакет из файла - пересчитать точную длину файла с учетом его длины
			  set_data_len(((data_packet.header.seq_total-1) * DATA_SIZE) + data_len);
		  }
    }
    
  unsigned int Data::get_packets_num()
	{
		return (unsigned int) ( datasize / DATA_SIZE ) + (((datasize % DATA_SIZE)==0)?(0):(1));
	}	

    void Data::print_stats()
      {
		printf("DataSize=%u, PacketsNum=%u, PacketsProcessed=%u\n", datasize, get_packets_num(), get_packets_processed());
      }
  
	uint32_t Data::calc_crc()
		{
			uint32_t crc = 0;

			crc = crc32c(crc, (const unsigned char *)data_buffer, datasize);
			 
			return crc;
		}
 
	void Data::show_file_id(unsigned char *file_id)
		{
			printf(" FILE ID: ");	  
			for (int i=0; i<8; i++) printf("%02X",(unsigned char) *(file_id+i));	
			printf(" ");	  
		}
	  
	void Data::show_file_id(std::array<char, 8> &p_fileID)
		{
			unsigned char fileID[8];
			memcpy(fileID, &p_fileID, sizeof(fileID));	  
			show_file_id(fileID);
			printf("is complete.\n");	  
		}

	uint64_t DataSource::get_random()
		{			  
		srand((unsigned int)time(NULL)*(unsigned int)clock());
		return (uint64_t)(rand());	
		}

    void DataSource::get_next_packet_num(unsigned int& next_pack_num, unsigned int& packet_len)
		{
			next_pack_num = (rand() % packets_num);
			int repeats=0;
			while ((packet_flags[next_pack_num] != 0) && (repeats++<packets_num))
			{
				next_pack_num = (next_pack_num+1) % packets_num;	        
			}
			packet_len = (next_pack_num < (packets_num - 1))?(DATA_SIZE):(datasize - (packets_num-1)*DATA_SIZE);
		}	
     
	void DataSource::show_file_id()
		{
			Data::show_file_id(file_id);
		}
    void DataSource::print_stats()
		{
			printf("DataSize=%u, PacketsNum=%u, FileID=", datasize, ( datasize / DATA_SIZE ) + (((datasize % DATA_SIZE)==0)?(0):(1)));		  
			for (int i=0; i<8; i++) printf("%X", file_id[i]);
			printf("\n");
		}
	
	void DataSource::set_file_id(uint64_t random_id)
		{
			for (int i=0; i<8; i++)
			{
				file_id[i]=*(((unsigned char*)&random_id)+i);
			}  
		}	
	  
    unsigned char* DataSource::get_file_id()
		{
			return file_id;
		}  

    int DataSource::load_data(const char* datafilename)
	  {
		streampos datafilesize;
		std::ifstream datafile;
		datafile.open (datafilename, ios::in | ios::binary);
		if (datafile.is_open())
		  {
			set_file_id((uint64_t)get_random()<<32 | (uint64_t)get_random());
			datafile.seekg (0, ios::end);
			datafilesize = datafile.tellg();
			
			init((unsigned int)datafilesize);
				
			datafile.seekg (0, ios::beg);
			datafile.read ((char *)data_buffer, datafilesize);
			datafile.close();	  

			return 1;
		  }
		else
		  return 0;
	  }

	unsigned int DataSource::get_next_data_packet(data_str& next_packet)
		{
			unsigned int full_packet_len=0, data_packet_len=0;

			memcpy((unsigned char*)&next_packet.header.id, file_id, sizeof(packet_header().id));

			next_packet.header.type = TYPE_PUT;
			next_packet.header.seq_total = packets_num;

			get_next_packet_num(next_packet.header.seq_number, data_packet_len);

			memcpy( (unsigned char*)&next_packet.data , (unsigned char*)get_data_packet(next_packet), data_packet_len);
			full_packet_len = data_packet_len + HEADER_SIZE;

			return full_packet_len;
		}	
	  
	void DataSource::dump_data()
		{
			puts(""); debug(data_buffer, get_data_len());
		}


	unsigned int DataStorage::register_data_packet(f_id file_id, data_str& data_packet, unsigned int packet_len)
		{
			if (fd_list[file_id]==NULL)
			  {
				fd_list[file_id] = new Data;
				fd_list[file_id]->init_by_packets(data_packet.header.seq_total);			  
			  }
			unsigned int data_packet_len = packet_len - HEADER_SIZE;
			fd_list[file_id]->put_data_packet(data_packet, data_packet_len);
			fd_list[file_id]->confirm_packet_as_processed(data_packet);
			
			return 0;
		}	
	    
	int DataStorage::all_packets_processed(f_id file_id)
		{
			if (fd_list[file_id]->all_packets_processed())
				return 1; 
			else
				return 0;
		}

	int DataStorage::all_files_processed()
		{   
			int ret = 1;
			for (auto const &file_data_ptr : fd_list)
			{
					Data::show_file_id((unsigned char*)&file_data_ptr.first);
					file_data_ptr.second->print_stats();
					if (!file_data_ptr.second->all_packets_processed())
						{
							ret = 0;
						}
			}
			return ret; 
		}
	  
	void DataStorage::delete_file_data(f_id file_id)
		{
			Data::show_file_id(file_id.id);
			delete fd_list[file_id];
			fd_list.erase(file_id);
		}
	void DataStorage::dump_data(f_id file_id)
		{
			puts("");debug(fd_list[file_id]->get_data_buffer(), fd_list[file_id]->get_data_len());
		}
	  
	uint32_t DataStorage::calc_crc(f_id file_id)
		{
			return fd_list[file_id]->calc_crc();
		}
