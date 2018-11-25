/*-----------------------------------------------------------
 *
 * SSE2033: System Software Experiment 2 (Fall 2018)
 *
 * Skeleton code for PA#1
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
int size_db;
char arr[4];
db_t* db_open(int size) {
	int i;
	size_db = size;
	db_t* db = NULL;	
	db = (db_t*)malloc(sizeof(db_t)*size);
	for(i=0;i<size;i++) db[i].link = NULL;
	return db;
}

void db_close(db_t* db) {
	int i;
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
	free(db);
}

void db_put(db_t* db, char* key, int keylen, char* val, int vallen) {
	int index = hash_function(key, keylen);
	list* temp = (list*)malloc(sizeof(list));
	list* temp2;
	arr[1] = 0x01;
	if(db[index].link == NULL){
		db[index].link = temp;
		temp->key = (char*)malloc(sizeof(char)*(keylen+1));
		strcpy(temp->key,key);
		temp->value = *((int*)val);
		temp->next = NULL;
	} else {
		temp2 = db[index].link;
		while(1){
			if(strcmp(temp2->key,key) == 0){
				temp2->value = *((int*)val);
				return;
			}
			if(temp2->next == NULL) break;
			temp2 = temp2->next;
		}
		temp2->next = temp;
		temp->key = (char*)malloc(sizeof(char)*(keylen+1));
		strcpy(temp->key,key);
		temp->value = *((int*)val);
		temp->next = NULL;
	}
}

/* Returns NULL if not found. A malloc()ed array otherwise.
 * Stores the length of the array in *vallen */
char* db_get(db_t* db, char* key, int keylen, int* vallen) {
	char* value = (char*)malloc(sizeof(char));
	int index = hash_function(key, keylen);
	list* temp;
	if(db[index].link != NULL){
		temp = db[index].link;
		while(1){
			if(strcmp(temp->key,key) == 0){
				value = (char*)(&temp->value);
				break;
			}
			if(temp->next == NULL){
				value = NULL;
				break;
			}
			temp = temp->next;
		}
		return value;
	} else {
		value = NULL;
		return value;
	}
}

int hash_function(char* key, int keylen) {
	int hashed = 0;
	int i;
	for(i=0;i<keylen;i++){
		hashed += (int)key[i];
	}
	if(hashed < 0) hashed = 0 - hashed;
	return hashed%size_db;
}
