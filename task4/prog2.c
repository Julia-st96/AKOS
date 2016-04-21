#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define MAXBUF 80

typedef struct msgbuf
{
	long mtype;
	char buf[MAXBUF];
	int size;
} Message;


int main()
{
	int shmem_fd;
	char *memory;
	Message message;
	key_t key;
	int msgid, msglength;

	if ((key = ftok("prog1", 'A')) < 0)
	{
		perror("ftok");
		exit(1);
	}

	if ((msgid = msgget(key, 0)) < 0)
	{
		perror("msgget");
		exit(1);
	}

	int i;
	for(i = 0; i < MAXBUF; i++)
		message.buf[i] = '\0';

	message.mtype = 1L; // получаем путь к разделяемой памяти и ее размер от prog1
	if (msgrcv(msgid, &message, sizeof(message), message.mtype, 0) < 0)
	{
		perror("msgrcv");
		exit(1);
	}

	shmem_fd = shm_open(message.buf, O_RDWR, 0666);
	if (shmem_fd < 0)
	{
		perror("shm_open");
		exit(1);
	}

	if ((ftruncate(shmem_fd, message.size)))
	{
		perror("ftruncate");
		exit(1);
	}

	memory = mmap(NULL, message.size,
             PROT_READ | PROT_WRITE,
             MAP_SHARED, shmem_fd, 0);
	if (!memory)
	{
		perror("mmap");
		exit(1);
	}

	printf("Message: %s\n", memory); // чтение из разделяемой памяти

	if ((munmap(memory, message.size)))
	{
		perror("munmap");
		exit(1);
	}

	if ((close(shmem_fd)))
	{
		perror("close");
		exit(1);
	}

	printf("Wait 5 seconds to show that prog1 works until I send message.\n");
	sleep(5);
	printf("OK\n");

	message.mtype = 2L; // отправляем сообщение prog1 для удаления разделяемой памяти
	if (msgsnd(msgid, (void *) &message, 0, 0) < 0)
	{
		perror("msgsnd");
		exit(1);
	}

	return 0;
}
