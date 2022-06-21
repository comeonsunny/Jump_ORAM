#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <string>
#include <cmath>
#include <iostream>
// parameter about cryptography
#define IV_SIZE 16
const unsigned char ENCRYPT_KEY[IV_SIZE] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

// directory of database and log files
const std::string DATABASE_DIR = "data";
const std::string LOG_DIR      = "log";
const std::string METADATA_DIR = "metadata";
// rename some data types
typedef long long unsigned int TYPE_INDEX;
typedef int TYPE_COMMAND;

// the parameters about command and its meaning
const TYPE_COMMAND CMD_SEND_DATABASE	= 0x0001;
const TYPE_COMMAND CMD_ACCESS			= 0x0002;
const TYPE_COMMAND CMD_EVICT			= 0x0003;
const TYPE_COMMAND CMD_CMD_OK			= 0x0004;
const TYPE_COMMAND CMD_DATA_OK			= 0x0000;
const TYPE_COMMAND CMD_DATABASE_RECEIVED = 0x0005;
const TYPE_COMMAND CMD_PHYSICAL_ID 		 = 0x0006;
/* parameter about socket*/
#define PORT 8888
#define IP   "127.0.0.1"

/* parameter about database */
#define BLOCK_SIZE		4096		// byte as unit
/* 4096.0(4KB), 8192.0(8KB), 16384.0(16KB), 32768.0(32KB), 65536.0(64KB), 131072.0(128KB), 262144.0(256KB), 524288.0(512KB), 1048576.0(1024KB)*/
#define DATABASE_SIZE	536870912 	// byte as unit
/* 536870912.0(0.5GB) 1073741824(1GB) 2684354560.0(2.5GB) 5368709120(5GB) 10737418240(10GB) 21474836480(20GB) 42949672960(40GB) */

const static TYPE_INDEX BLOCK_NUM = DATABASE_SIZE % BLOCK_SIZE == 0 ? DATABASE_SIZE / BLOCK_SIZE : DATABASE_SIZE / BLOCK_SIZE + 1;
#define BLOCK_NUM_IN_BUCKET	  5
#define LEVEL_NUM		      (TYPE_INDEX)ceil(log2(BLOCK_NUM))
#define BUCKET_NUM		      (TYPE_INDEX)(pow(2, LEVEL_NUM) - 1)
#define BUCKET_LEAF_NUM	      (TYPE_INDEX)(pow(2, LEVEL_NUM - 1))
#define BUCKET_ID_FIRST_LEAF  (TYPE_INDEX)(pow(2, LEVEL_NUM - 1) - 1)

static void progress_bar(TYPE_INDEX current, TYPE_INDEX total, std::string name){
		if (current < total - 1){
			std::cout << "\r " << name << " progress: [" << current * 100 / (total - 1) << "%]";
		} else {
			std::cout << "\r " << name << " progress completed: [100%]";
		}
		TYPE_INDEX show_num = current * 20 / total;
		for (TYPE_INDEX i = 0; i < show_num; i++){
			std::cout << "█";
		}
	}
#endif /* CONFIG_HPP */