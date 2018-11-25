#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_KEYLEN 1024

int main(int argc, char* argv[]) {
	db_t* DB;
	char key[MAX_KEYLEN];
	int size, thread_num, val;
	int ret, cnt, keylen, vallen;
	int file_n, offset;
	if(argc < 3) {
		printf("Usage : %s size\n", argv[0]);
		return -1;
	}

	size = atoi(argv[1]);
	thread_num = atoi(argv[2]);
	DB = db_open(size, thread_num);
	if(DB ==NULL) {
		printf("DB not opend\n");
		return -1;
	}
	printf("DB opened\n");
	
	while((ret =scanf("%s", key)) != -1) {
		keylen = strlen(key);
		val = db_get(DB, key, keylen, &vallen, thread_num, &file_n, &offset);
		if(val == -1) {
			printf("GET [%s] [NULL]\n", key);
			cnt = 1;
		} else {
			printf("GET [%s] [%d]\n", key, val);
			cnt = val + 1;
		}
		db_put(DB, key, keylen, cnt, sizeof(int), thread_num, file_n, offset);
		printf("PUT [%s] [%d]\n", key, cnt);

	}
	db_close(DB, thread_num);
	printf("DB closed\n");
	return 0;
}