# 深入理解计算机系统 - 第3章

```shell
# p1.c p2.c
gcc -Og -o p p1.c p2.c

# gcc 指 GCC C 编译器， 也可以用 cc 来启动它
# -Og 告诉编译器使用会生产符合原始C代码整体结构的机器代码的优化等级， 
# 对于一些编译器，可以用 -O1 替代 -Og
```

对于**机器级的编程**， **2种抽象**最重要

* ISA(Instruction Set Architecture). 由指令集体系结构来定义机器级程序的格式和行为
* 机器级程序使用的内存地址是虚拟地址

```c
// 文件名： mstore.c
long mult2(long,long);

void multstore(long x,long y, long * dest){
	long t = mult2(x,y);
	*dest = t;
}
```

```shell
gcc -Og -S mstore.c # 产生汇编代码 mstore.s， ATT风格
gcc -Og -S -masm=intel mstore.c # Intel风格汇编
gcc -Og -c mstore.c # 产生机器代码 mstore.o
# 使用gdb查看mstore.o 的二进制代码
$ gdb mstore.o
(gdb) x/14b multstore # 查看从函数multstore所处地址开始的14个hex
$ objdump -d mstore.o # 查看反汇编
```

在C程序中插入汇编代码有2种方法：

* 编写完整的函数，放进一个独立的汇编代码文件中，让汇编器和链接器把它和用C语言书写的代码合并起来。
* 使用GCC的内联汇编(inline assembly)特性，用asm伪指令

x86-64对`mov`系列的指令加了1条限制，传送指令的2个操作数**不能都**指向内存位置

| C声明  | Intel数据类型 | 汇编代码后缀 | 大小（byte） |
| ------ | ------------- | ------------ | ------------ |
| char   | byte（字节）  | b            | 1            |
| short  | word（字）    | w            | 2            |
| int    | double word   | l            | 4            |
| long   | 四字          | q            | 8            |
| char * | 四字          | q            | 8            |
| float  | 单精度        | s            | 4            |
| double | 双精度        | l            | 8            |



> 注意：大多数情况，MOV指令只会更新目的操作数指定的那些寄存器字节或内存位置。唯一的例外是movl指令以寄存器作为目的时，它会把该寄存器的高位4字节设置为0. 这是历史遗留问题，即任何为寄存器生成32位值的指令都会把该寄存器的高位部分置成0.

`movz`系列指令把目的中剩余的字节填充为0

`movs`系列指令通过符号扩展来填充

`INC`和`DEC`会改变`OF`和`ZF`，但是不会改变`CF`

