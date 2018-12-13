#include "db.h"

int main(int argc, char** argv){
	int serverfd, clientfd, csAddrLen, n;
	struct sockaddr_in serversocket, clientsocket;
    char buf[MAX_KEYLEN];
    pthread_t tid;
    client_n = 0;
	if(argc < 4){
		printf("Need port & personnel & Hash size.\n");
		exit(0);
	}
	person = atoi(argv[2]);
	DB = db_open(atoi(argv[3]));
	Asynn = (async_t*)malloc(sizeof(async_t)*MAX_ASYNC);
	mtx = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*atoi(argv[3]));
	for(n=0;n<atoi(argv[3]);n++) pthread_mutex_init(&mtx[n], NULL);
	for(n=0;n<MAX_ASYNC;n++){
		Asynn[n].valid = 0;
		Asynn[n].key = NULL;
		Asynn[n].value = NULL;
	}
	contact_n = 0;
	asy_cmd = 0;
	pthread_mutex_init(&cnct_mutex, NULL);
	pthread_mutex_init(&dbs_mutex, NULL);
	pthread_mutex_init(&kv_mutex, NULL);
	pthread_cond_init(&cnct_cond, NULL);
	if((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket error");
		exit(1);
	}
	memset(&serversocket, 0, sizeof(serversocket));
	serversocket.sin_family = AF_INET;
	serversocket.sin_addr.s_addr = inet_addr("127.0.0.1");
	serversocket.sin_port = htons(atoi(argv[1]));

	if(bind(serverfd, (struct sockaddr*)&serversocket, sizeof(serversocket)) < 0){
		perror("bind error");
		exit(1);
	}
	while(1){
		if(listen(serverfd, person) < 0){
			perror("listen_listen error");
			exit(1);
		}
		csAddrLen = sizeof(clientsocket);
		if((clientfd = accept(serverfd, (struct sockaddr*)&clientsocket, (socklen_t*)&csAddrLen)) < 0){
			perror("accept listen error");
			continue;
		}
        memset(buf, 0, MAX_KEYLEN);
        n = read(clientfd, buf, MAX_KEYLEN);
        if(!strncmp(buf, "CONNECT", strlen("CONNECT"))){
            pthread_mutex_lock(&cnct_mutex);
            if(client_n == person){
				pthread_mutex_unlock(&cnct_mutex);
                n = write(clientfd, "CONNECT_FAIL", strlen("CONNECT_FAIL"));
                continue;
            }else{
				pthread_mutex_unlock(&cnct_mutex);
                n = write(clientfd, "CONNECT_OK\n", strlen("CONNECT_OK\n"));
			}
            pthread_create(&tid, NULL, thread_main, (void*)&clientfd);
        }else
            n = write(clientfd, "UNDEFINED PROTOCOL\n", strlen("UNDEFINED PROTOCOL\n"));
	}
	db_close(DB);
	for(n=0;n<atoi(argv[3]);n++) pthread_mutex_destroy(&mtx[n]);
    return 0;
}

