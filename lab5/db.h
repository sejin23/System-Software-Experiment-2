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
#define MAX_ASYNC 256
#define MAX_KEYLEN 1024
typedef struct db {
	int origin, count;
	char *key, *value;
	struct db* next;
}db_t;

typedef struct _async {
	int valid, cfd;
	char *key, *value;
}async_t;

typedef struct _asyget {
	int tag;
	char* key;
}asyget_t;

typedef struct _asywait {
	int tag, cfd;
}asywait_t;

int client_n, contact_n, person, db_s, kv_s, asy_cmd;
db_t* DB;
async_t* Asynn;
pthread_mutex_t* mtx;
pthread_mutex_t cnct_mutex, dbs_mutex, kv_mutex, asynn_mutex;
pthread_cond_t cnct_cond, asynn_cond;

db_t* db_open(int size);
void db_close(db_t* db);
char* db_get(char* key, int keylen);
void db_put(char* key, int keylen, char* val, int vallen);
void db_put_file();
int hash_func(char* str, int size);

void* thread_main(void* arg);
void* async_get(void* arg);
void* async_test(void* arg);
void* async_wait(void* arg);
void* readpth(void* arg);
void* writepth(void* arg);
