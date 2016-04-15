#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUF_SIZE 1024

char message[] = "Hello!\n";
char buf[BUF_SIZE];

int main()
{
	int sock;
	struct sockaddr_in addr;
	int bytes_read;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("socket");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(5000);
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

	while((bytes_read = read(sock, &buf, BUF_SIZE)))
	{
		if (write(1, &buf, bytes_read) < 0)
		{
			perror("write");
			exit(1);
		}
	}
	if (bytes_read < 0)
	{
		perror("read");
		exit(1);
	}

	if(close(sock) < 0)
	{
		perror("close");
		exit(1);
	}

	return 0;
}
