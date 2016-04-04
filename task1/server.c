#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main()
{
	int sock, listener;
	struct sockaddr_in addr;
	char buf[1024];
	int bytes_read;

	listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0)
	{
		perror("socket");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(6000);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("bind");
		exit(2);
	}

	if(listen(listener, 1) < 0)
	{
		perror("listen");
		exit(3);
	}

	/* закомментирована часть для общего случая: бесконечной работы сервера */
	/*while(1)
	{ */
		sock = accept(listener, NULL, NULL);
		if(sock < 0)
		{
			perror("accept");
			exit(4);
		}

		while((bytes_read = read(sock, buf, 1024)))
		{
			if(write(1, buf, bytes_read) < 0)
			{
				perror("write");
				exit(5);
			}
		}
		if (bytes_read < 0)
		{
			perror("read");
			exit(6);
		}

		if(close(sock) < 0)
		{
			perror("close");
			exit(7);
		}
	/*} */

	return 0;
}
