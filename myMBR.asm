
user_disk equ 100 ; 约定好的用户程序所在扇区

SECTION mbr align=16 vstart=0x7c00
    
; 设置堆栈
    mov ax,0
    mov ss,ax
    mov sp,ax

; 用户程序要加载的目标内存
    mov ax,[phy_base_ip]
    mov di,ax ; 偏移地址
    mov ax,[phy_base_cs]
    mov es,ax ; 段地址

; 将用户程序第一个扇区读取出来,起始扇区号为ds:si
; 加载到es:di地址上
    mov ax,0
    mov ds,ax
    mov si,user_disk ; 用户程序起始扇区
    call read_disk_1 ;

; 重定位
; 约定如下,基于加载器设置的目标内存base(也就是上边的es:di)
; dw 程序长度       [es:di+0]
; dw 入口偏移地址   [es:di+2]
; dw 入口段地址     [es:di+4]
; dw 段重定位表长度 +6
; 段表

    mov ax,[es:di] ; 程序长度(根据这个判断还要不要继续读硬盘)
    ; 由于是测试程序,先忽略要继续读硬盘的可能
    mov ax,[phy_base_ip]
    add [es:di+2],ax
    mov ax,[phy_base_cs]
    add [es:di+4],ax
    mov bx,8
    mov cx,[es:di+6]
    relocate_table:
        add [es:di+bx],ax
        add bx,2
        loop relocate_table
    
    jmp dword [es:di]

    mov ax,0xB800
    mov ds,ax
    mov si,0x10
    mov byte [ds:si],'h'
; 接下来交给用户程序执行

; 读硬盘的子程序,每次只读1个扇区(512字节)
; 入口参数:
; es -- 目标内存的段地址
; di -- 目标内存的偏移地址
; si -- 要读取的扇区号0~15位
; ds -- 要读取的扇区号16~28位
read_disk_1:
    pusha

    ; 1.要读取扇区个数
    mov al,1 
    mov dx,0x1f2
    out dx,al

    ; 2.起始扇区号
    mov ax,si ; 0~15位 -> ax
    mov dx,0x1f3
    out dx,al ; 0~7位 -> 0x1f2
    mov al,ah ; 8~15位 -> al
    mov dx,0x1f4
    out dx,al ; 8~15位 -> 0x1f3
    mov ax,ds ; 0~15位 -> ds
    mov dx,0x1f5 
    out dx,al ; 16~23位 -> 0x1f5
    mov al,ah ; 24~31位 -> al
    ;但是注意扇区号数只到24~27位
    and al,0b10101111
    mov dx,0x1f6
    out dx,al ; 24~31位 -> 0x1f6

    ; 3.请求读硬盘
    mov al,0x20
    mov dx,0x1f7
    out dx,al

    ; 4.等硬盘准备好
    mov dx,0x1f7
    .wait_disk:
        in al,dx
        cmp al,0b00001000
        jne .wait_disk ;不为这个值就继续等待
    
    ; 5.读出数据,512字节,到es:di上
    mov dx,0x1f0
    mov cx,256 ; 由于是16位端口,相当于一次读2个字节,所以读256次就可以了
    read_256:
        in ax,dx
        mov  [es:di],ax
        add di,2
        loop read_256
    popa
    ret

phy_base_ip dw 0
phy_base_cs dw 1000

times 510-($-$$) db 0
dw 0xaa55