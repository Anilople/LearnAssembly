; 加载"内核"的主引导扇区程序
; 全局参数
	gdt_address equ 0x00007e00 ; 假设为16位对齐
	gdt_number equ 5; include empty descriptor, i.e 0 descriptor
	core_base_address equ 0x00040000
	core_start_disk equ 0x00000001

; 设置堆栈
	mov ax,0
	mov ss,ax
	mov sp,0x7c00

; 创建GDT(数据含义可通过查表)
	mov eax,gdt_address/16 ; 16位模式下的段地址
	mov ds,eax ; 用来访问gdt所在段地址
	xor ebx,ebx ; gdt偏移地址,为了方便,置为0
	; GDT 0 跳过
	; GDT 1 数据段(当然直接开到最大)
	mov dword [ebx+8],0x0000ffff
	mov dword [ebx+12],0x00cf9200
	; GDT 2 数据段(当然直接开到最大)
	mov dword [ebx+16],0x7c0001ff
	mov dword [ebx+20],0x00409800
	; GDT 3 堆栈段
	mov dword [ebx+24],0x7c00fffe
	mov dword [ebx+28],0x00cf9600
	; GDT 4 显示缓冲区(另一个数据段)
	; 每页80*25个字符,每个字符2个字节,共8页
	mov dword [ebx+32],0x80007cff
	mov dword [ebx+36],0x0040920b


	; 内核会引用GDT 1来作为全局数据段,所以这段代码扑街了..
	; ; GDT 1 代码段(只给执行主扇区内的代码)
	; mov dword [ebx+8],0x7c0001ff
	; mov dword [ebx+12],0x00409800
	; ; GDT 2 数据段(当然直接开到最大)
	; mov dword [ebx+16],0x0000ffff
	; mov dword [ebx+20],0x00cf9200
	; ; GDT 3 堆栈段
	; mov dword [ebx+24],0x7c00fffe
	; mov dword [ebx+28],0x00cf9600
	; ; GDT 4 显示缓冲区(另一个数据段)
	; ; 每页80*25个字符,每个字符2个字节,共8页
	; mov dword [ebx+32],0x80007cff
	; mov dword [ebx+36],0x0040920b

	;mov word [cs:GDTR+0x7c00],gdt_number*8-1
	
	lgdt [cs:0x7c00+GDTR] ; 加载gdt表

; 进入保护模式
	; 准备工作
	in al,0x92
	or al,0b10
	out 0x92,al
	
	cli ; 关中断
	
	mov eax,cr0
	or eax,1
	mov cr0,eax ; 设置PE位
	; 通过跳转清空流水线进入保护模式
	jmp dword 0b10_0_00:setNewSeg
	
	[bits 32] ; 和nasm说以下的代码是32位的代码

setNewSeg: ; 设置新的段ds,ss,es
	; 数据段ds
	mov eax,0b1_0_00
	mov ds,eax
	; 堆栈段ss
	mov eax,0b11_0_00
	mov ss,eax
	xor esp,esp
	; 显示缓冲区es
	mov eax,0b100_0_00
	mov es,eax
; 从硬盘读取"内核"所有内容(利用16位的程序)
	mov ebx,core_base_address
	mov eax,core_start_disk
	call read_all_program
; 重定位内核
setup_core:
	mov esi,core_base_address ; 内核起始地址
	; 公用例程段描述符
	mov eax,[esi+8]
	sub eax,[esi+4]
	dec eax ; 段界限
	mov edx,[esi+4]
	add edx,core_base_address
	push dword 0x00409800 ; 段描述符属性
	push eax ; 段界限
	push edx ; 起始汇编地址(段基址)
	call make_gdt_descriptor
	mov [gdt_address+0x28],eax
	mov [gdt_address+0x2c],edx
	; 核心数据段描述符
	mov eax,[esi+12]
	sub eax,[esi+8]
	dec eax ; 段界限
	mov edx,[esi+8]
	add edx,core_base_address
	push dword 0x00409200 ; 段描述符属性
	push eax ; 段界限
	push edx ; 起始汇编地址(段基址)
	call make_gdt_descriptor
	mov [gdt_address+0x30],eax
	mov [gdt_address+0x34],edx
	; 核心代码段描述符
	mov eax,[esi+0] ; 程序长度
	sub eax,[esi+12] ; 程序长度-核心代码起始汇编地址
	dec eax
	mov edx,[esi+12]
	add edx,core_base_address
	push dword 0x00409800 ; 段描述符属性
	push eax ; 段界限
	push edx ; 起始汇编地址(段基址)
	call make_gdt_descriptor
	mov [gdt_address+0x38],eax
	mov [gdt_address+0x3c],edx

	; 重新加载gdt
	add word [0x7c00+GDTR],3*8
	lgdt [0x7c00+GDTR]

	; 剩下的交给内核
	jmp far [esi+0x10]

	hlt
	
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
; -----------------------------------------------
; 读取所有程序(约定程序长度为前4个字节的内容)
; 入口参数
; EAX -- 起始逻辑扇区号(LBA模式)
; DS:EBX -- 目标内存
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
; ----------------------------------------------------
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

GDTR 	dw gdt_number*8-1 ; 描述符表界限
		dd gdt_address ; 描述符表起始地址
times 510-($-$$) db 0
dw 0xaa55 ; 主引导扇区标识