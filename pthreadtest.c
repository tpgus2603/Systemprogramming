#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <pthread.h>
#include <time.h>

#include <sys/ioctl.h>
#include <stdint.h>

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define VALUE_MAX 256

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

void error_handling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void* t_function1(void* data) {
	//pid_t pid;
	//pthread_t tid;

	char buffer[10] = "Hello111\n";

	//pid = getpid();
	//tid = pthread_self();

	int* sock = (int*)data;

	int i = 0;

	while (1)
	{
		write(*sock, buffer, sizeof(buffer));
		usleep(1000000);
	}
}
void* t_function2(void* data) {
	//pid_t pid;
	//pthread_t tid;

	char buffer[10] = "Hello222\n";

	//pid = getpid();
	//tid = pthread_self();

	int* sock = (int*)data;

	int i = 0;

	while (1)
	{
		write(*sock, buffer, sizeof(buffer));
		usleep(1000000);
	}
}

int main(int argc, char* argv[]) {

	pthread_t p_thread[2];

	int thr_id;
	int status;

	int sock;
	struct sockaddr_in serv_addr;
	int str_len;

	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);

	if (sock == -1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");
	//--------------------------------------------------------------------------


	thr_id = pthread_create(&p_thread[0], NULL, t_function1, (void*)&sock);
	if (thr_id < 0) {
		perror("thread create error : ");
		exit(0);
	}
	thr_id = pthread_create(&p_thread[1], NULL, t_function2, (void*)&sock);
	if (thr_id < 0) {
		perror("thread create error : ");
		exit(0);
	}
	pthread_join(p_thread[0], (void**)&status);
	pthread_join(p_thread[1], (void**)&status);

	return 0;
}
