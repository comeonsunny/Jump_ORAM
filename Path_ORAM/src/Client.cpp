#include "Client.hpp"
#include "Config.hpp"
#include "Socket.hpp"
#include "Block.hpp"
#include "Log.hpp"

#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
// achieve the constructor
Client::Client(std::string name_prefix, std::string timestamp) {
    // get the parameters of database
    this->block_size = BLOCK_SIZE;
    this->database_size = DATABASE_SIZE;
    this->block_num = BLOCK_NUM;
    this->level_num = LEVEL_NUM ; // the height of the tree is level_num 
    this->bucket_num =  BUCKET_NUM;
    this->bucket_leaf_num = BUCKET_LEAF_NUM;
    this->bucket_id_first_leaf = BUCKET_ID_FIRST_LEAF;
    this->block_num_in_bucket = BLOCK_NUM_IN_BUCKET;
    this->block_of_interest_buffer = new char[BLOCK_SIZE];
    // initialize the this-port by &PORT
    this->port = PORT;
    this->ip = (char *)IP;
    // initialize the names for database and log file
    this->position_map_name = METADATA_DIR + "/" + name_prefix + "_position_map";
    this->stash_name = METADATA_DIR + "/" + name_prefix +"_client_stash";
    this->cache_name = METADATA_DIR + "/" + name_prefix + "_client_cache";
    this->build_log_name = LOG_DIR + "/" + name_prefix + "_Jump_build_breakdown_log" + timestamp + ".txt";
    this->access_log_name = LOG_DIR + "/" + name_prefix + "_Jump_access_breakdown_log" + timestamp + ".txt";
    this->access_encrypted_log_name = LOG_DIR + "/" + name_prefix + "_Jump_access_encrypted_log_" + timestamp + ".txt";
    this->access_decrypted_log_name = LOG_DIR + "/" + name_prefix + "_Jump_access_decrypted_log_" + timestamp + ".txt";
    this->access_recvData_log_name = LOG_DIR + "/" + name_prefix + "_Jump_access_revcData_log_" + timestamp + ".txt";
    this->access_sendData_log_name = LOG_DIR + "/" + name_prefix + "_Jump_access_sendData_log_" + timestamp + ".txt";
    this->access_downToDisk_log_name = LOG_DIR + "/" + name_prefix + "_Jump_downToDisk_log_" + timestamp + ".txt";
    this->evict_encrypted_log_name = LOG_DIR + "/" + name_prefix + "_Jump_evict_encrypted_log_" + timestamp + ".txt";
    this->evict_decrypted_log_name = LOG_DIR + "/" + name_prefix + "_Jump_evict_decrypted_log_" + timestamp + ".txt";
    this->evict_recvData_log_name = LOG_DIR + "/" + name_prefix + "_Jump_evict_recvData_log_" + timestamp + ".txt";
    this->evict_sendData_log_name = LOG_DIR + "/" + name_prefix + "_Jump_evict_sendData_log_" + timestamp + ".txt";
    this->evict_downToDisk_log_name = LOG_DIR + "/" + name_prefix + "_Jump_evict_downToDisk_log_" + timestamp + ".txt";
}
// achieve the destructor
Client::~Client() {
    // write the vector of position_map to the file
    std::ofstream position_map_file(this->position_map_name, std::ios::binary);
    for (auto i : this->position_map) {
        position_map_file.write((char *)&i, sizeof(TYPE_INDEX));
    }
    position_map_file.close();
}
// a function could find the first parent node its visited value is less than this->block_num_in_bucket
TYPE_INDEX find_first_unfull_parent(TYPE_INDEX bucket_leaf_node_index, std::vector<int> &visited) {
    TYPE_INDEX parent_node_index = bucket_leaf_node_index;
    while (visited[parent_node_index] == BLOCK_NUM_IN_BUCKET) {
        parent_node_index = (parent_node_index - 1) / 2;
    }
    return parent_node_index;
}
// achieve the init()
int Client::init() {
    Log log(this->build_log_name);
    log.start();
    //step 1: use dummy blocks to fill up the bucket files
    std::fstream buckets_with_dummy_blocks;
    std::cout << "===========================================================" << std::endl;
    std::cout << "[Client]Initializing the database with dummy blocks..." << std::endl;
    std::cout << "===========================================================" << std::endl << std::endl;
    /* write the dummy blocks into the bucket files*/
    std::cout << "***********************************************************" << std::endl;
    for (TYPE_INDEX i = 0; i < bucket_num; ++i) {
        // show the running process of writing the dummy blocks into bucket files
        progress_bar(i, this->bucket_num, "Dummy blocks into bucket files");
        this->database_name = DATABASE_DIR + "/" + "client" + "/" + std::to_string(i) + ".db";
        buckets_with_dummy_blocks.open(database_name, std::ios::out | std::ios::binary);
        if (!buckets_with_dummy_blocks) {
            std::cerr << "[Client]Error: Cannot open the database file " << database_name << " to write." << std::endl;
            exit(1);
        }
        for (int j = 0; j < block_num_in_bucket; ++j) {
            Block dummy_block(this->block_num, this->block_size);
            dummy_block.encrypt();
            buckets_with_dummy_blocks.write((char *)dummy_block.get_iv(), IV_SIZE);
            buckets_with_dummy_blocks.write(dummy_block.get_data(), this->block_size);
        }
        buckets_with_dummy_blocks.close();
    }
    std::cout << std::endl;
    std::cout << "[Client] The elapsed time of generating the tree based database with dummy blocks is " << log.end() << " ns" << std::endl;
    std::cout << "*************************************************************" << std::endl << std::endl;
    // step 2: build the tree-shaped database
    log.start();
    /* create the vector of position_map and fill it with even distributed integers in [0, bucket_leaf_num)*/
    this->position_map.resize(block_num);
    for (TYPE_INDEX i = 0; i < block_num; ++i) {
        position_map[i] = i % bucket_leaf_num;
    }
    /* shuffle the position_map*/
    std::random_shuffle(position_map.begin(), position_map.end());
    /* write the position_map into the metadata*/
    std::fstream metadata_file;
    metadata_file.open(position_map_name, std::ios::out | std::ios::binary);
    metadata_file.write((char *)&position_map[0], sizeof(TYPE_INDEX) * block_num);
    metadata_file.close();
    std::cout << "[Client] The elapsed time of generating metadata is " << log.end() << " ns" << std::endl;
    /* input real block into the tree-shaped database according to the position_map*/
    log.start();
    std::vector<int> visited_buckets(bucket_num, 0);
    std::cout << "***********************************************************" << std::endl;
    for (TYPE_INDEX i = 0; i < block_num; ++i) {
        progress_bar(i, this->block_num, "Real blocks into bucket files");
        TYPE_INDEX bucket_leaf_node_index = bucket_id_first_leaf + position_map[i];
        TYPE_INDEX index = find_first_unfull_parent(bucket_leaf_node_index, visited_buckets);

        Block real_block(i, this->block_size);
        real_block.encrypt();

        this->database_name = DATABASE_DIR + "/" + "client" + "/" + std::to_string(index) + ".db";
        std::fstream bucket_file(database_name, std::ios::out | std::ios::binary | std::ios::in);
        if (!bucket_file) {
            std::cerr << "[Client]Error: Cannot open the database file " << database_name << " to write." << std::endl;
            return -1;
        }
        /* write the block into the crresponding bucket file*/
        
        // make the bucket_file point to the end of the file
        bucket_file.seekp(visited_buckets[index] * (block_size + IV_SIZE), std::ios::beg);
        bucket_file.write((char *)real_block.get_iv(), IV_SIZE);
        bucket_file.write(real_block.get_data(), this->block_size);
        bucket_file.close();
        /* update the visited_buckets*/
        ++visited_buckets[index];
    }
    std::cout << std::endl;
    std::cout << "[Client] The elapsed time of generating the tree based database with real blocks is " << log.end() << " ns" << std::endl;
    std::cout << "***********************************************************" << std::endl << std::endl;
    // step 3: send the tree-shaped database to the server
    log.start();
    /* connect to the server*/
    client_socket = new ClientSocket(this->ip, this->port);
    client_socket->connect_server();
    client_socket->send_command((char *)&CMD_SEND_DATABASE, sizeof(TYPE_COMMAND));
    TYPE_COMMAND cmd;
    client_socket->recv_command((char *)&cmd, sizeof(TYPE_COMMAND));
    if (cmd != CMD_CMD_OK) {
        std::cerr << "[Client]Error: The server does not accept the command(CMD_SEND_DATABASE)." << std::endl;
        return -1;
    }
    /* send the tree-shaped database to the server*/
    std::cout << "***********************************************************" << std::endl;
    for (TYPE_INDEX i = 0; i < bucket_num; ++i) {
        progress_bar(i, this->bucket_num, "Send database to the server");
        
        this->database_name = DATABASE_DIR + "/" + "client" + "/" + std::to_string(i) + ".db";
        std::fstream bucket_file(database_name, std::ios::in | std::ios::binary);
        if (!bucket_file) {
            std::cerr << "[Client]Error: Cannot open the database file " << database_name << " to read." << std::endl;
            return -1;
        }
        /* read the bucket file and send it to the server*/
        char *bucket_data = new char[block_num_in_bucket * (IV_SIZE + this->block_size)];
        bucket_file.read(bucket_data, block_num_in_bucket * (IV_SIZE + this->block_size));
        client_socket->send_data(bucket_data, block_num_in_bucket * (IV_SIZE + this->block_size));
        delete[] bucket_data;
        bucket_file.close();
    }
    std::cout << std::endl;
    std::cout << "***********************************************************" << std::endl << std::endl;
    /* receive the command from the server*/
    client_socket->recv_command((char *)&cmd, sizeof(TYPE_COMMAND));
    if (cmd != CMD_DATABASE_RECEIVED) {
        std::cerr << "[Client]Error: The server does not accept the data." << std::endl;
        return -1;
    }
    client_socket->close_client();
    delete client_socket;
    std::cout << "[Client] The elapsed time of sending the tree-shaped database to the server is " << log.end() << " ns" << std::endl;
    log.write_to_file();
    return 0;
}
// achieve the load() function
int Client::load() {
    this->position_map.resize(block_num);
    /* fill up the position_map by the content in the file of position_map_name */
    std::fstream metadata_file;
    metadata_file.open(position_map_name, std::ios::in | std::ios::binary);
    if (!metadata_file) {
        std::cerr << "[Client]Error: Cannot open the file " << position_map_name << " to read." << std::endl;
        return -1;
    }
    metadata_file.read((char *)&position_map[0], sizeof(TYPE_INDEX) * block_num);
    metadata_file.close();
    return 0;
}
// achieve the access(TYPE_INDEX index) function
char *Client::access(TYPE_INDEX index)
{
    Log log_encrypted(this->access_encrypted_log_name);
    Log log_decrypted(this->access_decrypted_log_name);
    Log log_recvData(this->access_recvData_log_name);
    Log log_sendData(this->access_sendData_log_name);
    Log log_downToDisk(this->access_downToDisk_log_name);
    Log log(this->access_log_name);
    log.start();
    /* check the index*/
    if (index >= this->block_num) {
        std::cerr << "[Client]Error: The index is out of range." << std::endl;
        exit(1);
    }
    /* translate the index into the physical id based on position map*/
    TYPE_INDEX physical_id = this->position_map[index];
    this->position_map[index] = rand() % this->bucket_leaf_num;
    std::cout << "[Client] The elapsed time of translating the index into the physical id is " << log.end() << " ns" << std::endl;
    /* connect to the server*/
    log.start();
    client_socket = new ClientSocket(this->ip, this->port);
    client_socket->connect_server();
    client_socket->send_command((char *)&CMD_ACCESS, sizeof(TYPE_COMMAND));
    TYPE_COMMAND cmd;
    client_socket->recv_command((char *)&cmd, sizeof(TYPE_COMMAND));
    if (cmd != CMD_CMD_OK) {
        std::cerr << "[Client]Error: The server does not accept the command(CMD_ACCESS)." << std::endl;
        exit(1);
    }
    client_socket->send_data((char *)&physical_id, sizeof(TYPE_INDEX));
    /* receive the command from the server*/
    client_socket->recv_command((char *)&cmd, sizeof(TYPE_COMMAND));
    if (cmd != CMD_PHYSICAL_ID) {
        std::cerr << "[Client]Error: The server does not accept the physical id(CMD_ACCESS)." << std::endl;
        exit(1);
    }
    /* receive the block from the server*/
    std::fstream path_file(stash_name, std::ios::out | std::ios::binary);
    char *bucket_data = new char[this->block_num_in_bucket * (IV_SIZE + this->block_size)];
    int flag_interest = 0; // flag_interest = 1 means the client's block of interest is in the path.
    for (TYPE_INDEX i = 0; i < level_num; ++i){
        log_recvData.start();
        client_socket->recv_data(bucket_data, this->block_num_in_bucket * (IV_SIZE + this->block_size));
        log_recvData.end();
        TYPE_INDEX block_id{this->block_num};
        for (int j = 0; j < block_num_in_bucket; ++j) {
            log_decrypted.start();
            Block block(this->block_size);
            block.set_iv((unsigned char *)(bucket_data + j * (IV_SIZE + this->block_size)));
            block.set_data(bucket_data + j * (IV_SIZE + this->block_size) + IV_SIZE);
            block.decrypt();
            log_decrypted.end();
            log_downToDisk.start();
            memcpy((char *)&block_id, block.get_data(), sizeof(TYPE_INDEX));
            if (block_id == index) {
                flag_interest = 1;
                std::cout << "***********************************************************" << std::endl;
                std::cout << "The block with index " << index << " is found." << std::endl;
                std::cout << "***********************************************************" << std::endl;
                memcpy(block_of_interest_buffer, block.get_data(), this->block_size);
                Block dummy_block(block_num, this->block_size);
                block.set_data(dummy_block.get_data());
            }
            path_file.write(block.get_data(), this->block_size);  // only need to write the plaintext data into stash file
            log_downToDisk.end();
        }
    }
    /* if the block of interest is not found in the retrieval path, go to the cache to exchange the block of interest to the dummy block in root bucket */
    if (flag_interest == 0) {
        std::cout << "***********************************************************" << std::endl;
        std::cout << "The block with index " << index << " is not found in the retrieval path." << std::endl;
        std::cout << "***********************************************************" << std::endl;
        std::fstream cache_file(cache_name, std::ios::in | std::ios::binary | std::ios::out);
        // get the size of cache file
        cache_file.seekg(0, std::ios::end);
        int cache_size = cache_file.tellg();
        if (cache_size == 0) {
            std::cerr << "[Client]Error: The cache is empty." << std::endl;
            exit(1);
        }
        cache_file.seekg(0, std::ios::beg);
        TYPE_INDEX block_num_in_cache = cache_size / (IV_SIZE + this->block_size);
        TYPE_INDEX block_id;
        for (TYPE_INDEX i = 0; i < block_num_in_cache; ++i) {
            cache_file.seekg(i * (IV_SIZE + this->block_size) + IV_SIZE, std::ios::beg);
            cache_file.read((char *)&block_id, sizeof(TYPE_INDEX));
            if (block_id == index) {
                cache_file.seekg(i * (IV_SIZE + this->block_size) + IV_SIZE, std::ios::beg);
                cache_file.read(block_of_interest_buffer, this->block_size);
                std::cout << "***********************************************************" << std::endl;
                std::cout << "The block with index " << index << " is found in the cache." << std::endl;
                std::cout << "***********************************************************" << std::endl;
                Block dummy_block(block_num, this->block_size);
                cache_file.seekg(i * (IV_SIZE + this->block_size) + IV_SIZE, std::ios::beg);
                cache_file.write(dummy_block.get_data(), this->block_size);
                cache_file.close();
                break;
            }
        }
    }
    path_file.close();
    client_socket->send_command((char *)&CMD_DATA_OK, sizeof(TYPE_COMMAND));
    std::cout << "[Client] The elapsed time of receiving the path blocks from the server is " << log.end() << " ns" << std::endl;
    // try to put the real block in path close to the leaf node of the tree and send the re-encrypted blocks to the server
    log.start();
    std::fstream path_file_2(stash_name, std::ios::in | std::ios::binary);
    if (!path_file_2) {
        std::cerr << "[Client]Error: Cannot open the stash file " << stash_name << " to read and write." << std::endl;
        exit(1);
    }
    bool flag = false; // flag to indicate whether the block of interest has been inserted into the root node
    for (TYPE_INDEX i = 0; i < level_num; ++i) {
        /* read the bucket file and send it to the server*/
        Block re_encrypted_block(this->block_size);
        for (int j = 0; j < block_num_in_bucket; ++j) {
            log_downToDisk.start();
            char *block_data = new char[this->block_size];
            path_file_2.seekg(i * this->block_num_in_bucket * this->block_size + j * this->block_size, std::ios::beg);
            path_file_2.read(block_data, this->block_size);
            // insert the block of interest into the root node
            if (i == level_num - 1 && flag == false) {
                TYPE_INDEX block_id{0};
                memcpy((char *)&block_id, block_data, sizeof(TYPE_INDEX));
                if (block_id == block_num){
                    memcpy(block_data, block_of_interest_buffer, this->block_size);
                    flag = true;
                }
            }
            log_downToDisk.end();
            log_encrypted.start();
            re_encrypted_block.set_data(block_data);
            re_encrypted_block.encrypt();
            log_encrypted.end();
            memcpy(bucket_data + j * (IV_SIZE + this->block_size), re_encrypted_block.get_iv(), IV_SIZE);
            memcpy(bucket_data + j * (IV_SIZE + this->block_size) + IV_SIZE, re_encrypted_block.get_data(), this->block_size);
            delete[] block_data;
        }
        log_sendData.start();
        client_socket->send_data(bucket_data, this->block_num_in_bucket * (IV_SIZE + this->block_size));
        log_sendData.end();
    }
    path_file_2.close();
    if (flag == false) {
        std::cerr << "[Client]Error: The block of interest cannot be inserted into the root node." << std::endl;
        exit(1);
    }
    client_socket->recv_command((char *)&cmd, sizeof(TYPE_COMMAND));
    if (cmd != CMD_DATA_OK) {
        std::cerr << "[Client]Error: The server does not accept the data(CMD_DATA_OK)." << std::endl;
        exit(1);
    }
    client_socket->close_client();
    delete client_socket;
    delete[] bucket_data;
    std::cout << "[Client] The elapsed time of sending the re-encrypted path blocks to the server is " << log.end() << " ns" << std::endl;
    log.write_to_file();
    log_downToDisk.write_to_file();
    log_encrypted.write_to_file();
    log_decrypted.write_to_file();
    log_sendData.write_to_file();
    log_recvData.write_to_file();
    return block_of_interest_buffer;
}
// achieve the evict() function
int Client::evict() {
    Log log_evict_encrypted(this->evict_encrypted_log_name);
    Log log_evict_decrypted(this->evict_decrypted_log_name);
    Log log_evict_recvData(this->evict_recvData_log_name);
    Log log_evict_sendData(this->evict_sendData_log_name);
    Log log_evict_downToDisk(this->evict_downToDisk_log_name);
    std::cout << "***********************************************************" << std::endl;
    std::cout << "The client is starting to do eviction operation." << std::endl;
    std::cout << "***********************************************************" << std::endl;
    log_evict_recvData.start();
    client_socket = new ClientSocket(this->ip, this->port);
    client_socket->connect_server();
    /* send the command to the server*/
    client_socket->send_command((char *)&CMD_EVICT, sizeof(TYPE_COMMAND));
    /* receive the corresponding command from the client*/
    TYPE_COMMAND cmd;
    client_socket->recv_command((char *)&cmd, sizeof(TYPE_COMMAND)); 
    if (cmd != CMD_CMD_OK){
        std::cerr << "[Client]Error: The server does not accept the command(CMD_EVICT)." << std::endl;
        exit(1);
    }
    log_evict_recvData.end();
    char *bucket_buffer[3];
    for (int i = 0; i < 3; ++i) {
        bucket_buffer[i] = new char[this->block_num_in_bucket * (IV_SIZE + this->block_size)];
    }
    for (TYPE_INDEX i = 0; i < this->level_num - 1; ++i) {  // remember to check the index of child node of the bucket_id in the server
        int flag {-1};
        TYPE_INDEX bucket_id{0};
        if (i == 0){
            client_socket->send_command((char *)&bucket_id, sizeof(TYPE_INDEX));
            for (int j = 0; j < 3; ++j) {
                log_evict_recvData.start();
                client_socket->recv_data(bucket_buffer[j], this->block_num_in_bucket * (IV_SIZE + this->block_size));
                log_evict_recvData.end();
                /* decrypt the bucket data*/
                for (int k = 0; k < this->block_num_in_bucket; ++k) {
                    log_evict_decrypted.start();
                    Block block(this->block_size);
                    memcpy((char *)block.get_iv(), bucket_buffer[j] + k * (IV_SIZE + this->block_size), IV_SIZE);
                    memcpy(block.get_data(), bucket_buffer[j] + k * (IV_SIZE + this->block_size) + IV_SIZE, this->block_size);
                    block.decrypt();
                    memcpy(bucket_buffer[j] + k * (IV_SIZE + this->block_size) + IV_SIZE, block.get_data(), this->block_size);
                    log_evict_decrypted.end();               
                }
            }
            client_socket->send_command((char *)&CMD_DATA_OK, sizeof(TYPE_COMMAND));
            flag = evict_from_1_to_2(bucket_buffer, i, bucket_id);
            // if flag == 1, fill up the bucket[0] with dummy blocks
            if (flag == 1){
                // write the whole bucket_buffer[0] into the cache_name file
                log_evict_downToDisk.start();
                std::fstream cache_file(this->cache_name, std::ios::out | std::ios::binary | std::ios::app);
                if (!cache_file) {
                    std::cerr << "[Client]Error: Cannot open the cache file " << this->cache_name << " to write." << std::endl;
                    exit(1);
                }
                cache_file.write(bucket_buffer[0], this->block_num_in_bucket * (IV_SIZE + this->block_size));
                cache_file.close();

                for (int m = 0; m < this->block_num_in_bucket; ++m) {
                    Block block(this->block_num, this->block_size);
                    memcpy(bucket_buffer[0] + m * (IV_SIZE + this->block_size) + IV_SIZE, block.get_data(), this->block_size);
                }
                log_evict_downToDisk.end();
            }
            for (int j = 0; j < 3; ++j) {
                /* encrypt the bucket data*/
                for (int k = 0; k < this->block_num_in_bucket; ++k) {
                    log_evict_encrypted.start();
                    Block block(this->block_size);
                    memcpy((char *) block.get_data(), bucket_buffer[j] + k * (IV_SIZE + this->block_size) + IV_SIZE, this->block_size);
                    block.encrypt();
                    memcpy(bucket_buffer[j] + k * (IV_SIZE + this->block_size), block.get_iv(), IV_SIZE);
                    memcpy(bucket_buffer[j] + k * (IV_SIZE + this->block_size) + IV_SIZE, block.get_data(), this->block_size);
                    log_evict_encrypted.end();
                }
                log_evict_sendData.start();
                client_socket->send_data(bucket_buffer[j], this->block_num_in_bucket * (IV_SIZE + this->block_size));
                log_evict_sendData.end();
                
            }
            client_socket->recv_command((char *)&cmd, sizeof(TYPE_COMMAND));
            if (cmd != CMD_DATA_OK){
                std::cerr << "[Client]Error: The server does not accept the data(CMD_DATA_OK)." << std::endl;
                exit(1);
            }
        } else if (i == 1){
            for (int k_ = 0; k_ < 2; ++k_) {
                bucket_id = k_ + 1;
                client_socket->send_command((char *)&bucket_id, sizeof(TYPE_INDEX));
                for (int j = 0; j < 3; ++j) {
                    log_evict_recvData.start();
                    client_socket->recv_data(bucket_buffer[j], this->block_num_in_bucket * (IV_SIZE + this->block_size));
                    log_evict_recvData.end();
                    /* decrypt the bucket data*/
                    for (int k = 0; k < this->block_num_in_bucket; ++k) {
                        log_evict_decrypted.start();
                        Block block(this->block_size);  
                        memcpy((char *)block.get_iv(), bucket_buffer[j] + k * (IV_SIZE + this->block_size), IV_SIZE);
                        memcpy(block.get_data(), bucket_buffer[j] + k * (IV_SIZE + this->block_size) + IV_SIZE, this->block_size);
                        block.decrypt();
                        memcpy(bucket_buffer[j] + k * (IV_SIZE + this->block_size) + IV_SIZE, block.get_data(), this->block_size);
                        log_evict_decrypted.end();
                    }
                }
                client_socket->send_command((char *)&CMD_DATA_OK, sizeof(TYPE_COMMAND));
                flag = evict_from_1_to_2(bucket_buffer, i, bucket_id);
                // if flag == 1, fill up the bucket[0] with dummy blocks
                if (flag == 1){
                    log_evict_downToDisk.start();
                    // write the whole bucket_buffer[0] into the cache_name file 
                    std::fstream cache_file(this->cache_name, std::ios::out | std::ios::binary | std::ios::app);
                    if (!cache_file) {
                        std::cerr << "[Client]Error: Cannot open the cache file " << this->cache_name << " to write." << std::endl;
                        exit(1);
                    }
                    cache_file.write(bucket_buffer[0], this->block_num_in_bucket * (IV_SIZE + this->block_size));
                    cache_file.close();
                    for (int m = 0; m < this->block_num_in_bucket; ++m) {
                        Block block(this->block_num, this->block_size);
                        memcpy(bucket_buffer[0] + m * (IV_SIZE + this->block_size) + IV_SIZE, block.get_data(), this->block_size);
                    }
                    log_evict_downToDisk.end();
                }
                for (int j = 0; j < 3; ++j) {
                    /* encrypt the bucket data*/
                    for (int k = 0; k < this->block_num_in_bucket; ++k) {
                        log_evict_encrypted.start();
                        Block block(this->block_size);
                        memcpy((char *) block.get_data(), bucket_buffer[j] + k * (IV_SIZE + this->block_size) + IV_SIZE, this->block_size);
                        block.encrypt();
                        memcpy(bucket_buffer[j] + k * (IV_SIZE + this->block_size), block.get_iv(), IV_SIZE);
                        memcpy(bucket_buffer[j] + k * (IV_SIZE + this->block_size) + IV_SIZE, block.get_data(), this->block_size);
                        log_evict_encrypted.end();
                    }
                    log_evict_sendData.start();
                    client_socket->send_data(bucket_buffer[j], this->block_num_in_bucket * (IV_SIZE + this->block_size));
                    log_evict_sendData.end();
                }
                client_socket->recv_command((char *)&cmd, sizeof(TYPE_COMMAND));
                if (cmd != CMD_DATA_OK){
                    std::cerr << "[Client]Error: The server does not accept the data(CMD_DATA_OK)." << std::endl;
                    exit(1);
                }
            }
        } else {
            for (int k_ = 0; k_ < 2; ++k_) {
                // the bucket_id is a random number range from (pow(2, i) - 1) to (pow(2, i + 1)-1)
                bucket_id = rand() % (TYPE_INDEX)(pow(2, i + 1) - pow(2, i)) + (TYPE_INDEX)pow(2, i) - 1;
                client_socket->send_command((char *)&bucket_id, sizeof(TYPE_INDEX));
                for (int j = 0; j < 3; ++j) {
                    log_evict_recvData.start();
                    client_socket->recv_data(bucket_buffer[j], this->block_num_in_bucket * (IV_SIZE + this->block_size));
                    log_evict_recvData.end();
                    /* decrypt the bucket data*/
                    log_evict_decrypted.start();
                    for (int k = 0; k < this->block_num_in_bucket; ++k) {
                        Block block(this->block_size);
                        memcpy((char *)block.get_iv(), bucket_buffer[j] + k * (IV_SIZE + this->block_size), IV_SIZE);
                        memcpy(block.get_data(), bucket_buffer[j] + k * (IV_SIZE + this->block_size) + IV_SIZE, this->block_size);
                        block.decrypt();
                        memcpy(bucket_buffer[j] + k * (IV_SIZE + this->block_size) + IV_SIZE, block.get_data(), this->block_size);
                    }
                    log_evict_decrypted.end();
                }
                client_socket->send_command((char *)&CMD_DATA_OK, sizeof(TYPE_COMMAND));
                flag = evict_from_1_to_2(bucket_buffer, i, bucket_id);
                // if flag == 1, fill up the bucket[0] with dummy blocks
                if (flag == 1){
                    // write the whole bucket_buffer[0] into the cache_name file
                    log_evict_downToDisk.start();
                    std::fstream cache_file(this->cache_name, std::ios::out | std::ios::binary | std::ios::app);
                    if (!cache_file) {
                        std::cerr << "[Client]Error: Cannot open the cache file " << this->cache_name << " to write." << std::endl;
                        exit(1);
                    }
                    cache_file.write(bucket_buffer[0], this->block_num_in_bucket * (IV_SIZE + this->block_size));
                    cache_file.close();

                    for (int m = 0; m < this->block_num_in_bucket; ++m) {
                        Block block(this->block_num, this->block_size);
                        memcpy(bucket_buffer[0] + m * (IV_SIZE + this->block_size) + IV_SIZE, block.get_data(), this->block_size);
                    }
                    log_evict_downToDisk.end();
                }
                for (int j = 0; j < 3; ++j) {
                    /* encrypt the bucket data*/
                    for (int k = 0; k < this->block_num_in_bucket; ++k) {
                        log_evict_encrypted.start();
                        Block block(this->block_size);
                        memcpy((char *) block.get_data(), bucket_buffer[j] + k * (IV_SIZE + this->block_size) + IV_SIZE, this->block_size);
                        block.encrypt();
                        memcpy(bucket_buffer[j] + k * (IV_SIZE + this->block_size), block.get_iv(), IV_SIZE);
                        memcpy(bucket_buffer[j] + k * (IV_SIZE + this->block_size)+ IV_SIZE, block.get_data(), this->block_size);
                        log_evict_encrypted.end();
                    }
                    log_evict_sendData.start();
                    client_socket->send_data(bucket_buffer[j], this->block_num_in_bucket * (IV_SIZE + this->block_size));
                    log_evict_sendData.end();
                }
                client_socket->recv_command((char *)&cmd, sizeof(TYPE_COMMAND));
                if (cmd != CMD_DATA_OK){
                    std::cerr << "[Client]Error: The server does not accept the data(CMD_DATA_OK)." << std::endl;
                    exit(1);
                }
            }
        }
    }
    std::cout << "----------------------------------------------------" << std::endl;
    std::cout << "[Client]Info: The client has successfully done the eviction." << std::endl;
    std::cout << "----------------------------------------------------" << std::endl << std::endl;
    for (int i = 0; i < 3; ++i) {
        delete[] bucket_buffer[i];
    }
    client_socket->close_client();
    delete client_socket;
    log_evict_encrypted.write_to_file();
    log_evict_decrypted.write_to_file();
    log_evict_recvData.write_to_file();
    log_evict_sendData.write_to_file();
    log_evict_downToDisk.write_to_file();
    return 0;
}
// achieve the evict_from_1_to_2 function
int Client::evict_from_1_to_2(char *bucket_buffer[], TYPE_INDEX level, TYPE_INDEX parent_index){
    int cnt{0}; // log the conserved real block number in the bucket_buffer[0]
    TYPE_INDEX left_child_index = 2 * parent_index + 1;
    TYPE_INDEX right_child_index = 2 * parent_index + 2;
    TYPE_INDEX left_bound_for_left_child{0}, right_bound_for_left_child{0};
    TYPE_INDEX left_bound_for_right_child{0}, right_bound_for_right_child{0};
    left_right_boundary(level + 1, left_child_index, left_bound_for_left_child, right_bound_for_left_child);
    left_right_boundary(level + 1, right_child_index, left_bound_for_right_child, right_bound_for_right_child);
    /* check the path id of real block number in the bucket_buffer[0] and evict them to corresponding bucket_buffer[1] or bucket_buffer[2] */
    TYPE_INDEX left_logical_id = *(TYPE_INDEX *)(bucket_buffer[1] + IV_SIZE);
    TYPE_INDEX right_logical_id = *(TYPE_INDEX *)(bucket_buffer[2] + IV_SIZE);
    int left_flag = 0, right_flag = 0;
    for (int i = 0; i < this->block_num_in_bucket; ++i) {
        TYPE_INDEX logical_id = *(TYPE_INDEX *)(bucket_buffer[0] + i * (IV_SIZE + this->block_size) + IV_SIZE);
        char tmp_buffer[IV_SIZE + this->block_size];
        char *tmp_data_buffer = tmp_buffer;
        if (logical_id >= 0 && logical_id < this-> block_num) {
            ++cnt;
            TYPE_INDEX path_id = this->position_map[logical_id];
            if (path_id >= left_bound_for_left_child && path_id <= right_bound_for_left_child){
                while (left_logical_id != block_num && left_flag < this->block_num_in_bucket - 1) {
                    ++left_flag;
                    left_logical_id = *(TYPE_INDEX *)(bucket_buffer[1] + left_flag * (IV_SIZE + this->block_size) + IV_SIZE);
                }
                if (((left_flag == (this->block_num_in_bucket - 1)) && (left_logical_id != block_num)) || left_flag >= this->block_num_in_bucket){
                    ++left_flag;
                    continue;
                }
                // exchange the bucket_buffer[0] and bucket_buffer[1]
                memcpy(tmp_data_buffer, bucket_buffer[0] + i * (IV_SIZE + this->block_size), IV_SIZE + this->block_size);
                memcpy(bucket_buffer[0] + i * (IV_SIZE + this->block_size), bucket_buffer[1] + left_flag * (IV_SIZE + this->block_size), IV_SIZE + this->block_size);
                memcpy(bucket_buffer[1] + left_flag * (IV_SIZE + this->block_size), tmp_data_buffer, IV_SIZE + this->block_size);
                //delete[] tmp_data_buffer;
                ++left_flag;
                left_logical_id = *(TYPE_INDEX *)(bucket_buffer[1] + left_flag * (IV_SIZE + this->block_size) + IV_SIZE);
                --cnt;
            } else if (path_id >= left_bound_for_right_child && path_id <= right_bound_for_right_child){
                while (right_logical_id != block_num && right_flag < this->block_num_in_bucket - 1) {
                    ++right_flag;
                    right_logical_id = *(TYPE_INDEX *)(bucket_buffer[2] + right_flag * (IV_SIZE + this->block_size) + IV_SIZE);
                }
                if (((right_flag == (this->block_num_in_bucket - 1)) && (right_logical_id != block_num)) || right_flag >= this->block_num_in_bucket){
                    ++right_flag;
                    continue;
                }
                // exchange the bucket_buffer[0] and bucket_buffer[2]
                memcpy(tmp_data_buffer, bucket_buffer[0] + i * (IV_SIZE + this->block_size), IV_SIZE + this->block_size);
                memcpy(bucket_buffer[0] + i * (IV_SIZE + this->block_size), bucket_buffer[2] + right_flag * (IV_SIZE + this->block_size), IV_SIZE + this->block_size);
                memcpy(bucket_buffer[2] + right_flag * (IV_SIZE + this->block_size), tmp_data_buffer, IV_SIZE + this->block_size);
                //delete[] tmp_data_buffer;
                ++right_flag;
                right_logical_id = *(TYPE_INDEX *)(bucket_buffer[2] + right_flag * (IV_SIZE + this->block_size) + IV_SIZE);
                --cnt;
            } else {
                std::cerr << "[Client]Error: The logical id " << logical_id << " is not in the bucket " << parent_index << "." << std::endl;
                std::cerr << "[Client]Error: The path id " << path_id << " is not in the bucket " << parent_index << "." << std::endl;
                std::cout << "[Client]Error: The level is " << level << "." << std::endl;
                //exit(1);
            }
        }
    }
    if (cnt != this->block_num_in_bucket) {
        return 0;
    }
    return 1;
}
// achieve the left_right_boundary function
void Client::left_right_boundary(TYPE_INDEX next_level, TYPE_INDEX node_index, TYPE_INDEX &left_boundary, TYPE_INDEX &right_boundary){
    TYPE_INDEX level_node_num = pow(2, next_level);
    TYPE_INDEX level_first_node_id = pow(2, next_level) - 1;
    TYPE_INDEX step = this->bucket_leaf_num / level_node_num;
    left_boundary = (node_index - level_first_node_id) * step;
    right_boundary = left_boundary + step - 1;
}
