section .text

global writeu
global sleepu
global waitu
global exitu

writeu:
    sub rsp, 16 ; allocate 16 bytes for paraters buffer and buffer_size
    xor eax, eax

    mov [rsp], rdi ; number of arguments we passed to kernel
    mov [rsp+8], rsi ; rsi points to address of arguments

    mov rdi, 2 
    mov rsi, rsp
    int 0x80 ; exectute syscall

    add rsp, 16 ; restore stack
    ret

sleepu: ; gets the duration os sleep as parameters
    sub rsp, 8
    mov eax, 1 ; index number of sleep in kernel
    mov [rsp], rdi ; copy args
    mov rdi, 1 ; number of arguments saved in rdi
    mov rsi, rsp ; address of arguments saved in rsi

    int 0x80 ; exec syscall
    add rsp, 8
    ret

waitu: ; has no parameters passed
    mov eax, 2 ; index number of wait sys call in kernel
    mov rdi, 0 ; no parameters passed

    int 0x80
    ret

exitu: ; has no parameters passed
    mov eax, 3 ; index number of exit sys call in kernel
    mov rdi, 0 ; no parameters passed

    int 0x80
    ret