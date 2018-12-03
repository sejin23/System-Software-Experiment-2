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
	int i, j, cnt;
	node *temp, *prev, *temp2;
	for(i=0;i<th_n;i++){
		if(db[i].head == NULL) continue;
		temp = db[i].head;
		for(j=0;j<db_s;j++){
			if(temp[j].key == NULL) continue;
			temp2 = &temp[j];
			cnt = 0;
			while(temp2 != NULL){
				prev = temp2->next;
				free(temp2->key);
				if(cnt == 1) free(temp2);
				cnt = 1;
				temp2 = prev;
			}
		}
		free(temp);
	}
	free(db);
}

int db_store(db_t* db, char* key, int keylen){
	int i, fd, wtp, keysize;
	int value = 0, offset = -1;
	int hashed = hash_func(key, db_s), fhash = hash_func(key, MAX_FILE), thash;
	char dir[MAX_FILE];
	char buf[MAX_KEYLEN];
	pthread_t* tid;
	node *temp, *temp_n, *temp_p;
	//memory search
	thash = fhash%th_n;
	if(thash < 0) thash += th_n;
	if(db[thash].head != NULL){
		temp = db[thash].head;
		if(temp[hashed].key != NULL){
			temp_n = &temp[hashed];
			while(temp_n != NULL){
				if(!strcmp(key, temp_n->key)){
					value = temp_n->value;
					temp_n->value++;
					return value;
				}
				temp_n = temp_n->next;
			}
		}
	}
	kv_s++;
	//file search
	sprintf(dir, "./db/%d.txt", fhash);
	fd = open(dir, O_RDONLY);
	while((wtp = read(fd, &keysize, sizeof(int))) > 0){
		memset(buf, 0, MAX_KEYLEN);
		wtp = read(fd, buf, keysize);
		buf[keysize] = '\0';
		if(!strcmp(buf, key)){
			offset = lseek(fd, 0, SEEK_CUR);
			wtp = read(fd, &value, sizeof(int));
			break;
		}else lseek(fd, sizeof(int), SEEK_CUR);
	}
	close(fd);
	//memory store
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
		temp[hashed].value = value+1;
	}else{
		temp = db[thash].head;
		if(temp[hashed].key == NULL){
			temp[hashed].key = (char*)malloc(keylen+1);
			strcpy(temp[hashed].key, key);
			temp[hashed].value = value+1;
			temp[hashed].offset = offset;
		}else{
			temp_n = &temp[hashed];
			while(temp_n->next != NULL)
				temp_n = temp_n->next;
			temp_p = (node*)malloc(sizeof(node));
			temp_p->key = (char*)malloc(keylen+1);
			strcpy(temp_p->key, key);
			temp_p->next = NULL;
			temp_p->value = value+1;
			temp_p->offset = offset;
			temp_n->next = temp_p;
		}
	}
	//file store
	if(kv_s < db_s) return value;
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
	/*for(i=0;i<MAX_FILE;i++){
		if(db[i].head == NULL) continue;
		temp = db[i].head;
		sprintf(dir, "./db/%d.txt", i);
		fd = open(dir, O_CREAT | O_WRONLY);
		for(j=0;j<db_s;j++){
			if(temp[j].key == NULL) continue;
			temp_n = &temp[j];
			while(temp_n != NULL){
				temp_p = temp_n->next;
				if(temp_n->offset == -1){
					lseek(fd, 0, SEEK_END);
					keysize = strlen(temp_n->key);
					wtp = write(fd, &keysize, sizeof(int));
					wtp = write(fd, temp_n->key, keysize);
				}else lseek(fd, temp_n->offset, SEEK_SET);
				wtp = write(fd, &temp_n->value, sizeof(int));
				free(temp_n->key);
				if(temp_p != temp[j].next) free(temp_n);
				temp_n = temp_p;
			}
		}
		close(fd);
		free(temp);
		db[i].head = NULL;
	}*/
	return value;
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