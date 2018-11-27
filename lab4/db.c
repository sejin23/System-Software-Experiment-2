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
pthread_t* tid;

db_t* db_open(int size, int t_num) {
	int i, max_n = -1;
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
		file_s = -1;
		mkdir(dir, 0755);
	}
	db = (db_t*)malloc(sizeof(db_t)*t_num);
	for(i=0;i<t_num;i++)
		db[i].head = NULL;
	return db;
}

void db_close(db_t* db, int t_num) {
	int i, j, cnt;
	node *temp, *prev, *temp2;
	for(i=0;i<t_num;i++){
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

int db_store(db_t* db, char* key, int keylen, int thread_n){
	int wtp, hashed = hash_func(key, db_s);
	int fnum = -1, offset = -1;
	int i, j, t, val, index, fd, fd_s;
	int len, point, prep, nex, cond;
	int db_prev, hash_prev;
	char buffer[MAX_KEYLEN];
	char dir[MAX_DIR];
	node *temp_p, *temp, *temp_n;
	val = 0;
	prep = 0;
	//memory search 쓰레드 1개일 때, 다른 db size가 들어 올때 변경
	for(i=0;i<thread_n;i++){
		if(db[i].head == NULL) continue;
		temp = db[i].head;
		if(temp[hashed].key == NULL) continue;
		temp_n = &temp[hashed];
		while(temp_n != NULL){
			if(!strcmp(temp_n->key, key)){
				val = temp_n->value;
				temp_n->value++;
				return val;
			}
			temp_n = temp_n->next;
		}
	}
	kv_s++;
	//file search
	for(i=0;i<thread_n;i++){
		for(j=i;j<=file_s;j+=thread_n){
			sprintf(dir, "./db/%d.txt", j);
			fd = open(dir, O_RDONLY);
			if((wtp = read(fd, &db_prev, sizeof(int))) == 0){
				close(fd);
				break;
			}
			hash_prev = hash_func(key, db_prev);
			lseek(fd, (hash_prev+1)*sizeof(int), SEEK_CUR);
			wtp = read(fd, &point, sizeof(int));
			while(point != 0){
				lseek(fd, point, SEEK_SET);
				wtp = read(fd, &len, sizeof(int));
				wtp = read(fd, buffer, sizeof(char)*len);
				buffer[len] = '\0';
				if(!strcmp(buffer, key)){
					offset = lseek(fd, 0, SEEK_CUR);
					wtp = read(fd, &val, sizeof(int));
					fnum = j;
					break;
				}
				lseek(fd, sizeof(int), SEEK_CUR);
				wtp = read(fd, &point, sizeof(int));
			}
			close(fd);
			if(val > 0) break;
		}
		if(val > 0) break;
	}
	//memory store
	if(val == 0){
		index = thread_n - 1;
		if(db[index].head == NULL){
			db[index].head = (node*)malloc(sizeof(node)*db_s);
			temp = db[index].head;
			for(i=0;i<db_s;i++){
				temp[i].key = NULL;
				temp[i].next = NULL;
			}
			temp[hashed].key = (char*)malloc(strlen(key)+1);
			strcpy(temp[hashed].key, key);
			temp[hashed].num_f = -1;
			temp[hashed].offset_v = -1;
			temp[hashed].value = 1;
		}else{
			temp = db[index].head;
			if(temp[hashed].key == NULL){
				temp[hashed].key = (char*)malloc(strlen(key)+1);
				strcpy(temp[hashed].key, key);
				temp[hashed].num_f = -1;
				temp[hashed].offset_v = -1;
				temp[hashed].value = 1;
				temp[hashed].next = NULL;
			}else{
				temp_n = &temp[hashed];
				while(temp_n->next != NULL)
					temp_n = temp_n->next;
				temp_p = (node*)malloc(sizeof(node));
				temp_p->key = (char*)malloc(strlen(key)+1);
				strcpy(temp_p->key, key);
				temp_p->next = NULL;
				temp_p->num_f = -1;
				temp_p->offset_v = -1;
				temp_p->value = 1;
				temp_n->next = temp_p;
			}
		}
	}else{
		if(file_s != fnum) index = fnum%(thread_n-1);
		else index = thread_n - 1;
		if(db[index].head == NULL){
			db[index].head = (node*)malloc(sizeof(node)*db_s);
			temp = db[index].head;
			for(i=0;i<db_s;i++){
				temp[i].key = NULL;
				temp[i].next = NULL;
			}
			temp[hashed].key = (char*)malloc(strlen(key)+1);
			strcpy(temp[hashed].key, key);
			temp[hashed].num_f = fnum;
			temp[hashed].offset_v = offset;
			temp[hashed].value = val+1;
		}else{
			temp = db[index].head;
			if(temp[hashed].key == NULL){
				temp[hashed].key = (char*)malloc(strlen(key)+1);
				strcpy(temp[hashed].key, key);
				temp[hashed].num_f = fnum;
				temp[hashed].offset_v = offset;
				temp[hashed].value = val+1;
				temp[hashed].next = NULL;
			}else{
				temp_n = &temp[hashed];
				while(temp_n->next != NULL)
					temp_n = temp_n->next;
				temp_p = (node*)malloc(sizeof(node));
				temp_p->key = (char*)malloc(strlen(key)+1);
				strcpy(temp_p->key, key);
				temp_p->next = NULL;
				temp_p->num_f = fnum;
				temp_p->offset_v = offset;
				temp_p->value = val+1;
				temp_n->next = temp_p;
			}
		}
	}
	//file store
	if(kv_s < db_s) return val;
	kv_s = 0;
	if(file_s == -1){
		strcpy(dir, "./db/0.txt");
		file_s = 0;
	}else sprintf(dir, "./db/%d.txt", file_s);
	fd_s = open(dir, O_CREAT | O_RDWR, 0755);
	wtp = read(fd_s, &db_prev, sizeof(int));
	if(wtp == 0){
		lseek(fd_s, 0, SEEK_SET);
		wtp = write(fd_s, &db_s, sizeof(int));
		for(i=0;i<=db_s;i++)
			wtp = write(fd_s, &zero, sizeof(int));
		cond = 0;
		db_prev = db_s;
	}else wtp = read(fd_s, &cond, sizeof(int));

	for(i=0;i<thread_n;i++){
		if(db[i].head == NULL)
			continue;
		temp = db[i].head;
		for(j=0;j<db_s;j++){
			if(temp[j].key == NULL)
				continue;
			if(i == thread_n - 1 && db_s == db_prev){
				prep = lseek(fd_s, sizeof(int)*(j+2), SEEK_SET);
				wtp = read(fd_s, &point, sizeof(int));
				while(point != 0){
					lseek(fd_s, point, SEEK_SET);
					wtp = read(fd_s, &len, sizeof(int));
					prep = lseek(fd_s, sizeof(char)*len+sizeof(int), SEEK_CUR);
					wtp = read(fd_s, &point, sizeof(int));
				}
			}
			temp_n = &temp[j];
			while(temp_n != NULL){
				temp_p = temp_n->next;
				if(temp_n->num_f == -1){
					if(db_s != db_prev){
						hash_prev = hash_func(temp_n->key, db_prev);
						prep = lseek(fd_s, (hash_prev+2)*sizeof(int), SEEK_SET);
						wtp = read(fd_s, &point, sizeof(int));
						while(point != 0){
							lseek(fd_s, point, SEEK_SET);
							wtp = read(fd_s, &len, sizeof(int));
							prep = lseek(fd_s, sizeof(char)*len+sizeof(int), SEEK_CUR);
							wtp = read(fd_s, &point, sizeof(int));
						}
					}
					point = lseek(fd_s, 0, SEEK_END);
					len = strlen(temp_n->key);
					wtp = write(fd_s, &len, sizeof(int));
					wtp = write(fd_s, temp_n->key, sizeof(char)*len);
					wtp = write(fd_s, &temp_n->value, sizeof(int));
					nex = lseek(fd_s, 0, SEEK_CUR);
					wtp = write(fd_s, &zero, sizeof(int));
					lseek(fd_s, prep, SEEK_SET);
					wtp = write(fd_s, &point, sizeof(int));
					cond++;
					prep = nex;
					if(cond == db_prev){
						lseek(fd_s, sizeof(int), SEEK_SET);
						wtp = write(fd_s, &cond, sizeof(int));
						close(fd_s);
						cond = 0;
						file_s++;
						sprintf(dir, "./db/%d.txt", file_s);
						fd_s = open(dir, O_CREAT | O_RDWR, 0755);
						wtp = write(fd_s, &db_s, sizeof(int));
						for(t=0;t<=db_s;t++)
							wtp = write(fd_s, &zero, sizeof(int));
						db_prev = db_s;
						prep = lseek(fd_s, sizeof(int)*(j+2), SEEK_SET);
					}
				}else{
					if(temp_n->num_f == file_s){
						lseek(fd_s, temp_n->offset_v, SEEK_SET);
						wtp = write(fd_s, &temp_n->value, sizeof(int));
					}else{
						sprintf(dir, "./db/%d.txt", temp_n->num_f);
						fd = open(dir, O_WRONLY);
						lseek(fd, temp_n->offset_v, SEEK_SET);
						wtp = write(fd, &temp_n->value, sizeof(int));
						close(fd);
					}
				}
				free(temp_n->key);
				if(temp_p != temp[j].next) free(temp_n);
				temp_n = temp_p;
			}
		}
		free(db[i].head);
		db[i].head = NULL;
	}
	lseek(fd_s, sizeof(int), SEEK_SET);
	wtp = write(fd_s, &cond, sizeof(int));
	close(fd_s);
	return val;
}

int hash_func(char* str, int size){
	int i, len, ret = 1;
	len = strlen(str);
	for(i=0;i<len;i++)
		ret *= str[i];
	if(ret%size < 0) return ret%size + size;
	return ret%size;
}