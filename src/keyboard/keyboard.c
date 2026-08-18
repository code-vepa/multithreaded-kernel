#include "keyboard.h"
#include "kernel.h"
#include "status.h"
#include "task/task.h"
#include "classic.h"

static struct keyboard* virtual_keyboard_head = 0;
static struct keyboard* virtual_keyboard_tail = 0;

void keyboard_init(){
    keyboard_insert(classic_init());
}

int keyboard_insert(struct keyboard* keyboard){
    int response = 0;
    if(!keyboard->init){
        return -EINVARG;
    }

    if(virtual_keyboard_tail != 0){
        virtual_keyboard_tail->next = keyboard;
        virtual_keyboard_tail = virtual_keyboard_tail->next;
        
    }
    else
        virtual_keyboard_head = virtual_keyboard_tail = keyboard;

    response = keyboard->init();
    return response;
}

static int keyboard_get_tail(process_t* process){
    return process->keyboard.tail % sizeof(process->keyboard.buffer);
}

void keyboard_backspace(process_t* process){
    --process->keyboard.tail;
    int index = keyboard_get_tail(process);
    process->keyboard.buffer[index] = 0x00;
}

void keyboard_push(char to_push){
    process_t* process = get_current_process();
    if(!process)
        return;

    if(to_push == 0x00){
        return;
    }
    
    int index = keyboard_get_tail(process);
    process->keyboard.buffer[index] = to_push;
    ++process->keyboard.tail;
}

char keyboard_pop(){
    task_t* current_task = get_current_task();
    if(!current_task)
        return 0;

    process_t* process = get_current_task()->process;
    int index = process->keyboard.head % sizeof(process->keyboard.buffer);
    char to_pop = process->keyboard.buffer[index];
    if(to_pop == 0x00){
        return 0;
    }
    else{
        process->keyboard.buffer[index] = 0x00;
        ++process->keyboard.head;
    }

    return to_pop;    
}