#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Config.hpp"
#include "Socket.hpp"

#include <vector>
/*
A class of Clinet that can init(), access(), and evict()
*/
class Client {
    public:
        Client(std::string name_prefix, std::string timestamp);
        ~Client();
        int init();
        int load();
        char *access(TYPE_INDEX index);
        int evict();
        int evict_from_1_to_2(char *bucket_buffer[], TYPE_INDEX level, TYPE_INDEX parent_index);
        void left_right_boundary(TYPE_INDEX next_level, TYPE_INDEX node_index, TYPE_INDEX &left_boundary, TYPE_INDEX &right_boundary);
    private:
        //the parameters of database
        TYPE_INDEX block_size;
        TYPE_INDEX database_size;
        TYPE_INDEX block_num;
        TYPE_INDEX level_num;
        TYPE_INDEX bucket_num;
        TYPE_INDEX bucket_leaf_num;
        TYPE_INDEX bucket_id_first_leaf;
        int        block_num_in_bucket;
        // the parameters of socket
        int port;
        int client_fd;
        char *ip;
        ClientSocket *client_socket;
        std::vector<TYPE_INDEX> position_map;
        char *block_of_interest_buffer;
        // the names for database and log file
        std::string database_name;
        std::string position_map_name;
        std::string stash_name;
        std::string cache_name;
        std::string build_log_name;
        std::string access_log_name;
        std::string access_encrypted_log_name;
        std::string access_decrypted_log_name;
        std::string access_recvData_log_name;
        std::string access_sendData_log_name;
        std::string access_downToDisk_log_name;
        std::string evict_encrypted_log_name;
        std::string evict_decrypted_log_name;
        std::string evict_recvData_log_name;
        std::string evict_sendData_log_name;
        std::string evict_downToDisk_log_name;
};

#endif