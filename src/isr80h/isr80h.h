#ifndef ISR80H_H
#define ISR80H_H

enum System_Commands{
    system_command1_print,
    system_command2_getkey,
	system_command3_putchar,
    system_command4_malloc,
    system_command5_free
};

void isr80h_register_commands();

#endif
