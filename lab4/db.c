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
int zero;
db_t* db_open(int size, int t_num) {
	int i, max_n = 0;
	char* dir = "./db";
	db_t* db = NULL;
	DIR* dp = NULL;
	struct dirent* dir_entry;
	zero = 0;
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
				if(temp[i].key != NULL){
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

void db_put(db_t* db, char* key, int keylen, int val, int vallen, int thread_n, int file_n, int offset) {
	int i, index, hashed = hash_func(key);
	pthread_t* tid;
	node* temp, *temp2, *new_temp;
	params* arg;
	if(file_n == -1 || file_n == file_s) index = thread_n - 1;
	else index = file_n % (thread_n - 1);
	if(db[index].head == NULL){
		if(kv_s < db_s){
			temp = (node*)malloc(sizeof(node)*db_s);
			for(i=0;i<db_s;i++){
				temp[i].key = NULL;
				temp[i].next = NULL;
			}
			temp[hashed].key = strdup(key);
			temp[hashed].value = val;
			temp[hashed].num_f = file_n;
			temp[hashed].offset_v = offset;
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
				temp[hashed].offset_v = offset;
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
				new_temp->offset_v = offset;
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
	for(i=0;i<thread_n;i++){
		pthread_join(tid[i], NULL);
		free(db[i].head);
		db[i].head = NULL;
	}
	temp = (node*)malloc(sizeof(node)*db_s);
	for(i=0;i<db_s;i++){
		temp[i].key = NULL;
		temp[i].next = NULL;
	}
	temp[hashed].key = strdup(key);
	temp[hashed].value = val;
	temp[hashed].num_f = file_n;
	temp[hashed].offset_v = offset;
	db[index].head = temp;
	return;
}

int db_get(db_t* db, char* key, int keylen, int* vallen, int thread_n, int* file_n, int* offset) {
	int i, value = -1;
	int hashed = hash_func(key);
	int* ret;
	pthread_t* tid;
	params* argm;
	tid = (pthread_t*)malloc(sizeof(pthread_t)*thread_n);
	argm = (params*)malloc(sizeof(params)*thread_n);
	for(i=0;i<thread_n;i++){
		argm[i].key = (char*)malloc(sizeof(char)*strlen(key)+1);
		strcpy(argm[i].key, key);
		argm[i].node_s = db[i].head;
		pthread_create(&tid[i], NULL, get_m, (void*)&argm[i]);
	}
	for(i=0;i<thread_n;i++){
		pthread_join(tid[i], (void**)&ret);
		if(ret != NULL){
			value = ret[0];
			*file_n = ret[1];
			*offset = ret[2];
		}
	}
	if(value == -1){
		for(i=0;i<thread_n;i++){
			argm[i].t_num = i;
			argm[i].t_max = thread_n;
			pthread_create(&tid[i], NULL, thread_get, (void*)&argm[i]);
		}
		for(i=0;i<thread_n;i++){
			pthread_join(tid[i], );
		}
	}
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

void* get_m(void* arg){
	int i, hashed;
	int ret[3];
	params* arg_s = (params*)arg;
	node* head_n = arg_s->node_s;
	node* temp;
	if(head_n == NULL) return (void*)NULL;
	hashed = hash_func(arg_s->key);
	if(head_n[hashed].key != NULL){
		temp = &head_n[hashed];
		while(temp != NULL){
			if(!strcmp(temp->key, arg_s->key)){
				ret[0] = temp->value;
				ret[1] = temp->num_f;
				ret[2] = temp->offset_v;
				return (void*)ret;
			}
			temp = temp->next;
		}
	}
	return (void*)NULL;
}

void* thread_put(void* arg){
	params* arg_s = (params*)arg;
	int i, j, file_copy, kv_num = 0;
	int fd, fd_s, keylen, len;
	char dir_f[MAX_DIR];
	char buf[MAX_KEYLEN];
	int offset, prev_off, next_off;
	node* head_n = arg_s->node_s;
	node* temp, *pre_temp;
	fd = 0;
	if(head_n == NULL) return NULL;
	if(arg_s->t_num == arg_s->t_max - 1){
		file_copy = file_s;
		sprintf(dir_f, "./%d", file_s);
		fd_s = open(dir_f, O_CREAT | O_RDWR | O_APPEND, 0755);
		lseek(fd_s, 0, SEEK_SET);
		offset = 0;
		if((len = read(fd_s, &kv_num, sizeof(int))) <= 0){
			lseek(fd_s, 0, SEEK_SET);
			for(i=0;i<db_s+1;i++)
				write(fd_s, &zero, sizeof(int));
		}
		for(i=0;i<db_s;i++){
			if(head_n[i].key == NULL)
				continue;
			prev_off = sizeof(int)*(i+1);
			read(fd_s, &offset, sizeof(int));
			if(offset != 0){
				while(1){
					lseek(fd_s, offset, SEEK_SET);
					read(fd_s, &len, sizeof(int));
					read(fd_s, buf, sizeof(char)*len);
					read(fd_s, &len, sizeof(int));
					prev_off = lseek(fd_s, 0, SEEK_CUR);
					read(fd_s, &next_off, sizeof(int));
					if(next_off == 0) break;
					offset = next_off;
				}
			}
			temp = &head_n[i];
			while(temp != NULL){
				keylen = strlen(temp->key);
				if(temp->num_f == -1){
					offset = lseek(fd_s, 0, SEEK_END);
					write(fd_s, &keylen, sizeof(int));
					write(fd_s, temp->key, sizeof(char)*keylen);
					write(fd_s, &temp->value, sizeof(int));
					next_off = lseek(fd_s, 0, SEEK_CUR);
					write(fd_s, &zero, sizeof(int));
					lseek(fd_s, prev_off, SEEK_SET);
					write(fd_s, &offset, sizeof(int));
					prev_off = next_off;
					kv_num++;
				}else{
					if(file_copy == file_s){
						lseek(fd_s, temp->offset_v, SEEK_SET);
						write(fd_s, &temp->value, sizeof(int));
					}else{
						lseek(fd, temp->offset_v, SEEK_SET);
						write(fd, &temp->value, sizeof(int));
					}
				}
				if(kv_num == db_s){
					file_s++;
					lseek(fd_s, 0, SEEK_SET);
					write(fd_s, &kv_num, sizeof(int));
					fd = fd_s;
					memset(dir_f, '\0', MAX_DIR);
					sprintf(dir_f, "./%d", file_s);
					fd_s = open(dir_f, O_CREAT | O_RDWR, 0755);
					for(j=0;j<db_s+1;j++)
						write(fd_s, &zero, sizeof(int));
					kv_num = 0;
					prev_off = sizeof(int)*(i+1);
				}
				pre_temp = temp->next;
				if(temp != &head_n[i]){
					free(temp->key);
					free(temp);
				}else{
					free(head_n[i].key);
					head_n[i].key = NULL;
					head_n[i].next = NULL;
				}
				temp = pre_temp;
			}
		}
		if(fd > 0) close(fd);
		close(fd_s);
	}else{
		for(i=0;i<db_s;i++){
			if(head_n[i].key == NULL)
				continue;
			temp = &head_n[i];
			while(temp != NULL){
				sprintf(dir_f, "./%d", temp->num_f);
				fd_s = open(dir_f, O_WRONLY);
				lseek(fd_s, temp->offset_v, SEEK_SET);
				write(fd_s, &temp->value, sizeof(int));
				close(fd_s);
				pre_temp = temp->next;
				if(temp != &head_n[i]){
					free(temp->key);
					free(temp);
				}else{
					free(head_n[i].key);
					head_n[i].key = NULL;
					head_n[i].next = NULL;
				}
				temp = pre_temp;
			}
		}
	}
	return NULL;
}

void* thread_get(void* arg){
	int i, id_s, max_s, fd;
	int hashed, offset;
	params* arg_s = (params*)arg;
	node* head_n = arg_s->node_s;
	char* key = (char*)malloc(sizeof(char)*strlen(arg_s->key)+1);
	char dir_f[MAX_DIR];
	strcpy(key, arg_s->key);
	hashed = hash_func(key);
	max_s = arg_s->t_max;
	id_s = arg_s->t_num;
	for(i=id_s;i<file_s;i+=max_s){
		sprintf(dir_f, "./%d", i);
		fd = open(dir_f, O_RDONLY);
		lseek(fd, (hashed+1)*sizeof(int), SEEK_SET);
		read(fd, &offset, sizeof(int));
		if()	//////////////////////////////////
	}
}