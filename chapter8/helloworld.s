	.file	"helloworld.c"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	leaq	out.1757(%rip), %rsi
	movl	$1, %edi
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	$13, %edx
	xorl	%eax, %eax
	call	write@PLT
	xorl	%edi, %edi
	call	_exit@PLT
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.data
	.align 8
	.type	out.1757, @object
	.size	out.1757, 13
out.1757:
	.string	"hello,world\n"
	.ident	"GCC: (Debian 6.4.0-2) 6.4.0 20170724"
	.section	.note.GNU-stack,"",@progbits
