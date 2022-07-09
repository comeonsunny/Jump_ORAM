#include <cstdio>

#include "JUMPORAM/config.h"
#include "JUMPORAM/client.h"
#include "JUMPORAM/jump_oram.h"
#include "JUMPORAM/server.h"
#include "JUMPORAM/utils.h"

using namespace std;
static bool test_access(Client *client, int time);

int main()
{
	printf("How many bytes of block: %d\n", BLOCK_SIZE);
	printf("How many blocks of database: %d\n", DATABASE_SIZE);
	printf("How many blocks of cache: %d\n", CACHE_SIZE);
	printf("Size of Block: %zu\n", sizeof(Block));

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
	string name_prefix = to_string(STASH_SIZE) + "_" + to_string(BLOCK_SIZE);

	printf("[Main] Run Test\n\n");
	Client client(name_prefix + "_client", timestamp);
	client.load();

	if (!test_access(&client, 100)) {
		printf("[Test] test access err.\n");
	}

	return 0;
}

static bool test_access(Client *client, int time)
{
	Block *block;

	for (int i = 0; i < time; i++) {
		TYPE_INDEX idx = rand() % STORE_SIZE;
		block = client->access(idx);
		if (idx != block->virtual_index()) {
			return false;
		}
	}

	return true;
}
