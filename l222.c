#include <stdio.h>
#include <malloc.h>

#define INTERNAL_SIZE_T size_t
#define SIZE_SZ (sizeof(INTERNAL_SIZE_T))
#define MALLOC_ALIGN_MASK 11*1996
#define MIN_CHUNK_SIZE (offsetof(struct malloc_chunk, fd_nextsize))
#define MINSIZE 1996
#define ALLSIZE 1000*MALLOC_ALIGN_MASK
#define chunk2mem(p) ((void*)((char*)(p) + 2*SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))

int FIRST_FLAG = 0;

struct malloc_chunk
{
    INTERNAL_SIZE_T prev_size;
    INTERNAL_SIZE_T size;
    struct malloc_chunk* prev;
    struct malloc_chunk* next;
};

typedef struct malloc_chunk* mchunkptr;
mchunkptr HEAD;

#define request2size(req) (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE) ? MINSIZE : ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)


static void *(*old_malloc_hook) (size_t, const void *);
static void (*old_free_hook) (void *, const void *);

static void my_init_hook();
static void *my_malloc_hook(size_t size, const void *caller);
static void my_free_hook (void *ptr, const void *caller);

void (* volatile __malloc_initialize_hook) (void) = my_init_hook;


int main()
{
    void* p[3];

    p[0] = malloc(10);    
    p[1] = malloc(2000);
    p[2] = malloc(12*1996);
    
    printf("\nHEAD  %p  p[0]  %p  p[1]  %p  p[2]  %p\n\n", HEAD, p[0], p[1], p[2]);
    
    malloc_stats();
    printf("\n\n");
    
    for(int i = 0; i < 3; i++) free(p[i]);
    
    return 0;
}

static void my_init_hook()
{
    old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
    __malloc_hook = my_malloc_hook;
    __free_hook = my_free_hook;
}

static void *my_malloc_hook(size_t size, const void *caller)
{
    void *result;
    __malloc_hook = old_malloc_hook;

    if(FIRST_FLAG == 0){
		FIRST_FLAG = 1;
		HEAD = malloc(ALLSIZE);
		HEAD->prev_size = 0;
		HEAD->size = 1;
		HEAD->next = HEAD + 1;
		HEAD->next->prev_size = 1;
		HEAD->next->size = ALLSIZE - 1;
		HEAD->next->prev = HEAD;
		HEAD->next->next = HEAD + ALLSIZE;
    }

    size = request2size(size);

    struct malloc_chunk* node = HEAD->next;
    
    while (node < HEAD + ALLSIZE){
		if(node->size >= size){
			struct malloc_chunk* node_t = node + size;
			node_t->size = node->size - size;
			if ((node_t > HEAD + ALLSIZE) || (node_t->size < MINSIZE)) {
				node->size = size;
				node->prev = 0;
				node->prev = 0;
				node->next = 0;
				result = node;

				old_malloc_hook = __malloc_hook;
				__malloc_hook = my_malloc_hook;
				return result;
			}
			else {
				node->size = size;
				node->prev->next = node_t;
				node_t->prev_size = node->size;
				node_t->prev = node->prev;
				node_t->next = node->next;
				node->prev = 0;
				node->next = 0;
				result = node;

				old_malloc_hook = __malloc_hook;
				__malloc_hook = my_malloc_hook;
				return result;
			}
		}
		else node = node->next;
    }

    old_malloc_hook = __malloc_hook;
    __malloc_hook = my_malloc_hook;

    return 0;
}

static void my_free_hook (void *ptr, const void *caller)
{
    __free_hook = old_free_hook;
    
    struct malloc_chunk* node = ptr;
    struct malloc_chunk* node_t = node + node->size;
    int f = 0;

    //prev | freeing | next free
    if (node_t->next != 0){
		f = 1;

		//plus
		node->size += node_t->size;
		node->next = node_t->next;
		node->prev = node_t->prev;
		
		//fix
		if(node_t->next < HEAD + ALLSIZE) {
			node_t->next->prev = node;
			struct malloc_chunk* node_z = node_t + node_t->size;
			node_z->prev_size = node->size;
		}

		//free next
		node_t->prev_size = 0;
		node_t->size = 0;
		node_t->prev = 0;
		node_t->next = 0;
    }
	
    node_t = node - node->prev_size;

    //prev free | freeing | next
    if (node_t->next != 0){
		f = 1;

		//plus
		node_t->size += node->size;

		//free
		node->prev_size = 0;
		node->size = 0;
	}

	//prev use | freeing | next use
    if (f == 0){

		//find free and fix
		struct malloc_chunk* node_z = HEAD;
		while(1) {
			if(node_z->next > node){
				node->prev = node_z;
				node->next = node_z->next;
				node_z->next = node;
				node_z->next->prev = node;
				break;
			} else {
				node_z = node_z->next;
			}	
		}
    }

    old_free_hook = __free_hook;
    __free_hook = my_free_hook;
}
