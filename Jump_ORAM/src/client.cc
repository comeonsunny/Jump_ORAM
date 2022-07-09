#include "client.h"
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "utils.h"
#include "progressbar.hpp"

#include <iostream>

using namespace std;

Client::Client(std::string name_prefix, std::string timestamp)
	: pos_map(STORE_SIZE + CACHE_SIZE)
{
	/* file names */
	database_name = data_directory + '/' + name_prefix + "_database";
	client_data_name = data_directory + '/' + name_prefix + "_data";
	build_log_name = log_directory + '/' + name_prefix + "_build_" + timestamp + ".txt";
	load_log_name = log_directory + '/' + name_prefix + "_load_" + timestamp + ".txt";
	access_log_name = log_directory + '/' + name_prefix +"_access_breakdown_cost_" + timestamp + ".txt";
	/* file */
	client_file = fopen(client_data_name.c_str(), "r+b");
	if (NULL == client_file && errno == 2) {
		errno = 0;
		client_file = fopen(client_data_name.c_str(), "w+b");
	}
	if (NULL == client_file) {
		printf("[Client] Can't open file %s %d\n", client_data_name.c_str(), errno);
		exit(1);
	}

	/* socket */
	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1) {
		printf("[Client] Socket create failed\n");
		exit(1);
	}

	/* set address */
	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	server_addr.sin_port = htons(SERVER_PORT);
	if (0 != connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
		printf("[Client] Failed to link the server, please check if the server is open.\n");
		exit(1);
	}
}

Client::~Client()
{
	/* Disconnect the link of server */
	write(fd, &CMD_DISCONNECT, sizeof(CMD_DISCONNECT));

	close(fd);
	fclose(client_file);
}

void Client::build_database()
{
	Log log(build_log_name);

	FILE *database = fopen(database_name.c_str(), "w+b");
	if (database == NULL) {
		printf("[Build] Can't open file %s %d\n", database_name.c_str(), errno);
		exit(1);
	}

	/* Build client metaData and write it to disk */
	log.start();
	this->init();
	this->write_to_file();
	printf("[Build] Elapsed time for building metaData and  writing metaData to disk: %lu ns\n", log.end());

	/* Write random block to file for creating a database */
	printf("----------------------------------------------------\n");
	printf("[Build] Creating the database...\n");
	log.start();
	progressbar bar(STORE_SIZE);
	for (TYPE_INDEX i = 0; i < STORE_SIZE; i++) {
		Block block(i);
		block.encrypt();
		fseek(database, pos_map[i] * sizeof(Block), SEEK_SET);
		fwrite(&block, 1, sizeof(Block), database);
		bar.update();
	}
	printf("\n[Build] Elapsed time for create database in disk: %lu ns\n", log.end());
	printf("----------------------------------------------------\n");
	/* Read the blocks from database and send them to server */
	rewind(database);
	Block block;
	TYPE_COMMAND CMD = CMD_SEND_DATABASE;
	printf("[Build] Sending the database to server...\n");
	log.start();
	write(fd, &CMD, sizeof(CMD));
	progressbar bar_send(STORE_SIZE);
	for (TYPE_INDEX i = 0; i < STORE_SIZE; i++) {
		fread(&block, 1, sizeof(Block), database);
		write(fd, &block, sizeof(Block));
		bar_send.update();
	}
	read(fd, &CMD, sizeof(CMD));
	printf("\n[Build] Elapsed time for sending database: %lu ns\n", log.end());
	printf("----------------------------------------------------\n");
	fclose(database);

	log.write_to_file();
}

void Client::init()
{
	for (TYPE_INDEX i = 0; i < STORE_SIZE + CACHE_SIZE; i++)
		pos_map[i] = i;
	std::mt19937 g(rand());
	std::shuffle(pos_map.begin(), pos_map.begin() + STORE_SIZE, g);

	for (TYPE_INDEX i = 0; i < CACHE_SIZE; i++) {
		data_cache.virtual_index[i] = STORE_SIZE + i;
		data_cache.data[i] = Block(STORE_SIZE + i);
	}
}

void Client::load()
{
	Log log(load_log_name);

	log.start();
	fseek(client_file, 0, SEEK_SET);
	int pos_map_read_n = pos_map.size() * sizeof(TYPE_INDEX);
	if (pos_map_read_n != fread(pos_map.data(), 1, pos_map_read_n, client_file)) {
		printf("[Load] Read pos_map err %d\n", errno);
		exit(1);
	}

	int cache_read_n = sizeof(Cache);
	if (cache_read_n != fread(&data_cache, 1, cache_read_n, client_file)) {
		printf("[Load] Read data_cache err.\n");
		exit(1);
	}
	printf("[Load] Elapsed time for load client data: %lu ns \n", log.end());

	log.write_to_file();
}

