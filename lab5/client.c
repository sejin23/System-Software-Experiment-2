#include "db.h"

int main(int argc, char** argv){
	int clientfd, i, k;
	struct sockaddr_in serversocket;
    char word;
    char buf[MAX_KEYLEN];
    pthread_t tid;
    if(argc < 3){
        printf("Need ip address & port number.\n");
        exit(0);
    }
	if((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("listen socket error");
		exit(1);
	}
	memset(&serversocket, 0, sizeof(serversocket));
	serversocket.sin_family = AF_INET;
	serversocket.sin_port = htons(atoi(argv[2]));
	serversocket.sin_addr.s_addr = inet_addr(argv[1]);
	if(connect(clientfd, (struct sockaddr*)&serversocket, sizeof(serversocket)) < 0){
		perror("connect error");
		exit(1);
	}
    i = 0;
    memset(buf, 0, sizeof(buf));
    while((k = read(0, &word, sizeof(char))) > 0){
        if(word == '\n') break;
        buf[i++] = word;
    }
    buf[i] = '\0';
    if(!strncmp(buf, "CONNECT", strlen("CONNECT"))){
        k = write(clientfd, buf, strlen(buf));
        memset(buf, 0, sizeof(buf));
        k = read(clientfd, buf, MAX_KEYLEN);
        buf[k] = '\0';
        if(!strncmp(buf, "CONNECT_OK\n", strlen("CONNECT_OK\n"))){
            k = write(1, buf, strlen(buf));
            pthread_create(&tid, NULL, readpth, (void*)&clientfd);
            pthread_join(tid, NULL);
        }else
            printf("Too many clients\n");
    }else{
        printf("UNDEFINED PROTOCOL\n");
    }
    close(clientfd);
	return 0;
}

void* readpth(void* arg){
    int n, i, cfd = *((int*)arg);
    char word;
    char buf[MAX_KEYLEN];
    while(1){
        memset(buf, 0, sizeof(buf));
        i = 0;
        while((n = read(0, &word, sizeof(char))) > 0){
            if(word == '\n') break;
            buf[i++] = word;
        }
        buf[i] = '\n';
        buf[i+1] = '\0';
        n = write(cfd, buf, strlen(buf));
        memset(buf, 0, sizeof(buf));
        n = read(cfd, buf, MAX_KEYLEN);
        printf("%s", buf);
        if(!strncmp(buf, "BYE\n", strlen("BYE\n"))){
            pthread_exit(NULL);
        }
    }
    n = n;
}