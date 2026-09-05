#include "io.h"
#include "task/task.h"
#include "kernel.h"
#include "keyboard/keyboard.h"


void* isr80h_command1_print(struct interrupt_frame* frame){
    void* user_buf = task_get_stack_item(get_current_task(), 0);
    char buf[1024];
    copy_string_from_task(get_current_task(), user_buf, buf, sizeof(buf));
    print(buf);
    return 0;
}

void* isr80h_command2_getkey(struct interrupt_frame* frame){
    char to_pop = keyboard_pop();
    return (void*) ((int)to_pop);
}

void* isr80h_command3_putchar(struct interrupt_frame* frame){
	char c = (char)(int) task_get_stack_item(get_current_task(), 0);
	terminal_writechar(c, 15);
	return 0;
}