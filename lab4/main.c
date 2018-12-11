#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	db_t* DB;
	char key[MAX_KEYLEN];
	int size, val, th_num;
	int ret, keylen, offset;
	if(argc < 3) {
		printf("Usage : %s size\n", argv[0]);
		return -1;
	}
	size = atoi(argv[1]);
	th_num = atoi(argv[2]);
	DB = db_open(size, th_num);
	if(DB == NULL) {
		printf("DB not opend\n");
		return -1;
	}
	printf("DB opened\n");
	while((ret =scanf("%s", key)) != -1) {
		keylen = strlen(key);
		offset = -1;
		val = db_get(DB, key, keylen, &offset);
		if(val == 0)
			printf("GET [%s] [NULL]\n", key);
		else
			printf("GET [%s] [%d]\n", key, val);
		db_put(DB, key, keylen, val+1, offset);
		printf("PUT [%s] [%d]\n", key, val+1);
	}
	db_close(DB);
	printf("DB closed\n");
	return 0;
}
