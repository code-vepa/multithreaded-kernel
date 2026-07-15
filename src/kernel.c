#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"

uint16_t* video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

//Shift the color by 8 bits to the left and perform a bitwise OR 
//with the character
uint16_t terminal_make_char(char c, char color){
    return (color << 8) | c;
}

void termina_putchar(int x, int y, char c, char color){
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, color);
}

void terminal_writechar(char c, char color){
    if(c == '\n'){
        ++terminal_row;
        terminal_col = 0;
    }
    else{
        termina_putchar(terminal_col, terminal_row, c, color);
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
            termina_putchar(x,y, ' ', 0);
        }
    }
}

size_t strlen(const char* str){
    size_t len = 0;

    while(str[len]){
        ++len;
    }

    return len;
}


void print(const char* str){
    size_t len = strlen(str);

    for(int i = 0; i < len; ++i){
        terminal_writechar(str[i], 15);
    }
}

extern void test_problem();

void kernel_main(void){

    terminal_initialize();
    // first byte (03) is the color and second byte is (41) is the char
    print("Hello world\ntest");

    //initialize the IDT
    idt_init();

    test_problem();
}