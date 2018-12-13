#include "db.h"

int main(int argc, char** argv){
	int listen_serv_socket, send_serv_socket, listen_cli_socket, send_cli_socket, listenAddrLen, sendAddrLen, n;
	struct sockaddr_in listenserver, sendserver, listen_clientaddr, send_clientaddr;
    char buf[MAX_KEYLEN];
	arg_t* newarg;
    pthread_t tid;
    client_n = 0;
	if(argc < 4){
		printf("Need port & personnel & Hash size.\n");
		exit(0);
	}
	person = atoi(argv[2]);
	DB = db_open(atoi(argv[3]));
	contact_n = 0;
	mtx = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*atoi(argv[3]));
	for(n=0;n<atoi(argv[3]);n++) pthread_mutex_init(&mtx[n], NULL);
	pthread_mutex_init(&cnct_mutex, NULL);
	pthread_mutex_init(&dbs_mutex, NULL);
	pthread_mutex_init(&kv_mutex, NULL);
	pthread_mutex_init(&cnct_mutex, NULL);
	pthread_cond_init(&cnct_cond, NULL);
	if((listen_serv_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket error");
		exit(1);
	}
	if((send_serv_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket error");
		exit(1);
	}
	memset(&listenserver, 0, sizeof(listenserver));
	memset(&sendserver, 0, sizeof(sendserver));
	listenserver.sin_family = AF_INET;
	listenserver.sin_addr.s_addr = inet_addr("127.0.0.1");
	listenserver.sin_port = htons(atoi(argv[1]));
	sendserver.sin_family = AF_INET;
	sendserver.sin_addr.s_addr = inet_addr("127.0.0.1");
	sendserver.sin_port = htons(atoi(argv[1])+1);

	if(bind(listen_serv_socket, (struct sockaddr*)&listenserver, sizeof(listenserver)) < 0){
		perror("bind error");
		exit(1);
	}
	if(bind(send_serv_socket, (struct sockaddr*)&sendserver, sizeof(sendserver)) < 0){
		perror("bind error");
		exit(1);
	}
	while(1){
		if(listen(send_serv_socket, person) < 0){
			perror("listen_send error");
			exit(1);
		}
		if(listen(listen_serv_socket, person) < 0){
			perror("listen_listen error");
			exit(1);
		}
		listenAddrLen = sizeof(listen_clientaddr);
		sendAddrLen = sizeof(send_clientaddr);
		if((send_cli_socket = accept(send_serv_socket, (struct sockaddr*)&send_clientaddr, (socklen_t*)&sendAddrLen)) < 0){
			perror("accept send error");
			continue;
		}
		if((listen_cli_socket = accept(listen_serv_socket, (struct sockaddr*)&listen_clientaddr, (socklen_t*)&listenAddrLen)) < 0){
			perror("accept listen error");
			continue;
		}
        memset(buf, 0, MAX_KEYLEN);
        n = read(listen_cli_socket, buf, MAX_KEYLEN);
        if(!strncmp(buf, "CONNECT", strlen("CONNECT"))){
            pthread_mutex_lock(&cnct_mutex);
            if(client_n == person){
				pthread_mutex_unlock(&cnct_mutex);
                n = write(listen_cli_socket, "CONNECT_FAIL", strlen("CONNECT_FAIL"));
                continue;
            }else{
				pthread_mutex_unlock(&cnct_mutex);
                n = write(listen_cli_socket, "CONNECT_OK\n", strlen("CONNECT_OK\n"));
			}
			newarg = (arg_t*)malloc(sizeof(arg_t));
			newarg->listenfd = listen_cli_socket;
			newarg->sendfd = send_cli_socket;
            pthread_create(&tid, NULL, thread_main, (void*)newarg);
        }else
            n = write(listen_cli_socket, "UNDEFINED PROTOCOL\n", strlen("UNDEFINED PROTOCOL\n"));
	}
	db_close(DB);
	for(n=0;n<atoi(argv[3]);n++) pthread_mutex_destroy(&mtx[n]);
    return 0;
}

void* thread_main(void* arg){
	int i, j, asy_cmd;
	char buf[MAX_KEYLEN];
	char key[MAX_KEYLEN];
	char* val;
	pthread_mutex_t smutex, lmutex;
	pthread_cond_t fcond, lcond;
	async_t* toasyn = (async_t*)malloc(sizeof(async_t)*MAX_ASYNC);
	arg_t* argmt = (arg_t*)arg;
	asyget_t* passget_a;
	asywait_t* passwait_a;
	pthread_t tid;
	pthread_detach(pthread_self());
	pthread_mutex_lock(&cnct_mutex);
	client_n++;
	pthread_mutex_unlock(&cnct_mutex);
	pthread_mutex_init(&lmutex, NULL);
	pthread_mutex_init(&smutex, NULL);
	pthread_cond_init(&lcond, NULL);
	for(i=0;i<MAX_ASYNC;i++){
		toasyn->key = NULL;
		toasyn->value = NULL;
		toasyn->valid = 0;
	}
	asy_cmd = 0;
	while(1){
		memset(buf, 0, MAX_KEYLEN);
		memset(key, 0, MAX_KEYLEN);
		i = read(argmt->listenfd, buf, MAX_KEYLEN);
		if(i <= 0) break;
		if(!strncmp(buf, "GET", strlen("GET"))){
			for(i=strlen("GET "),j=0;buf[i]!=32&&i<strlen(buf);i++,j++)
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
			for(i=strlen("PUT "),j=0;buf[i]!=32&&i<strlen(buf);i++,j++)
				key[j] = buf[i];
			key[j] = '\0';
			if(i < strlen(buf) - 1){
				val = (char*)malloc(strlen(buf)-i);
				i++;
				for(j=0;i<strlen(buf);i++,j++) val[j] = buf[i];
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
			for(i=strlen("AGET "),j=0;buf[i]!=32&&i<strlen(buf);i++,j++)
				key[j] = buf[i];
			key[j] = '\0';
			if(i > strlen("AGET ") && i == strlen(buf)){
				passget_a = (asyget_t*)malloc(sizeof(asyget_t));
				passget_a->key = (char*)malloc(strlen(key)+1);
				strcpy(passget_a->key, key);
				passget_a->sendmutex = smutex;
				passget_a->countmutex = lmutex;
				passget_a->findcond = fcond;
				passget_a->countcond = lcond;
				passget_a->asy_num = &asy_cmd;
				passget_a->asy = toasyn;
				passget_a->sendfd = argmt->sendfd;
				pthread_create(&tid, NULL, async_get, (void*)passget_a);
			}else{
				pthread_mutex_lock(&smutex);
				i = write(argmt->sendfd, "UNDEFINED PROTOCOL\n", strlen("UNDEFINED PROTOCOL\n"));
				pthread_mutex_unlock(&smutex);
			}
			continue;
		}else if(!strncmp(buf, "ATEST", strlen("ATEST")) || !strncmp(buf, "AWAIT", strlen("AWAIT"))){
			for(i=strlen("ATEST "),j=0;buf[i]!=32&&i<strlen(buf);i++,j++)
				key[j] = buf[i];
			key[j] = '\0';
			if(i > strlen("ATEST ") && i == strlen(buf)){
				if(!strncmp(buf, "ATEST", strlen("ATEST"))) j = 1;
				else j = 0;
				memset(buf, 0, MAX_KEYLEN);
				sprintf(buf, "%d", atoi(key));
				if(!strcmp(buf, key) && (atoi(key) > 0 && atoi(key) <= MAX_ASYNC)){
					passwait_a = (asywait_t*)malloc(sizeof(asywait_t));
					passwait_a->sendmutex = smutex;
					passwait_a->countmutex = lmutex;
					passwait_a->findcond = fcond;
					passwait_a->countcond = lcond;
					passwait_a->tag = atoi(key);
					passwait_a->asy = toasyn;
					passwait_a->sendfd = argmt->sendfd;
					passwait_a->asy_num = &asy_cmd;
					if(j) pthread_create(&tid, NULL, async_test, (void*)passwait_a);
					else pthread_create(&tid, NULL, async_wait, (void*)passwait_a);
				}else{
					pthread_mutex_lock(&smutex);
					i = write(argmt->sendfd, "UNDEFINED PROTOCOL\n", strlen("UNDEFINED PROTOCOL\n"));
					pthread_mutex_unlock(&smutex);
				}
			}else{
				pthread_mutex_lock(&smutex);
				i = write(argmt->sendfd, "UNDEFINED PROTOCOL\n", strlen("UNDEFINED PROTOCOL\n"));
				pthread_mutex_unlock(&smutex);
			}
			continue;
		}else if(!strncmp(buf, "DISCONNECT", strlen("DISCONNECT"))){
			i = write(argmt->listenfd, "BYE\n", strlen("BYE\n"));
			pthread_mutex_lock(&smutex);
			i = write(argmt->sendfd, "BYE\n", strlen("BYE\n"));
			pthread_mutex_unlock(&smutex);
            pthread_mutex_lock(&cnct_mutex);
			client_n--;
            pthread_mutex_unlock(&cnct_mutex);
			break;
		}else{
			printf("%s", buf);
			memset(buf, 0, MAX_KEYLEN);
			strcpy(buf, "UNDEFINED PROTOCOL\n");
		}
		i = write(argmt->listenfd, buf, strlen(buf));
	}
	close(argmt->sendfd);
	close(argmt->listenfd);
	free(argmt);
	pthread_exit(NULL);
}

void* async_get(void* arg){
	int n, i;
	char buf[MAX_KEYLEN];
	char* val;
	asyget_t* argsyn = (asyget_t*)arg;
	pthread_detach(pthread_self());
	memset(buf, 0, MAX_KEYLEN);
	pthread_mutex_lock(&argsyn->countmutex);
	while(*argsyn->asy_num >= MAX_ASYNC) pthread_cond_wait(&argsyn->countcond, &argsyn->countmutex);
	*argsyn->asy_num += 1;
	for(i=0;i<MAX_ASYNC;i++){
		if(!argsyn->asy[i].valid) break; 
	}
	pthread_mutex_unlock(&argsyn->countmutex);
	sprintf(buf, "AGETOK %d\n", i+1);
	pthread_mutex_lock(&argsyn->sendmutex);
	n = write(argsyn->sendfd, buf, strlen(buf));
	pthread_mutex_unlock(&argsyn->sendmutex);
	val = db_get(argsyn->key, strlen(argsyn->key));
	pthread_mutex_lock(&argsyn->countmutex);
	argsyn->asy[i].valid = 1;
	argsyn->asy[i].key = argsyn->key;
	argsyn->asy[i].value = val;
	pthread_cond_signal(&argsyn->findcond);
	pthread_mutex_unlock(&argsyn->countmutex);
	free(argsyn);
	n = n;
	pthread_exit(NULL);
}

void* async_test(void* arg){
	int n;
	char buf[MAX_KEYLEN];
	asywait_t* argsyn = (asywait_t*)arg;
	pthread_detach(pthread_self());
	memset(buf, 0, MAX_KEYLEN);
	pthread_mutex_lock(&argsyn->countmutex);
	if(argsyn->asy[argsyn->tag-1].key != NULL){
		if(argsyn->asy[argsyn->tag-1].valid){
			if(argsyn->asy[argsyn->tag-1].value == NULL){
				strcpy(buf, "GETINV\n");
				free(argsyn->asy[argsyn->tag-1].key);
				argsyn->asy[argsyn->tag-1].key = NULL;
				argsyn->asy[argsyn->tag-1].valid = 0;
				*argsyn->asy_num -= 1;
				pthread_cond_signal(&argsyn->countcond);
			}else{
				sprintf(buf, "GETOK %s %s\n", argsyn->asy[argsyn->tag-1].key, argsyn->asy[argsyn->tag-1].value);
				free(argsyn->asy[argsyn->tag-1].key);
				free(argsyn->asy[argsyn->tag-1].value);
				argsyn->asy[argsyn->tag-1].key = NULL;
				argsyn->asy[argsyn->tag-1].value = NULL;
				argsyn->asy[argsyn->tag-1].valid = 0;
				*argsyn->asy_num -= 1;
				pthread_cond_signal(&argsyn->countcond);
			}
		}else sprintf(buf, "%d_BUSY\n", argsyn->tag);
	}else strcpy(buf, "AINV\n");
	pthread_mutex_unlock(&argsyn->countmutex);
	pthread_mutex_lock(&argsyn->sendmutex);
	n = write(argsyn->sendfd, buf, strlen(buf));
	pthread_mutex_unlock(&argsyn->sendmutex);
	free(argsyn);
	n = n;
	pthread_exit(NULL);
}

void* async_wait(void* arg){
	int n;
	char buf[MAX_KEYLEN];
	asywait_t* argsyn = (asywait_t*)arg;
	pthread_detach(pthread_self());
	pthread_mutex_lock(&argsyn->countmutex);
	if(argsyn->asy[argsyn->tag-1].key != NULL){
		while(!argsyn->asy[argsyn->tag-1].valid) pthread_cond_wait(&argsyn->findcond, &argsyn->countmutex);
		if(argsyn->asy[argsyn->tag-1].value == NULL){
			strcpy(buf, "GETINV\n");
			free(argsyn->asy[argsyn->tag-1].key);
			argsyn->asy[argsyn->tag-1].key = NULL;
			argsyn->asy[argsyn->tag-1].valid = 0;
			*argsyn->asy_num -= 1;
			pthread_cond_signal(&argsyn->countcond);
		}else{
			sprintf(buf, "GETOK %s %s\n", argsyn->asy[argsyn->tag-1].key, argsyn->asy[argsyn->tag-1].value);
			free(argsyn->asy[argsyn->tag-1].key);
			free(argsyn->asy[argsyn->tag-1].value);
			argsyn->asy[argsyn->tag-1].key = NULL;
			argsyn->asy[argsyn->tag-1].value = NULL;
			argsyn->asy[argsyn->tag-1].valid = 0;
			*argsyn->asy_num -= 1;
			pthread_cond_signal(&argsyn->countcond);
		}
	}else strcpy(buf, "AINV\n");	
	pthread_mutex_unlock(&argsyn->countmutex);
	pthread_mutex_lock(&argsyn->sendmutex);
	n = write(argsyn->sendfd, buf, strlen(buf));
	pthread_mutex_unlock(&argsyn->sendmutex);
	free(argsyn);
	n = n;
	pthread_exit(NULL);
}