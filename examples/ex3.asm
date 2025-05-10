    ;;  Вывод символа

section .main
    int 0x00
    mov AL $[
    int 0x10
    mov AL 0x0A
    int 0x10
    
