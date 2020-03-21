/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define WSIZE    4
#define DSIZE    8
#define CHUNKSIZE (1<<12)
#define MAX(x,y)  ((x) > (y)?(x):(y))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p)  (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp)  ((char *)(bp) - WSIZE)
#define FTRP(bp)  ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp)   ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)   ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define PREV_LINKED_BLKP(bp) ((char *)(bp))
#define NEXT_LINKED_BLKP(bp) ((char *)(bp)+WSIZE)


/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "noteam",
    /* First member's full name */
    "KUR",
    /* First member's email address */
    "nope"
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

static void* heap_dirp;
static void* heap_listp;
/* 
 * mm_init - initialize the malloc package.
 * 
 * to use my malloc package, you must call this function first.
 *
 * ret
 * -1   initialization failed.
 * 0    initialization succeeded.
 *
 */
int mm_init(void)
{
    if((heap_listp = mem_sbrk(4*WSIZE)) == NULL) {
        return -1;
    }
    PUT(heap_listp, 0);
    PUT(heap_listp + WSIZE, NULL);   // 8~16 每次申请的最小的大小也要比8大,8是指header+footer
    PUT(heap_listp + 2*WSIZE, NULL); // 16~32 
    PUT(heap_listp + 3*WSIZE, NULL); // 32~64
    PUT(heap_listp + 4*WSIZE, NULL); // 64~128
    PUT(heap_listp + 5*WSIZE, NULL); // 128~256
    PUT(heap_listp + 6*WSIZE, NULL); // 256~512
    PUT(heap_listp + 7*WSIZE, NULL); // 512~1024
    PUT(heap_listp + 8*WSIZE, NULL); // 1024~

    PUT(heap_listp + 9*WSIZE, PACK(OVERHEAD, 1));
    PUT(heap_listp + 10*WSIZE, PACK(OVERHEAD, 1));
    PUT(heap_listp + 11*WSIZE, PACK(0, 1));

    heap_dirp = heap_listp;
    heap_listp += 11*WSIZE;

    if(extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
    } 
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *
 * Always allocate a block whose size is a multiple of the alignment.
 * size of block is AT LEAST size(param) byte. it would be bigger than
 * size(param)
 *
 *ret
 * NULL failed.
 * ptr  valid address of allocated block PAYLOAD.
 */
void* mm_malloc(size_t size)
{
    size_t asize;
    size_t extend_size;
    char* bp;

    if(size <= 0){
        return NULL;
    }

    if(size <= DSIZE) {
        asize = DSIZE + OVERHEAD;
    }else{
        asize = DSIZE * ((size + OVERHEAD + (DSIZE - 1)) / DSIZE);
    }

    bp = find_fit(asize);
    if(bp != NULL) {
        place(bp, asize);
        return bp;
    }

    extend_size = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extend_size/WSIZE)) == NULL) {
        return NULL;
    }
    place(bp,asize);
    return bp;
}

/*
 * mm_free - Freeing a block.
 */
void mm_free(void *ptr)
{
    if(ptr == NULL) {
        return;
    }

    size_t size = GET_SIZE(HDRP(ptr));

    // update
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    PUT(PREV_LINKED_BLKP(ptr), NULL);
    PUT(NEXT_LINKED_BLKP(ptr), NULL);

    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 *
 * (not now)
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t old_size;
    void* newptr;

    if(size <= 0) {
        mm_free(ptr);
        return 0;
    }else if(ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);
    if(newptr == NULL) {
        return 0;
    }

    old_size = GET_SIZE(HDRP(ptr));
    if(size < old_size) {
        old_size = size;
    }
    memcpy(newptr, ptr, old_size);
    mm_free(ptr);
    return newptr;
}





/*
 * helper_functions
 */
static void* extend_heap(size_t words) {
    char* bp;
    size_t size;
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if((bp = mem_sbrk(size)) == NULL) {
        return NULL;
    }

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(PREV_LINKED_BLKP(bp),NULL);
    PUT(NEXT_LINKED_BLKP(bp),NULL);
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}


