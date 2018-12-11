#include "db.h"

pthread_mutex_t sockmtx = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char** argv){
	int clientSocket, n, wtp;
	struct sockaddr_in serverAddr;
    char buf[MAX_KEYLEN];
    pthread_t tidr, tidw;
    if(argc < 3){
        printf("Need ip address & port number.\n");
        exit(0);
    }
	if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket error");
		exit(1);
	}
	bzero((char*)&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

	if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
		perror("connect error");
		exit(1);
	}
    if(fgets(buf, MAX_KEYLEN, stdin) == NULL) exit(0);
    wtp = write(clientSocket, buf, strlen(buf));
    n = read(clientSocket, buf, MAX_KEYLEN);
    if(!strncmp(buf, "CONNECT_OK", strlen("CONNECT_OK"))){
        wtp = write(1, buf, n);
        pthread_create(&tidr, NULL, readpth, (void*)&clientSocket);
        pthread_create(&tidw, NULL, writepth, (void*)&clientSocket);
        pthread_join(tidr, NULL);
        pthread_join(tidw, NULL);
    }else if(!strncmp(buf, "CONNECT_FAIL", strlen("CONNECT_FAIL")))
        wtp = write(1, "Too many clients\n", strlen("Too many clients\n"));
    else
        wtp = write(1, buf, n);
    close(clientSocket);
    wtp = wtp;
	return 0;
}

void* readpth(void* arg){
    int n, cfd = *((int*)arg);
    char buf[MAX_KEYLEN];
    while(1){
        memset(buf, 0, MAX_KEYLEN);
        if(fgets(buf, MAX_KEYLEN, stdin) != NULL){
            n = write(cfd, buf, strlen(buf));
            if(!strncmp(buf, "DISCONNECT", strlen("DISCONNECT"))){
                pthread_exit(NULL);
            }
        }
    }
    n = n;
}

void* writepth(void* arg){
    int n, wtp, cfd = *((int*)arg);
    char buf[MAX_KEYLEN];
    while(1){
        memset(buf, 0, MAX_KEYLEN);
        n = read(cfd, buf, MAX_KEYLEN);
        if(n > 0){
            wtp = write(1, buf, n);
            if(!strncmp(buf, "BYE", strlen("BYE"))){
                pthread_exit(NULL);
            }
        }
    }
    wtp = wtp;
}