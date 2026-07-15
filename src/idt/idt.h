#ifndef IDT_H
#define IDT_H
#include <stdint.h>

struct idt_desc
{
    uint16_t offset_1; // Offset bits 0 - 15
    uint16_t selector;
    uint16_t zero; //Unused set to zero
    uint16_t type_attr;
    uint16_t offset_2; // Offset buts 16 - 31
} __attribute__((packed));

struct idtr_desc
{
    uint16_t limit; // size of the descriptor table - 1
    uint16_t base; // base address of the start of the IDT
} __attribute__((packed));


#endif