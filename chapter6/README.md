# CSAPP - 第6章 - 存储器层次结构

## 6.1 存储技术

随机访问存储器（Random-Access Memory，RAM）：

* 静态（SRAM）：双稳态
* 动态（DRAM）：电容，需要定时刷新

磁盘：

柱面，磁道，扇区号，盘片

容量 - 记录密度，磁道密度，面密度



固态硬盘：

速度快



各类存储设备的价格，速度，大致工作原理

## 6.2 局部性

具有良好**局部性（locality）**的程序倾向于引用邻近其他最近引用过的数据项的数据项，或者最近引用过的数据项本身；这种倾向性，被称为局部性原理（principle of locality）

时间局部性（temporal locality）：被引用过一次的内存位置可能在不远的将来再被多次引用

空间局部性（spatial locality）：如果一个内存位置被引用了一次，那么程序很可能在不远的将来引用附近的一个内存位置。

```c
// 这个函数
// 有很好的空间局部性
// 很差的时间局部性
int sumvec(int v[N])
{
    int i, sum = 0;
    
    for (i = 0; i < N; i++)
        sum += v[i];
    return sum;
}
```

一个连续向量中，每隔k的元素进行访问，就称为**步长为k的引用模式（stride-k reference pattern）**

步长为1的引用模式是程序中空间局部性常见和重要的来源，一般而言，随着步长的增加，空间局部性下降

```c
/*
步长为1，有良好的局部性
*/
int sumarrayrows(int a[M][N])
{
    int i,j,sum = 0;
    
    for(i = 0; i < M; i++)
        for(j = 0; j < N; j++)
            sum += a[i][j];
    return sum;
}

/*
步长为N，局部性很差
*/
int sumarraycols(int a[M][N])
{
    int i,j,sum = 0;
    
    for(j = 0; j < N; j++)
        for(i = 0; i < M; i++)
            sum += a[i][j];
   	return sum;
}
```



```c
#define N 1000
typedef struct {
    int vel[3];
    int acc[3];
} point;
point p[N];

void clear1(point *p, int n)
{
    int i,j;
    
    for(i = 0; i < n; i++){
        for(j = 0; j < 3; j++)
            p[i].vel[j] = 0;
        for(j = 0; j < 3; j++)
            p[i].acc[j] = 0;
    }
}

void clear2(point *p, int n)
{
    int i,j;
    
    for(i = 0; i < n; i++){
        for(j = 0; j < 3; j++){
            p[i].vel[j] = 0;
            p[i].acc[j] = 0;
        }

    }
}

/*
clear1有着比clear2更好的局部性
因为在clear2中
p[i].vel[0]与p[i].acc[j]不相邻
*/
```



## 6.3 存储器层次结构

* 存储技术
* 计算机软件

允许程序访问存储在远程的网络服务器上的文件：

* 安德鲁文件系统（Andrew File System，AFS）
* 网络文件系统（Network File System，NFS）



**高速缓存（cache）**是一个小而快速的存储设备，它作为存储在更大、也更慢的设备中的数据对象的缓冲区域。

使用高速缓存的过程称为**缓存（caching）**

存储器有层次，越高层的存储，速度越快，容量越小，价格越贵

存储器层次结构的中心思想是，对于每个k，位于k层的更快更小的存储设备作为位于k+1层的更大更慢的存储设备的缓存。

数据总是以块大小为**传送单元（transfer unit)**在第k层和第k+1层之间来回复制的

在第i层中，层里的块的大小是固定的

在第i层与第j层（i != j）中，块的大小不一样。一般而言，层次结构中较低层（离CPU较远）的设备的访问时间较长，因此为了步长这些较长的访问时间，倾向于使用较大的块

1. 缓存命中：当程序需要第k+1层的某个数据对象d时，它首先在当前存储在第k层的一个块中查找d。如果d刚好缓存在第k层中，那么就是我们所说的**缓存命中（cache hit）**
2. 缓存不命中：如果第k层中没有缓存数据对象d，那么就是我们所说的**缓存不命中（cache miss）**。当发生缓存不命中时，第k层的缓存从第k+1层缓存中取出包含d的那个块，如果第k层的缓存已经满了，可能就会覆盖现存的一个块。

覆盖一个现存的块的过程称为**替换（replacing）**或**驱逐（evicting）**这个块。被驱逐的这个块有时也称为**牺牲块（victim block）**。决定该替换哪个块是由缓存的**替换策略（replacement policy）**来控制的。

3. 缓存不命中的种类
4. 缓存管理

## 6.4 高速缓存存储器

早期计算机系统的存储器层次结构只有3层：**CPU寄存器、DRAM主存储器和磁盘存储**

但是由于CPU和主存之间逐渐增大的差距，系统设计者被迫在CPU寄存器文件和主存之间插入了一个小的SRAM高速缓存存储器，称为L1高速缓存

