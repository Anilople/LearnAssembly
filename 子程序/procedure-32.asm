; 选择排序
; 入口参数
; ds:ebx -- 数据地址
; ecx -- 数据数量
junkSort:
	pushad
	once:
		call selectOnce 
		inc ebx
		loop once
	popad
	ret

; 选择排序中的子程序
; 将剩下的元素逐渐与第一个元素进行对比,
; 如果比第一个元素小,那么就交换它们
; ds:ebx -- 数据开始地址
; ecx > 0 -- 数据长度
selectOnce:
	pushad
	cmp ecx,1
	jna selectOnceEnd ; 如果ecx<=1 那么直接返回
	; 长度为ecx, 但是只需要比较ecx-1次
	dec ecx
	mov esi,1
	mov al,[ebx] ; 第一个数据
	.for:
		cmp al,[ebx+esi] ; 比较剩下的数据,如果比第一个小,那么swap
		jb .forLast ; al比[ebx+esi]小,则不进行交换
		mov ah,[ebx+esi] ; 第n个字符->ah
		mov [ebx+esi],al ; 第1个字符->第n个字符
		mov al,ah ; 第n个字符->第1个字符
	.forLast:
		inc si
		loop .for
	mov [ebx],al ; 将第一个数据归位
selectOnceEnd:
	popad
	ret

; -------------------------------------------------------------------
; 每次读取一个扇区的内容,LBA模式
; 入口参数
; EAX -- 要读取的逻辑扇区号
; DS:EBX -- 目标内存
; 出口参数
; EBX -- EBX + 512
read_disk_1:
	push ecx
	push edx
	push eax
	; 读取扇区个数
	mov dx,0x1f2
	mov al,1
	out dx,al
	; 起始扇区号
	pop eax ; 注意这里把eax给pop了
	push eax ; 但是这里又把eax给push了
	mov cl,8 ; 每次eax右移8位
	; -- 0~7, 0x1f3
	inc dx
	out dx,al
	; -- 8~15, 0x1f4
	inc dx
	shr eax,cl
	out dx,al
	; -- 16~23, 0x1f5
	inc dx
	shr eax,cl
	out dx,al
	; -- 24~27, 0x1f6
	inc dx
	shr eax,cl
	or al,0xe0
	out dx,al
	; 读硬盘命令, 0x1f7
	inc dx
	mov al,0x20
	out dx,al
	.wait_to_read:
		in al,dx
		and al,0x88
		cmp al,0x08
		jne .wait_to_read ; 如果硬盘忙,就返回.wait_to_read处
	
	; 如果执行到这里,说明现在可以读取数据了
	mov ecx,256 ; 每次读取16bit,总共512字节
	mov dx,0x1f0
	.read_to_memory:
		in ax,dx
		mov [ebx],ax
		add ebx,2
		loop .read_to_memory
	
	; 读取完毕,准备返回
	pop eax
	pop edx
	pop ecx
	ret
; ----------------------------------------------------------
; 读取所有程序(约定程序长度为前4个字节的内容)
; 入口参数
; EAX -- 起始逻辑扇区号(LBA模式)
; DS:EBX -- 目标内存
; 出口参数
; EBX -- 入口EBX+(程序长度/512)+(程序长度%512 > 0)
read_all_program:
	push ecx
	push ebx
	push eax
	
	; 先读取1个扇区
	call read_disk_1 ; 读取后ebx+512
	; 判断程序长度,读取剩下的内容
	mov eax,[ds:ebx-512] ; 读入程序长度
	cmp eax,512
	jna read_all_program_end ; 程序不大于512字节,直接返回
	xor edx,edx
	mov ecx,512
	div ecx
	; 现在,edx=程序长度%512, eax=程序长度/512
	and edx,edx ; 改变zero标志位
	jz skip_inc_eax
		inc eax ; 如果edx>0那么实际读取的扇区数量为eax+1
	skip_inc_eax:
	dec eax ; 但是由于已经读取了1个扇区,剩下要读取的扇区数量要-1
	mov ecx,eax ; 给循环用
	pop eax ; 弹出起始扇区
	push eax
	read_remain_disk: ; 读取剩下的内容
		inc eax ; 先指向下一扇区
		call read_disk_1 ; 再读取
		loop read_remain_disk
	
	; 读取完毕,返回
	read_all_program_end:
	pop eax
	pop ebx
	pop ecx
	ret

; ------------------------------------------
; 构造段描述符
; 入口参数(注意[ss:esp]为eip的值)
; [ss:esp+4] -- 段基地址, 32bit
; [ss:esp+4+4] -- 段界限, 32bit
; [ss:esp+8+4] -- 属性(没用到的位为0), 32bit
; 出口参数
; EDX -- 段描述符高4字节
; EAX -- 段描述符低4字节
make_gdt_descriptor:
	; 高4字节
	mov edx,[ss:esp+4] ; 段基地址->edx
	mov dl,[ss:esp+2+4] ; dl = 段基地址16~23位
	and edx,0xff0000ff
	mov eax,[ss:esp+4+4] ;段界限->eax
	xor ax,ax ; 段界限0~15位置0
	or edx,eax ; 附上段界限16~19位
	or edx,[ss:esp+8+4] ; 附上属性
	; 低4字节
	mov eax,[ss:esp+4] ; 段基地址 -> eax
	shl eax,16
	mov ax,[ss:esp+4+4] ;段界限0~15位 -> ax
	
	ret
