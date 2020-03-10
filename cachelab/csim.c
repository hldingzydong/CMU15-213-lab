#include "cachelab.h"
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

typedef struct {
	int  h;
	int  v;
	int  s;
	int  E;
	int  b;
	char tracefile[20];
} Arg;

typedef struct {
    int valid_flag;
    long long tag; // tag < 64bits
    int timestamp;
}  Cache_line;

// set
int set_row = 0;
int set_col = 0;
// init counter
int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;
// use counter as a clock, every operation, counter++
int counter = 0;

/*
* execute
*/
void execute_data(Cache_line* cache, Arg arg, unsigned address) {
    // filter block offset
    address >>= arg.b;
    // read row_index;
    unsigned long mask = 0xffffffffffffffff;
    int row_index = address & (mask>>(64 - arg.s));
    // read tag
    long long tag = (address >>= arg.s);
    // sequential search
    for(int i = 0; i < set_col; i++) {
        // if hit
        if((cache + row_index*set_col + i)->valid_flag == 1 && (cache + row_index*set_col + i)->tag == tag) {
            hit_count++;
            if(arg.v == 1) {
                printf(" hit");
            }
            (cache + row_index*set_col + i)->timestamp = counter++;
            return;
        }
    }
    // miss, find somewhere to store data
    miss_count++;
    if(arg.v == 1) {
        printf(" miss");
    }
    for(int i = 0; i < set_col; i++) {
        //if find somewhere to store data
        if((cache + row_index*set_col + i)->valid_flag == 0) {
            (cache + row_index*set_col + i)->valid_flag = 1;
            (cache + row_index*set_col + i)->tag = tag;
            (cache + row_index*set_col + i)->timestamp = counter++;
            return;
        }
    }
    // this set is filled with data, use LRU find eviction
    eviction_count++;
    if(arg.v == 1) {
        printf(" eviction");
    }
    // find smallest timestamp, and evict it
    int min_timestamp = INT_MAX;
    int min_timestamp_index;
    for(int i = 0; i < set_col; i++) {
        if((cache + row_index*set_col + i)->timestamp < min_timestamp) {
            min_timestamp = (cache + row_index*set_col + i)->timestamp;
            min_timestamp_index = i;
        }
    }
    // update cache
    (cache + row_index*set_col + min_timestamp_index)->tag = tag;
    (cache + row_index*set_col + min_timestamp_index)->timestamp = counter++;
}


int main(int argc, char** argv)
{
	int opt;
    Arg arg;
    while(-1 != (opt = getopt(argc, argv, "hvs:E:b:t:"))) {
    	switch(opt) {
    		case 'h':
    			arg.h = 1;
    			break;
    		case 'v':
    			arg.v = 1;
    			break;
    		case 's':
    			arg.s = atoi(optarg);
    			break;
    		case 'E':
    			arg.E = atoi(optarg);
    			break;
    		case 'b':
    			arg.b = atoi(optarg);
    			break;
    		case 't':
    			strcpy(arg.tracefile, optarg);
    			break;
    		default:
    			printf("wrong argument\n");
    			arg.h = 1;
    	}
    }

    if(arg.h || arg.s < 1 || arg.E < 1 || arg.b < 1) {
    	printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n. -h: Optional help flag that prints usage info\n. -v: Optional verbose flag that displays trace info\n. -s <s>: Number of set index bits (S = 2s is the number of sets)\n. -E <E>: Associativity (number of lines per set)\n. -b <b>: Number of block bits (B = 2b is the block size)\n. -t <tracefile>: Name of the valgrind trace to replay\n");
    	return 0;
    }

    // init cache
    set_row = pow(2, arg.s);
    set_col = arg.E;
    Cache_line* cache = (Cache_line*)malloc(sizeof(Cache_line) * set_row * set_col);
    memset(cache, 0, sizeof(Cache_line) * set_row * set_col);
    // read files
    FILE *pFile;
    pFile = fopen(arg.tracefile, "r");

    if(pFile == NULL) {
        printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n. -h: Optional help flag that prints usage info\n. -v: Optional verbose flag that displays trace info\n. -s <s>: Number of set index bits (S = 2s is the number of sets)\n. -E <E>: Associativity (number of lines per set)\n. -b <b>: Number of block bits (B = 2b is the block size)\n. -t <tracefile>: Name of the valgrind trace to replay\n");
        return 0;
    }

    char identifier;
    unsigned address;
    int size;
    while(fscanf(pFile, " %c %x,%d", &identifier, &address, &size) > 0) {
        if(arg.v == 1) {
            printf("%c %x,%d", identifier, address, size);
        }

        if(identifier == 'I') {
            continue;
        }else if(identifier == 'L'){
            execute_data(cache, arg, address);
        }else if(identifier == 'S'){
            execute_data(cache, arg, address);
        }else{
            execute_data(cache, arg, address);
            execute_data(cache, arg, address);
        }

        if(arg.v == 1) {
            printf("\n");
        }
    }

    printSummary(hit_count, miss_count, eviction_count);
    fclose(pFile);
    free(cache);
    return 0;
}
