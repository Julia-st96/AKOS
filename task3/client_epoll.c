#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

char message[] = "Hello world!\n";
char buf[sizeof(message)];

int main()
{
	int sock;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("socket");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(6000);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("connect");
		exit(1);
	}

	if(write(sock, message, sizeof(message)) < 0)
	{
		perror("write");
		exit(1);
	}

	while(1)
	{
		read(sock, buf, sizeof(buf));
		write(1, buf, sizeof(buf));
	}

	return 0;
}
