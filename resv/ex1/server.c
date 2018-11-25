#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 80

void main(int argc, char** argv){
	int n;
	int randnum;
	int serverSocket;
	int clientSocket;
	int clntAddrLen;
	int sendBlockSize;
	int port = atoi(argv[1]);
	char buffer[MAXLINE];
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;

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
	if(listen(serverSocket, 5) < 0){
		perror("listen() failed");
		exit(1);
	}
	srand(time(NULL));
	while(1){
		randnum = rand()%100;
		clntAddrLen = sizeof(clientAddr);
		if((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clntAddrLen)) < 0){
			perror("accept() failed.");
			continue;
		}

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
	}
	close(clientSocket);
}
