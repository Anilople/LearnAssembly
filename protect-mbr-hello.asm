    gdt_base_seg equ 0x7e00
SECTION code align=16 vstart=0x7c00
start:
; 1.初始化堆栈
    mov ax,0
    mov ss,ax
    mov sp,start

; 2.gdt加载
    mov eax,gdt_base_seg/16
    mov ds,eax

    ; 0.空描述符
    mov dword [bx],0
    mov dword [bx+4],0

    ; 1.代码段描述符
    mov dword [bx+8],   0x0000ffff 
    mov dword [bx+12],  0x00cf9800 ;段基址为0,G=1,D/B=1, limit=0xfffff

    ; 2.数据段描述符
    mov dword [bx+16],  0x0000ffff
    mov dword [bx+20],  0x00cf9200

    ; 3.堆栈段描述符
    mov dword [bx+24],  0x7c007000
    mov dword [bx+28],  0x00409600 ;段基址为0x7c00,G=0,D/B=1, type = 0110, limit = 0x7000

    ; 设置gdt长度,或者说是界限
    mov word [cs:gdt_limit],4*8-1
    ; 装载gdt
    lgdt [cs:gdt_limit]

; 3.进入保护模式
    ; 1.enable A20
    in al,0x90
    or al,2
    out 0x90,al

    ; 2.关中断
    cli

    ; 3.修改cr0的pe位
    mov eax,cr0
    or eax,1
    mov cr0,eax

    ; 4.修改cs,清空流水线,进入保护模式
    jmp dword 0b1_0_00:hello ; 注意TI位,PL的2为,index实际上是从第4位,也就是bit 3开始的

; 4.保护模式下的hello,world
    bits 32 ; 切换成32位
hello:
    ; 设置数据段
    mov eax,0b10_0_00 ; index=2, TI=0
    mov ds,eax
    ; 设置堆栈段
    mov eax,0b11_0_00 ; index=3, TI=0
    mov ss,eax
    mov esp,0x7c00 

    ; 往显存上写hello,world
    mov ebx,0xB8000 ;显存段基址
    mov esi,helloData ; 要显示字符的偏移地址
    mov ecx,0 ;计数
    showHello:
        mov al,byte [esi]
        mov [ebx+ecx*2],al
        inc esi
        inc ecx
        cmp ecx,wordNumber ; 看显示的字符数量是不是够了
        jne showHello

    ; mov byte [ebx],'H'
    ; mov byte [ebx+2],'e'
    ; mov byte [ebx+4],'l'
    ; mov byte [ebx+6],'l'
    ; mov byte [ebx+8],'o'
    ; mov byte [ebx+10],','
    ; mov byte [ebx+12],'w'
    ; mov byte [ebx+14],'o'
    ; mov byte [ebx+16],'r'
    ; mov byte [ebx+18],'l'
    ; mov byte [ebx+20],'d'
    ; mov byte [ebx+22],'!'

; 5.hlt
    hlt ; 等中断,由于前边已经关中断,cpu将不会被唤醒

helloData:
    db 'Hello,world!'
    wordNumber equ $-helloData

gdt_limit   dw 0 ; 界限
            dd gdt_base_seg ;段基址

times 510-($-$$) db 0
dw 0xaa55