void* thread_main(void* arg){
	int i, j, cfd = *(int*)arg;
	char word;
	char buf[MAX_KEYLEN];
	char key[MAX_KEYLEN];
	char* val;
	asyget_t* aget;
	asywait_t* await;
	pthread_t tid;
	pthread_detach(pthread_self());
	pthread_mutex_lock(&cnct_mutex);
	client_n++;
	pthread_mutex_unlock(&cnct_mutex);
	while(1){
		memset(buf, 0, MAX_KEYLEN);
		memset(key, 0, MAX_KEYLEN);
		j = 0;
		while((i = read(cfd, &word, sizeof(char))) > 0){
			if(word == '\n') break;
			buf[j++] = word;
		}
		buf[j] = '\0';
		printf("%s\n", buf);
		if(!strncmp(buf, "GET", strlen("GET"))){
			for(i=strlen("GET "),j=0;buf[i]!=32&&buf[i]!='\n'&&i<strlen(buf);i++,j++)
				key[j] = buf[i];
			key[j] = '\0';
			if(i > strlen("GET ") && i == strlen(buf)){
				val = db_get(key, strlen(key));
				memset(buf, 0, MAX_KEYLEN);
				if(val == NULL) strcpy(buf, "GETINV\n");
				else{
					sprintf(buf, "GETOK %s %s\n", key, val);
					free(val);
				}
			}else{
				memset(buf, 0, MAX_KEYLEN);
				strcpy(buf, "UNDEFINED PROTOCOL\n");
			}
		}else if(!strncmp(buf, "PUT", strlen("PUT"))){
			for(i=strlen("PUT "),j=0;buf[i]!=32&&buf[i]!='\n'&&i<strlen(buf);i++,j++)
				key[j] = buf[i];
			key[j] = '\0';
			if(i < strlen(buf) - 1){
				i++;
				val = (char*)malloc(strlen(buf)-i);
				for(j=0;buf[i]!='\n'&&i<strlen(buf);i++,j++) val[j] = buf[i];
				val[j] = '\0';
				memset(buf, 0, MAX_KEYLEN);
				db_put(key, strlen(key), val, strlen(val));
				free(val);
				strcpy(buf, "PUTOK\n");
			}else{
				memset(buf, 0, MAX_KEYLEN);
				strcpy(buf, "UNDEFINED PROTOCOL\n");
			}
		}else if(!strncmp(buf, "AGET", strlen("AGET"))){
			for(i=strlen("AGET "),j=0;buf[i]!=32&&buf[i]!='\n'&&i<strlen(buf);i++,j++)
				key[j] = buf[i];
			key[j] = '\0';
			if(i > strlen("AGET ") && i == strlen(buf)){
				pthread_mutex_lock(&asynn_mutex);
				for(i=0;i<MAX_ASYNC;i++){
					if(Asynn[i].key == NULL){
						Asynn[i].cfd = cfd;
						Asynn[i].key = (char*)malloc(strlen(key)+1);
						strcpy(Asynn[i].key, key);
						break;
					}
				}
				pthread_mutex_unlock(&asynn_mutex);
				aget = (asyget_t*)malloc(sizeof(asyget_t));
				aget->tag = i;
				aget->key = (char*)malloc(strlen(key)+1);
				strcpy(aget->key, key);
				pthread_create(&tid, NULL, async_get, (void*)aget);
				memset(buf, 0, MAX_KEYLEN);
				sprintf(buf, "AGETOK %d\n", i+1);
			}else{
				memset(buf, 0, MAX_KEYLEN);
				strcpy(buf, "UNDEFINED PROTOCOL\n");
			}
		}else if(!strncmp(buf, "ATEST", strlen("ATEST")) || !strncmp(buf, "AWAIT", strlen("AWAIT"))){
			for(i=strlen("ATEST "),j=0;buf[i]!=32&&buf[i]!='\n'&&i<strlen(buf);i++,j++)
				key[j] = buf[i];
			key[j] = '\0';
			if(i > strlen("ATEST ") && i == strlen(buf)){
				if(!strncmp(buf, "ATEST", strlen("ATEST"))) j = 1;
				else j = 0;
				memset(buf, 0, MAX_KEYLEN);
				sprintf(buf, "%d", atoi(key));
				if(!strcmp(buf, key) && (atoi(key) > 0 && atoi(key) <= MAX_ASYNC)){
					await = (asywait_t*)malloc(sizeof(asywait_t));
					await->cfd = cfd;
					await->tag = atoi(key);
					if(j) pthread_create(&tid, NULL, async_test, (void*)await);
					else pthread_create(&tid, NULL, async_wait, (void*)await);
					pthread_join(tid, NULL);
					continue;
				}else{
					memset(buf, 0, MAX_KEYLEN);
					strcpy(buf, "UNDEFINED PROTOCOL\n");
				}
			}else{
				memset(buf, 0, MAX_KEYLEN);
				strcpy(buf, "UNDEFINED PROTOCOL\n");
			}
		}else if(!strncmp(buf, "DISCONNECT", strlen("DISCONNECT"))){
			i = write(cfd, "BYE\n", strlen("BYE\n"));
            pthread_mutex_lock(&cnct_mutex);
			client_n--;
            pthread_mutex_unlock(&cnct_mutex);
			break;
		}else{
			printf("%s", buf);
			memset(buf, 0, MAX_KEYLEN);
			strcpy(buf, "UNDEFINED PROTOCOL\n");
		}
		i = write(cfd, buf, strlen(buf));
	}
	close(cfd);
	pthread_exit(NULL);
}

