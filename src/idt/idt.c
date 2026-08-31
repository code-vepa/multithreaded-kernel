#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "kernel.h"
#include "io/io.h"
#include "task/task.h"
#include "status.h"

struct idt_desc idt_descriptors[VARGOOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void* interrupt_pointer_table[VARGOOS_TOTAL_INTERRUPTS];
static INTERRUPT_CALLBACK_FUNCTION interrupt_callbacks[VARGOOS_TOTAL_INTERRUPTS];
static ISR80H_COMMAND isr80h_commands[VARGOOS_MAX_ISR80H_COMMANDS];

extern void idt_load(struct idtr_desc* ptr);
extern void int21h();
extern void no_interrupt();
extern void isr80h_wrapper();

void interrupt_handler(int interrupt, struct interrupt_frame* frame){
    kernel_page();
    if(interrupt_callbacks[interrupt] != 0){
        //current_task_state_save(frame);
        interrupt_callbacks[interrupt](frame);
    }

   // task_page();
    outb(0x20, 0x20);
}

// void int21h_handler(){
//     print("keyboard pressed\n");
//     outb(0x20, 0x20);
// }

void no_interrupt_handler(){
    outb(0x20, 0x20);
}

void idt_zero(){
    print("Division by zero\n");
}

void idt_set(int interrupt_no, void* address){
    struct idt_desc* desc = &idt_descriptors[interrupt_no];
    desc->offset_1 = (uint32_t) address & 0xffff;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;
    desc->type_attr = 0xEE;
    desc->offset_2 = (uint32_t) address >> 16; 
}

void idt_init(){
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t) idt_descriptors;

    for(int i = 0; i < VARGOOS_TOTAL_INTERRUPTS; ++i){
        idt_set(i, interrupt_pointer_table[i]);
    }

    idt_set(0, idt_zero);
    //idt_set(0x21, int21h);
    idt_set(0x80, isr80h_wrapper);

    //load the interrup descriptor table
    idt_load(&idtr_descriptor);
}

int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback){
    if(interrupt < 0 || interrupt >= VARGOOS_TOTAL_INTERRUPTS){
        return -EINVARG;
    }

    interrupt_callbacks[interrupt] = interrupt_callback;
    return 0;
}

void isr80h_register_command(int id, ISR80H_COMMAND command){
    if(id < 0 || id >= VARGOOS_MAX_ISR80H_COMMANDS){
        panic("PANIC: command id is out of bounds\n");
        return;
    }

    if(isr80h_commands[id]){
        panic("PANIC: command by this id already exists");
    }

    isr80h_commands[id] = command;
}

void* isr80h_command_handler(int command, struct interrupt_frame* frame){
    void* result = 0;

    if(command < 0 || command >= VARGOOS_MAX_ISR80H_COMMANDS){
        return 0;
    }

    ISR80H_COMMAND command_function = isr80h_commands[command];
    if(!command_function){
        print("Cannot process\n");
        return 0;
    }

    return result;
}

void* isr80h_handler(int command, struct interrupt_frame* frame){
	void* response = 0;
	
	kernel_page();
	current_task_state_save(frame);
	response = isr80h_command_handler(command, frame);
	task_page();
	
	return response;
}



