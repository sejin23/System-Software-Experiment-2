#include "db.h"

pthread_mutex_t clin_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t user_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char** argv){
	int serverSocket, clientSocket, clntAddrLen, n;
	struct sockaddr_in serverAddr, clientAddr;
    char buf[MAX_KEYLEN];
	user_t* newuser;
	arg_t* newarg;
    pthread_t tid;
    client_n = 0;
	if(argc < 4){
		printf("Need port & personnel & Hash size.\n");
		exit(0);
	}
	person = atoi(argv[2]);
	USER = (user_t*)malloc(sizeof(user_t));
	USER->prev = NULL;
	USER->next = NULL;
	DB = db_open(atoi(argv[3]));
	//mtx = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*atoi(argv[3]));
	//for(n=0;n<atoi(argv[3]);n++) pthread_mutex_init(&mtx[n], NULL);
	if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket error");
		exit(1);
	}
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(atoi(argv[1]));

	if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
		perror("bind error");
		exit(1);
	}
	while(1){
		if(listen(serverSocket, person) < 0){
			perror("listen error");
			exit(1);
		}
		clntAddrLen = sizeof(clientAddr);
		if((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, (socklen_t*)&clntAddrLen)) < 0){
			perror("accept error");
			continue;
		}
        memset(buf, 0, MAX_KEYLEN);
        n = read(clientSocket, buf, MAX_KEYLEN);
        if(!strncmp(buf, "CONNECT", strlen("CONNECT"))){
            pthread_mutex_lock(&clin_mutex);
            if(client_n == person){
				pthread_mutex_unlock(&clin_mutex);
                n = write(clientSocket, "CONNECT_FAIL\n", strlen("CONNECT_FAIL\n"));
                continue;
            }else{
				pthread_mutex_unlock(&clin_mutex);
                n = write(clientSocket, "CONNECT_OK\n", strlen("CONNECT_OK\n"));
			}
			newuser = (user_t*)malloc(sizeof(user_t));
			newarg = (arg_t*)malloc(sizeof(arg_t));
			pthread_mutex_init(&newuser->serv, NULL);
			newuser->next = NULL;
			pthread_mutex_lock(&user_mutex);
			newuser->prev = USER;
			USER->next = newuser;
			USER = newuser;
			newarg->mutex = newuser;
			pthread_mutex_unlock(&user_mutex);
			newarg->fd = clientSocket;
			
            pthread_create(&tid, NULL, thread_main, (void*)newarg);
        }else
            n = write(clientSocket, "UNDEFINED PROTOCOL\n", strlen("UNDEFINED PROTOCOL\n"));
	}
	db_close(DB);
	//for(n=0;n<atoi(argv[3]);n++) pthread_mutex_destroy(&mtx[n]);
	n = n;
    return 0;
}

void* thread_main(void* arg){
	int i, j;
	char buf[MAX_KEYLEN];
	char key[MAX_KEYLEN];
	char* val;
	user_t* temp;
	arg_t* argmt = (arg_t*)arg;
	pthread_mutex_lock(&user_mutex);
	pthread_mutex_t pmt = argmt->mutex->serv;
	pthread_mutex_unlock(&user_mutex);
	pthread_detach(pthread_self());

	pthread_mutex_lock(&clin_mutex);
	client_n++;
	pthread_mutex_unlock(&clin_mutex);
    
	while(1){
		memset(buf, 0, MAX_KEYLEN);
		pthread_mutex_lock(&pmt);
		i = read(argmt->fd, buf, MAX_KEYLEN);
		pthread_mutex_unlock(&pmt);
		if(i <= 0) break;
		if(!strncmp(buf, "GET", strlen("GET"))){
			for(i=strlen("GET "),j=0;buf[i]!='\n';i++,j++)
				key[j] = buf[i];
			key[j] = '\0';
			val = db_get(DB, key, strlen(key));
			memset(buf, 0, MAX_KEYLEN);
			if(val == NULL) strcpy(buf, "GETINV\n");
			else{
				sprintf(buf, "GETOK %s %s\n", key, val);
				free(val);
			}
		}else if(!strncmp(buf, "PUT", strlen("PUT"))){
			for(i=strlen("PUT "),j=0;buf[i]!=32;i++,j++)
				key[j] = buf[i];
			key[j] = '\0';
			i++;
			val = (char*)malloc(strlen(buf)-i);
			for(j=0;buf[i]!='\n';i++,j++) val[j] = buf[i];
			val[j] = '\0';
			memset(buf, 0, MAX_KEYLEN);
			db_put(DB, key, strlen(key), val, strlen(val));
			free(val);
			strcpy(buf, "PUTOK\n");
		}else if(!strncmp(buf, "DISCONNECT", strlen("DISCONNECT"))){
			pthread_mutex_lock(&pmt);
			i = write(argmt->fd, "BYE\n", strlen("BYE\n"));
			pthread_mutex_unlock(&pmt);
            pthread_mutex_lock(&clin_mutex);
			client_n--;
            pthread_mutex_unlock(&clin_mutex);
			break;
		}else{
			printf("%s\n", buf);
			memset(buf, 0, MAX_KEYLEN);
			strcpy(buf, "UNDEFINED PROTOCOL\n");
		}
		pthread_mutex_lock(&pmt);
		i = write(argmt->fd, buf, strlen(buf));
		pthread_mutex_unlock(&pmt);
	}
	pthread_mutex_lock(&user_mutex);
	temp = argmt->mutex;
	temp->prev->next = temp->next;
	pthread_mutex_destroy(&temp->serv);
	free(temp);
	pthread_mutex_unlock(&user_mutex); 
	close(argmt->fd);
	free(argmt);
	pthread_exit(NULL);
}

/*if(buf[0] == 'A'){
			pthread_create(&tid, NULL, async_sock, (void*)&connfd);
		}else */

void* async_sock(void* arg){
	int n, fd = *(int*)arg;
	pthread_detach(pthread_self());
	sleep(3);	//
	n = write(fd, "AGETOK\n", strlen("AGETOK\n"));
	n = n;
	pthread_exit(NULL);
}