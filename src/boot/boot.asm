ORG 0x7c00 
BITS 16 ; generate 16 bit code (real-mode)

; gives offsets
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start



jmp short start ; skip over BIOS Parameter Block (BPB)
nop

; FAT16 header
OEMIdentifier       db 'VARGOOS ' ; 8 bytes
BytesPerSector      dw 0x200 ; 512 bytes per sector
SectorsPerCluster   db 0x80
ReservedSectors     dw 200
FATCopes            db 0x02 ; Original and Backup
RootDirEntries      dw 0x40
NumSectors          dw 0x00
MediaType           db 0xF8
SectorsPerFat       dw 0x100
SectorsPerTrack     dw 0x20
NumberOfHeads       dw 0x40
HiddenSectors       dd 0x00
SectorsBig          dd 0x773594

; Extended BPB (Dos 4.0)
DriveNumber         db 0x80
WinNTbit            db 0x00
Signature           db 0x29
VolumeID            dd 0xD105
VolumeIDString      db 'VARGOOS BOT'
SystemIDString      db 'FAT16   '

start:
    ; far jump (changes Code Segment and Instruction Pointer)
    jmp 0:step2 


step2:
    cli ; disable interrupts
    
    ; changes the segment registers
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti ; enables interrupts



.load_protected:
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32
    jmp $

; Global Descriptor Table (GDT)
gdt_start:

gdt_null:
    ; 64 bits of memory
    dd 0x0
    dd 0x0

; offset 0x8

; Code segment points here
gdt_code: 
    ; These are the default values
    dw 0xffff ; 0 - 15 bits Segment limit
    dw 0 ; Base first 0 - 15 bits
    db 0 ; Base 16-23 bits
    db 0x9a ; Access byte
    db 11001111b ; High/Low 4 bit flags
    db 0 ; Base 24-31 bits

; offset 0x10
gdt_data: ; DS, SS, ES, FS, GS should be linked here
    dw 0xffff ; 0 - 15 bits Segment limit
    dw 0 ; Base first 0 - 15 bits
    db 0 ; Base 16-23 bits
    db 0x92 ; Access byte
    db 11001111b ; High/Low 4 bit flags
    db 0 ; Base 24-31 bits

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; the size of the descriptor
    dd gdt_start ; offset

[BITS 32]
load32:
    mov eax, 1
    mov ecx, 100
    mov edi, 0x0100000
    call ata_lba_read
    jmp CODE_SEG:0x0100000

ata_lba_read:
    mov ebx, eax
    ; Highest 8 bits of the lba to hand disk controller sending (I)
    shr eax, 24
    or eax, 0xE0 ; Master drive selection
    mov dx, 0x1F6 ; Port
    out dx, al 
    ; Finished sending (I)

    ; Total sectors to read sending (II)
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ; Finished sending (II)

    ; Sending more bits of the LBA (III)
    mov eax, ebx
    mov dx, 0x1F3
    out dx, al
    ; Finished sending (III)

    mov dx, 0x1F4
    mov eax, ebx
    shr eax, 8
    out dx, al

    ; Sending upper 16 bits of the LBA (IV)
    mov dx, 0x1F5
    mov eax, ebx
    shr eax, 16
    out dx, al
    ; Finished sending (IV)

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

    ; Reading all sectors into memory
.next_sector:
    push ecx


.try_again:
    mov dx, 0x1F7
    in al, dx
    test al, 8
    jz .try_again

    mov ecx, 256
    mov dx, 0x1F0
    rep insw
    pop ecx
    loop .next_sector
    ret


times 510-($ - $$) db 0
dw 0xAA55