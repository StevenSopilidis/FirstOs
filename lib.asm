section .text
global memset
global memcpy
global memmove
global memcmp


memset: ; rdi->buffer, rsi->value, rdx->size
    cld
    mov ecx, edx
    mov al, sil
    rep stosb
    ret

memcmp: ; rdi->src1, rsi->src2, rdx->size
    cld
    xor eax, eax
    mov ecx, edx
    repe cmpsb ; rereapts compare until not equal, if no equal set zero flag
    setnz al ; set al to 1 if zero flag is cleared
    ret ; al is zero if equal

memmove: ; rdi->dst, rsi->src, rdx->size
memcpy:
    cld
    cmp rsi, rdi
    jae .copy ; if dont overlap
    mov r8,rsi
    add r8, rdx
    cmp r8, rdi
    jbe .copy

.overlap: ; copy backwards if locations overlap (from finish to start as oposed from start to finish so we dont mess the memory)
    std 
    add rdi, rdx
    add rsi, rdx
    sub rdi, 1
    sub rsi, 1
.copy:
    mov ecx, edx
    rep movsb
    cld
    ret