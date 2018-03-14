; SECTION header align=16 vstart=0
;     program_length dw program_end ; 程序长度

;     code_entry ; 入口
;         dw start ; offset
;         dw section.code.start ; segment
    
;     relocation_len dw (header_end-relocation)/2

;     relocation:; 段重定位表
;     code_segment dw section.code.start

;     header_end:

SECTION code align=16 vstart=0x7c00
start:
    mov ax,section.code.start ; 字符地址
    mov ds,ax
    mov si,hello

    mov ax,0xB800 ;显存
    mov es,ax 
    mov di,0

    mov cx,count
    s:
        mov al,[ds:si]
        mov [es:di],al ; 复制字符
        inc si ; 下一字符来源
        inc di
        mov al,0x07
        mov [es:di],al ; 字符属性
        inc di ;
        loop s

    hlt ; 等待中断,节约CPU资源
;jmp near $

    hello:
    db "hello,world!"
    count equ $-hello

    code_end:

; SECTION trail align=16 ;注意没有vstart,这样标号就会从这个程序的开头开始算
; program_end:

times 510-($-$$) db 0
dw 0xaa55