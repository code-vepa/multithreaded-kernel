#include "string.h"

size_t strlen(const char* ptr){
    size_t count = 0;

    while(ptr[count] != '\0')
        ++count;

    return count;
}

size_t strnlen(const char* ptr, int max){
    size_t i = 0;

    for(i = 0; i < max; ++i){
        if(ptr[i] == 0)
            break;
    }

    return i;
}

int isdigit(char c){
    return c >= 48 && c <= 57;
}

int to_numeric_digit(char c){
    return c - 48;
}

char* strcpy(char* dest, const char* src){
    char* tmp = dest;

    while(*src != 0){
        *dest = *src;
        ++src;
        ++dest;
    }

    *dest = 0x00;

    return tmp;
}