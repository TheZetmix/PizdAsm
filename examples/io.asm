section .func
puts:
    pusha
    arg SI
    mov CX SI
    mov DX SI
    int 0x30
    add DX BP
puts_loop:
    load AL CX
    int 0x10
    inc CX
    cmp CX DX
    jne puts_loop
    popa
    ret
endsec
