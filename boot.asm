[bits 16] ; tell the assembler we run on 16 bit code (because we first start at  real mode)
[org 0x7c00] ; boot code loaded from boot sector at mem loc 0x7c00

start: ; initialize segment register and stack pointer
    xor ax, ax ; zero ax register
    mov ds, ax ; initialize ds register to 0
    mov es, ax ; initialize ss register to 0
    mov ss, ax ; initialize ss register to 0
    mov sp, 0x7c00 ; boot code above 0x7c00 mem stack below 0x7c00 mem stack (as we push the stack grows downwords)

TestDiskExtension: ; test if LBA addressing is available
    mov [DriveId], dl ; save the drive id to variable
    mov ah, 0x41
    mov bx, 0x55aa
    int 0x13
    jc  NotSupported ;jump to NotSupported if extension is not supported (carry flag is set to 1)
    cmp bx, 0xaa55; if bx != 0xaa55 disk extension is not supported
    jne NotSupported

LoadLoader:
    mov si, ReadPacket ; source index of struct to be used in order to load the loader
    mov word[si], 0x10 ; size of struct
    mov word[si + 2], 5; number of sectors
    mov word[si + 4], 0x7e00 ; offset, mem location in which loader will be loaded
    mov word[si + 6], 0 ; segment, value of segment part of address
    mov dword[si + 8], 1 ; address lo, LBA address of starting sector (0 is the MBR so we load from 1 and so on)
    mov dword[si + 12], 0 ; address hi 
    mov dl, [DriveId] ; device to load from
    mov ah, 0x42 ; indicate using disk extension service
    int 0x13
    jc ReadError ; if sector reading failed
    mov dl, [DriveId] ; save drive id which will be used to load the kernel 
    jmp 0x7e00 ; jump to loader

ReadError:
NotSupported:
    mov ah, 0x13 ; function code for bios interupt
    mov al, 1 ; specify the cursor to be at the end of the string
    mov bx, 0xa
    xor dx, dx
    mov bp, Message
    mov cx, MessageLen
    int 0x10

End:
    hlt
    jmp End

DriveId: db 0
Message: db "We have an error in boot process"
MessageLen: equ $-Message
ReadPacket: times 16 db 0 ; for reading  the loader

;define first partition
times (0x1be - ($ - $$)) db 0 ; fill the boot sector until 0x1be where partion entries reside with zeros

db 80h ; boot indicator
db 0,2,0 ; start chs
db 0f0h ; type
db 0ffh,0ffh,0ffh ;ending chs
dd 1 ; starting sector
dd (20 * 16 * 63-1)
times (16*3) db 0

db 0x55
db 0xaa