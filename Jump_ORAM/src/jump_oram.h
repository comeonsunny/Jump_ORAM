#ifndef JUMPORAM_JUMP_ORAM_H_
#define JUMPORAM_JUMP_ORAM_H_

#include <vector>
#include "config.h"

class Block {
public:
	Block() { Block(-1); }
	Block(const TYPE_INDEX virtual_index) {
		data[0] = virtual_index;
		data[DATA_CHUNKS-1] = 0;
	}

	/* Must be called in main */
	static void init();

	TYPE_INDEX virtual_index() {
		return data[0];
	}

	void access() {
		data[DATA_CHUNKS-1] += 1;
	}
	int access_times() {
		return data[DATA_CHUNKS-1];
	}

	void encrypt();
	void decrypt();
private:
	unsigned char iv[IV_SIZE];
	TYPE_INDEX data[DATA_CHUNKS];
};

struct Stash {
	TYPE_INDEX stash_index;
	Block data[STASH_SIZE];

	/* Client */
	void client_send_stash_index(int fd);
	void client_recv_stash(int fd);
	void client_send_stash(int fd);
	void client_recv_success(int fd);

	/* Server */
	void server_recv_stash_index(int fd);
	void server_send_stash(int fd);
	void server_recv_stash(int fd);
	void server_send_success(int fd);

	void server_read_from_disk(const std::string &name);
	void server_write_to_disk(const std::string &name);
};

struct Cache {
	TYPE_INDEX virtual_index[CACHE_SIZE];
	Block data[CACHE_SIZE];

	TYPE_INDEX find_index(TYPE_INDEX v_index);
};

#endif // JUMPORAM_JUMP_ORAM_H
