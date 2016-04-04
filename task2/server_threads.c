#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 1024

struct DATA_
{
	int sock_;
	char fname_[13];
};
typedef struct DATA_ DATA;

void *entry(void *arg)
{
	char buf[BUF_SIZE];
	int bytes_read;
	FILE* fout;
	DATA *a = (DATA *)arg;

	if((fout = fopen(a->fname_, "w")) == NULL)
	{
		perror("fopen");
		exit(1);
	}

	while((bytes_read = read(a->sock_, &buf, BUF_SIZE)))
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

	if(close(a->sock_) < 0)
	{
		perror("close");
		exit(1);
	}

	if((fclose(fout)))
	{
		perror("fclose");
		exit(1);
	}

	free(arg);	
}

int main(int argc, char *argv[])
{
	pthread_t thread;
	int sock, listener;
	struct sockaddr_in addr;
	DATA *arg;
	char fnum[4];
	int num = 1;

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
		
		arg = (DATA *)malloc(sizeof(DATA));
		if (!arg)
		{
			perror("malloc");
			exit(1);
		}

		arg->sock_ = sock;

		strcat(arg->fname_, "file_");
		sprintf(fnum, "%d", num);
		strcat(arg->fname_, fnum);
		strcat(arg->fname_, ".txt");

		if((pthread_create(&thread, NULL, entry, (void *)arg)))
		{
			perror("pthread_create");
			exit(1);
		}

		num++;
	}

	if(close(listener) < 0)
	{
		perror("close");
		exit(1);
	}

	return 0;
}
