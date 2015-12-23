#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

using namespace std;

#define handle_error(msg) \
    do { perror(msg); return -1; } while (0)
int main()
{
	int sock_fd;
	if ((sock_fd = socket(AF_INET, SOCK_STREAM/*|SOCK_NONBLOCK*/, 0)) < 0)
	{
		handle_error("socket()");
	}
	//TODO handle error
	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	//TODO take it as argument
	addr.sin_port = htons(3000);
	addr.sin_addr.s_addr = INADDR_ANY;
	int yes = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
	{
		handle_error("setsockopt()");
	}
	if(bind(sock_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
	{
		handle_error("bind()");
	}
	//The value 128 refers to number of connections established by OS for our program, before it starts rejecting new connections. 
	//Rejection will start happening when the program is not accepting connections fast enough.
	if(listen(sock_fd, 128) < 0)
	{
		handle_error("listen()");
	}
	sockaddr_in cli_addr = {};
	socklen_t cli_addr_len = sizeof(cli_addr);
	int conn_fd;
	if((conn_fd = accept(sock_fd, (sockaddr*)&cli_addr, &cli_addr_len)) < 0)
	{
		handle_error("accept()");
	}
	char message[1024] = "You are connected to server. write something now...";
	while(1)
	{
		write(conn_fd, message, sizeof(message));
		memset(message, 0, 1024);
		read(conn_fd, message, 1024);
		cout <<"Client:"<< message << endl; 
		cout <<"\nYou:"; 
		memset(message, 0, 1024);
		gets(message);
	}
}
