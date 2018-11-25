/*-----------------------------------------------------------
 *
 * SSE2033: System Software Experiment 2 (Fall 2018)
 *
 * Skeleton code for PA#4
 * 
 * CSLab, Sungkyunkwan University
 *
 * Student Id   : 2014313299
 * Student Name : park sejin
 *
 *-----------------------------------------------------------
 */
#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
int file_num;
int count;
int size_db, sthash, ndhash;
firstdb* storage;
db_t* db_open(int size) {
	int i, j, k;
	if(fork() == 0) execl("/bin/mkdir","mkdir","./db",NULL);
	else wait(NULL);
	db_t* db = NULL;
	count = 0;
	file_num = 0;
	size_db = size;
	if(size > 30000){
		sthash = 1;
		ndhash = 1;
	}
	else{
		sthash = size;
		ndhash = (30000/size)/size;
		if(ndhash == 0) ndhash = 1;
	}
	db = (db_t*)malloc(sizeof(db_t)*size);
	storage = (firstdb*)malloc(sizeof(firstdb)*size);
	for(i=0;i<size;i++) db[i].link = NULL;
	for(i=0;i<size;i++){
		storage[i].head = (secondb*)malloc(sizeof(secondb)*sthash);
		for(j=0;j<sthash;j++){
			storage[i].head[j].head = (thirdb*)malloc(sizeof(thirdb)*ndhash);
			for(k=0;k<ndhash;k++){
				storage[i].head[j].head[k].head = NULL;
			}
		}
	}
	return db;
}

void db_close(db_t* db) {
	int i, j, k;
	filedb* fdbtemp;
	filedb* fdbtemp2;
	list* temp;
	list* temp2;
	for(i=0;i<size_db;i++){
		if(db[i].link == NULL) continue;
		temp = db[i].link;
		while(1){
			temp2 = temp->next;
			free(temp->key);
			free(temp);
			temp = temp2;
			if(temp == NULL) break;
		}
	}
	for(i=0;i<size_db;i++){
		for(j=0;j<sthash;j++){
			for(k=0;k<ndhash;k++){
				fdbtemp = storage[i].head[j].head[k].head;
				while(fdbtemp != NULL){
					fdbtemp2 = fdbtemp->next;
					free(fdbtemp);
					fdbtemp = fdbtemp2;
				}
			}
			free(storage[i].head[j].head);
		}
		free(storage[i].head);
	}
	free(storage);
	free(db);
}

void db_put(db_t* db, char* key, int keylen, char* val, int vallen) {
	int i, k, fd, wtp, str_dir;
	int stdb, ndb, rdb;
	int hashed = hash_function(key, keylen);
	char* str;
	char* fname;
	filedb* fdbtemp;
	filedb* fdbtemp2;
	list* temp;
	list* temp2;
	if(db[hashed].link == NULL){
		if(count < size_db){
			count++;
			temp = (list*)malloc(sizeof(list));
			db[hashed].link = temp;
			temp->key = (char*)malloc(sizeof(char)*(keylen+1));
			strcpy(temp->key,key);
			temp->value = *((int*)val);
			temp->next = NULL;
			return;
		}
	} else {
		temp2 = db[hashed].link;
		while(1){
			if(strcmp(temp2->key,key) == 0){
				temp2->value = *((int*)val);
				return;
			}
			if(temp2->next == NULL) break;
			temp2 = temp2->next;
		}
		if(count < size_db){
			temp = (list*)malloc(sizeof(list));
			count++;
			temp2->next = temp;
			temp->key = (char*)malloc(sizeof(char)*(keylen+1));
			strcpy(temp->key,key);
			temp->value = *((int*)val);
			temp->next = NULL;
			return;
		}
	}
	count = 1;
	file_num++;
	fname = (char*)malloc(20);
	str = (char*)malloc(10);
	sprintf(str, "%d", file_num);
	strcpy(fname, "./db/");
	strcat(fname, str);

	fd = open(fname, O_CREAT | O_WRONLY, 0755);
	free(str);
	free(fname);
	for(i=0;i<size_db;i++){
		if(db[i].link == NULL){
			continue;
		}
		temp = db[i].link;
		while(temp != NULL){
			k = strlen(temp->key);
			str_dir = lseek(fd, 0, SEEK_CUR);
			wtp = write(fd, &k, sizeof(int));
			wtp = write(fd, temp->key, k);
			wtp = write(fd, &(temp->value), sizeof(int));
			temp2 = temp->next;
			stdb = hash_function(temp->key,k);
			ndb = second_hash(temp->key,k);
			rdb = third_hash(temp->key,k);
			fdbtemp = (filedb*)malloc(sizeof(filedb));
			fdbtemp->file = file_num;
			fdbtemp->point = str_dir;
			if(storage[stdb].head[ndb].head[rdb].head == NULL){
				fdbtemp->next = NULL;
				storage[stdb].head[ndb].head[rdb].head = fdbtemp;
			} else {
				fdbtemp2 = storage[stdb].head[ndb].head[rdb].head;
				storage[stdb].head[ndb].head[rdb].head = fdbtemp;
				fdbtemp->next = fdbtemp2;
			}
			free(temp->key);
			free(temp);
			temp = temp2;
		}
		db[i].link = NULL;
	}
	wtp = wtp;
	close(fd);
	temp = (list*)malloc(sizeof(list));
	db[hashed].link = temp;
	temp->key = (char*)malloc(sizeof(char)*(keylen+1));
	strcpy(temp->key,key);
	temp->value = *((int*)val);
	temp->next = NULL;
	return;
}

