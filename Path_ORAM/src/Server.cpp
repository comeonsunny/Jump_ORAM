#include "Socket.hpp"
#include "Config.hpp"
#include "Server.hpp"

#include <iostream>
#include <fstream>
#include <vector>
// constructor
Server::Server(std::string name_prefix, std::string timestamp) {
    this->block_size = BLOCK_SIZE;
    this->database_size = DATABASE_SIZE;
    this->block_num = BLOCK_NUM;
    this->level_num = LEVEL_NUM;
    this->bucket_num = BUCKET_NUM;
    this->bucket_leaf_num = BUCKET_LEAF_NUM;
    this->bucket_id_first_leaf = BUCKET_ID_FIRST_LEAF;
    this->block_num_in_bucket = BLOCK_NUM_IN_BUCKET;
    this->port = PORT;
    server_socket = new ServerSocket(this->port);
}
// destructor
Server::~Server() {
    delete server_socket;
}
// launch the server
void Server::launch() {
        while (true)
        {
            std::cout << "================================================================================" << std::endl;
            std::cout << "[Server] Waiting for client to connect..." << std::endl;
            std::cout << "================================================================================" << std::endl;
            this->client_fd = server_socket->accept_client();
            if (this->client_fd == -1) {
               std::cout << "[Server]Error: accept_client() failed" << std::endl;
                exit(1);
            }
            std::cout << "================================================================================" << std::endl;
            std::cout << "[Server] Client connected" << std::endl;
            std::cout << "================================================================================" << std::endl;
            TYPE_COMMAND command;
            server_socket->recv_command(client_fd, (char *)&command, sizeof(TYPE_COMMAND));
            server_socket->send_command(client_fd, (char *)&CMD_CMD_OK, sizeof(TYPE_COMMAND));
            switch (command) 
            {
                case CMD_SEND_DATABASE:
                std::cout << "================================================================================" << std::endl;
                std::cout << "================================================================================" << std::endl;
                std::cout << "[Server]Receive command CMD_SEND_DATABASE and start receiving ..." << std::endl;
                std::cout << "================================================================================" << std::endl;
                std::cout << "================================================================================" << std::endl << std::endl;
                // receive the database from the client
                this->receive_database();
                break;
                
                case CMD_ACCESS:
                std::cout << "================================================================================" << std::endl;
                std::cout << "================================================================================" << std::endl;
                std::cout << "[Server]Receive command CMD_ACCESS and start sending path ..." << std::endl;
                std::cout << "================================================================================" << std::endl;
                std::cout << "================================================================================" << std::endl << std::endl;
                // access the path corresponding to the block of interest 
                this->access_path();
                break;

                case CMD_EVICT:
                std::cout << "================================================================================" << std::endl;
                std::cout << "================================================================================" << std::endl;
                std::cout << "[Server]Receive command CMD_EVICT and start evicting ..." << std::endl;
                std::cout << "================================================================================" << std::endl;
                std::cout << "================================================================================" << std::endl << std::endl;
                // evict the level of the database
                this->evict_level();
                std::cout << "================================================================================" << std::endl;
                std::cout << "================================================================================" << std::endl;
                std::cout << "[Server]Finish evicting" << std::endl;
                std::cout << "================================================================================" << std::endl;
                std::cout << "================================================================================" << std::endl << std::endl;
                break;

                default:
                std::cout << "================================================================================" << std::endl;
                std::cout << "================================================================================" << std::endl;
                std::cout << "[Server]Receive command CMD_UNKNOWN and start sending error message ..." << std::endl;
                std::cout << "================================================================================" << std::endl;
                std::cout << "================================================================================" << std::endl << std::endl;
                break;
            }
        }
}
void Server::receive_database() 
{
    // receive the database from the client
    std::cout << "********************************************************************************" << std::endl;
    for (TYPE_INDEX i = 0; i < this->bucket_num; i++) {
        progress_bar(i, this->bucket_num, "Receiving database");
        char *bucket_buffer = new char[block_num_in_bucket * (IV_SIZE + block_size)];
        server_socket->recv_data(client_fd, bucket_buffer, block_num_in_bucket * (IV_SIZE + block_size));
        this->database_name = DATABASE_DIR + "/" + "server" + "/" +  std::to_string(i) + ".db";
        std::fstream database_file;
        database_file.open(this->database_name, std::ios::out | std::ios::binary);
        database_file.write(bucket_buffer, block_num_in_bucket * (IV_SIZE + block_size));
        database_file.close();
        delete[] bucket_buffer;
    }
    std::cout << std::endl;
    std::cout << "********************************************************************************" << std::endl << std::endl;
    
    std::cout << "================================================================================" << std::endl;
    std::cout << "================================================================================" << std::endl;
    std::cout << "[Server] Finish writing database to file" << std::endl;
    std::cout << "================================================================================" << std::endl;
    std::cout << "================================================================================" << std::endl << std::endl;
    server_socket->send_command(client_fd, (char *)&CMD_DATABASE_RECEIVED, sizeof(TYPE_COMMAND));
    server_socket->close_client(client_fd);
}

