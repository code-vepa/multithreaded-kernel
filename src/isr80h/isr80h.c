#include "isr80h.h"
#include "idt/idt.h"
#include "io.h"
#include "misc.h"

void isr80h_register_commands(){
    isr80h_register_command(system_command0_sum, isr80h_command0_sum);
    isr80h_register_command(system_command1_print, isr80h_command1_print);
    isr80h_register_command(system_command2_getkey, isr80h_command2_getkey);
	isr80h_register_command(system_command3_putchar, isr80h_command3_putchar);
}
