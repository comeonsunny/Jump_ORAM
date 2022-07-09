#include "jump_oram.h"
#include <unistd.h>
#include <tomcrypt.h>

void Block::init()
{
	if (-1 == register_cipher(&aes_desc)) {
		printf("[Block] Register cipher err\n");
		exit(-1);
	}
}

void Block::encrypt()
{
	/* Generate random IV */
	for (int i = 0; i < IV_SIZE; i += sizeof(int)) {
		int random_int = rand();
		memcpy(iv + i, &random_int, sizeof(int));
	}
	if (IV_SIZE / sizeof(int) != 0) {
		int random_int = rand();
		memcpy(iv + IV_SIZE - sizeof(int), &random_int, sizeof(int));
	}

	int err = 0;
	symmetric_CTR ctr;
	TYPE_DATA cripter_data[DATA_CHUNKS];

	/* encrypt */
	if ((err =  ctr_start(find_cipher("aes"), iv, ENCRYPT_KEY, sizeof(ENCRYPT_KEY), 0,
				CTR_COUNTER_LITTLE_ENDIAN, &ctr)) != CRYPT_OK) {
		printf("[Block] ctr_start err: %s\n", error_to_string(err));
		exit(-1);
	}
	if ((err = ctr_encrypt((unsigned char *)data, (unsigned char *)cripter_data,
				BLOCK_SIZE, &ctr)) != CRYPT_OK) {
		printf("[Block] ctr_encrypt err: %s\n", error_to_string(err));
		exit(-1);
	}
	if ((err = ctr_done(&ctr)) != CRYPT_OK) {
		printf("[Block] ctr_done err: %s\n", error_to_string(err));
		exit(-1);
	}

	memcpy(data, cripter_data, BLOCK_SIZE);
}

void Block::decrypt()
{
	int err;
	symmetric_CTR ctr;
	TYPE_DATA cripter_data[DATA_CHUNKS];

	if ((err =  ctr_start(find_cipher("aes"), iv, ENCRYPT_KEY, sizeof(ENCRYPT_KEY), 0,
				CTR_COUNTER_LITTLE_ENDIAN, &ctr)) != CRYPT_OK) {
		printf("[Block] ctr_start err: %s\n", error_to_string(err));
		exit(-1);
	}
	if ((err = ctr_decrypt((unsigned char *)data, (unsigned char *)cripter_data,
				BLOCK_SIZE, &ctr)) != CRYPT_OK) {
		printf("[Block] ctr_encrypt err: %s\n", error_to_string(err));
		exit(-1);
	}
	if ((err = ctr_done(&ctr)) != CRYPT_OK) {
		printf("[Block] ctr_done err: %s\n", error_to_string(err));
		exit(-1);
	}

	memcpy(data, cripter_data, BLOCK_SIZE);
}

void Stash::client_send_stash_index(int fd)
{
	write(fd, &stash_index, sizeof(stash_index));
}

void Stash::client_recv_stash(int fd)
{
	int offset = 0;
	while (offset < sizeof(Block) * STASH_SIZE) {
		int ret = read(fd, ((char *)data) + offset, sizeof(Block) * STASH_SIZE - offset);
		offset += ret;
	}
}

void Stash::client_send_stash(int fd)
{
	write(fd, data, sizeof(Block) * STASH_SIZE);
}

void Stash::client_recv_success(int fd)
{
	TYPE_COMMAND CMD;
	read(fd, &CMD, sizeof(CMD));
}

void Stash::server_recv_stash_index(int fd)
{
	read(fd, &stash_index, sizeof(stash_index));
}

void Stash::server_send_stash(int fd)
{
	write(fd, data, sizeof(Block) * STASH_SIZE);
}

void Stash::server_recv_stash(int fd)
{
	int offset = 0;
	while (offset < sizeof(Block) * STASH_SIZE) {
		int ret = read(fd, ((char *)data) + offset, sizeof(Block) * STASH_SIZE - offset);
		offset += ret;
	}
}

void Stash::server_send_success(int fd)
{
	write(fd, &CMD_SUCCESS, sizeof(CMD_SUCCESS));
}

TYPE_INDEX Cache::find_index(TYPE_INDEX v_index)
{
	for (TYPE_INDEX i = 0; i < CACHE_SIZE; i++)
		if (v_index == virtual_index[i])
			return i;
	return -1;
}