void Server::access_path()
{
    // receive the path id from the client
    TYPE_INDEX path_id;
    server_socket->recv_command(client_fd, (char *)&path_id, sizeof(TYPE_INDEX));
    server_socket->send_command(client_fd, (char *)&CMD_PHYSICAL_ID, sizeof(TYPE_COMMAND));
    // read the path buckets corresponding to the path id and send it to the client 
    std::cout << "********************************************************************************" << std::endl;
    std::cout << "********************************************************************************" << std::endl;
    std::cout << "[Server]Receive path id and start sending path ..." << std::endl;
    std::cout << "********************************************************************************" << std::endl;
    std::cout << "********************************************************************************" << std::endl << std::endl;
    char *path_bucket_buffer = new char[block_num_in_bucket * (IV_SIZE + block_size)];
    TYPE_INDEX bucket_id = this->bucket_id_first_leaf + path_id;
    std::cout << "********************************************************************************" << std::endl;
    for (TYPE_INDEX i = 0; i < level_num; i++) {
        progress_bar(i, level_num, "Sending path");
        this->database_name = DATABASE_DIR + "/" + "server" + "/" + std::to_string(bucket_id) + ".db";
        std::fstream path_file(this->database_name, std::ios::in | std::ios::binary);
        path_file.read(path_bucket_buffer, block_num_in_bucket * (IV_SIZE + block_size));
        server_socket->send_data(client_fd, path_bucket_buffer, block_num_in_bucket * (IV_SIZE + block_size));
        bucket_id = bucket_id == 0 ? 0 : (bucket_id - 1) / 2;
        path_file.close();
    }
    std::cout << std::endl;
    std::cout << "********************************************************************************" << std::endl;
    std::cout << "********************************************************************************" << std::endl;
    std::cout << "[Server] Finish sending path" << std::endl;
    std::cout << "********************************************************************************" << std::endl;
    std::cout << "********************************************************************************" << std::endl << std::endl;
    TYPE_COMMAND command;
    server_socket->recv_command(client_fd, (char *)&command, sizeof(TYPE_COMMAND));
    if (command != CMD_DATA_OK){
        std::cerr << "[Server]Error: client did not receive the path buckets successfully" << std::endl;
        exit(1);
    }
    // start receiving the path buckets from the client and write them to the original path
    std::cout << "********************************************************************************" << std::endl;
    std::cout << "********************************************************************************" << std::endl;
    std::cout << "[Server]Receive path buckets and start writing path to file" << std::endl;
    std::cout << "********************************************************************************" << std::endl;
    std::cout << "********************************************************************************" << std::endl << std::endl;
    bucket_id = this->bucket_id_first_leaf + path_id;
    for (TYPE_INDEX i = 0; i < level_num; i++) {
        progress_bar(i, level_num, "Receiving path");
        this->database_name = DATABASE_DIR + "/" + "server" + "/" + std::to_string(bucket_id) + ".db";
        std::fstream path_file(this->database_name, std::ios::out | std::ios::binary);
        server_socket->recv_data(client_fd, path_bucket_buffer, block_num_in_bucket * (IV_SIZE + block_size));
        path_file.write(path_bucket_buffer, block_num_in_bucket * (IV_SIZE + block_size));
        path_file.close();
        bucket_id = bucket_id == 0 ? 0 : (bucket_id - 1) / 2;
    }
    std::cout << std::endl;
    std::cout << "********************************************************************************" << std::endl;
    std::cout << "********************************************************************************" << std::endl;
    std::cout << "[Server] Finish writing path to file" << std::endl;
    std::cout << "********************************************************************************" << std::endl;  
    std::cout << "********************************************************************************" << std::endl << std::endl;
    server_socket->send_command(client_fd, (char *)&CMD_DATA_OK, sizeof(TYPE_COMMAND));
    server_socket->close_client(client_fd);
    delete[] path_bucket_buffer;
}

