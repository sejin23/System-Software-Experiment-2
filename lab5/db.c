/*-----------------------------------------------------------
 * SSE2033: System Software Experiment 2 (Fall 2018)
 *
 * Skeleton code for PA#1
 * 
 * CSLab, Sungkyunkwan University
 *
 * Student Id   : 2014313299
 * Student Name : Park Sejin
 *-----------------------------------------------------------
 */
#include "db.h"

db_t* db_open(int size) {
	int i, fd;
	char dir[MAX_DIR] = "./db";
	db_t* db = (db_t*)malloc(sizeof(db_t)*size);;
	db_s = size;
	kv_s = 0;
	if(opendir(dir) == NULL){
		mkdir(dir, 0755);
		for(i=0;i<MAX_KEYLEN;i++){
			sprintf(dir, "./db/%d.key", i);
			fd = open(dir, O_CREAT | O_WRONLY, 0755);
			close(fd);
		}
	}
	for(i=0;i<size;i++){
		db[i].key = NULL;
		db[i].value = NULL;
		db[i].next = NULL;
	}
	return db;
}

void db_close(db_t* db) {
	int i;
	db_t *temp, *temp_n;
	for(i=0;i<db_s;i++){
		if(db[i].key == NULL) continue;
		temp = &db[i];
		while(temp != NULL){
			temp_n = temp->next;
			free(temp->key);
			free(temp->value);
			if(temp_n != db[i].next) free(temp);
			temp = temp_n;
		}
	}
	free(db);
}

char* db_get(db_t* db, char* key, int keylen){
	int fd, wtp, keysize, vlid, cnt;
	int mhash = hash_func(key, db_s), fhash = hash_func(key, MAX_KEYLEN);
	char dir[MAX_DIR];
	char buf[MAX_KEYLEN];
	char* value;
	db_t *temp, *temp_n;
	vlid = 0;
	if(db[mhash].key != NULL){
		temp = &db[mhash];
		vlid = 1;
		while(1){
			if(!strcmp(key, temp->key)){
				value = (char*)malloc(strlen(temp->value)+1);
				strcpy(value, temp->value);
				return value;
			}
			if(temp->next == NULL) break;
			temp = temp->next;
		}
	}
	cnt = 0;
	memset(buf, 0, MAX_KEYLEN);
	sprintf(dir, "./db/%d.key", fhash);
	fd = open(dir, O_RDONLY);
	while((wtp = read(fd, &keysize, sizeof(int))) > 0){
		wtp = read(fd, buf, keysize);
		buf[keysize] = '\0';
		if(!strcmp(buf, key)){
			close(fd);
			sprintf(dir, "./db/%d_%d.val", fhash, cnt);
			fd = open(dir, O_RDONLY);
			wtp = read(fd, &keysize, sizeof(int));
			value = (char*)malloc(keysize+1);
			wtp = read(fd, value, keysize);
			value[keysize] = '\0';
			close(fd);
			kv_s++;
			if(kv_s < db_s){
				if(vlid){
					temp_n = (db_t*)malloc(sizeof(db_t));
					temp_n->key = (char*)malloc(keylen+1);
					temp_n->value = (char*)malloc(keysize+1);
					strcpy(temp_n->key, key);
					strcpy(temp_n->value, value);
					temp_n->count = cnt;
					temp_n->origin = 1;
					temp_n->next = NULL;
					temp->next = temp_n;
				}else{
					db[mhash].key = (char*)malloc(keylen+1);
					db[mhash].value = (char*)malloc(keysize+1);
					strcpy(db[mhash].key, key);
					strcpy(db[mhash].value, value);
					db[mhash].count = cnt;
					db[mhash].origin = 1;
					db[mhash].next = NULL;
				}
			}else{
				kv_s = 0;
				db_put_file(db);
			}
			return value;
		}
		cnt++;
	}
	close(fd);
	return NULL;
}

void db_put(db_t* db, char* key, int keylen, char* val, int vallen){
	int mhash = hash_func(key, db_s);
	db_t *temp, *temp_n;
	if(db[mhash].key == NULL){
		db[mhash].key = (char*)malloc(keylen+1);
		db[mhash].value = (char*)malloc(vallen+1);
		db[mhash].count = -1;
		db[mhash].origin = 0;
		db[mhash].next = NULL;
		strcpy(db[mhash].key, key);
		strcpy(db[mhash].value, val);
		kv_s++;
	}else{
		temp = &db[mhash];
		while(1){
			if(!strcmp(key, temp->key)){
				free(temp->value);
				temp->value = (char*)malloc(vallen+1);
				temp->origin = 0;
				strcpy(temp->value, val);
				return;
			}
			if(temp->next == NULL) break;
			temp = temp->next;
		}
		temp_n = (db_t*)malloc(sizeof(db_t));
		temp_n->key = (char*)malloc(keylen+1);
		temp_n->value = (char*)malloc(vallen+1);
		strcpy(temp_n->key, key);
		strcpy(temp_n->value, val);
		temp_n->count = -1;
		temp_n->origin = 0;
		temp_n->next = NULL;
		temp->next = temp_n;
		kv_s++;
	}
	if(kv_s == db_s){
		kv_s = 0;
		db_put_file(db);
	}
	return;
}

void db_put_file(db_t* db){
	int i, size, fd_k, fd_v, wtp, keysize, fhash, cnt, vlid;
	char dir[MAX_DIR];
	char key[MAX_KEYLEN];
	db_t *temp, *temp_n;
	size = db_s;
	for(i=0;i<size;i++){
		if(db[i].key == NULL) continue;
		temp = &db[i];
		while(temp != NULL){
			temp_n = temp->next;
			if(temp->origin){
				free(temp->key);
				free(temp->value);
				if(temp_n != db[i].next) free(temp);
				temp = temp_n;				
				continue;
			}
			memset(dir, 0, MAX_DIR);
			fhash = hash_func(temp->key, MAX_KEYLEN);
			if(temp->count == -1){
				sprintf(dir, "./db/%d.key", fhash);
				fd_k = open(dir, O_RDWR);
				cnt = 0;
				vlid = 0;
				while((wtp = read(fd_k, &keysize, sizeof(int))) > 0){
					wtp = read(fd_k, key, keysize);
					key[keysize] = '\0';
					if(!strcmp(key, temp->key)){
						vlid = 1;
						break;
					}
					cnt++;
				}
				if(vlid == 0){
					lseek(fd_k, 0, SEEK_END);
					keysize = strlen(temp->key);
					wtp = write(fd_k, &keysize, sizeof(int));
					wtp = write(fd_k, temp->key, keysize);
				}
				close(fd_k);
			}else cnt = temp->count;
			sprintf(dir, "./db/%d_%d.val", fhash, cnt);
			fd_v = open(dir, O_CREAT|O_WRONLY, 0755);
			keysize = strlen(temp->value);
			wtp = write(fd_v, &keysize, sizeof(int));
			wtp = write(fd_v, temp->value, keysize);
			close(fd_v);
			free(temp->key);
			free(temp->value);
			if(temp_n != db[i].next) free(temp);
			temp = temp_n;
		}
		db[i].key = NULL;
		db[i].next = NULL;
	}
}

int hash_func(char* str, int size){
	int i, len, ret = 1;
	len = strlen(str);
	for(i=0;i<len;i++)
		ret *= str[i];
	if(ret%size < 0) return ret%size + size;
	return ret%size;
}