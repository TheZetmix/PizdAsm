    ;; Конвертирования последовательности чисел в одно

section .func
convert_int:
    pusha
    arg SI
    int 0x30
    mov DX SI
    add DX BP
    mov EAX 10
    sub BP 1    
    pow EAX BP
    mov CX SI
convert_int_loop:
    load ECX CX
    mul ECX EAX
    add EBX ECX
    div EAX 10
    inc CX
    cmp CX DX
    jne convert_int_loop
    define convert_result EBX
    popa
    ret
endsec
    
section .main
    int 0x00
    dump 0x00 1
    dump 0x01 2
    dump 0x02 5
    dump 0x03 3
    dump 0x04 1
    dump 0x05 2
    dump 0x06 3
    dump 0x07 0xEF
    call convert_int 0x00
