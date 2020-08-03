#include "mymalloc.h"
#include <stdio.h>

#ifdef MAIN
int main() {
    char* cp = (char *)mymalloc(10000); 
    char* cp1 = (char *)mymalloc(10000); 
    char* cp2 = (char *)mymalloc(10000); 
    char* cp3 = (char *)mymalloc(10000); 
    char* cp4 = (char *)mymalloc(10000); 
    myfree(cp1);
    myfree(cp3);
    char* cp5 = (char *)mymalloc(10000); 
    
    char* p = cp;
    *p++ = 'a';
    *p++ = 'b';
    *p++ = '\0';
    printf("%s", cp);
}
#endif