void* async_get(void* arg){
	char buf[MAX_KEYLEN];
	char* val;
	asyget_t* argsyn = (asyget_t*)arg;
	pthread_detach(pthread_self());
	memset(buf, 0, MAX_KEYLEN);
	
	val = db_get(argsyn->key, strlen(argsyn->key));
	pthread_mutex_lock(&asynn_mutex);
	Asynn[argsyn->tag].valid = 1;
	Asynn[argsyn->tag].value = val;
	pthread_cond_signal(&asynn_cond);
	pthread_mutex_unlock(&asynn_mutex);
	free(argsyn);
	pthread_exit(NULL);
}

void* async_test(void* arg){
	int n;
	char buf[MAX_KEYLEN];
	asywait_t* argsyn = (asywait_t*)arg;
	pthread_detach(pthread_self());
	memset(buf, 0, MAX_KEYLEN);
	pthread_mutex_lock(&asynn_mutex);
	if(Asynn[argsyn->tag-1].key != NULL){
		if(Asynn[argsyn->tag-1].valid){
			if(Asynn[argsyn->tag-1].value == NULL){
				strcpy(buf, "GETINV\n");
				free(Asynn[argsyn->tag-1].key);
				Asynn[argsyn->tag-1].key = NULL;
				Asynn[argsyn->tag-1].valid = 0;
				asy_cmd--;
				pthread_cond_signal(&asynn_cond);
			}else{
				sprintf(buf, "GETOK %s %s\n", Asynn[argsyn->tag-1].key, Asynn[argsyn->tag-1].value);
				free(Asynn[argsyn->tag-1].key);
				free(Asynn[argsyn->tag-1].value);
				Asynn[argsyn->tag-1].key = NULL;
				Asynn[argsyn->tag-1].value = NULL;
				Asynn[argsyn->tag-1].valid = 0;
				asy_cmd--;
				pthread_cond_signal(&asynn_cond);
			}
		}else sprintf(buf, "%d_BUSY\n", argsyn->tag);
	}else strcpy(buf, "AINV\n");
	pthread_mutex_unlock(&asynn_mutex);
	n = write(argsyn->cfd, buf, strlen(buf));
	free(argsyn);
	n = n;
	pthread_exit(NULL);
}

void* async_wait(void* arg){
	int n;
	char buf[MAX_KEYLEN];
	asywait_t* argsyn = (asywait_t*)arg;
	pthread_detach(pthread_self());
	memset(buf, 0, MAX_KEYLEN);
	pthread_mutex_lock(&asynn_mutex);
	if(Asynn[argsyn->tag-1].key != NULL){
		while(!Asynn[argsyn->tag-1].valid) pthread_cond_wait(&asynn_cond, &asynn_mutex);
		if(Asynn[argsyn->tag-1].value == NULL){
			strcpy(buf, "GETINV\n");
			free(Asynn[argsyn->tag-1].key);
			Asynn[argsyn->tag-1].key = NULL;
			Asynn[argsyn->tag-1].valid = 0;
			asy_cmd--;
			pthread_cond_signal(&asynn_cond);
		}else{
			sprintf(buf, "GETOK %s %s\n", Asynn[argsyn->tag-1].key, Asynn[argsyn->tag-1].value);
			free(Asynn[argsyn->tag-1].key);
			free(Asynn[argsyn->tag-1].value);
			Asynn[argsyn->tag-1].key = NULL;
			Asynn[argsyn->tag-1].value = NULL;
			Asynn[argsyn->tag-1].valid = 0;
			asy_cmd--;
			pthread_cond_signal(&asynn_cond);
		}
	}else strcpy(buf, "AINV\n");
	pthread_mutex_unlock(&asynn_mutex);
	n = write(argsyn->cfd, buf, strlen(buf));
	free(argsyn);
	n = n;
	pthread_exit(NULL);
}