#ifndef STRING_H
#define STRING_H

#include <stddef.h>

char tolower(char c);
size_t strlen(const char* ptr);
size_t strnlen(const char* ptr, int max);
int strlen_terminator(const char* str, int max, char terminator);
int isdigit(char c);
int to_numeric_digit(char c);
int istrncmp(const char* str1, const char* str2, int n);
int strncmp(const char* str1, const char* str2, int bytes);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, int n);

#endif
