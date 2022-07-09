#ifndef JUMPORAM_CONFIG_H_
#define JUMPORAM_CONFIG_H_

#include <string>
#include <climits>

/* PARAMETERS */
#define BLOCK_SIZE		4096		// bytes of a block
/* 4096.0(4KB), 8192.0(8KB), 16384.0(16KB), 32768.0(32KB), 65536.0(64KB), 131072.0(128KB), 262144.0(256KB), 524288.0(512KB), 1048576.0(1024KB)*/
#define DATABASE_SIZE	536870912 	// bytes of a database
/* 536870912.0(0.5GB) 1073741824(1GB) 2684354560.0(2.5GB) 5368709120(5GB) 10737418240(10GB) 21474836480(20GB) 42949672960(40GB) */
#define STASH_SIZE		2
#define CACHE_SIZE		2

#define SERVER_PORT		5201
#define SERVER_ADDR		"127.0.0.1"

#define IV_SIZE			16
const unsigned char ENCRYPT_KEY[IV_SIZE] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

/* DATA TYPE */
typedef long long int TYPE_INDEX;
typedef long long int TYPE_DATA;
typedef int TYPE_COMMAND;

/* PATHS */
const std::string data_directory = "data/JUMPORAM";
const std::string log_directory = "log/JUMPORAM";

const size_t STORE_SIZE_CLIENT = DATABASE_SIZE / BLOCK_SIZE;
const size_t DATA_CHUNKS = BLOCK_SIZE / sizeof(TYPE_DATA);
const size_t STEP = STORE_SIZE_CLIENT % STASH_SIZE ?
	STORE_SIZE_CLIENT / STASH_SIZE + 1: STORE_SIZE_CLIENT / STASH_SIZE;
const size_t STORE_SIZE = STEP * STASH_SIZE;

/* SOCKET COMMAND */
const TYPE_COMMAND CMD_SEND_DATABASE	= 0x0001;
const TYPE_COMMAND CMD_ACCESS			= 0x0002;
const TYPE_COMMAND CMD_DISCONNECT		= 0x0003;
const TYPE_COMMAND CMD_SUCCESS			= 0x0000;

static_assert(BLOCK_SIZE % sizeof(TYPE_DATA) == 0, "Block size is incorrect.");
static_assert(DATA_CHUNKS > 1, "Bytes of a block is too small.");
static_assert(STEP > 1, "Step is too small.");

#endif // JUMPORAM_CONFIG_H
