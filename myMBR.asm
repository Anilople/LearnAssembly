
user_disk equ 100 ; 约定好的用户程序所在扇区

SECTION mbr align=16 vstart=0x7c00
    
; 设置堆栈
    mov ax,0
    mov ss,ax
    mov sp,ax

; 用户程序要加载的目标内存
    ; mov ax,[cs:phy_base_ip] ;注意要用cs来访问
    ; mov di,ax ; 偏移地址
    mov ax,[cs:phy_base_cs]
    mov es,ax ; 段地址
    mov ds,ax

; 将用户程序第一个扇区读取出来,起始扇区号为di:si
; 目标为ds:bx
    xor di,di
    mov si,user_disk ; 用户程序起始扇区di:si
    xor bx,bx
    call read_hard_disk_0 ;

; 重定位
; 约定如下,基于加载器设置的目标内存base(也就是上边的es:di)
; dw 程序长度       [es:di+0]
; dw 入口偏移地址   [es:di+2]
; dw 入口段地址     [es:di+4]
; dw 段重定位表长度 +6
; 段表

    ; mov ax,[es:di] ; 程序长度(根据这个判断还要不要继续读硬盘)
    ; ; 由于是测试程序,先忽略要继续读硬盘的可能
    ; mov ax,[cs:phy_base_ip]
    ; add [es:di+4],ax
    ; mov ax,[cs:phy_base_cs]
    ; add [es:di+6],ax
    ; mov bx,8
    ; mov cx,[es:di+10] ; 段重定位表长度
    ; relocate_table:
    ;     add [es:di+bx],ax
    ;     add bx,4
    ;     loop relocate_table
    
    ;忽略计算程序长度

    ;计算入口代码段基址
    ; mov ax,[cs:phy_base_ip]
    ; add [0x04],ax
    mov ax,[0x06]
    mov dl,16
    div dl
    add ax,[cs:phy_base_cs]
    mov [0x06],ax

;段重定位表  
    mov cx,[0x0a] ;长度
    mov bx,0x0c ;段重定位表偏移量
    realloc:
        mov ax,[bx]
        mov dl,16
        div dl
        add ax,[cs:phy_base_cs]
        mov [bx],ax
        add bx,4
        loop realloc

; 接下来交给用户程序执行
    jmp far [0x04]

    hlt

; 读硬盘的子程序,每次只读1个扇区(512字节)
; 入口参数:
; es -- 目标内存的段地址
; di -- 目标内存的偏移地址
; si -- 要读取的扇区号0~15位
; ds -- 要读取的扇区号16~28位
; read_disk_1:
;     pusha

;     ; 1.要读取扇区个数
;     mov al,1 
;     mov dx,0x1f2
;     out dx,al

;     ; 2.起始扇区号
;     mov ax,si ; 0~15位 -> ax
;     mov dx,0x1f3
;     out dx,al ; 0~7位 -> 0x1f2
;     mov al,ah ; 8~15位 -> al
;     mov dx,0x1f4
;     out dx,al ; 8~15位 -> 0x1f3
;     mov ax,ds ; 0~15位 -> ds
;     mov dx,0x1f5 
;     out dx,al ; 16~23位 -> 0x1f5
;     mov al,ah ; 24~31位 -> al
;     ;但是注意扇区号数只到24~27位
;     and al,0b11101111
;     mov dx,0x1f6
;     out dx,al ; 24~31位 -> 0x1f6

;     ; 3.请求读硬盘
;     mov al,0x20
;     mov dx,0x1f7
;     out dx,al

;     ; 4.等硬盘准备好
;     mov dx,0x1f7
;     .wait_disk:
;         in al,dx
;         and al,0x88
;         cmp al,0b00001000
;         jne .wait_disk ;不为这个值就继续等待
    
;     ; 5.读出数据,512字节,到es:di上
;     mov dx,0x1f0
;     mov cx,256 ; 由于是16位端口,相当于一次读2个字节,所以读256次就可以了
;     read_256:
;         in ax,dx
;         mov  [es:di],ax
;         add di,2
;         loop read_256
;     popa
;     ret

;-------------------------------------------------------------------------------
read_hard_disk_0:                        ;从硬盘读取一个逻辑扇区
                                         ;输入：DI:SI=起始逻辑扇区号
                                         ;      DS:BX=目标缓冲区地址
         push ax
         push bx
         push cx
         push dx
      
         mov dx,0x1f2
         mov al,1
         out dx,al                       ;读取的扇区数

         inc dx                          ;0x1f3
         mov ax,si
         out dx,al                       ;LBA地址7~0

         inc dx                          ;0x1f4
         mov al,ah
         out dx,al                       ;LBA地址15~8

         inc dx                          ;0x1f5
         mov ax,di
         out dx,al                       ;LBA地址23~16

         inc dx                          ;0x1f6
         mov al,0xe0                     ;LBA28模式，主盘
         or al,ah                        ;LBA地址27~24
         out dx,al

         inc dx                          ;0x1f7
         mov al,0x20                     ;读命令
         out dx,al

  .waits:
         in al,dx
         and al,0x88
         cmp al,0x08
         jnz .waits                      ;不忙，且硬盘已准备好数据传输 

         mov cx,256                      ;总共要读取的字数
         mov dx,0x1f0
  .readw:
         in ax,dx
         mov [bx],ax
         add bx,2
         loop .readw

         pop dx
         pop cx
         pop bx
         pop ax
      
         ret

phy_base_ip dw 0x0
phy_base_cs dw 0x1000

;入口参数
;bh -- 字符所在行
;bl -- 字符所在列
;cx -- 字符及属性
showChar:
    pusha
    mov ax,160
    mul bh
    add bl,bl
    mov bh,0
    add ax,bx
    mov bx,ax ; 字符所在显存计算完毕

    mov ax,0xB800
    mov ds,ax
    mov [bx],cx
    popa
    ret


times 510-($-$$) db 0
dw 0xaa55