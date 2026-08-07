[BITS 32]

section .asm

global restore_genpurp_reg 
global task_return
global user_registers

restore_genpurp_reg:
    push ebp
    mov ebp, esp
    mov ebx, [ebp + 8]
    mov edi, [ebx]
    mov esi, [ebx + 4]
    mov ebp, [ebx + 8]
    mov edx, [ebx + 16]
    mov ecx, [ebx + 20]
    mov eax, [ebx + 24]
    mov ebx, [ebx + 12]

    pop ebp
    ret

task_return:
    mov ebp, esp

    ; These [ebx + n] numbers are from register_t structure
    mov ebx, [ebp + 4]
    push dword [ebx + 44] ; data/stack selector (registers->ss)
    push dword [ebx + 40] ; stack ptr (registers->esp)
    
    pushf ; push the flags (registers->flags)
    pop eax
    or eax, 0x200
    push eax
 
    push dword [ebx + 32] ; push the registers->cs (code segment)
    push dword [ebx + 28] ; push the instruction pointer (registers->ip)

    ; set segment regs (except ss)
    mov ax, [ebx + 44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword [ebp + 4]
    call restore_genpurp_reg
    add esp, 4

    iretd

user_registers:
    mov ax, 0x23 ; user data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret