#include "isr80h.h"
#include "idt/idt.h"
#include "io.h"
#include "heap.h"

void isr80h_register_commands(){
    isr80h_register_command(system_command1_print, isr80h_command1_print);
    isr80h_register_command(system_command2_getkey, isr80h_command2_getkey);
	isr80h_register_command(system_command3_putchar, isr80h_command3_putchar);
    isr80h_register_command(system_command4_malloc, isr80h_command4_malloc);
    isr80h_register_command(system_command5_free, isr80h_command5_free);
}
