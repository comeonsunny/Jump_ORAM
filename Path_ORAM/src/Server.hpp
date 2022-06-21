#ifndef SERVER_HPP
#define SERVER_HPP
#include "Config.hpp"
// a class of server that could launch() and process the task from the client
class Server{
    public:
        Server(std::string name_prefix, std::string timestamp);
        ~Server();
        void launch();

    private:
        // the parameters of database
        TYPE_INDEX block_size;
        TYPE_INDEX database_size;
        TYPE_INDEX block_num;
        TYPE_INDEX level_num;
        TYPE_INDEX bucket_num;
        TYPE_INDEX bucket_leaf_num;
        TYPE_INDEX bucket_id_first_leaf;
        TYPE_INDEX block_num_in_bucket;
        TYPE_INDEX pathID;
        // the parameters of socket
        int port;
        int client_fd;
        ServerSocket *server_socket;
        // the names for database and log file
        std::string database_name;

        void receive_database();
        void access_path();
        void evict_level();
        void send_evict_buffer(TYPE_INDEX parent_id);
        void recv_evict_buffer(TYPE_INDEX parent_id);
};



#endif