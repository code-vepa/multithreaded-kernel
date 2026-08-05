#ifndef GDT_H
#define GDT_H

#include <stdint.h>

/*
    This structure represents the GDT code part from the boot.asm
*/
typedef struct{
    uint16_t segment;
    uint16_t base_first;
    uint8_t base;
    uint8_t access;
    uint8_t high_flags;
    uint8_t base_24_31_bits;
} gdt_t;

typedef struct{
    uint32_t base;
    uint32_t limit;
    uint8_t type;
} gdt_structured_t;

void gdt_load(gdt_t* gdt, int size);
void gdt_structured_to_gdt(gdt_t* gdt, gdt_structured_t* structured_gdt, int total_entries);

#endif