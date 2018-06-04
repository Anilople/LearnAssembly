movl $1,%edi
leaq outstring(%rip),%rsi
movl $7,%edx
call write@PLT


outstring:
    .string "hello\n"