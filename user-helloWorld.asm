SECTION header align=16 vstart=0
    program_length dd program_end ; 程序长度

    code_entry ; 入口
        dw start ; offset [4]
        dd section.code.start ; segment [6]
    
    relocation_len dw (header_end-relocation)/4 ;[10]

    relocation:; 段重定位表 
    code_segment dd section.code.start ;[12->0x0c]
    data_segment dd section.data.start ;[16->0x10]
    stack_segment dd section.stack.start

    header_end:

SECTION code align=16 vstart=0
start:
	; push ss
    ; pusha

    ; mov ax,[stack_segment]
    ; mov ss,ax
    ; mov sp,stackEND ; 设置用户自己的堆栈

    mov ax,[data_segment] ; 字符地址
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

    ; popa
    ; pop ss
    retf

    code_end:

SECTION data align=16 vstart=0
    times 500 db 0 
    hello:
    db "hello,world!"
    db "It's is a data segment test."
    count equ $-hello

    data_end:

SECTION stack align=16 vstart=0 ; 堆栈段
    times 256 dw 0
    stackEND:

SECTION trail align=16 ;注意没有vstart,这样标号就会从这个程序的开头开始算
program_end:

; times 510-($-$$) db 0
; dw 0xaa55