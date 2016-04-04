#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define CLEAN_SIZE 9

int main(int argc, char *argv[])
{
	int sock, listener;
	struct sockaddr_in addr;
	char buf[BUF_SIZE];
	int bytes_read;
	FILE* fout;
	char fname[13] = "\0";
	char fnum[4] = "\0";
	int num = 1;
	int zombi;
	int i;

	if(argc < 3)
	{
		printf("Too few arguments.\n");
		exit(1);
	}

	listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0)
	{
		perror("socket");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("bind");
		exit(1);
	}

	if(listen(listener, 1) < 0)
	{
		perror("listen");
		exit(1);
	}

	while(1)
	{
		sock = accept(listener, NULL, NULL);
		if(sock < 0)
		{
			perror("accept");
			exit(1);
		}
		
		zombi++;

		switch(fork())
		{
		case -1:
			perror("fork");
			exit(1);

		case 0:
			strcat(fname, "file_");
			sprintf(fnum, "%d", num);
			strcat(fname, fnum);
			strcat(fname, ".txt");

			if((fout = fopen(fname, "w")) == NULL)
			{
				perror("fopen");
				exit(1);
			}

			while((bytes_read = read(sock, &buf, BUF_SIZE)))
			{
				if(fwrite(&buf, sizeof(char), bytes_read, fout) < 0)
				{
					perror("fwrite");
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

			if((fclose(fout)))
			{
				perror("fclose");
				exit(1);
			}

			exit(0);

		default:
			if(zombi == CLEAN_SIZE)
			{
				for (i = 0; i < CLEAN_SIZE; i++)
					wait();
				zombi = 0;
			}

			num++;
		}
	}

	if (close(listener) < 0)
	{
		perror("close");
		exit(1);
	}

	return 0;
}
