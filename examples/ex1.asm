section .func
putchar:    
    arg AH                      ; Character
    arg AX                      ; Y
    arg DX                      ; X
    cmp DX 70
    jg x_error
    cmp AX 30
    jg y_error
    jmp putchar_main
x_error:
    ret
y_error:
    ret
putchar_main:   
    mov CX 0
.loop_y:
    mov AL 0x7E
    int 0x10
    mov AL 0x0A
    int 0x10
    inc CX
    cmp CX AX
    jne .loop_y
    mov CX 0
.loop_x:
    mov AL 0x7E
    int 0x10
    mov AL 0x20
    int 0x10
    inc CX
    cmp CX DX
    jne .loop_x
    mov AL AH
    int 0x10
    mov AL 0x0A
    int 0x10
    ret
    ;; block for functions
endsec

section .data
    
endsec

section .main
    int 0x00
    call putchar 70 30 0x24
    
