#include "server.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "config.h"
#include "jump_oram.h"
#include "utils.h"

using namespace std;

static void recv_database(const string &file_name, int fd);
static void access_response(const string &database_name, const string &log_name, int fd);

void server_start(string name_prefix, string timestamp)
{
	string log_name = log_directory + '/' + name_prefix + '_' + timestamp + ".txt";
	string database_name = data_directory + '/' + name_prefix + "_database";

	int sock_fd;
	struct sockaddr_in server_addr = {};
	Log log(log_name);

	/* socket initial */
	sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock_fd == -1) {
		printf("[Server] socket create failed\n");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	server_addr.sin_port = htons(SERVER_PORT);
	if (::bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
		printf("[Server] socket bind failed %d\n", errno);
		exit(1);
	}

	if (-1 == listen(sock_fd, 5)) {
			printf("[Server] socket listen failed\n");
			exit(1);
		}
re_accept:
	printf("[Server] Waiting for Command...\n");
	printf("===========================================================\n");
	int connection_fd = accept(sock_fd, (struct sockaddr *)NULL, NULL);
	if (-1 == connection_fd) {
		printf("[Server] socket accept failed\n");
		goto re_accept;
	}
	TYPE_COMMAND CMD;
	for (;;) {
		read(connection_fd, &CMD, sizeof(CMD));

		switch(CMD) {
		case CMD_SEND_DATABASE:
			printf("===========================================================\n");
			printf("[Server] Receiving Database...\n");
			recv_database(database_name, connection_fd);
			printf("[Server] Database RECEIVED!\n");
			printf("===========================================================\n\n");

			break;
		case CMD_ACCESS:
			printf("===========================================================\n");
			printf("[Server] Client Access Request...\n");
			access_response(database_name, log_name, connection_fd);
			printf("[Server] Client Access End!\n");
			printf("===========================================================\n\n");

			break;
		case CMD_DISCONNECT:
			printf("===========================================================\n");
			printf("[Server] Client Disconnect\n");
			printf("===========================================================\n\n");
			close(connection_fd);

			goto re_accept;
		}

		CMD = CMD_SUCCESS;
	}

}

static void recv_database(const string &file_name, int fd)
{
	FILE *database = fopen(file_name.c_str(), "wb");
	if (database == NULL) {
		printf("[recv_database] Can't open file %s %d\n", file_name.c_str(), errno);
		exit(1);
	}

	Block block;
	for (int i = 0; i < STORE_SIZE; i++) {
		int offset = 0;
		while (offset < sizeof(Block)) {
			int ret = read(fd, ((char *)&block) + offset, sizeof(Block) - offset);
			offset += ret;
		}
		fwrite(&block, 1, sizeof(Block), database);
	}

	TYPE_COMMAND CMD = CMD_SUCCESS;
	write(fd, &CMD, sizeof(CMD));

	fclose(database);
}

static void access_response(const string &database_name, const string &log_name, int fd)
{
	FILE *database = fopen(database_name.c_str(), "r+b");
	if (database == NULL) {
		printf("[Access] Can't open file %s\n", database_name.c_str());
		exit(1);
	}
	Log log(log_name);
	unsigned long int time;
	Stash stash;

	stash.server_recv_stash_index(fd);
	printf("[Access] Get stash index: %lld\n", stash.stash_index);

	log.start();
	for (int i = 0; i < STASH_SIZE; i++) {
		size_t file_offset = ((stash.stash_index + i * STEP) % STORE_SIZE) * sizeof(Block);
		fseek(database, file_offset, SEEK_SET);
		fread(stash.data + i, 1, sizeof(Block), database);
	}
	time = log.end();
	printf("[Access] Elapsed time for read stash from database: %lu\n", time);
	stash.server_send_stash(fd);
	printf("[Access] Send stash to client\n");
	stash.server_recv_stash(fd);
	log.start();
	for (int i = 0; i < STASH_SIZE; i++) {
		size_t file_offset = ((stash.stash_index + i * STEP) % STORE_SIZE) * sizeof(Block);
		fseek(database, file_offset, SEEK_SET);
		fwrite(stash.data + i, 1, sizeof(Block), database);
	}
	time = log.end();
	printf("[Access] Elapsed time for write stash to database: %lu\n", time);
	stash.server_send_success(fd);

	fclose(database);
}
