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
void* readpth(void* arg);
void* writepth(void* arg);

int main(int argc, char** argv){
	int receiveBlockSize, len, writesize, n, port;
	char* str;
	char buffer[MAXLINE];
	struct sockaddr_in serverAddr;
    int* clientSocket;
    pthread_t tidr, tidw;
    if(argc < 2){
        printf("need port number\n");
        exit(0);
    }
    clientSocket = (int*)malloc(sizeof(int));
    port = atoi(argv[1]);
	if((*clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket() failed.");
		exit(1);
	}
	bzero((char*)&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(*clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
		perror("connect() failed.");
		exit(1);
	}
    memset(buffer, 0, MAXLINE);
    read(*clientSocket, buffer, MAXLINE);
    pthread_create(&tidr, NULL, readpth, (void*)clientSocket);
    pthread_create(&tidw, NULL, writepth, (void*)clientSocket);
    pthread_join(tidr, NULL);
    pthread_join(tidw, NULL);
    close(*clientSocket);
    free(clientSocket);
	return 0;
}

void* readpth(void* arg){
    int cfd = *((int*)arg);
    char buf[MAXLINE];
    int n;
    while(1){
        memset(buf, 0, MAXLINE);
        if((n = read(0, buf, MAXLINE)) > 0){
            write(cfd, buf, n);
            if(!strncmp(buf, "exit", strlen("exit"))){
                pthread_exit(NULL);
            }
        }
    }
}

void* writepth(void* arg){
    int cfd = *((int*)arg);
    char buf[MAXLINE];
    int n;
    while(1){
        memset(buf, 0, MAXLINE);
        if((n = read(cfd, buf, MAXLINE)) > 0){
            if(!strncmp(buf, "exit", strlen("exit"))){
                pthread_exit(NULL);
            }
            write(1, buf, n);
        }
    }
}