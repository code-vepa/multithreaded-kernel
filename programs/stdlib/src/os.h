#ifndef OS_H
#define OS_H

#include "stddef.h"

void print(const char* message);
int getkey();
void* os_malloc(size_t size);
void os_free(void* ptr);
#endif