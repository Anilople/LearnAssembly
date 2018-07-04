// 编写程序 dd2hex.c, 将它的点分十进制参数转换为
// 十六进制串并打印出结果
// 例如 
// linux> ./dd2hex 128.2.194.242
// 0x8002c2f2

#include<arpa/inet.h>
#include<stdio.h>

int main(int argc, char **argv)
{
    if(argc != 2){
        printf("argument parameters is wrong\n");
    }
    else{
        struct in_addr addr;
        int status = inet_pton(AF_INET, argv[1], &addr);
        if(status == 1){ // bingo
            printf("0x%x\n", ntohl(addr.s_addr));
        }
        else if(status == 0){
            printf("%s is a illegal address.\n", argv[1]);
        }
        else{
            printf("error\n");
        }
    }
    return 0;
}