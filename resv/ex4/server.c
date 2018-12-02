#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#define MAXLINE 80
#define MAXCLIENT 20
void* thread_main(void* arg);
int n, m;
int clientfd[MAXCLIENT];
int c_index[20];
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

void main(int argc, char** argv){
	int serverSocket, clntAddrLen, port, i;
	int c_socket;
	char buffer[MAXLINE];
	pthread_t tid;
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	if(argc < 3){
		printf("need port & personnel number.\n");
		exit(0);
	}
	memset(c_index, 0, sizeof(int)*20);
	port = atoi(argv[1]);
	m = atoi(argv[2]);
	if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Error: Listen socket failed!");
		exit(1);
	}
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);

	if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
		perror("bind() failed.");
		exit(1);
	}
	n = 0;
	while(1){
		if(listen(serverSocket, 5) < 0){
			perror("listen() failed");
			exit(1);
		}
		clntAddrLen = sizeof(clientAddr);
		if((c_socket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clntAddrLen)) < 0){
			perror("accept() failed.");
			continue;
		}
		for(i=0;i<20;i++) if(c_index[i] == 0) break;
		clientfd[i] = c_socket;
		c_index[i] = 2;
		pthread_create(&tid, NULL, thread_main, (void*)&c_socket);
	}
}

void* thread_main(void* arg){
	int num, i, temp, index;
	char buf[MAXLINE];
	int connfd = *(int*)arg;
	pthread_detach(pthread_self());
	memset(buf, 0, MAXLINE);

	pthread_mutex_lock(&g_mutex);
	for(i=0;i<20;i++){
		if(clientfd[i] == connfd){
			index = i;
			break;
		}
	}
	while(n >= m)
		pthread_cond_wait(&g_cond, &g_mutex);
	n++;
	c_index[i] = 1;
	write(connfd, "start", strlen("start"));
	pthread_mutex_unlock(&g_mutex);
	while((num = read(connfd, buf, MAXLINE)) > 0){
		if(!strncmp(buf, "exit", strlen("exit"))){
			write(connfd, buf, num);
			for(i=0;i<20;i++){
				if(clientfd[i] == connfd) c_index[i] = 0;
			}
			n--;
			pthread_cond_signal(&g_cond);
			memset(buf, 0, MAXLINE);
			continue;
		}
		for(i=0;i<20;i++){
			if(c_index[i] == 1){
				if(clientfd[i] != connfd) write(clientfd[i], buf, strlen(buf));
			}
		}
		memset(buf, 0, MAXLINE);
	}
	close(connfd);
	pthread_exit(NULL);
}
