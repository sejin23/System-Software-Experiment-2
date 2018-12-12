#include "db.h"

int main(int argc, char** argv){
	int listen_serv_socket, send_serv_socket, listen_cli_socket, send_cli_socket, listenAddrLen, sendAddrLen, n;
	struct sockaddr_in listenserver, sendserver, listen_clientaddr, send_clientaddr;
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
	contact_n = 0;
	mtx = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*atoi(argv[3]));
	for(n=0;n<atoi(argv[3]);n++) pthread_mutex_init(&mtx[n], NULL);
	pthread_mutex_init(&cnct_mutex, NULL);
	pthread_mutex_init(&dbs_mutex, NULL);
	pthread_mutex_init(&kv_mutex, NULL);
	pthread_mutex_init(&user_mutex, NULL);
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
	int i, j;
	char buf[MAX_KEYLEN];
	char key[MAX_KEYLEN];
	char* val;
	user_t* temp;
	arg_t* argmt = (arg_t*)arg;
	pthread_mutex_t pmt;
	pthread_detach(pthread_self());
	pthread_mutex_lock(&user_mutex);
	pmt = argmt->mutex->serv;
	pthread_mutex_unlock(&user_mutex);

	pthread_mutex_lock(&cnct_mutex);
	client_n++;
	pthread_mutex_unlock(&cnct_mutex);
    
	while(1){
		memset(buf, 0, MAX_KEYLEN);
		memset(key, 0, MAX_KEYLEN);
		pthread_mutex_lock(&pmt);
		i = read(argmt->listenfd, buf, MAX_KEYLEN);
		pthread_mutex_unlock(&pmt);
		if(i <= 0) break;
		if(!strncmp(buf, "GET", strlen("GET"))){
			for(i=strlen("GET "),j=0;i<strlen(buf);i++,j++)
				key[j] = buf[i];
			key[j] = '\0';
			val = db_get(key, strlen(key));
			memset(buf, 0, MAX_KEYLEN);
			if(val == NULL) strcpy(buf, "GETINV\n");
			else{
				sprintf(buf, "GETOK %s %s\n", key, val);
				free(val);
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
		}else if(!strncmp(buf, "DISCONNECT", strlen("DISCONNECT"))){
			pthread_mutex_lock(&pmt);
			i = write(argmt->listenfd, "BYE\n", strlen("BYE\n"));
			i = write(argmt->sendfd, "BYE\n", strlen("BYE\n"));
			pthread_mutex_unlock(&pmt);
            pthread_mutex_lock(&cnct_mutex);
			client_n--;
            pthread_mutex_unlock(&cnct_mutex);
			break;
		}else{
			printf("%s", buf);
			memset(buf, 0, MAX_KEYLEN);
			strcpy(buf, "UNDEFINED PROTOCOL\n");
		}
		pthread_mutex_lock(&pmt);
		i = write(argmt->listenfd, buf, strlen(buf));
		pthread_mutex_unlock(&pmt);
	}
	pthread_mutex_lock(&user_mutex);
	temp = argmt->mutex;
	temp->prev->next = temp->next;
	pthread_mutex_destroy(&temp->serv);
	free(temp);
	pthread_mutex_unlock(&user_mutex); 
	close(argmt->sendfd);
	close(argmt->listenfd);
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