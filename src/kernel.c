#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kernel_heap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "disk/disk_streamer.h"
#include "filesystem/path_parser.h"
#include "filesystem/file.h"
#include "string/string.h"
#include "gdt/gdt.h"
#include "config.h"
#include "status.h"
#include "memory/memory.h"
#include "task/tss.h"
#include "task/process.h"
#include "task/task.h"
#include "isr80h/isr80h.h"
#include "keyboard/keyboard.h"

uint16_t* video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

//Shift the color by 8 bits to the left and perform a bitwise OR 
//with the character
uint16_t terminal_make_char(char c, char color){
    return (color << 8) | c;
}

void terminal_putchar(int x, int y, char c, char color){
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, color);
}

void terminal_backspace(){
	if(!terminal_row && !terminal_col){
		return;
	}

	if(terminal_col == 0){
		--terminal_row;
		terminal_col = VGA_WIDTH;
		return;
	}
	--terminal_col;
	terminal_writechar(' ', 15);
	--terminal_col;
}

void terminal_writechar(char c, char color){
    if(c == '\n'){
        ++terminal_row;
        terminal_col = 0;
        return;
    }
    if(c == 0x08){
		terminal_backspace();
		return;
	}
	else{
        terminal_putchar(terminal_col, terminal_row, c, color);
        ++terminal_col;
        if(terminal_col >= VGA_WIDTH){
            terminal_col = 0;
            ++terminal_row;
        }
    }
}

void terminal_initialize(){
    video_mem = (uint16_t*)(0xB8000);
    for(int y = 0; y < VGA_HEIGHT; ++y){
        for (int x = 0; x < VGA_WIDTH; ++x)
        {
            terminal_putchar(x,y, ' ', 0);
        }
    }
}


void print(const char* str){
    size_t len = strlen(str);

    for(int i = 0; i < len; ++i){
        terminal_writechar(str[i], 15);
    }
}

static paging_4gb_chunk* kernel_chunk = 0;


void kernel_page(){
	kernel_registers();
	paging_switch(kernel_chunk);
}

void panic(const char* message){
    print(message);

    while(1){}
}

struct tss tss;

gdt_t gdt_real[VARGOOS_TOTAL_GDT_SEGMENTS];
gdt_structured_t gdt_structured[VARGOOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00}, // null segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x9a}, // kernel code segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x92}, //kernel data segment
    // User data and code segments
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF8}, // user code segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF2}, // user data segment

    //tss
    {.base = (uint32_t) &tss, .limit = sizeof(tss), .type = 0xE9} // tss segment
};

void pic_timer_callback(struct interrupt_frame* frame){
    print("Timer activated\n");
}

void kernel_main(void){

    terminal_initialize();
    
    // first byte (03) is the color and second byte is (41) is the char
    print("Hello world\n");

    memset(gdt_real, 0x00, sizeof(gdt_real));
    print("1\n");
    gdt_structured_to_gdt(gdt_real, gdt_structured, VARGOOS_TOTAL_GDT_SEGMENTS);
    print("2\n");
    gdt_load(gdt_real, sizeof(gdt_real) - 1);
    print("3\n");

    print("heap\n");
    kernel_heap_init();

    print("fs\n");
    fs_init();

    print("disk\n");
    disk_search_init();

    print("idt\n");
    idt_init();

    print("tss memset\n");
    memset(&tss, 0, sizeof(tss));

    print("tss setup\n");
    tss.esp0 = 0x600000;
    tss.ss0 = KERNEL_DATA_SELECTOR;

    print("tss load\n");
    tss_load(0x28);
    print("before paging\n");


    print("paging new\n");
    kernel_chunk = paging_new_4gb(
        PAGING_IS_WRITABLE |
        PAGING_IS_PRESENT |
        PAGING_ACCESS_FROM_ALL
    );

    print("paging switch\n");
    paging_switch(kernel_chunk);

    print("enable paging\n");
    enable_paging();

    print("keyboard\n");
    keyboard_init();

    print("interrupts\n");
    enable_interrupts();

    print("4\n");



    while(1){
    }

    
}