* L1高速缓存：4周期
* L2高速缓存：10周期
* L3高速缓存：50周期

对于存储器，可以用四元组（S，E，B，m）来表示

* 基本参数

  | 参数            | 描述                      |
  | --------------- | ------------------------- |
  | $S = 2^s$       | 组数                      |
  | E               | 每个组的行数(caches line) |
  | $B = 2^b$       | 块大小（字节）(block)     |
  | $m=\log _2 (M)$ | （主存）物理地址位数      |

* 衍生出来的参数

  | 参数                      | 描述                |
  | ------------------------- | ------------------- |
  | $M=2^m$                   | 内存地址的最大数量  |
  | $s=\log _2 (S)$           | 组索引位数量        |
  | $b=log_ 2(B)$             | 块偏移位数量        |
  | $t = m - (s + b)$         | 标记位(tag bit)数量 |
  | $C = B \times E \times S$ |                     |

每个组只有一行（E = 1）的高速缓存称为**直接映射高速缓存（direct-mapped cache）**

高速缓存确定一个请求是否命中，然后抽取出被请求的字的过程，分为3步：

1. **组选择**
2. **行匹配**
3. **字抽取**

如果想访问地址13，假设t = 1，s = 2，b = 1。首先把13 = 1101，得知标记位t = 1，组索引s = 10，块偏移t = 1，故先访问组10，在看是否命中（根据标记位），如果命中，返回相应块偏移地址上的值；如果不命中，接着访问更低层的（更慢的）存储

如果用高位做组索引，那么一些连续的内存块就会映射到相同的高速缓存块。

**抖动（thrash）**：高速缓存反复地加载和驱逐相同的高速缓存块的组

**组相联高速缓存（set associative cache）**：每个组都保存有多于一个的高速缓存行（E > 1）。当不命中时，需要考虑新的数据放在哪个行的问题，如果有空行，直接放入空行即可，如果没有，那就需要采取一定的替换策略了。

全相联高速缓存（fully associative cache）：s = 1，即只有一组，E = C / B。只适合做小的高速缓存，TLB（缓存页表项）

### 6.4.5 有关写的问题

#### 写命中（write hit）

上边是关于读的处理，当写的时候，比如要写一个已经缓存的字w（**写命中，write hit**）。在高速缓存更新了它的w的副本之后，怎么更新w在层次结构中紧接着低一层中的副本呢？

**直写（write-through）**：立即将w的高速缓存块写回到紧接着的低一层中，缺点是每次写都会引起总线流量。

**写回（write-back）**: 尽可能地推迟更新，只有当替换算法要驱逐这个更新过的块时，才把它写到紧接着的低一层中。优点是能显著地减少总线流量，但是缺点是增加了复杂性。高速缓存必须为每个高速缓存行维护一个额外的修改位（dirty bit），表明这个高速缓存块是否被修改过。

#### 写不命中

**写分配（write-allocate）**: 加载相应的低一层中的块到高速缓存中，然后更新这个高速缓存块

**非写分配（not-write-allocate）**: 避开高速缓存，直接把这个字（数据）写到低一层中

### 6.4.6 一个真实的高速缓存层次结构的解剖

实际上，高速缓存既保存数据，也保存指令。

### 6.4.7 高速缓存参数的性能影响

* 不命中率（miss rate）: *不命中数量*/*引用数量*
* 命中率（hit rate）： 1 - 不命中率
* 命中时间（hit time）。从高速缓存传送一个字到
  CPU所需的时间。
* 不命中处罚（miss penalty）：由于不命中所需要的额外的时间。

### 6.5 编写高速缓存友好的代码

局部性较好的程序更容易有较低的不命中率，而不命中率较低的程序往往比不命中率较高的程序运行得更快。

好的程序员总是应该试着去编写**高速缓存友好（cache friendly）**的代码，这里有一些基本的方法

1. 让最常见的情况运行得快。把注意力集中在核心函数里的循环上，而忽略其他部分。

2. 尽量减小每个循环内部的缓存不命中数量。

   ```c
   int sumvec(int v[N])
   {
       int i,sum = 0;
       
       for(i = 0; i < N; i++)
           sum += v[i];
       return sum;
   }
   // 这个函数告诉我们
   ```

   * 对于局部变量的反复引用是好的，因为编译器能够将它们缓存在寄存器文件中（时间局部性）
   * 步长为1的引用模式是好的，因为存储器层次结构中所有层次上的缓存都是将数据存储为连续的块（空间局部性）。

### 6.6 综合：高速缓存对程序性能的影响

**吞吐量（read throughput）**或**带宽（read bandwidth）**。如果一个程序在s秒的时间段内读n个字节，那么这段时间内的吞吐量就等于n/s，通常以兆字节美妙（MB/s）为单位。