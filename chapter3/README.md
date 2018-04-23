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