Block *Client::access(TYPE_INDEX virtual_index)
{
	printf("[Access] Accessing virtual_index: %lld\n", virtual_index);
	Block *block;

	write(fd, &CMD_ACCESS, sizeof(CMD_ACCESS));

	if (pos_map[virtual_index] < STORE_SIZE) {
		block = get_block_from_stash(virtual_index);
	} else {
		block = get_block_from_cache(virtual_index);
	}

	return block;
}

void Client::write_to_file()
{
	fseek(client_file, 0, SEEK_SET);
	fwrite(pos_map.data(), pos_map.size(), sizeof(TYPE_INDEX), client_file);
	fwrite(&data_cache, 1, sizeof(data_cache), client_file);
	fflush(client_file);
}

Block *Client::get_block_from_stash(TYPE_INDEX virtual_index)
{

	TYPE_INDEX stash_pos, cache_pos;
	Log log(access_log_name);

	/* Calculate retrieved_index */
	log.start();
	TYPE_INDEX real_index = pos_map[virtual_index];
	stash.stash_index = real_index % STEP;
	printf("[Access] Elapsed time for calculating stash_index: %lu ns\n", log.end());
	printf("[Access] Block store in server: %lld\n", real_index);

	/* Fill stash */
	log.start();
	stash.client_send_stash_index(fd);
	stash.client_recv_stash(fd);
	printf("[Access] Elapsed time for sending stash_index and receiving stash from server: %lu ns\n", log.end());

	cache_pos = rand() % CACHE_SIZE;
	stash_pos = real_index / STEP;

	/* decrypt */
	log.start();
	for (int i = 0; i < STASH_SIZE; i++)
		stash.data[i].decrypt();
	printf("[Access] Elapsed tiem for decryption: %lu ns \n", log.end());

	/* Swap block of interest from stash into data cache */
	log.start();
		std::swap(pos_map[virtual_index], pos_map[data_cache.virtual_index[cache_pos]]);
		data_cache.virtual_index[cache_pos] = virtual_index;
		std::swap(stash.data[stash_pos], data_cache.data[cache_pos]);
	printf("[Access] Elapsed time for swapping block: %lu ns\n", log.end());

	/* re-encrypt */
	log.start();
	for (int i = 0; i < STASH_SIZE; i++)
		stash.data[i].encrypt();
	printf("[Access] Elapsed tiem for re-encryption: %lu ns \n", log.end());

	/* Rewrite refreshed stash */
	log.start();
	stash.client_send_stash(fd);
	stash.client_recv_success(fd);
	printf("[Access] Elapsed time for uploading stash to server: %lu ns\n", log.end());

	log.start();
	this->write_to_file();
	printf("[Access] Elapsed time for storing client metaData into disk: %lu ns\n", log.end());

	log.write_to_file();
	return data_cache.data + cache_pos; // beautiful
}

Block *Client::get_block_from_cache(TYPE_INDEX virtual_index)
{
	TYPE_INDEX stash_pos, cache_pos;
	Log log(access_log_name);

	/* Calculate retrieved_index */
	log.start();
	TYPE_INDEX real_index = pos_map[virtual_index];
	stash.stash_index = rand() % STEP;
	printf("[Access] Elapsed time for calculating stash_index: %lu ns\n", log.end());
	printf("[Access] Block store in cache.\n");

	/* Fill stash */
	log.start();
	stash.client_send_stash_index(fd);
	stash.client_recv_stash(fd);
	printf("[Access] Elapsed time for receive stash from server: %lu ns\n", log.end());

	/* decrypt */
	log.start();
	for (int i = 0; i < STASH_SIZE; i++)
		stash.data[i].decrypt();
	printf("[Access] Elapsed time for decryption: %lu ns \n", log.end());

	/* Get the position of the block of interest from the data cache */
	log.start();
	cache_pos = data_cache.find_index(virtual_index);
	printf("[Access] Elapsed time for getting position from the data cache: %lu ns\n", log.end());

	/* re-encrypt */
	log.start();
	for (int i = 0; i < STASH_SIZE; i++)
		stash.data[i].encrypt();
	printf("[Access] Elapsed tiem for re-encryption: %lu ns \n", log.end());

	/* Rewrite stash */
	log.start();
	stash.client_send_stash(fd);
	stash.client_recv_success(fd);
	printf("[Access] Elapsed time for uploading refreshed stash to server: %lu ns\n", log.end());

	log.start();
	this->write_to_file();
	printf("[Access] Elapsed time for store client metaData into Disk: %lu ns\n", log.end());

	log.write_to_file();
	return data_cache.data + cache_pos;
}
