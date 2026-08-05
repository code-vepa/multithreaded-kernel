section .asm

global tss_load

; This function loads in a task switch segment
tss_load:
    push ebp
    mov ebp, esp
    mov ax, [ebp + 8] ; tss segment
    ltr ax
    pop ebp
    ret