// achieve the evict_level() function 
void Server::evict_level(){
    TYPE_INDEX evict_parent_id{0};
    TYPE_COMMAND command;
    for (TYPE_INDEX level_id = 0; level_id < level_num - 1; ++level_id) {
        // get the parent id of the level to be evicted
        if (level_id == 0) {
            server_socket->recv_command(client_fd, (char *)&evict_parent_id, sizeof(TYPE_INDEX));
            send_evict_buffer(evict_parent_id);
            server_socket->recv_command(client_fd, (char *)&command, sizeof(TYPE_COMMAND));
            if (command != CMD_DATA_OK){
                std::cerr << "[Server]Error: client did not receive the evict buffer successfully" << std::endl;
                exit(1);
            }
            recv_evict_buffer(evict_parent_id);
            server_socket->send_command(client_fd, (char *)&CMD_DATA_OK, sizeof(TYPE_COMMAND));
        } else {
            for (int i = 0; i < 2; i++) {
                server_socket->recv_command(client_fd, (char *)&evict_parent_id, sizeof(TYPE_INDEX));
                send_evict_buffer(evict_parent_id);
                server_socket->recv_command(client_fd, (char *)&command, sizeof(TYPE_COMMAND));
                if (command != CMD_DATA_OK){
                    std::cerr << "[Server]Error: client did not receive the evict buffer successfully" << std::endl;
                    exit(1);
                }
                recv_evict_buffer(evict_parent_id);
                server_socket->send_command(client_fd, (char *)&CMD_DATA_OK, sizeof(TYPE_COMMAND));
            }
        }
    }
    server_socket->close_client(client_fd);
}
void Server::send_evict_buffer(TYPE_INDEX parent_id){
    char *evict_buffer_out = new char[block_num_in_bucket * (IV_SIZE + block_size)];
    std::vector<TYPE_INDEX> evict_bucket_id;
    evict_bucket_id.emplace_back(parent_id);
    evict_bucket_id.emplace_back(2 * parent_id + 1);
    evict_bucket_id.emplace_back(2 * parent_id + 2);
    for (auto bucket_id : evict_bucket_id) {
        this->database_name = DATABASE_DIR + "/" + "server" + "/" + std::to_string(bucket_id) + ".db";
        std::fstream evict_file(this->database_name, std::ios::in | std::ios::binary);
        evict_file.read(evict_buffer_out, block_num_in_bucket * (IV_SIZE + block_size));
        server_socket->send_data(client_fd, evict_buffer_out, block_num_in_bucket * (IV_SIZE + block_size));
        evict_file.close();
    }
    delete[] evict_buffer_out;
}

void Server::recv_evict_buffer(TYPE_INDEX parent_id){
    char *evict_buffer_in = new char[block_num_in_bucket * (IV_SIZE + block_size)];
    std::vector<TYPE_INDEX> evict_bucket_id;
    evict_bucket_id.emplace_back(parent_id);
    evict_bucket_id.emplace_back(2 * parent_id + 1);
    evict_bucket_id.emplace_back(2 * parent_id + 2);
    for (auto bucket_id : evict_bucket_id) {
        this->database_name = DATABASE_DIR + "/" + "server" + "/" + std::to_string(bucket_id) + ".db";
        std::fstream evict_file(this->database_name, std::ios::out | std::ios::binary);
        server_socket->recv_data(client_fd, evict_buffer_in, block_num_in_bucket * (IV_SIZE + block_size));
        evict_file.write(evict_buffer_in, block_num_in_bucket * (IV_SIZE + block_size));
        evict_file.close();
    }
    delete[] evict_buffer_in;
}