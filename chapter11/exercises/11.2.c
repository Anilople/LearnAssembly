// 编写程序 hex2dd.c, 将它的十六进制参数转换为
// 点分十进制串并打印出结果
// 例如 
// linux> ./hex2dd 0x8002c2f2
// 128.2.194.242

#include<arpa/inet.h>
#include<stdio.h>

int main(int argc, char **argv)
{
    if(argc != 2){
        printf("argument parameters is wrong\n");
    }
    else{
        struct in_addr add;
        sscanf(argv[1], "%x", &(add.s_addr)); 
        add.s_addr = htonl(add.s_addr); // 得到网络字节顺序的值

        static char buffer[100];
        const char *show = inet_ntop(AF_INET, &(add), buffer, 100);
        printf("%s\n", show);
    }
    return 0;
}