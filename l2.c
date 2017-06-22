#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#define INTERNAL_SIZE_T size_t
#define SIZE_SZ (sizeof(INTERNAL_SIZE_T))
#define MALLOC_ALIGN_MASK 2 * SIZE_SZ
#define MIN_CHUNK_SIZE (offsetof(struct malloc_chunk, fd_nextsize))
#define MINSIZE (unsigned long)(((MIN_CHUNK_SIZE+MALLOC_ALIGN_MASK)&~MALLOC_ALIGN_MASK))
#define chunk2mem(p) ((void*)((char*)(p) + 2*SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))

struct malloc_chunk
{
    INTERNAL_SIZE_T prev_size;
    INTERNAL_SIZE_T size;
    struct malloc_chunk* fd;
    struct malloc_chunk* bk;
    struct malloc_chunk* fd_nextsize;
    struct malloc_chunk* bk_nextsize;
};

typedef struct malloc_chunk* mchunkptr;

#define request2size(req) (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE) ? MINSIZE : ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)

int main()
{
    int count = 100;
    int size = 120;
    int *p[count];

    for (int i = 0; i < count; i++) {
        p[i] = malloc(request2size(i*size));
        printf ("iter = %d\n", i+1);
    }

	printf("\n\n");

	for(int i = 0; i < count; i++) free(p[i]);
    
    //malloc_stats();

    return 0;
}
