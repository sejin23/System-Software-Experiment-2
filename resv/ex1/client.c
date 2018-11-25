#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 80

int main(int argc, char** argv){
	int clientSocket, n;
	int receiveBlockSize;
	int writeSize;
	char* str;
	int len;
	int client_sockfd;
	char buffer[MAXLINE];
	char* host = argv[1];
	int port = atoi(argv[2]);
	struct sockaddr_in serverAddr;

	if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket() failed.");
		exit(1);
	}
	bzero((char*)&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(host);

	if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
		perror("connect() failed.");
		exit(1);
	}
	while(1){
		write(1, "Guess? ",strlen("Guess? "));
		n = read(0, buffer, MAXLINE);
		if(n <= 0) break;
		write(clientSocket, buffer, n);
		n = read(clientSocket, buffer, MAXLINE);
		write(1, buffer, n);
		write(1, "\n", strlen("\n"));
		if(n == strlen("Correct!")) break;
	}
	close(clientSocket);
}
