#include "io.h"
#include "task/task.h"
#include "kernel.h"

void* isr80h_command1_print(struct interrupt_frame* frame){
    void* user_buf = task_get_stack_item(get_current_task(), 0);
    char buf[1024];
    copy_string_from_task(get_current_task(), user_buf, buf, sizeof(buf));
    print(buf);
    return 0;
}