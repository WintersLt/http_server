#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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
	if (inet_aton("0.0.0.0", &addr.sin_addr) == 0)
	{
		handle_error("inet_aton()");
	}
	if(connect(sock_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
	{
		handle_error("connect()");
	}
	
	char message[1024] = {};
	while(1)
	{
		memset(message, 0, 1024);
		read(sock_fd, message, 1024);
		cout << "Server:"<< message << endl; 
		cout << "\nYou:"; 
		memset(message, 0, 1024);
		gets(message);
		write(sock_fd, message, sizeof(message));
	}
}
