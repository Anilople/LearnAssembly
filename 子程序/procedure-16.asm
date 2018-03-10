; 读硬盘的子程序,每次只读1个扇区(512字节)
; 入口参数:
; ds -- 目标内存的段地址
; bx -- 目标内存的偏移地址
; si -- 要读取的扇区号0~15位
; di -- 要读取的扇区号16~28位
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


    mov ax,di ; 16~31位 -> ax
    mov dx,0x1f5 
    out dx,al ; 16~23位 -> 0x1f5

    mov al,ah ; 24~31位 -> al
    ;但是注意扇区号数只到24~27位
    or al,0b11100000 ; 小心这里,不要写成and al,0b11101111了,不然会不是LBA模式
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
        and al,0x88
        cmp al,0x08
        jne .wait_disk ;不为这个值就继续等待
    
    ; 5.读出数据,512字节,到ds:bx上
    mov dx,0x1f0
    mov cx,256 ; 由于是16位端口,相当于一次读2个字节,所以读256次就可以了
    read_256:
        in ax,dx
        mov  [bx],ax
        add bx,2
        loop read_256

    popa
    ret

; -------------------------------------------------------------------

; 根据协议读取用户程序到指定内存上 -- 借助了子程序read_disk_1
; 入口参数
; ds:0x0000 -- 指定内存
; di:si -- 指定开始扇区
read_user_program:
    pusha

    xor bx,bx
    call read_disk_1 ; 先读取第一个扇区

; 看后边还用不用继续读用户程序
    ;计算程序长度,决定还要读入多少扇区(1个扇区512字节)
    mov ax,[0x00]
    mov dx,[0x02] ; 程序长度为dx(16~31),ax(0~15)
    mov bx,512
    div bx
    ; 现在dx存余数,ax存商
    ; 如果dx>0 那么实际总共要读取的扇区数为ax+1
    ; 如果dx=0 那么实际总共要读取的扇区数为ax
    ; 所以, if(dx>0) dx=1
    ; 然后add ax,dx
    cmp dx,0
    je add_dx_to_ax
    mov dx,1 ; 如果dx不为0,那么将它置1
    add_dx_to_ax:
        add ax,dx ; 然后加到ax上
    ; 但是由于我们已经读取过一个扇区
    sub ax,1 ; 剩余扇区数等于实际总扇区数-1

    mov cx,ax ; 设置循环,注意如果cx=0,那就不用继续读取了
    ; 如果cx=0,还用loop,就会循环0xffff次
    jcxz not_continue_read_disk
    
    xor bx,bx 
    continue_read_disk:
        inc si
        adc di,0 ; 移动到下一个扇区
        add bx,512 ; 指向下一段内存
        call read_disk_1
        loop continue_read_disk
    
    not_continue_read_disk:

    popa
    ret