#include "stdlib.h"
#include "os.h"


void* malloc(size_t size){
    return os_malloc(size);
}

void free(void* ptr){
    os_free(ptr);
}

char* itoa(int val){
    static char text[12];
    int loc = 11;
    text[11] = 0;
    char neg = 1;

    if(val >= 0){
        neg = 0;
        val = -val;
    }

    while(val){
        text[--loc] = '0' - (val % 10);
        val /= 10;
    }

    if(loc == 11){
        text[--loc] = '0';
    }

    if(neg){
        text[--loc] = '-';
    }

    return &text[loc];
}