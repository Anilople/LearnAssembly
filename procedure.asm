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