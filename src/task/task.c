#include "task.h"
#include "kernel.h"
#include "status.h"
#include "memory/heap/kernel_heap.h"
#include "memory/memory.h"
#include "process.h"
#include "idt/idt.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "loader/formats/elfloader.h"

task_t* head = 0;
task_t* tail = 0;
task_t* current = 0;

task_t* get_current_task(){
    return current;
}

int task_init(task_t* task, process_t* process){
    memset(task, 0, sizeof(task_t));
    task->page_directory = paging_new_4gb(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if(!task->page_directory){
        return -EIO;
    }
    
    
    if(process->filetype == PROCESS_FILETYPE_ELF){
        task->registers.ip = elf_header(process->elf_file)->e_entry;
    }
    else{
        task->registers.ip = VARGOOS_PROGRAM_VIRTUAL_ADDRESS;
    }

    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = VARGOOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;
    task->process = process;
    return 0;
}

task_t* new_task(process_t* process){
    int response = 0;
    task_t* task = kzalloc(sizeof(task_t));
    if(!task){
        response = -ENOMEM;
        goto out;
    }

    response = task_init(task, process);
    if(response != STATUS_OK){
        goto out;
    }

    if(head == 0x00){
        head = task;
        tail = task;
        current = task;
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
task_t* get_next_task(){
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
        current = get_next_task();
    }

}

void task_free(task_t* task){
    paging_free_4gb(task->page_directory);
    task_list_remove(task);
    kfree(task);
}

int task_switch(task_t* task){
    current = task;
    paging_switch(task->page_directory);
    return 0;
}

/*
    @brief This function takes us out of the kernel page directory and loads us into the task page directory.
*/
int task_page(){
    user_registers();
    task_switch(current);
    return 0;
}

int switch_page_task(task_t* task){
    user_registers();
    paging_switch(task->page_directory);
    return 0;
}

/*
    @brief This function runs the very first task in the system.
*/
void first_task_run(){
    if(!current){
        panic("PANIC: no current task\n");
    }

    task_switch(head);
    task_return(&head->registers);
}

void save_task_state(task_t* task, struct interrupt_frame* frame){
	task->registers.ip = frame->ip;
	task->registers.cs = frame->cs;
	task->registers.flags = frame->flags;
	task->registers.esp = frame->esp;
	task->registers.ss = frame->ss;
	task->registers.eax = frame->eax;
	task->registers.ebp = frame->ebp;
	task->registers.ebx = frame->ebx;
	task->registers.ecx = frame->ecx;
	task->registers.edi = frame->edi;
	task->registers.edx = frame->edx;
	task->registers.esi = frame->esi;
}

/*
	@brief This function needs to be called from the kernel land (kernel_page())
*/
void current_task_state_save(struct interrupt_frame* frame){
	if(current == 0){
		panic("PANIC: no current task to save\n");
	}	
	
	task_t* task = current;
	save_task_state(task, frame);
}


int copy_string_from_task(task_t* task, void* virtual, void* absolute, int max){
    if(max >= PAGING_PAGE_SIZE){
        return -EINVARG;
    }

    int response = 0;

    char* hold = kzalloc(max);
    if(!hold){
        return -ENOMEM;
    }

    uint32_t* task_dir = task->page_directory->directory_entry;
    uint32_t prev = get_page(task_dir, hold);
    paging_map(task->page_directory, hold, hold, PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(task->page_directory);
    strncpy(hold, virtual, max);
    kernel_page();

    response = paging_set(task_dir, hold, prev);
    if(response < 0){
        kfree(hold);
        return -EIO;
    }

    strncpy(absolute, hold, max);
    return response;
}


void* task_get_stack_item(task_t* task, int index){
    void* response = 0;

    uint32_t* stack_ptr = (uint32_t*) task->registers.esp;
    switch_page_task(task);
    
    response = (void*) stack_ptr[index];
    kernel_page();

    return response;
}