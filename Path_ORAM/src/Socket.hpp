#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <sys/socket.h>
#include <netinet/in.h>
//a class for socket in Server
class ServerSocket{
	public:
		ServerSocket(int port);
		~ServerSocket();
		int get_fd();
		int accept_client();
        // send and receive commands to client
        int send_command(int fd, char *buf, int len);
        int recv_command(int fd, char *buf, int len);
		int send_data(int fd,char *buf,int len);
		int recv_data(int fd,char *buf,int len);
		int close_client(int fd);
	private:
		int port;
		int listen_fd;
		int client_fd;
		struct sockaddr_in server_addr;
		socklen_t client_len;
};


class ClientSocket{
	public:
		ClientSocket(char *ip,int port);
		~ClientSocket();
		int get_fd();
		int connect_server();
        int send_command(char *buf, int len);
        int recv_command(char *buf, int len);
		int send_data(char *buf,int len);
		int recv_data(char *buf,int len);
		int close_client();
	private:
		int port;
		int client_fd;
		char *ip;
		struct sockaddr_in client_addr;
};


#endif
