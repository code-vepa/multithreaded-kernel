#ifndef ISR80H_H
#define ISR80H_H

enum System_Commands{
    system_command0_sum,
    system_command1_print,
    system_command2_getkey,
	system_command3_putchar
};

void isr80h_register_commands();

#endif
