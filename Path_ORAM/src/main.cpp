#include <iostream>
#include <cstring>
#include <fstream>

#include "Config.hpp"
#include "Client.hpp"
#include "Socket.hpp"
#include "Server.hpp"
#include "Log.hpp"

using namespace std;
int main()
{
    srand((unsigned)time(NULL));
    // prepare the necessary files 
    string command_log = "mkdir -p " + LOG_DIR;
    system(command_log.c_str());
    string command_database = "mkdir -p " + DATABASE_DIR;
    system(command_database.c_str());
    string command_metadata = "mkdir -p " + METADATA_DIR;
    system(command_metadata.c_str());
    string command_database_client = "mkdir -p " + DATABASE_DIR + "/" + "client";
    system(command_database_client.c_str());
    string command_database_server = "mkdir -p " + DATABASE_DIR + "/" + "server";
    system(command_database_server.c_str());
   //get the current time in shape of %d%m_%H%M%S
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char time_now[20];
    strftime(time_now, sizeof(time_now), "%d%m_%H%M%S", tm_now);
    std::cout << "[Main]time_now: " << time_now << std::endl;
    // generate name_prefix for log and database file
    TYPE_INDEX database_size = DATABASE_SIZE;
    TYPE_INDEX block_size = BLOCK_SIZE;
    int temp_flag = database_size/(1024*1024*1024);
    std::string database_size_GB = temp_flag ? std::to_string(temp_flag) : "0.5"; 
    // if the value of database_size_GB is equal to '2', set it to "2.5"
    if (temp_flag == 2)
    {
        database_size_GB = "2.5";
    }
    std::cout << "DATABASE: " << database_size_GB << "GB and " << "BLOCK_SIZE: "<<  block_size/1024<< "KB" << std::endl;
    TYPE_INDEX height = LEVEL_NUM;
	std::string name_prefix = database_size_GB + "GB_" + std::to_string(block_size/1024) + "KB" + std::to_string(height) + "Level";
begin:
    int choice;
    cout << "Choose 1 for server and 2 for client" << endl;
    cin >> choice;
    if (choice == 1)
    {
        cout << "===========================================================" << endl;
        cout << "[Main]Server is running" << endl;
        cout << "===========================================================" << endl;
        // create the server
        Server server(name_prefix, time_now);
        server.launch();
    }
    else if (choice == 2)
    {
        Client client(name_prefix, time_now);
        cout << "===========================================================" << endl;
        cout << "[Main]Client is running" << endl;
        cout << "===========================================================" << endl << endl;
        int choice_init_or_load;
        cout << "Choose 1 for init and 2 for load: ";
        cin >> choice_init_or_load;
        if (choice_init_or_load == 1){
            cout << "===========================================================" << endl;
            cout << "[Main]Client is initializing..." << endl;
            cout << "===========================================================" << endl;
            client.init();
        } else if (choice_init_or_load == 2){
            cout << "===========================================================" << endl;
            cout << "[Main]Client is loading..." << endl;
            cout << "===========================================================" << endl;
            client.load();
        } else {
            cout << "===========================================================" << endl;
            cout << "[Main]Wrong choice" << endl;
            cout << "===========================================================" << endl;
            goto begin;
        }
        int choice_access_randomly_or_sequentially;
begin_access:
        cout << "Choose 1 for access randomly and 2 for access sequentially: ";
        cin >> choice_access_randomly_or_sequentially;
        if (choice_access_randomly_or_sequentially == 1){
            cout << "How many times do you want to access the block of interest: ";
            int times;
            cin >> times;
            cout << "===========================================================" << endl;
            cout << "[Main]Client is accessing randomly..." << endl;
            cout << "===========================================================" << endl << endl;
            Log log(LOG_DIR + "/" + name_prefix + "_JUMP_ORAM_client_access_" + time_now + ".txt");
            for (int i = 0; i < times; i++)
            {
                std::cout << "[Main]Accessing block " << BLOCK_NUM << std::endl;
                TYPE_INDEX access_block_id = rand() % BLOCK_NUM;
                cout << "***********************************************************" << endl;
                cout << "Accessing block " << access_block_id << "is beginning" << endl;
                log.start();
                client.access(access_block_id);
                client.evict();
                cout << "The elapsed time for accessing block " << access_block_id << " is " << log.end() << "ns" << endl << endl;
                cout << "***********************************************************" << endl;
            }
            log.write_to_file();
            goto begin_access;
        } else if (choice_access_randomly_or_sequentially == 2){

            TYPE_INDEX access_block_id;
            cout << "Input the block id you want to access[0," + to_string(BLOCK_NUM -1) + " ]: ";
            cin >> access_block_id;
            // check if the block id is valid which means it is in the range of [0, BLOCK_NUM)
            if (!(0 <= access_block_id && access_block_id < BLOCK_NUM))
            {
                cout << "===========================================================" << endl;
                cout << "[Main]Wrong block id" << endl;
                cout << "===========================================================" << endl;
                goto begin_access;
            }
            cout << "===========================================================" << endl;
            cout << "Accessing block " << access_block_id << "is beginning" << endl;
            client.access(access_block_id);
            client.evict();
            cout << "Accessing block " << access_block_id << "is done" << endl;
            cout << "===========================================================" << endl << endl;
            goto begin_access;
            
        } else {
            cout << "===========================================================" << endl;
            cout << "[Main]Wrong choice" << endl;
            cout << "===========================================================" << endl;
            goto begin;
        }
    }
    else
    {
        cout << "Wrong choice" << endl;
        goto begin;
    }
    return 0;
}