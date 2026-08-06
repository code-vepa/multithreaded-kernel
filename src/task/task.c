#include "task.h"
#include "kernel.h"
#include "status.h"
#include "memory/heap/kernel_heap.h"
#include "memory/memory.h"

task_t* head = 0;
task_t* tail = 0;
task_t* current = 0;

task_t* get_current(){
    return current;
}

int task_init(task_t* task){
    memset(task, 0, sizeof(task_t));
    task->page_directory = paging_new_4gb(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if(!task->page_directory){
        return -EIO;
    }
    
    task->registers.ip = VARGOOS_PROGRAM_VIRTUAL_ADDRESS;
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.esp = VARGOOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;
    return 0;
}

task_t* new_task(){
    int response = 0;
    task_t* task = kzalloc(sizeof(task_t));
    if(!task){
        response = -ENOMEM;
        goto out;
    }

    response = task_init(task);
    if(response != STATUS_OK){
        goto out;
    }

    if(head == 0x00){
        head = task;
        tail = task;
        goto out;
    }

    tail->next = task;
    task->prev = tail;
    tail = task;
out:
    if(ISERR(response)){
        task_free(task);
        return ERROR(response);
    }

    return task;
}

/*
    @brief This function will return the next task (if exists) or head (if doesn't).
    Head could be null as well.
*/
task_t* task_get_next(){
    if(!current->next){
        return head;
    }

    return current->next;
}

/*
    @brief This function will deattach the task from the list.
*/
static void task_list_remove(task_t* task){
    if(!task)
        return;
    
    if(task->prev){
        task->prev->next = task->next;
    }

    if(task == head){
        head = task->next;
    }

    if(task == tail){
        tail = task->prev;
    }

    if(task == current){
        current = task_get_next();
    }

}

void task_free(task_t* task){
    paging_free_4gb(task->page_directory);
    task_list_remove(task);
    kfree(task);
}