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

#define SHMEM_PATH "/my_shmem"
#define SHMEM_SIZE (1 << 16) // 64 KiB
#define MAXBUF 80

char text[] = "Hello there!";

typedef struct msgbuf
{
	long mtype;
	char buf[MAXBUF];
	int size;
} Message;


int main()
{
	int shmem_fd;
	void *memory;
	Message message;
	key_t msgkey;
	int msgid, msglength;

	shmem_fd = shm_open(SHMEM_PATH, O_RDWR | O_CREAT, 0666);
	if (shmem_fd < 0)
	{
		perror("shm_open");
		exit(1);
	}

	if ((ftruncate(shmem_fd, SHMEM_SIZE)))
	{
		perror("ftruncate");
		exit(1);
	}

	memory = mmap(NULL, SHMEM_SIZE,
             PROT_READ | PROT_WRITE,
             MAP_SHARED, shmem_fd, 0);
	if (!memory)
	{
		perror("mmap");
		exit(1);
	}

	memcpy(memory, text, sizeof(text)); // запись в разделяемую память

	if ((msgkey = ftok("prog1", 'A')) < 0)
	{
		perror("ftok");
		exit(1);
	}

	if ((msgid = msgget(msgkey, IPC_CREAT)) < 0)
	{
		perror("msgget");
		exit(1);
	}

	msglength = sprintf(message.buf, SHMEM_PATH) + sizeof(message.size);
	message.size = SHMEM_SIZE;

	message.mtype = 1L; // отправляем путь к разделяемой памяти и ее размер prog2
	if (msgsnd(msgid, (void *) &message, msglength, 0) < 0)
	{
		perror("msgsnd");
		exit(1);
	}

	message.mtype = 2L; // ждем сообщения от prog2 для удаления разделяемой памяти
	if (msgrcv(msgid, &message, sizeof(message), message.mtype, 0) < 0)
	{
		perror("msgrcv");
		exit(1);
	}

	if ((munmap(memory, SHMEM_SIZE)))
	{
		perror("munmap");
		exit(1);
	}

	if ((close(shmem_fd)))
	{
		perror("close");
		exit(1);
	}

	if ((shm_unlink(SHMEM_PATH)))
	{
		perror("shm_unlink");
		exit(1);
	}

	return 0;
}
