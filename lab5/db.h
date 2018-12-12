#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_DIR 32
#define MAX_KEYLEN 1024
typedef struct db {
	int origin, count;
	char *key, *value;
	struct db* next;
}db_t;

typedef struct _user {
	pthread_mutex_t serv;
	struct _user *prev, *next;
}user_t;

typedef struct _arg {
	int listenfd, sendfd;
	user_t* mutex;
}arg_t;

int client_n, contact_n, person, db_s, kv_s;
db_t* DB;
user_t* USER;
pthread_mutex_t* mtx;
pthread_mutex_t cnct_mutex, dbs_mutex, kv_mutex, user_mutex;
pthread_cond_t cnct_cond;

db_t* db_open(int size);
void db_close(db_t* db);
char* db_get(char* key, int keylen);
void db_put(char* key, int keylen, char* val, int vallen);
void db_put_file();
int hash_func(char* str, int size);

void* thread_main(void* arg);
void* async_sock(void* arg);
void* readpth(void* arg);
void* writepth(void* arg);