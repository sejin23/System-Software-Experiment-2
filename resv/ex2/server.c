#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAXLINE 80

int term;

void handler(int sig){
	pid_t pid;
	int status;
	while((pid = waitpid(-1, &status, WNOHANG)) > 0);
	return; 
}

void terminal(int sig){
	exit(0);
	return;
}

void main(int argc, char** argv){
	int n;
	int fd[2];
	int randnum;
	int serverSocket;
	int clientSocket;
	int clntAddrLen;
	int sendBlockSize;
	int port = atoi(argv[1]);
	char buffer[MAXLINE];
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;

	term = 0;
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
	signal(SIGINT, terminal);
	signal(SIGCHLD, handler);
	srand(time(NULL));
	while(1){
		if(listen(serverSocket, 5) < 0){
			perror("listen() failed");
			exit(1);
		}
		randnum = rand()%100;
		clntAddrLen = sizeof(clientAddr);
		if((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clntAddrLen)) < 0){
			perror("accept() failed.");
			continue;
		}
		if(fork() == 0){
			bzero(buffer, MAXLINE);
			while(1){
				n = read(clientSocket, buffer, MAXLINE);
				if(n <= 0) break;
				n = atoi(buffer);
				if(n == randnum){
					write(clientSocket, "Correct!", strlen("Correct!"));
					break;
				}else if(n > randnum){
					write(clientSocket, "Down", strlen("Down"));
				}else if(n){
					write(clientSocket, "Up", strlen("Up"));
				}

			}
			exit(0);
		}
		close(clientSocket);
	}
}
