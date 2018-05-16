# csapp - 第7章 链接

**链接(linking)**: 将各种代码和数据片段手机并组合成为一个单一文件的过程, 这个文件可被**加载**(复制)到内存并执行.

链接可以执行的时期:

* 编译时(compile time): 源代码被翻译成机器代码时
* 加载时(load time): 程序被**加载器(loader)**加载到内存并执行时
* 运行时(run time): 由应用程序来执行

在早期的计算机系统中, 链接是手动执行的. 在现代系统中, 链接是由叫做**链接器(linker)**的程序自动执行的.

链接器在软件开发中扮演着一个关键的角色, 因为它们使得**分离编译(separate compilation)**称为可能.

为什么要学关于链接的知识?

* 理解链接器将帮助你构造大型程序
* 理解链接器将帮助你避免一些危险的编程错误
* 理解链接将帮助你理解语言的作用域规则是如何实现的
* 理解链接将帮助你理解其他重要的系统概念
* 理解链接将使你能够利用共享库