static int Index(size_t size) {
    if(size > 1024) return 8;

    int ret = 0;
    while(size) {
        size /= 2;
        ret++;
    }
    return ret-3;
}


static void add_into_list(void* bp, size_t size) {
    int index = Index(size);
    void* list_head = GET(heap_dirp + index*WSIZE);
    if(list_head == NULL) {
        PUT(heap_dirp + index*WSIZE, bp);
        PUT(PREV_LINKED_BLKP(bp), heap_dirp + index*WSIZE);
    }else{
        char *pre = list_table+(index*WSIZE);
        char *st = GET(pre);
        while(st != NULL && st < bp) {
            pre = st;
            st = GET(NEXT_LINKED_BLKP(st));
        }

        if(pre != list_table+(index*WSIZE)) {
            PUT(NEXT_LINKED_BLKP(pre),bp);
            PUT(PREV_LINKED_BLKP(bp),pre);
            PUT(NEXT_LINKED_BLKP(bp),st);
        }else{
            PUT(pre, bp);
            PUT(PREV_LINKED_BLKP(bp),pre);
            PUT(NEXT_LINKED_BLKP(bp),st);
        }

        if(st != NULL) PUT(PREV_LINKED_BLKP(st),bp);
    }
}


static void delete_from_list(void* bp, size_t size) {
    int index = Index(size);
    char* prev_block = GET(PREV_LINKED_BLKP(bp));
    char* next_block = GET(NEXT_LINKED_BLKP(bp));
    if(prev_block == heap_dirp + index*WSIZE) {
        PUT(prev_block, next_block);
    }else{
        PUT(NEXT_LINKED_BLKP(prev_block), next_block);
    }

    if(next_block != NULL) PUT(PREV_LINKED_BLKP(next_block), prev_block);
}


static void* coalesce(void* bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc) {
        add_into_list(bp, size);      
    }else if(prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_from_list(NEXT_BLKP(bp), GET_SIZE(HDRP(NEXT_BLKP(bp))));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
        PUT(PREV_LINKED_BLKP(bp), NULL);
        PUT(NEXT_LINKED_BLKP(bp), NULL);
        add_into_list(bp, size);
    }else if(!prev_alloc && next_alloc) {
        delete_from_list(PREV_BLKP(bp), GET_SIZE(HDRP(PREV_BLKP(bp))));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
        PUT(PREV_LINKED_BLKP(bp), NULL);
        PUT(NEXT_LINKED_BLKP(bp), NULL);
        add_into_list(bp, size);
    }else{
        delete_from_list(PREV_BLKP(bp), GET_SIZE(HDRP(PREV_BLKP(bp))));
        delete_from_list(NEXT_BLKP(bp), GET_SIZE(HDRP(NEXT_BLKP(bp))));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
        PUT(PREV_LINKED_BLKP(bp), NULL);
        PUT(NEXT_LINKED_BLKP(bp), NULL);
        add_into_list(bp, size);
    }
    return bp;
}

static void place(void* bp, size_t size) {
    size_t origin_free_size = GET_SIZE(HDRP(bp));
    
    // 分割
    if(origin_free_size - size >= 2*DSIZE) { 
        size_t left_free_size = origin_free_size - size;
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        delete_from_list(bp, GET_SIZE(HDRP(bp)));

        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(left_free_size, 0));
        PUT(FTRP(bp), PACK(left_free_size, 0));
        PUT(PREV_LINKED_BLKP(bp), NULL);
        PUT(NEXT_LINKED_BLKP(bp), NULL);
        add_into_list(bp, left_free_size);
    }else{
        delete_from_list(bp, GET_SIZE(HDRP(bp)));
        PUT(HDRP(bp), PACK(origin_free_size, 1));
        PUT(FTRP(bp), PACK(origin_free_size, 1));
    }
}

static void* find_fit(size_t asize) {
    int index = Index(asize);
    for(;index<=8;index++){
        char* head = GET(heap_dirp + index*WSIZE);
        while(head != NULL) {
            if(GET_SIZE(head) >= asize) {
                return head;
            }
            head = NEXT_BLKP(head);
        }
    }
    return NULL;
}












