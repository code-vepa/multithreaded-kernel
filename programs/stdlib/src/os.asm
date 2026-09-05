[BITS 32]

section .asm

global print:function
global getkey:function
global os_malloc:function
global os_free:function

print:
    push ebp
    mov ebp, esp

    push dword[ebp + 8]
    mov eax, 1
    int 0x80
    add esp, 4

    pop ebp
    ret

getkey:
    push ebp
    mov ebp, esp

    mov eax, 2
    int 0x80

    pop ebp
    ret

os_malloc:
    push ebp
    mov ebp, esp

    push dword[ebp + 8]
    mov eax, 4
    int 0x80
    add esp, 4

    pop ebp
    ret

os_free:
    push ebp
    mov ebp, esp

    push dword[ebp + 8]
    mov eax, 5
    int 0x80
    add esp, 4

    pop ebp
    ret