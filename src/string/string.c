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

int strncmp(const char* str1, const char* str2, int bytes){
	unsigned char u1, u2;

	while(bytes-- > 0){
		u1 = (unsigned char)*str1++;
		u2 = (unsigned char)*str2++;

		if(u1 != u2)
			return u1 - u2;
		if(u1 == '\0')
			return 0;
		
	}

	return 0;
}

int strlen_terminator(const char* str, int max, char terminator){
	int i = 0;

	for(i = 0; i < max; ++i){
		if(str[i] == '\0' || str[i] == terminator)
			break;
	}

	return i;
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
