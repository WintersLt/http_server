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
#define MAX_EVENTS 100
#define READ_BUF_SIZE 2048
class EpollServer
{
	int epollfd;
	int server_sock_fd;
	int port;
	epoll_event ev, events[MAX_EVENTS];
	public:
	EpollServer(){}
	EpollServer(int aPort):epollfd(0), server_sock_fd(0), port(aPort)
	{}

	int startListen()
	{
		//Using non blocking because we may recieve spurious events by epoll and end up waiting indefinitely
		if ((server_sock_fd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0)) < 0)
		{
			handle_error("socket()");
		}
		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		//TODO take it as argument
		addr.sin_port = htons(3000);
		addr.sin_addr.s_addr = INADDR_ANY;
		int yes = 1;
		if (setsockopt(server_sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
		{
			handle_error("setsockopt()");
		}
		if(bind(server_sock_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
		{
			handle_error("bind()");
		}
		//The value 128 refers to number of connections established by OS for our program, before it starts rejecting new connections. 
		//Rejection will start happening when the program is not accepting connections fast enough.
		if(listen(server_sock_fd, 128) < 0)
		{
			handle_error("listen()");
		}
		return 0;
	}
	/*
	 * returns -1 if failed
	 * */
	int startEpollLoop()
	{
		//Epoll code here
		//Since Linux 2.6.8, the size argument is ignored, but must be greater than zero, so using 10
		if((epollfd = epoll_create(10)) == -1)
		{
			handle_error("epoll_create()");
		}

		epoll_event ev
		ev.events = EPOLLIN;
        ev.data.fd = server_sock_fd;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_sock_fd, &ev) == -1) 
		{
            handle_error("epoll_ctl(): listen_sock");
        }

		for (;;)
		{
			int num_fds = 0;
			//Specifying a timeout of -1 causes epoll_wait() to block indefinitely
			if(num_fds = epoll_wait(epollfd, events, MAX_EVENTS, -1) == -1)
			{
				handle_error("epoll_wait()");
			}

			//Loop to process events
			for (int n = 0; n < num_fds; ++n) 
			{
				if (events[n].data.fd == server_sock_fd) 
				{
					sockaddr_in cli_addr = {};
					socklen_t cli_addr_len = sizeof(cli_addr);

					if(client_sock_fd = 
							accept(server_sock_fd, 
								(struct sockaddr *) &cli_addr, &cli_addr_len) 
						== -1)
					{
						handle_error("accept()");
					}
					//Makin socket non blocking
					int status = fcntl(client_sock_fd, F_SETFL, 
										fcntl(client_sock_fd, F_GETFL, 0) | O_NONBLOCK);
					if(status == -1)
					{
						handle_error("fcntl(): O_NONBLOCK");
					}
					//Adding socket in epollset
					ev.events = EPOLLIN | EPOLLET;
					ev.data.fd = client_sock_fd;
					if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sock_fd, &ev) == -1) 
					{
            			handle_error("epoll_ctl(): client_sock_fd");
					}
				} 
				else 
				{
					handleEpollEvent(events[n]);
				}
			}
		}



		sockaddr_in cli_addr = {};
		socklen_t cli_addr_len = sizeof(cli_addr);
		int conn_fd;
		if((conn_fd = accept(server_sock_fd, (sockaddr*)&cli_addr, &cli_addr_len)) < 0)
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

	void handleEpollEvent(epoll_event aEv)
	{
		if(aEv.events & EPOLLIN)
		{
			cout << "new EPOLLIN event on fd:" << epoll_event.data.fd << endl;
			processReadEvent(epoll_event.data.fd);
		}
		else if(aEv.events & EPOLLOUT)
		{
			cout << "new EPOLLOUT event on fd:" << epoll_event.data.fd << endl;
			cout << "nothing to do here";
		}
		else if(aEv.events & EPOLLRDHUP)
		{
			cout << "new EPOLLRDHUP event on fd:" << epoll_event.data.fd << endl;
			terminateConnection(epoll_event.data.fd);
		}
		else if((aEv.events & EPOLLERR) || (aEv.events & EPOLLHUP))
		{
			cout << "new EPOLLERR or EPOLLHUP event on fd:" << epoll_event.data.fd << endl;
			terminateConnection(epoll_event.data.fd);
		}

	}

	void processReadEvent(int aClientFd)
	{
		char buf[READ_BUF_SIZE] = {};
		int free_buf_len = READ_BUF_SIZE - 1;
		char* buf_offset = buf;
		int bytesRead = 0;
		while(free_buf_len)
		{
			bytesRead = read(aClientFd, buf_offset, free_buf_len);
			if(bytesRead == -1)
			{
				perror("read()");
				//TODO handle exceptional conditions here
				cout << "Error on reading from socket"<< endl;
				cout<< buf << endl;
				break;
			}
			if (bytesRead == 0)
			{
				cout << "No bytes read, see if needed to be handled"<< endl;
				cout<< buf << endl;
				break;
			}
			if(bytesRead < free_buf_len)
			{
				free_buf_len -= bytesRead;
				buf_offset += bytesRead;
			}
		}
		if(!free_buf_len)
		{
			cout << "Buffer full!!" << endl;
			cout<< buf << endl;
		}
	}
}

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

	//Epoll code here


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
