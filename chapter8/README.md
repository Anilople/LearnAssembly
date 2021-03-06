并发流(concurrent flow): 一个逻辑流的执行在时间上与另一个流重叠，称为并发流，这2个流被称为并发地运行。

模式位(mode bit)：提供用户模式和内核模式的标志

上下文切换(context switch)

调度(scheduling):在进程执行的某些时刻，内核可以决定抢占当前进程，并重新开始一个先前被抢占了的进程，由内核中称为调度器(scheduler)的代码处理。

```c
pid_d fork(void);
```

fork函数是有趣的(也常常令人迷惑)，因为它只被调用一次，却会返回2次：

* 一次是在调用进程（父进程）中
* 一次是在新创建的子进程中

在父进程中，fork返回子进程的PID。在子进程中，fork返回0。因为进程的PID总是为非0，返回值就提供一个明确的方法来分辨程序是在父进程还是在子进程中执行。

注意，程序不会按照特定的顺序回收子进程。子进程回收的顺序是这台特定的计算机系统的属性。在另一个系统上，甚至在同一个系统上再执行一次，两个子进程都可能以相反的顺序被回收。这是**非确定性行为**的一个示例，这种非确定性行为使得对并发进行推理非常困难。

```c
#include<sys/types.h> // typedef或者define了一些东西，比如pid_t
#include<sys/wait.h>
pid_t waitpid(pid_t pid, int *statusp, int options);
WIFEXITED(status); // 指出子进程是否为正常退出
WEXITSTATUS(status); // 返回非0值时，可以知道子进程exit(x)中的x是什么

```

#### 让进程休眠

```c
#include<unistd.h>
unsigned int sleep(unsigned int secs); // 如果请求的时间到了，sleep返回0，否则返回剩下的要休眠的秒数
int pause(void); // 休眠直到该进程收到一个信号
```

#### 加载并运行程序
```c
// execve函数在当前进程的context中加载并运行一个新程序
#include<unistd.h>
int execve(const char *filename, const char *argv[], const char *envp[]);
```
`execve`函数加载并运行可执行目标文件`filename`，且带参数列表`argv`和环境变量列表`envp`。只有当出现错误时，例如找不到`filename`，`execve`才会返回到调用程序。所以，与`fork`一次调用返回2次不同，`execve`调用一次并从不返回。

##### 利用fork和execve运行程序

像Unix shell和Web服务器这样的程序大量使用了fork和execve函数。

## 8.5 信号

更高层的软件形式的异常，称为Linux信号，它允许进程和内核中断其他进程。

一个信号就是一条小消息，它通知进程系统中发生了一个某种类型的事件。

信号提供了一种机制，使得用户进程可以得知发生了一些低层硬件的异常，比如除0错误

#### 信号术语

* 发送信号。内核通过更新目的进程上下文中的某个状态，发送（递送）一个信号给目的进程。发送信号可以有如下2种原因：（1）内核检测到一个系统事件，比如除0错误或者子进程终止。（2）一个进程调用了kill函数，显式地要求内核发送一个信号给目的进程。一个进程可以发送信号给它自己。
* 接收信号。当目的进程被内核强迫以某种方式对信号的发送做出反应时，它就接收了信号。进程可以忽略这个信号，终止或者通过一个称为**信号处理程序（signal handler）**的用户层函数捕获这个信号。

**待处理信号（pending signal）**：一个发出而没有被接收的信号

如果一个进程有一个类型为**k**的待处理信号，那么任何接下来发送到这个进程的类型为**k**的信号都**不会**排队等待；它们只是被简单地丢弃。

一个进程可以有选择性地**阻塞**接收某种信号(比如调用`sigprocmask`来阻塞某种信号)。当一种信号被阻塞时，它仍可以被发送，但是产生的待处理信号不会被接收，直到进程取消对这种信号的阻塞。

一个待处理信号最多只能被接收一次。内核为每个进程在`pending`位向量（也称为信号掩码`signal mask`）中维护着待处理信号的集合，而在`blocked`位向量中维护着被阻塞的信号集合。只要传送了一个类型为`k`的信号，内核就会设置`pending`中的第`k`位，而只要接收了一个类型为`k`的信号，内核就会清除`pending`中的第`k`位。

#### 发送信号

SIGSTOP和SIGKILL不可捕获

1. 进程组（process group）

