#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <list>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define THREAD_N 20

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::list<int> queue;

char message[] = "World!\n";

void *entry(void *arg)
{
	int sock;
	char buf[BUF_SIZE];
	int bytes_read;

	pthread_mutex_lock(&mutex);

	while (!queue.size())
	{
		assert(!pthread_cond_wait(&cond, &mutex));
	}
	sock = queue.front();
	queue.pop_front();

	pthread_mutex_unlock(&mutex);

	//while((bytes_read = read(sock, &buf, BUF_SIZE)))  //так не получается
	bytes_read = read(sock, &buf, BUF_SIZE);
	{
		if(write(1, &buf, bytes_read) < 0)
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

	if(write(sock, &message, sizeof(message)) < 0)
	{
		perror("write");
		exit(1);
	}

	if(close(sock) < 0)
	{
		perror("close");
		exit(1);
	}
}

int main()
{
	pthread_t thread;
	int sock, listener;
	struct sockaddr_in addr;

	pthread_t *pool = new pthread_t[THREAD_N];
	if (!pool)
	{
		perror("new");
		exit(1);
	}

	for (int i = 0; i < THREAD_N; i++)
	{
		if (pthread_create(pool + i, NULL, entry, NULL))
		{
			perror("pthread_create");
			exit(1);
		}
	}

	listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0)
	{
		perror("socket");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(5000);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
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

		pthread_mutex_lock(&mutex);
		queue.push_back(sock);
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&cond);
	}

	for (int i = 0; i < THREAD_N; i++)
	{
		if((pthread_join(pool[i], NULL)))
		{
			perror("pthread_join");
			exit(1);
		}
	}

	if(close(listener) < 0)
	{
		perror("close");
		exit(1);
	}

	delete[] pool;

	return 0;
}
