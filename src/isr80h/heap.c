#include "heap.h"
#include "task/task.h"
#include "task/process.h"
#include <stddef.h>

void* isr80h_command4_malloc(struct interrupt_frame* frame){
    size_t size = (int) task_get_stack_item(get_current_task(), 0);
    return process_malloc(get_current_task()->process, size);
}

void* isr80h_command5_free(struct interrupt_frame* frame){
    void* to_free = task_get_stack_item(get_current_task(), 0);
    process_free(get_current_task()->process, to_free);
    return 0;
}