#include "db.h"

int main(int argc, char** argv){
	int listenclient, sendclient, i, k;
	struct sockaddr_in listenserver, sendserver;
    char word;
    char buf[MAX_KEYLEN];
    pthread_t tidr, tidw;
    if(argc < 3){
        printf("Need ip address & port number.\n");
        exit(0);
    }
	if((listenclient = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("listen socket error");
		exit(1);
	}
    if((sendclient = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("send socket error");
        exit(1);
    }
	memset(&listenserver, 0, sizeof(listenserver));
    memset(&sendserver, 0, sizeof(sendserver));
	listenserver.sin_family = AF_INET;
	listenserver.sin_port = htons(atoi(argv[2])+1);
	listenserver.sin_addr.s_addr = inet_addr(argv[1]);
    sendserver.sin_family = AF_INET;
	sendserver.sin_port = htons(atoi(argv[2]));
	sendserver.sin_addr.s_addr = inet_addr(argv[1]);
	if(connect(listenclient, (struct sockaddr*)&listenserver, sizeof(listenserver)) < 0){
		perror("connect error");
		exit(1);
	}
    if(connect(sendclient, (struct sockaddr*)&sendserver, sizeof(sendserver)) < 0){
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
        k = write(sendclient, buf, strlen(buf));
        memset(buf, 0, sizeof(buf));
        k = read(sendclient, buf, MAX_KEYLEN);
        buf[k] = '\0';
        if(!strncmp(buf, "CONNECT_OK\n", strlen("CONNECT_OK\n"))){
            k = write(1, buf, strlen(buf));
            pthread_create(&tidr, NULL, readpth, (void*)&sendclient);
            pthread_create(&tidw, NULL, writepth, (void*)&listenclient);
            pthread_join(tidr, NULL);
            pthread_join(tidw, NULL);
        }else
            printf("Too many clients\n");
    }else{
        printf("UNDEFINED PROTOCOL\n");
    }
    close(listenclient);
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
        buf[i] = '\0';
        n = write(cfd, buf, strlen(buf));
        if(buf[0] == 'A') continue;
        memset(buf, 0, sizeof(buf));
        n = read(cfd, buf, MAX_KEYLEN);
        printf("%s", buf);
        if(!strncmp(buf, "BYE\n", strlen("BYE\n"))){
            pthread_exit(NULL);
        }
    }
    n = n;
}

void* writepth(void* arg){
    int n, wtp, cfd = *((int*)arg);
    char buf[MAX_KEYLEN];
    while(1){
        memset(buf, 0, sizeof(buf));
        n = read(cfd, buf, MAX_KEYLEN);
        if(n > 0){
            if(!strncmp(buf, "BYE", strlen("BYE"))){
                pthread_exit(NULL);
            }
            printf("%s", buf);
        }
    }
    wtp = wtp;
}