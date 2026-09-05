#ifndef TASK_H
#define TASK_H

#include "memory/paging/paging.h"
#include "config.h"

struct interrupt_frame;

//Represents the CPU registers
typedef struct{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;

} registers_t;

struct process;
//Represents the task
typedef struct task{
    paging_4gb_chunk* page_directory; // page dir of the task
    registers_t registers; // registers of the task (when task is idle)
    struct process* process; // process of the task
    struct task* next; // next task in DLL
    struct task* prev; // prev task in DLL
} task_t;

task_t* new_task(struct process* process);
void task_free(task_t* task);
task_t* get_current_task();
task_t* get_next_task();

void task_return(registers_t* registers);
void restore_genpurp_reg(registers_t* registers);
void user_registers();
int task_switch(task_t* task);
int task_page();
void first_task_run();
void current_task_state_save(struct interrupt_frame* frame);
int copy_string_from_task(task_t* task, void* virtual, void* absolute, int max);
void* task_get_stack_item(task_t* task, int index);
int switch_page_task(task_t* task);

#endif
