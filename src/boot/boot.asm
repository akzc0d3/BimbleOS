ORG 0x7c00                         
BITS 16                             ; Assemble the instruction in 16 bit  

CODE_SEG equ gdt_code - gdt_start       ; Offset for code segment
DATA_SEG equ gdt_data - gdt_start       ; Offset for data segment


jmp short start                     ; BPB start ; Refer docs for format of BPB
nop


; FAT16 Header 
OEMIdentifier           db 'BIMBLEOS'
BytesPerSector          dw 0x200        ; This field is genrally ignored by kernel. Changing this doesn't change the bytes per sector in disk. You cannot chnage the way disks work
SectorsPerCluster       db 0x80                  
ReservedSectors         dw 200          ; Our kernel lies within this reserved space
FATCopies               db 0x02
RootDirEntries          dw 0x40
NumSectors              dw 0x00
MediaType               db 0xF8
SectorsPerFat           dw 0x100
SectorsPerTrack         dw 0x20
NumberOfHeads           dw 0x40
HiddenSectors           dd 0x00
SectorsBig              dd 0x773594

; Extended BPB (Dos 4.0)
DriveNumber             db 0x80
WinNTBit                db 0x00
Signature               db 0x29
VolumeID                dd 0xD105
VolumeIDString          db 'BIMBLE BOOT'
SystemIDString          db 'FAT16   '
; We can not mount file system on a linux kernel





start:
    jmp 0:step_2

step_2:
    cli ;   Clear Interrupts, No interrupts to be handled in middle of this critical operartion
    mov ax, 0x0       ; We can't set segment register directly. We move the value in AX then from AX to segment register
    mov ds, ax          ; Set Data Segment
    mov es, ax          ; Set Extra Segment
    mov ss, ax          ; Set Stack Segment
    mov sp, 0x7c00      ; Stack grows downward
    sti ;   Enable Interrupts
 
; Enable 32-bit protected mode
load_protected:
    cli
    lgdt[gdt_descriptor]    
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32     ; CODE_SEG is offset of code segment in GDT
    

; GDT
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

; Offset 0x8
gdt_code:       ; CS
    dw 0xffff   ; Limit 0-15 bits           
    dw 0x0      ; Base 0-15 bits             
    db 0        ; Base 16-23 bits             
    db 0x9a     ; Access byte               
    db 11001111b ; High and low 4 bit flags 
    db 0        ; Base 24-31 bits

; Offset 0x10
gdt_data:       ; DS, SS, ES, FS, GS
    dw 0xffff   ; Limit 0-15 bits
    dw 0x0      ; Base 0-15 bits
    db 0        ; Base 16-23 bits
    db 0x92     ; Access byte
    db 11001111b ; High and low 4 bit flags
    db 0        ; Base 24-31 bits
gdt_end:


gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start 



 
 [BITS 32]
 load32:
    mov eax, 1
    mov ecx, 100
    mov edi, 0x00100000
    call ata_lba_read
    jmp CODE_SEG:0x00100000

ata_lba_read:
    mov ebx, eax, ; Backup the LBA
    
    ; Send the highest 8 bits of the lba to hard disk controller
    shr eax, 24
    or eax, 0xE0 ; Select the  master drive
    mov dx, 0x1F6
    out dx, al
    ; Finished sending the highest 8 bits of the lba

    ; Send the total sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ; Finished sending the total sectors to read

    ; Send more bits of the LBA
    mov eax, ebx ; Restore the backup LBA
    mov dx, 0x1F3
    out dx, al
    ; Finished sending more bits of the LBA

    ; Send more bits of the LBA
    mov dx, 0x1F4
    mov eax, ebx ; Restore the backup LBA
    shr eax, 8
    out dx, al
    ; Finished sending more bits of the LBA

    ; Send upper 16 bits of the LBA
    mov dx, 0x1F5
    mov eax, ebx ; Restore the backup LBA
    shr eax, 16
    out dx, al 
    ; Finished sending upper 16 bits of the LBA

    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

    ; Read all sectors into memory
.next_sector:
    push ecx

; Checking if we need to read
.try_again:
    mov dx, 0x1f7
    in al, dx
    test al, 8
    jz .try_again

; We need to read 256 words at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw
    pop ecx
    loop .next_sector
    ; End of reading sectors into memory
    ret

times 510-($ - $$) db 0
dw 0xAA55