/* Returns NULL if not found. A malloc()ed array otherwise.
 * Stores the length of the array in *vallen */
char* db_get(db_t* db, char* key, int keylen, int* vallen) {
	char* value = (char*)malloc(sizeof(int));
	int hashed = hash_function(key, keylen);
	int secondhs, thirdhs;
	char* f_name;
	char* str;
	filedb* fdbtemp;
	filedb* fdbtemp2;
	list* temp;
	if(db[hashed].link != NULL){
		temp = db[hashed].link;
		while(1){
			if(strcmp(temp->key,key) == 0){
				*((int*)value) = temp->value;
				return value;
			}
			if(temp->next == NULL) break;
			temp = temp->next;
		}
	}
	if(file_num == 0) return NULL;
	secondhs = second_hash(key,keylen);
	thirdhs = third_hash(key,keylen);
	fdbtemp = storage[hashed].head[secondhs].head[thirdhs].head;
	fdbtemp2 = NULL;
	while(fdbtemp != NULL){
		f_name = (char*)calloc(20,sizeof(char));
		str = (char*)calloc(10,sizeof(char));
		sprintf(str,"%d",fdbtemp->file);
		strcpy(f_name,"./db/");
		strcat(f_name, str);
		value = search_file(f_name, key, hashed, fdbtemp->point);
		free(f_name);
		free(str);
		if(value != NULL){
			if(fdbtemp2 == NULL) storage[hashed].head[secondhs].head[thirdhs].head = fdbtemp->next;
			else fdbtemp2->next = fdbtemp->next;
			free(fdbtemp);
			return value;
		}
		fdbtemp2 = fdbtemp;
		fdbtemp = fdbtemp->next;
	}
	return NULL;
}

char* search_file(char* file_name, char* key, int hashed, int sdir) {
	int fd, wtp;
	int len, point;
	char val[1024];
	char* retval;
	fd = open(file_name, O_RDONLY);
	lseek(fd, sdir, SEEK_SET);
	wtp = read(fd, &len, sizeof(int));
	wtp = read(fd, &val, sizeof(char)*len);
	val[len] = '\0';
	retval = NULL;
	if(strcmp(val, key) == 0){
		wtp = read(fd, &point, sizeof(int));
		retval = (char*)malloc(sizeof(int));
		*((int*)retval) = point;
	}
	wtp = wtp;
	close(fd);
	return retval;
}

int hash_function(char* key, int keylen) {
	int hashed = 0;
	int i, j, a, b;
	b = 1;
	for(i=0;i<keylen;i++){
		a = (int)key[i];
		for(j=0;j<=i;j++){
			b *= 9931;
		}
		hashed += a*b;
		b %= size_db;
	}
	if(hashed < 0) hashed = 0 - hashed;
	return hashed%size_db;
}

int second_hash(char* key, int keylen) {
	int hashed = 0;
	int i, j, a, b;
	b = 1;
	for(i=0;i<keylen;i++){
		a = (int)key[keylen-1-i];
		for(j=0;j<=i;j++) b *= 9803;
		hashed += a*b;
		b %= sthash;
	}
	if(hashed < 0) hashed = 0 - hashed;
	return hashed%sthash;
}

int third_hash(char* key, int keylen){
	int hashed = 0;
	int i, j, a, b;
	b = 1;
	for(i=0;i<keylen;i++){
		a = (int)key[i];
		for(j=0;j<=i;j++) b *= 7757;
		hashed += a*b;
		b %= ndhash;
	}
	if(hashed < 0) hashed = 0 - hashed;
	return hashed%ndhash;
}