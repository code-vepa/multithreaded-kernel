#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char* ptr);
size_t strnlen(const char* ptr, int max);
int isdigit(char c);
int to_numeric_digit(char c);
char* strcpy(char* dest, const char* src);

#endif