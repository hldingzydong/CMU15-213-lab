#include "cache.h"

cache_unit cache[MAX_CACHE_SIZE/MAX_OBJECT_SIZE];
int readcnt; 
int system_cnt;		// 系统时钟,用于LRU
sem_t mutex, w;

void init_cache() {
	//Sem_init(&mutex, 0, 1);
	//Sem_init(&w, 0, 1);
	readcnt = 0;
	memset(cache, 0, sizeof(cache_unit) * (MAX_CACHE_SIZE/MAX_OBJECT_SIZE));
}

/*
 * 	从cache中读取resource,若是不在cache中,则返回NULL
 */
char* read_from_cache(char* uri) {
	char* buf = NULL;
/*
	P(&mutex);
	readcnt++;
	if(readcnt == 1) {
		P(&w);
	}
	V(&mutex);
*/
	// core: read cache
	int i;
	for(i = 0; i < MAX_CACHE_SIZE/MAX_OBJECT_SIZE; i++) {
		if(cache[i].valid == 1 && strcmp(cache[i].uri, uri) == 0) {
			cache[i].count = ++system_cnt;
			buf = cache[i].buf;
			break;
		}
	}
/*
	P(&mutex);
	readcnt--;
	if(readcnt == 0) {
		V(&w);
	}
	V(&mutex);
*/
	if(buf != NULL) {
		printf("read from cache for %s and i = %d\n", uri, i);
	}
	return buf;
}

void write_into_cache(char* uri, char* buf) {
	//P(&w);
	int index = find_slot();
	if(index > -1) {
		cache[index].count = ++system_cnt;
		cache[index].valid = 1;
		strcpy(cache[index].uri, uri);
		//printf("cache-url = %s\n", cache[index].uri);
		strcpy(cache[index].buf, buf);
		//printf("cache-buf = %s\n", cache[index].buf);
	}
	//V(&w);
	printf("save to cache from %s and index = %d\n", uri, index);
}

int find_slot() {
	// 是否有位置空缺？
	for(int i = 0; i < MAX_CACHE_SIZE/MAX_OBJECT_SIZE; i++) {
		if(cache[i].valid == 0) {
			return i;
		}
	}
	// 没有位置空缺,采用LRU policy
	int oldest_cache_count = INT_MAX;
	int oldest_cache_index = -1;
	for(int i = 0; i < MAX_CACHE_SIZE/MAX_OBJECT_SIZE; i++) {
		if(cache[i].count < oldest_cache_count) {
			oldest_cache_count = cache[i].count;
			oldest_cache_index = i;
		}
	}
	return oldest_cache_index;
}
