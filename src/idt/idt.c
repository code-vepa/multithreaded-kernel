#include "idt.h"
#include "config.h"
#include "memory/memory.h"

struct idt_desc idt_descriptors[VARGOOS_TOTAL_INTERRUPTS];

struct idtr_desc idtr_descriptor;

void idt_set(int interrupt_num, void* address){
    struct idt_desc* desc = &idt_descriptors[interrupt_num];
    desc->offset_1 = (uint32_t) address & 0xffff;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;
    
}

void idt_init(){
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = idt_descriptors;
}
