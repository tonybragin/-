#include <stdio.h>
#include <stdlib.h>
#define __USE_GNU
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdint.h>

static int iter = 0;

void *malloc (size_t size) {
    static void *(*real_malloc) (size_t) = NULL;

    if (iter >= 77) {
		fprintf (stderr, "\n!!! >77 !!!\n");
		return (NULL);
    }

    iter++;

    real_malloc = dlsym(RTLD_NEXT, "malloc");
    return real_malloc(size);
}

void free (void *ptr) {
	static void (*real_free) (void*) = NULL;
	if (iter - 1 < -1) return;

    fprintf (stderr, "Blocks: %d\n", iter);
	iter--;
    real_free = dlsym(RTLD_NEXT, "free");
	real_free(ptr);
}
