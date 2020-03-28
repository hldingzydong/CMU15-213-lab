#include <stdio.h>
#include <limits.h>
#include "csapp.h"


typedef struct {
	char uri[MAXLINE];
	char buf[MAX_OBJECT_SIZE];
	int count;
	int valid;
} cache_unit;

void init_cache();
char* read_from_cache(char* uri);
void write_into_cache(char* uri, char* buf);
int find_slot();