每个进程都只属于一个进程组，进程组是由一个正整数`进程组ID`来标识的。

一个子进程和它的父进程属于同一个进程组。

```c
#include<unistd.h>
// 返回当前进程的进程组ID
pid_t getpgrp(void); 
// 改变自己或者其他进程的进程组
// 将进程pid的进程组改为pgid
// 如果pid是0，那么就使用当前进程的PID
// 如果pgid是0，那么就用pid指定的进程的pid作为进程组
int setpgid(pid_t pid,pid_t pgid); 
```
2. 用/bin/kill程序发送信号

可以向另外的进程发送任意的信号

```shell
# 发送信号9(SIGKILL)给进程15213
/bin/kill -9 15213
```

3. 从键盘发送信号

4. 用kill函数发送信号

```c
#include<sys/types.h>
#include<signal.h>
// 发送信号给其他进程（包括它们自己）
int kill(pid_t pid, int sig);
```

5. 用alarm函数发送信号

进程可以通过调用`alarm`函数向它自己发送`SIGALRM`信号。

```c
#include<unistd.h>
// 安排内核在secs秒后发送一个SIGALRM信号给调用进程
unsigned int alarm(unsigned int secs);
```

在任何情况下，对`alarm`的调用都将取消任何待处理的（pending）闹钟，并且返回任何待处理的闹钟在被发送前还剩下的秒数（如果这次对alarm的调用没用取消它的话）；如果没有任何待处理的闹钟，就返回0.

#### 阻塞和接触阻塞信号

* 隐式阻塞机制。内核默认阻塞任何当前处理程序正在处理信号类型的待处理信号

* 显示阻塞机制。应用程序可以使用`sigprocmask`函数和它的辅助函数，明确地阻塞和解除阻塞选定的信号。

```c
#include<signal.h>
// sigprocmask的具体行为依赖于how的值
// SIG_BLOCK -- 把set中的信号添加到blocked中 (blocked |= set)
// SIG_UNBLOCK -- 从blocked中删除set中的信号 (blocked &= ~set)
// SIG_SETMASK -- block = set
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int signum);
int sigdelset(sigset_t *set, int signum);
```

#### 编写信号处理程序

信号处理是Linux系统变成最棘手的一个问题。处理程序有几个属性使得它们很难推理分析：

1. 处理程序与主程序并发运行，共享同样的全局变量，因此可能与主程序和其他处理程序相互干扰。

2. 如何以及何时接收信号的规则常常有违人的直觉

3. 不同的系统有不同的信号处理语义

##### 安全的信号处理

* 处理程序要尽可能简单

* 在处理程序中只调用异步信号安全的函数

* 保存和恢复`errno`

* 阻塞所有的信号，保护对全局共享数据结构的访问。

* 用`volatile`声明全局变量。`volatile int g;`, `volatile`限定符强迫编译器每次在代码引用`g`时，都要从内存中读取g的值（不然可能内存中的值被其他程序修改了都不知道）

* 用`sig_atomic_t`声明标志。C确保对这种数据类型的读写会是原子的(不可中断的)。`volatile sig_atomic_t flag;`，因为它们是不可中断的，所以可以安全地读和写`sig_atomic_t`变量，而不需要暂时阻塞信号。注意，这里对原子性的保证只适用于单个的读和写，不适用于像`flag++`或`flag = flag + 10`这样的更新，它们可能需要多条指令。

##### 正确的信号处理

信号的一个与直觉不符的方面是未处理的信号是不排队的。因为`pending`位向量中每种类型的信号只对应有一位，所以每种类型最多只能有一个未处理的信号。因此，如果2个类型`k`的信号发送给一个目的进程，而因为目的进程当前正在执行信号`k`的处理程序，所以信号`k`被阻塞了，那么第2个信号就简单地被丢弃了；它不会排队。关键思想是如果存在一个未处理的信号就表明**至少**有一个信号到达了。

**不可以用信号来对其他进程中发生的事件计数**，因为信号可能会被丢弃

#### 8.5.6 同步流以避免讨厌的并发错误

如何编写读写相同存储位置的并发流程序的问题，困扰这数代计算机科学家。一般而言，流可能交错的数量与指令的数量呈指数关系。这些交错中的一些会产生正确的结果，而有些则不会。是否能编写一种程序，使得无论以那种顺序执行，都能得到期望的结果呢？