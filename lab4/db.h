#define MAX_KEYLEN 1024
#define MAX_DIR 64
typedef struct _node{
	int value, num_f;
	char* key;
	struct _node* next;
}node;

typedef struct db {
	node* head;
}db_t;

typedef struct _params{
	int t_num, t_max;
	node* node_s;
}params;
db_t* db_open(int size, int t_num);
void* thread_put(void* );
void db_close(db_t* db, int t_num);
void db_put(db_t* db, char* key, int keylen, int val, int vallen, int thread_n, int file_n);
int db_get(db_t* db, char* key, int keylen, int* vallen, int thread_n, int* file_n);
int hash_func(char* str);
