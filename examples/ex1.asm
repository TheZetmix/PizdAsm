    ;; Функция puts и вывод хело ворлд

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

section .data
msg:    db "Hello, World!\n" 0xEF
endsec
    
section .main
    int 0x00
    call puts msg
endsec
