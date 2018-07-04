IP地址
一个IP地址就是一个32位无符号整数

```c
// 历史遗留
struct in_addr {
    uint32_t s_addr; // 大端表示
};
```

在IP地址结构中存放的地址总是以(大端法)网络字节顺序存放的, 即使主机字节顺序(host byte order)是小端法.

```c
#include<arpa/inet.h>

// 返回: 按照网络字节顺序的值
uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
                    
// 返回: 按照主机字节顺序的值
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(unit16_t netshort);
                    
// 没有处理64位值的函数
```

IP地址通常是以一种称为**点分十进制表示法**来表示的, 这里, 每个字节由它的十进制值表示, 并且用句点和其他字节间分开.

`128.2.194.242`是地址`0x8002c2f2`的点分十进制表示

应用程序使用`inet_pton`和`inet_ntop`函数来实现IP地址和点分十进制串之间的转换

```c
#include<arpa/inet.h>

// 返回: 若成功返回1, 若src为非法点分十进制地址则返回0
//      若出错则为-1
int inet_pton(AF_INET, const char *src, void *dst);

// 返回: 若成功则指向点分十进制字符串的指针, 若出错则为NULL
const char *inet_ntop(AF_INET, const void *src, char *dst, socklen_t size);
```

在这些函数名中, `n`代表网络, `p`代表表示.
它们可以处理32位IPv4地址(`AF_INET`), 或者128位IPv6地址(`AF_INET6`)

当客户端发起一个连接请求时, 客户端套接字地址中的端口是由内核自动分配的, 称为临时端口(ephemeral port)

Web服务器通常使用端口`80`

电子邮件服务器使用端口`25`

一个连接是由它两端的套接字地址唯一确定的. 这对套接字地址叫做**套接字对(socket pair)**, 由`(cliaddr:cliport, servaddr:servport)`表示

