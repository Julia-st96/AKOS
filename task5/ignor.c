#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main()
{
	signal(SIGINT, SIG_IGN);

	printf("I ignore SIGINT (ctrl+c)!\n");
	while(1);

	return 0;
}
