#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in addr;
	const int buf_size = 1024;
	char buf[buf_size];
	int bytes_read;

	if(argc < 3)
	{
		printf("Too few arguments.\n");
		exit(1);
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("socket");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("connect");
		exit(1);
	}

	while((bytes_read = read(0, &buf, buf_size)))
	{
		if (write(sock, &buf, bytes_read) < 0)
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
