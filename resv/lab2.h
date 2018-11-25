typedef struct Node {
	int value;
	char* key;
	struct Node* next;
}list;


typedef struct db {
	struct Node* link;
}db_t;

typedef struct filep{
	int file;
	int point;
	struct filep* next;
}filedb;

typedef struct {
	filedb* head;
}thirdb;

typedef struct {
	thirdb* head;
}secondb;

typedef struct {
	secondb* head;
}firstdb;
db_t* db_open(int size);
void db_close(db_t* db);
void db_put(db_t* db, char* key, int keylen, char* val, int vallen);
char* db_get(db_t* db, char* key, int keylen, int* vallen);
char* search_file(char* file_name, char* key, int hashed, int sdir);
int hash_function(char* key, int keylen);
int second_hash(char* key, int keylen);
int third_hash(char* key, int keylen);
