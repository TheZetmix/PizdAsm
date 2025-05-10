section .use

endsec

section .func
;; block for functions
endsec

section .data
msg:    db "PENIS!!!\n" 0xEF
endsec

section .main
    int 0x00
    
