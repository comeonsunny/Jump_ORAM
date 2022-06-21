#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "Socket.hpp"

#include <iostream>
#include <cstring>
using namespace std;
ServerSocket::ServerSocket(int port)
{
    this->port = port;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        cout << "[Socket]Error creating socket" << endl;
        exit(1);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        cout << "[Socket]Error binding socket" << endl;
        exit(1);
    }
    if (listen(listen_fd, 5) == -1)
    {
        cout << "[Socket]Error listening socket" << endl;
        exit(1);
    }
}

ServerSocket::~ServerSocket()
{
    close(listen_fd);
}
//get the listen_fd
int ServerSocket::get_fd()
{
    return listen_fd;
}
//accept the client
int ServerSocket::accept_client()
{
    client_len = sizeof(server_addr);
    client_fd = accept(listen_fd, (struct sockaddr *)&server_addr, &client_len);
    if (client_fd == -1)
    {
        cout << "[Socket]Error accepting client" << endl;
        exit(1);
    }
    return client_fd;
}
//send the command to the client
int ServerSocket::send_command(int fd, char *buf, int len)
{
    int send_len = 0;
    send_len = send(fd, buf, len, 0);
    if (send_len == -1)
    {
        cout << "[Socket]Error sending command" << endl;
        exit(1);
    }
    return send_len;
}
//receive the command from the client
int ServerSocket::recv_command(int fd, char *buf, int len)
{
    int recv_len = 0;
    recv_len = recv(fd, buf, len, 0);
    if (recv_len == -1)
    {
        cout << "[Socket]Error receiving command" << endl;
        exit(1);
    }
    return recv_len;
}

//send data to client
int ServerSocket::send_data(int fd,char *buf,int len)
{
    int send_len = 0;
	send_len = send(fd, buf, len, 0);
    if (send_len == -1)
    {
        cout << "[Socket]Error sending data" << endl;
        exit(1);
    }
    return send_len;
	return send_len;
}
//recv data from client
int ServerSocket::recv_data(int fd,char *buf,int len)
{
    int recv_len = 0;
    while (recv_len < len)
    {
        int tmp_len = recv(fd, buf + recv_len, len - recv_len, 0);
        if (tmp_len == -1)
        {
            cout << "[Socket]Error receiving data" << endl;
            exit(1);
        }
        recv_len += tmp_len;
    }
    return recv_len;
}
//close the client
int ServerSocket::close_client(int fd)
{
    close(fd);
    return 0;
}
//constructor
ClientSocket::ClientSocket(char *ip,int port)
{
    this->port = port;
    this->ip = ip;
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1)
    {
        cout << "[Socket]Error creating socket" << endl;
        exit(1);
    }
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(this->port);
    client_addr.sin_addr.s_addr = inet_addr(this->ip);
}
//destructor
ClientSocket::~ClientSocket()
{
    close(client_fd);
}
//get the client_fd
int ClientSocket::get_fd()
{
    return client_fd;
}
//connect to server
int ClientSocket::connect_server()
{
    if (connect(client_fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1)
    {
        cout << "[Socket]Error connecting to server" << endl;
        exit(1);
    }
    return 0;
}
//send the command to the server
int ClientSocket::send_command(char *buf, int len)
{
    int send_len = 0;
    send_len = send(client_fd, buf, len, 0);
    if (send_len == -1)
    {
        cout << "[Socket]Error sending command" << endl;
        exit(1);
    }
    return send_len;
}
//receive the command from the server
int ClientSocket::recv_command(char *buf, int len)
{
    int recv_len = 0;
    recv_len = recv(client_fd, buf, len, 0);
    if (recv_len == -1)
    {
        cout << "[Socket]Error receiving command" << endl;
        exit(1);
    }
    return recv_len;
}
//send data to server
int ClientSocket::send_data(char *buf,int len)
{
    int send_len = 0;
    send_len = send(client_fd, buf, len, 0);
    if (send_len == -1)
    {
        cout << "[Socket]Error sending data" << endl;
        exit(1);
    }
    return send_len;
}
//recv data from server
int ClientSocket::recv_data(char *buf,int len)
{
    int recv_len = 0;
    while (recv_len < len)
    {
        int tmp_len = recv(client_fd, buf + recv_len, len - recv_len, 0);
        if (tmp_len == -1)
        {
            cout << "[Socket]Error receiving data" << endl;
            exit(1);
        }
        recv_len += tmp_len;
    }
    return recv_len;
}
//close the client
int ClientSocket::close_client()
{
    close(client_fd);
    return 0;
}


