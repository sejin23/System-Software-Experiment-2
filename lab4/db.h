#define MAX_KEYLEN 1024
#define MAX_DIR 32
#define MAX_FILE 3000
typedef struct _node{
	int value, offset;
	char* key;
	struct _node* next;
}node;

typedef struct db {
	node* head;
}db_t;

db_t* db_open(int size, int t_num);
void* th_create(void* arg);
void db_close(db_t* db);
int db_store(db_t* db, char* key, int keylen);
void* th_file_put(void* arg);
int hash_func(char* str, int size);
