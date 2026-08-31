[BITS 32] ; All code underneath is seen as 32 bits

global _start
global kernel_registers

extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10


_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    ; ; Enable the A20 line 
    ; in al, 0x92
    ; or al, 2
    ; out 0x92, al

    ; ; remap master port
    ; mov al, 00010001b
    ; out 0x20, al ; tell master pic

    ; mov al, 0x20 ;
    ; out 0x21, al

    ; mov al, 00000001b
    ; out 0x21, al
    ; ; end remap the master pic

    mov al, 0x11
    out 0x20, al        ; master
    out 0xA0, al         ; slave

    ; ICW2 - vector offsets
    mov al, 0x20
    out 0x21, al         ; master IRQ0-7 -> int 0x20-0x27
    mov al, 0x28
    out 0xA1, al         ; slave IRQ8-15 -> int 0x28-0x2F

    ; ICW3 - cascade wiring
    mov al, 0x04
    out 0x21, al         ; master: slave attached at IRQ2
    mov al, 0x02
    out 0xA1, al         ; slave: cascade identity

    ; ICW4
    mov al, 0x01
    out 0x21, al
    out 0xA1, al


    call kernel_main
    jmp $


; After this, registers point to the kernel data segment
kernel_registers:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov gs, ax
	mov fs, ax
	ret


times 512-($ - $$) db 0
