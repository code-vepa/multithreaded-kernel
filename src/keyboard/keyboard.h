#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "task/process.h"

typedef int (*KEYBOARD_INIT_FUNCTION)();

struct keyboard{
    char keyboard_name[30];
    KEYBOARD_INIT_FUNCTION init;
    struct keyboard* next;
};

void keyboard_init();
void keyboard_backspace(process_t* process);
void keyboard_push(char to_push);
char keyboard_pop();

#endif