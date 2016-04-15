#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define MAX_EVENTS 10
#define MAX_CLIENTS 10

void setNonBlocking(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
	{
		perror("fcntl");
		exit(1);
	}

	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main()
{
	int conn_sock, listen_sock;
	int epollfd;
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];
	int events_num;
	struct sockaddr_in addr;
	char buf[BUF_SIZE];
	int bytes_read;
	int i, j;
	int clients_list[MAX_CLIENTS];
	int clients_num = 0;

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock < 0)
	{
		perror("socket");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(6000);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listen_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("bind");
		exit(1);
	}

	if(listen(listen_sock, 1) < 0)
	{
		perror("listen");
		exit(1);
	}

	epollfd = epoll_create(2);
	if (epollfd < 0)
	{
		perror("epoll_create");
		exit(1);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;  
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev))
	{
		perror("epoll_ctl: listen_sock");
		exit(1);
	}

	while(1)
	{ 
		events_num = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (events_num < 0)
		{
			perror("epoll_wait");
			exit(1);
		}

		for (i = 0; i < events_num; i++)
		{
			if (events[i].data.fd == listen_sock)
			{
        			conn_sock = accept(listen_sock, NULL, NULL);
				if(conn_sock < 0)
				{
					perror("accept");
					exit(1);
				}

				setNonBlocking(conn_sock);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_sock;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev))
				{
					perror("epoll_ctl: conn_sock");
					exit(1);
				}
				clients_list[clients_num] = conn_sock;
				clients_num++;
	
			}
			else
			{
				if((bytes_read = read(conn_sock, buf, BUF_SIZE)) <= 0)
				{
					if (epoll_ctl(epollfd, EPOLL_CTL_DEL, conn_sock, &ev) < 0) 
					{
						perror("epoll_ctl");
						exit(1);
					}

					for(j = 0; j < MAX_CLIENTS; j++)
					{
						if (clients_list[j] == conn_sock)
							clients_list[j] = -5;      //клиент отсоединился
					}

					close(conn_sock);
				}
				else
				{
					for(j = 0; j < clients_num; j++)
					{
						if (clients_list[j] > 0)  //отсоединенным клиентам не отправляем
						{
							if (write(clients_list[j], buf, bytes_read) < 0)
							{
								perror("write");
								exit(1);
							}
						}
					}
				}
 			}
		}
	}	

	if (close(epollfd) < 0)
	{
		perror("close: epollfd");
		exit(1);
	}

	return 0;
}
