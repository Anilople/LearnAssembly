# 系统及I/O

输入: I/O设备复制数据到主存

输出: 主存复制数据到I/O设备

大多数时候, 高级别I/O函数工作良好, 没有必要直接使用Unix I/O. 那么为什么还要麻烦地学习Unix I/O呢?

* 了解Unix I/O 将帮助你理解其他的系统概念
* 有时你除了使用Unix I/O 以外别无选择

## 10.1 Unix I/O

一个Linux文件就是一个m个字节的序列:
$$
B_0, B_1, \cdots, B_k, \cdots, B_{m-1}
$$
所有的I/O设备(例如网络, 磁盘和终端)都被模型化为文件, 而所有的输入和输出都被当作对相应文件的读和写来执行

* 打开文件
* Linux创建的每个进程开始时都有3个打开的文件: 标准输入(描述符为0), 标准输出(描述符为1)和标准错误(描述符为2)
* 改变当前的文件位置. `seek`可以改变文件中的位置k(初始默认是0)
* 读写文件. 当文件位置k>=m时(m代表这个文件的大小为m字节), 如果应用程序执行读操作, 就会触发一个称为EOF的条件, 并且能够被应用程序通过某种方法来检测到. 注意在文件结尾处并没有明确的"EOF"符号.
* 关闭文件

## 10.2 文件

每个Linux文件都有一个类型(type)来表明它在系统中的角色:

* 普通文件(regular file)包含任意数据. 文本文件或者二进制文件
* 目录(directory)是包含一组链接(link)的文件, 其中每个链接都将一个文件名映射到一个文件, 这个文件可能是另一个目录
* 套接字(socket)是用来与另一个进程进行跨网络通信的文件

每个进程都有一个当前工作目录(current working directory)来确定其在目录层次结构中的当前文职

路径名(pathname)有2种形式:

* 绝对路径名(absolute pathname)以一个斜杠"/"开始
* 相对路径名(relative pathname)以文件名开始, 表示从当前工作目录开始的路径

## 10.3 打开和关闭文件

open函数总是返回一个值最低的为打开的描述符, 在程序运行时, 0, 1, 2已经被打开

## 10.4 读和写文件

```c
#include<unistd.h>
ssize_t read(int fd, void *buf, size_t n);

ssize_t write(int fd, const void *buf, size_t n);
```

## 10.5 用RIO包健壮地读写(?)

RIO(Robust I/O, 健壮的I/O)

* 无缓冲的输入输出函数
* 带缓冲的输入函数

## 10.6 读取文件元数据

使用`stat`和`fstat`函数, 检索到关于文件的信息(元数据(metadata))

```c
#include<unistd.h>
#include<sys/stat.h>

int stat(const char *filename, struct stat *buf);
int fstat(int fd, struct stat *buf);
```

可以查看文件的size, mode等等

## 10.7 读取目录内容

`readdir`系列函数

```c
#include<sys/types.h>
#include<dirent.h>

DIR *opendir(const char*name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
```

## 10.8 共享文件

需要清楚内核是如何表示打开的文件, 某则文件共享的概念相当难懂

内核用3个相关的数据结构来表示打开的文件

* 描述符表(descriptor table)
* 文件表(file table), 所有进程共享
* v-node表, 包含stat结构中的大多信息

描述符表 -> 文件表 -> v-node表

一个文件可以被open函数用2次

## 10.9 I/O重定向

```c
#include<unistd.h>
int dup2(int oldfd, int newfd); # File[old]=File[new]
```

## 10.10 标准I/O

标准I/O库将一个打开的文件模型化为一个stream. 对于程序员而言, 一个stream就是一个指向FILE类型的结构的指针. 每个ANSI C程序开始时都有3个打开的流stdin, stdout和stderr

类型为FILE的流是对文件描述符和流缓冲区的抽象

流缓冲区的目的和RIO读缓冲区的目的一样: 就是使开销较高的Linux I/O 系统调用的数量尽可能得小

当第一次调用`getc`时, 库通过调用一次`read`函数来填充缓冲区, 然后将缓冲区中的第一个字节返回给应用程序. 只要缓冲区中还有未读的字节, 接下来对`getc`的调用就能直接从流缓冲区得到服务.

## 10.11 综合: 我该使用哪些I/O函数?

一些基本的知道原则:

* G1: 只要有可能就使用标准I/O
* G2: 不要使用`scanf`或`rio_readlineb`来读二进制文件. 这类函数是专门设计来读取文本文件的
* 对网络套接字的I/O使用RIO函数

标准I/O流, 从某种意义上而言是全双工的, 因为程序能够在同一个流上执行输入和输出. 然而, 对流的限制和对套接字的限制, 有时候会相互**冲突**, 而又极少有文档描述这些现象

* **限制一** : 跟在输出函数之后的输入函数. 如果中间没有插入对`fflush`, `fseek`, `fsetpos`或者`rewind`的调用, 一个输入函数不能跟随在一个输出函数之后. `fflush`函数清空与流相关的缓冲区. 后3个函数使用Unix I/O `lseek`函数来重置当前的文件位置
* **限制二** : 跟在输入函数之后的输出函数. 如果中间没有插入对`fseek, ` `fsetpos` 或者`rewind`的调用, 一个输出函数不能跟随在一个输入函数之后, 除非该输入函数遇到了一个文件结束.

对套接字来说, 对于限制一只要在输出之后都紧接着`fflush`就可以了, 但是对于限制二, 只有一个方法, 用函数是不行的, 只能对同一个打开的套接字描述符打开2个流, 一个用来读, 另一个用来写:

```c
FILE *fpin, *fpout;

fpin = fdopen(sockfd, "r");
fpout = fdopen(sockfd, "w");

// 在释放额时候, 要求用2次fclose
fclose(fpin);
fclose(fpout);
// 这些操作中的每一个都试图关闭同一个底层的套接字描述符, 所以第二个close操作就会失败.
// 对于顺序的程序来说, 这并不是问题, 但是在要给线程化的程序中关闭一个已经关闭了的描述符是会导致灾难的
```

因此, 我们建议你在网络套接字上不要使用标准I/O函数来进行输入和输出, 而要使用健壮的RIO函数. 如果你需要格式化的输出, 使用`sprintf`函数在内存中格式化一个字符串, 然后用`rio_writen`把它发送到套接口. 同样`sscanf`



