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
    PUT(heap_listp + WSIZE, PACK(OVERHEAD, 1));
    PUT(heap_listp + DSIZE, PACK(OVERHEAD, 1));
    PUT(heap_listp + WSIZE + DSIZE, PACK(0, 1));
    heap_listp += DSIZE;

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
    size_t size = GET_SIZE(ptr);

    // update allocate bit
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

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

    old_size = GET_SIZE(ptr);
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
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}


static void* coalesce(void* bp) {
    size_t prev_alloc = IS_ALLOC(PREV_BLKP(bp));
    size_t next_alloc = IS_ALLOC(NEXT_BLKP(bp));
    size_t size = GET_SIZE(bp);

    if(prev_alloc && next_alloc) {
        return bp;
    }else if(prev_alloc && !next_alloc) {
        size += GET_SIZE(NEXT_BLKP(bp));
    }else if(!prev_alloc && next_alloc) {
        bp = PREV_BLKP(bp);
        size += GET_SIZE(bp);
    }else{
        size += GET_SIZE(PREV_BLKP(bp)) + GET_SIZE(NEXT_BLKP(bp));
        bp = PREV_BLKP(bp);
    }
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    return bp;
}


static void place(void* bp, size_t size) {
    size_t origin_free_size = GET_SIZE(bp);
    
    // 分割
    if((origin_free_size/2) >= size) { 
        size_t left_free_size = origin_free_size - size;
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(left_free_size, 0));
        PUT(FTRP(bp), PACK(left_free_size, 0));
    }else{
        PUT(HDRP(bp), PACK(origin_free_size, 1));
        PUT(FTRP(bp), PACK(origin_free_size, 1));
    }
}

static void* find_fit(size_t asize) {
    char* head = (char*)heap_listp + 2 * DSIZE;
    while(GET_SIZE(head) > 0) {
        if(IS_ALLOC(head) == 0 && GET_SIZE(head) >= asize) {
            return head;
        }
        head = NEXT_BLKP(head);
    }
    return NULL;
}












