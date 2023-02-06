
section .text

global start
extern main
extern exitu

; for preparing user processes 
; so we get main func from user process
; and  call it
start:
    call main
    call exitu
    jmp $