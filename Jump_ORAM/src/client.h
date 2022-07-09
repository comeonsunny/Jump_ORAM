#ifndef JUMPORAM_CLIENT_H_
#define JUMPORAM_CLIENT_H_

#include <string>
#include <vector>
#include "config.h"
#include "jump_oram.h"

class Client{
public:
	Client(std::string name_prefix, std::string timestamp);
	~Client();
	void init();	// random position map...
	void load();	// load position map...

	void build_database();

	Block *access(TYPE_INDEX virtual_index);
private:
	void write_to_file();
	Block *get_block_from_stash(TYPE_INDEX virtual_index);
	Block *get_block_from_cache(TYPE_INDEX virtual_index);

	std::vector<TYPE_INDEX> pos_map;
	Cache data_cache;
	Stash stash;

	int fd;			// socket connect to server
	std::string database_name;
	std::string client_data_name;
	std::string build_log_name;
	std::string load_log_name;
	std::string access_log_name;
	FILE *client_file;	// store client data
};

#endif // JUMPORAM_CLIENT_H
