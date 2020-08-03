#include <unistd.h>

void *mymalloc(unsigned nbytes);

void myfree(void *ap);

typedef long Align;

union header {
    struct
    {
        union header *ptr;
        unsigned size;
    } s;
    Align x;
};
typedef union header Header;

static Header base;          /* empty list to get started */
static Header *freep = NULL; /* start of free list */

#define NALLOC 1024 /* minimun #units to request */
static Header *morecore(unsigned nu) {
    char *cp;
    Header *up;

    if (nu < NALLOC) {
        nu = NALLOC;
    }
    cp = (char *)sbrk(nu * sizeof(Header));
    if (cp == (char *)-1) {
        return NULL;
    }
    up = (Header *)cp;
    up->s.size = nu;
    myfree((void *)(up + 1));
    return freep;
}