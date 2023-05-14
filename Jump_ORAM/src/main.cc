#include <cstdio>
#include <cstdlib>
#include <string>
#include "client.h"
#include "server.h"
#include "utils.h"

#include <iostream>
#include <ctime>
using namespace std;

void sleep(float seconds){
    clock_t startClock = clock();
    float secondsAhead = seconds * CLOCKS_PER_SEC;
    // do nothing until the elapsed time has passed.
    while(clock() < startClock+secondsAhead);
    return;
}

int main(int argc, char *argv[])
{
	string mkdir_data_directory = "mkdir -p " + data_directory;
	string mkdir_log_directory = "mkdir -p " + log_directory;
	system(mkdir_data_directory.c_str());
	system(mkdir_log_directory.c_str());
	/* Get time string */
	time_t now = time(NULL);
	char timestamp[16];
	if (now == -1) {
		printf("Can not get time.\n");
		return 1;
	}
	tm *now_time = localtime(&now);
	strftime(timestamp, 16, "%d%m_%H%M", now_time);

	/* Crypto library initialization */
	Block::init();

	srand((unsigned)time(NULL));

	int temp_flag = DATABASE_SIZE/(1024ULL*1024ULL*1024ULL);
    std::string database_size_GB = temp_flag ? to_string(temp_flag) : "0.5"; 
    // if the value of database_size_GB is equal to '2', set it to "2.5"
    if (temp_flag == 2)
    {
        database_size_GB = "2.5";
    }
    std::cout << "database_size: " << database_size_GB << "GB; " << "BLOCK_SIZE: "<<  BLOCK_SIZE/1024<< "KB" << std::endl;
	std::string name_prefix = database_size_GB + "GB_" + to_string(BLOCK_SIZE/1024) + "KB" + "_JUMP_ACCESS_";

	cout << "Server(1) or Client(2): ";
	if (argc < 2)
		goto use_hint;
	if (argv[1][0] == '1') {
		printf("[Main] Run Server\n\n");
		server_start(name_prefix + "_server_", timestamp);
	} else if (argv[1][0] == '2') {
		printf("[Main] Run Client\n\n");
		Client client(name_prefix + "_client_", timestamp);
		cout << "[Main] Client is building database." << endl;
		client.build_database();
		Log log(log_directory + '/' + "stash_size_" + to_string(STASH_SIZE) + "_dataCache_size_" + to_string(CACHE_SIZE) + ".txt");
		for (TYPE_INDEX i = 0; i < 100; i++) {
			TYPE_INDEX idx = rand() % STORE_SIZE;
			log.start();
			client.access(idx);
			printf("[Main] Elapsed time for access block %lld: %ld ns\n\n", idx, log.end());
		}
		log.write_to_file();

	} else {
		goto use_hint;
	}

	return 0;

use_hint:
	printf("%s 1	- run server\n", argv[0]);
	printf("%s 2	- run client\n", argv[0]);
	printf("%s 3	- create new ORAM\n", argv[0]);
	return 1;
}
