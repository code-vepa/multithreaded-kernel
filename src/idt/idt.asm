section .asm

section .data
	temp_result: dd 0

extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler

global int21h
global idt_load
global no_interrupt
global enable_interrupts
global disable_interrupts
global isr80h_wrapper


enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

idt_load:
    push ebp
    mov ebp, esp

    mov ebx, [ebp + 8]
    lidt [ebx]

    pop ebp
    ret

int21h:
    cli
    pushad ; pushes all general purpose registers
    call int21h_handler
    popad ; restores all gen. purpose reg.
    sti 
    iret

no_interrupt:
    cli
    pushad 
    call no_interrupt_handler
    popad
    sti  
    iret

isr80h_wrapper:
	pushad	

	push esp
	push eax
	call isr80h_handler
	mov dword[temp_result], eax
	add esp, 8

	popad
	mov eax, [temp_result]
	iretd

