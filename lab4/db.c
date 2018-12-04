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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
pthread_mutex_t gmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gcond = PTHREAD_COND_INITIALIZER;
int db_s, th_n, kv_s;

db_t* db_open(int size, int t_num) {
	int i, fd;
	char dir[MAX_DIR] = "./db";
	db_t* db = NULL;
	th_n = t_num;
	db_s = size;
	kv_s = 0;
	if(opendir(dir) == NULL){
		mkdir(dir, 0755);
		for(i=0;i<MAX_FILE;i++){
			sprintf(dir, "./db/%d.txt", i);
			fd = open(dir, O_CREAT|O_WRONLY, 0755);
			close(fd);
		}
	}
	db = (db_t*)malloc(sizeof(db_t)*t_num);
	for(i=0;i<t_num;i++) db[i].head = NULL;
	return db;
}

void db_close(db_t* db) {
	int i;
	pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*th_n);
	for(i=0;i<th_n;i++)
		pthread_create(&tid[i], NULL, th_file_put, (void*)db[i].head);
	for(i=0;i<th_n;i++)
		pthread_join(tid[i], NULL);
	for(i=0;i<th_n;i++)
		free(db[i].head);
	free(db);
}

int db_get(db_t* db, char* key, int keylen, int* offset){
	int fd, wtp, keysize, value;
	int hashed = hash_func(key, db_s), fhash = hash_func(key, MAX_FILE), thash;
	char dir[MAX_FILE];
	char buf[MAX_KEYLEN];
	node *temp, *temp_n;
	//memory search
	thash = fhash%th_n;
	if(thash < 0) thash += th_n;
	if(db[thash].head != NULL){
		temp = db[thash].head;
		if(temp[hashed].key != NULL){
			temp_n = &temp[hashed];
			while(temp_n != NULL){
				if(!strcmp(key, temp_n->key)){
					*offset = temp_n->offset;
					return temp_n->value;
				}
				temp_n = temp_n->next;
			}
		}
	}
	kv_s++;
	//file search
	value = 0;
	sprintf(dir, "./db/%d.txt", fhash);
	fd = open(dir, O_RDONLY);
	while((wtp = read(fd, &keysize, sizeof(int))) > 0){
		memset(buf, 0, MAX_KEYLEN);
		wtp = read(fd, buf, keysize);
		buf[keysize] = '\0';
		if(!strcmp(buf, key)){
			*offset = lseek(fd, 0, SEEK_CUR);
			wtp = read(fd, &value, sizeof(int));
			break;
		}
		lseek(fd, sizeof(int), SEEK_CUR);
	}
	close(fd);
	return value;
}

void db_put(db_t* db, char* key, int keylen, int val, int offset){
	int hashed = hash_func(key, db_s), fhash = hash_func(key, MAX_FILE), thash, i;
	node *temp, *temp_p, *temp_n;
	pthread_t* tid;
	// memory store
	thash = fhash%th_n;
	if(thash < 0) thash += th_n;
	if(db[thash].head == NULL){
		db[thash].head = (node*)malloc(sizeof(node)*db_s);
		temp = db[thash].head;
		for(i=0;i<db_s;i++){
			temp[i].key = NULL;
			temp[i].next = NULL;
		}
		temp[hashed].key = (char*)malloc(keylen+1);
		strcpy(temp[hashed].key, key);
		temp[hashed].offset = offset;
		temp[hashed].value = val;
	}else{
		temp = db[thash].head;
		if(temp[hashed].key == NULL){
			temp[hashed].key = (char*)malloc(keylen+1);
			strcpy(temp[hashed].key, key);
			temp[hashed].value = val;
			temp[hashed].offset = offset;
		}else{
			temp_n = &temp[hashed];
			while(1){
				if(!strcmp(key, temp_n->key)){
					temp_n->value = val;
					return;
				}
				if(temp_n->next == NULL) break;
				temp_n = temp_n->next;
			}
			temp_p = (node*)malloc(sizeof(node));
			temp_p->key = (char*)malloc(keylen+1);
			strcpy(temp_p->key, key);
			temp_p->next = NULL;
			temp_p->value = val;
			temp_p->offset = offset;
			temp_n->next = temp_p;
		}
	}
	//file store
	if(kv_s < db_s) return;
	kv_s = 0;
	tid = (pthread_t*)malloc(sizeof(pthread_t)*th_n);
	for(i=0;i<th_n;i++)
		pthread_create(&tid[i], NULL, th_file_put, (void*)db[i].head);
	for(i=0;i<th_n;i++)
		pthread_join(tid[i], NULL);
	for(i=0;i<th_n;i++){
		free(db[i].head);
		db[i].head = NULL;
	}
	free(tid);
	return;
}

void* th_file_put(void* arg){
	int fd, keysize, wtp;
	int i;
	char dir[MAX_DIR];
	node *head = (node*)arg;
	node *temp, *temp_p;
	if(head == NULL) pthread_exit(NULL);
	for(i=0;i<db_s;i++){
		if(head[i].key == NULL) continue;
		temp = &head[i];
		while(temp != NULL){
			temp_p = temp->next;
			sprintf(dir, "./db/%d.txt", hash_func(temp->key, MAX_FILE));
			fd = open(dir, O_CREAT | O_WRONLY, 0755);
			if(temp->offset == -1){
				lseek(fd, 0, SEEK_END);
				keysize = strlen(temp->key);
				wtp = write(fd, &keysize, sizeof(int));
				wtp = write(fd, temp->key, keysize);
			}else lseek(fd, temp->offset, SEEK_SET);
			wtp = write(fd, &temp->value, sizeof(int));
			free(temp->key);
			if(temp_p != head[i].next) free(temp);
			close(fd);
			temp = temp_p;
		}
	}
	wtp = wtp;
	pthread_exit(NULL);
}

int hash_func(char* str, int size){
	int i, len, ret = 1;
	len = strlen(str);
	for(i=0;i<len;i++)
		ret *= str[i];
	if(ret%size < 0) return ret%size + size;
	return ret%size;
}