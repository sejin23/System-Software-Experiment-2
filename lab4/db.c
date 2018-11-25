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
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
int db_s, file_s, kv_s;

db_t* db_open(int size, int t_num) {
	int i, max_n = 0;
	char* dir = "./db";
	db_t* db = NULL;
	DIR* dp = NULL;
	struct dirent* dir_entry;
	db_s = size;
	kv_s = 0;
	if((dp = opendir(dir)) != NULL){
		while((dir_entry = readdir(dp)) != NULL){
			if(dir_entry->d_type == 8){
				if(max_n < atoi(dir_entry->d_name))
					max_n = atoi(dir_entry->d_name);
			}
		}
		file_s = max_n;
	}else{
		file_s = 0;
		mkdir(dir, 0755);
	}
	db = (db_t*)malloc(sizeof(db_t)*t_num);
	for(i=0;i<t_num;i++)
		db[i].head = NULL;
	return db;
}

void db_close(db_t* db, int t_num) {
	int i, j;
	node* temp;
	node* prev;
	node* temp2;
	for(i=0;i<t_num;i++){
		if(db[i].head != NULL){
			temp = db[i].head;
			for(j=0;j<db_s;j++){
				if(temp[i].value > -1){
					temp2 = &temp[i];
					while(temp2 != NULL){
						prev = temp2->next;
						free(temp2->key);
						free(temp2);
						temp2 = prev;
					}
				}
			}
			free(temp);
		}
	}
	free(db);
}

void db_put(db_t* db, char* key, int keylen, int val, int vallen, int thread_n, int file_n) {
	int hashed = hash_func(key);
	int i, index;
	pthread_t* tid;
	node* temp;
	node* temp2;
	node* new_temp;
	params* arg;
	if(file_n == -1 || file_n == file_s) index = thread_n - 1;
	else index = file_n % (thread_n - 1);
	if(db[index].head == NULL){
		if(kv_s < db_s){
			temp = (node*)malloc(sizeof(node)*db_s);
			for(i=0;i<db_s;i++){
				temp[i].key = NULL;
				temp[i].num_f = -1;
				temp[i].value = -1;
				temp[i].next = NULL;
			}
			temp[hashed].key = strdup(key);
			temp[hashed].value = val;
			temp[hashed].num_f = file_n;
			db[index].head = temp;
			kv_s++;
			return;
		}
	}else{
		temp = db[index].head;
		if(temp[hashed].key == NULL){	//
			if(kv_s < db_s){
				temp[hashed].key = strdup(key);
				temp[hashed].value = val;
				temp[hashed].num_f = file_n;
				kv_s++;
				return;
			}
		}else{
			temp2 = &temp[hashed];
			while(1){
				if(!strcmp(temp2->key, key)){
					temp2->value = val;
					return;
				}
				if(temp2->next == NULL) break;
				temp2 = temp2->next;
			}
			if(kv_s < db_s){
				new_temp = (node*)malloc(sizeof(node));
				new_temp->key = strdup(key);
				new_temp->value = val;
				new_temp->next = NULL;
				new_temp->num_f = file_n;
				temp2->next = new_temp;
				kv_s++;
				return;
			}
		}
	}
	kv_s = 1;
	tid = (pthread_t*)malloc(sizeof(pthread_t)*thread_n);
	arg = (params*)malloc(sizeof(params)*thread_n);
	for(i=0;i<thread_n;i++){
		arg[i].t_max = thread_n;
		arg[i].t_num = i;
		arg[i].node_s = db[i].head;
		pthread_create(&tid[i], NULL, thread_put, (void*)&arg[i]);
	}
	for(i=0;i<thread_n;i++)
		pthread_join(tid[i], NULL);
}
int db_get(db_t* db, char* key, int keylen, int* vallen, int thread_n, int* file_n) {
	int value = -1;
	int hashed = hash_func(key);
	return value;
}

int hash_func(char* str){
	int i, len, ret = 0;
	len = strlen(str);
	for(i=0;i<len;i++){
		ret *= str[i];
	}
	return ret%db_s;
}

void* thread_put(void* arg){
	params* arg_s = (params*)arg;
	int id_n, id_m, i, kv_num = 0;
	int fd, fd_s, keylen;
	char dir_f[MAX_DIR];
	char buf[MAX_KEYLEN];
	off_t zero = 0;
	off_t offset;
	off_t prev_off;
	off_t next_off;
	node* head_n = arg_s->node_s;
	node* temp;
	id_n = arg_s->t_num;
	id_m = arg_s->t_max;
	if(head_n == NULL) return NULL;
	if(arg_s->t_num == arg_s->t_max - 1){
		sprintf(dir_f, "./%d", file_s);
		fd_s = open(dir_f, O_CREAT | O_RDWR | O_APPEND, 0755);
		lseek(fd_s, 0, SEEK_SET);
		offset = 0;
		if(read(fd_s, &kv_num, sizeof(int)) == 0){
			kv_num = 0;
			lseek(fd_s, 0, SEEK_SET);
			write(fd_s, &kv_num, sizeof(int));
			for(i=0;i<db_s;i++)
				write(fd_s, &offset, sizeof(off_t));
		}
		for(i=0;i<db_s;i++){
			if(head_n[i].key == NULL)
				continue;
			temp = &head_n[i];
			while(temp != NULL && kv_num < db_s){
				prev_off = lseek(fd_s, sizeof(off_t)*i+sizeof(int), SEEK_SET);
				read(fd_s, &offset, sizeof(off_t));
				keylen = strlen(temp->key);
				if(offset == 0){
					next_off = lseek(fd_s, 0, SEEK_END);
					write(fd_s, &keylen, sizeof(int));
					write(fd_s, temp->key, strlen(temp->key));
					wrtie(fd_s, &temp->value, sizeof(int));
					offset = lseek(fd_s, 0, SEEK_CUR);
					write(fd_s, &zero, size_t(off_t));
					lseek(fd_s, prev_off, SEEK_SET);
					write(fd_s, &next_off, sizeof(off_t));
				}else{
					while(1){
						lseek(fd_s, offset, SEEK_SET);
						read(fd_s, &keylen, sizeof(int));
						read(fd_s, buf, sizeof(char)*keylen);
						buf[keylen] = '\0';
						if(!strcmp(buf, temp->key)){
							write(fd_s, &temp->value, sizeof(int));
							break;
						}
						read(fd_s, &keylen, sizeof(int));
						read(fd_s, &offset, sizeof(off_t));
						if(offset == 0) break;
					}
				}
				kv_num++;
				temp = temp->next;
			}
		}
	}else{

	}
	return NULL;
}