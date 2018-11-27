#define MAX_KEYLEN 1024
#define MAX_DIR 32
typedef struct _node{
	int value, num_f, offset_v;
	char* key;
	struct _node* next;
}node;

typedef struct db {
	node* head;
}db_t;

typedef struct _params{
	char* key;
	int t_num, t_max;
	node* node_s;
}params;

db_t* db_open(int size, int t_num);
void db_close(db_t* db, int t_num);
int db_store(db_t* db, char* key, int keylen, int thread_n);
int hash_func(char* str);
