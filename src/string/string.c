#include "string.h"

size_t strlen(const char* ptr){
    size_t count = 0;

    while(ptr[count] != '\0')
        ++count;

    return count;
}

int to_numeric_digit(char c){
    return c - 48;
}