// 编写HOSTINFO(图11-17)的一个版本, 
// 用inet_pton而不是getnameinfo
// 将每个套接字地址转换成点分十进制地址字符串

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>

void showCharData(const char* arr)
{
    int i;
    for(i = 0; i < 14; i++)
    {
        printf("0x%x ", arr[i]);
    }
    printf("!\n");
}

void showP(const struct sockaddr *s_addr)
{
    const struct sockaddr_in *addr_in = (struct sockaddr_in *) s_addr;
    printf("family:0x%x\n", addr_in->sin_family);
    printf("port:0x%x\n", addr_in->sin_port);
    printf("addr:0x%x\n", addr_in->sin_addr.s_addr); // newwork byte order

    static char buffer[100];
    const char *show = inet_ntop(AF_INET, &(addr_in->sin_addr), buffer, 100);
    if(NULL != show){
        printf("%s\n", show);
    }
    else{
        printf("error!\n");
    }
    // int status = inet_pton(AF_INET, data, &addr);
    // if(status == 1){
    //     printf("addr:0x%x\n", addr.s_addr);
    // }
    // else{
    //     printf("meet error!\n");
    // }
}

int main(int argc, char **argv)
{
    if(argc != 2){
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        exit(0);
    }
    else{
        ;
    }

    // get a list of addrinfo records
    struct addrinfo *listp, hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // ipv4
    // hints.ai_flags = 
    hints.ai_socktype = SOCK_STREAM; // connection only
    // hints.ai_protocol = 
    int rc = getaddrinfo(argv[1], NULL, &hints, &listp);
    if(rc != 0){
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        exit(1);
    }
    else{
        ;
    }

    // walk the list and display each IP address
    int flags = NI_NUMERICHOST; // domain name -> address string
    struct addrinfo *p;
    printf("AF_INET:0x%x\n", AF_INET);
    for(p = listp; p != NULL; p = p->ai_next){
        showP(p->ai_addr);
        // static char buf[100];
        // getnameinfo(p->ai_addr, p->ai_addrlen, buf, 100, NULL, 0, flags);
        // printf("%s\n", buf);
        printf("--------------------\n");
    }
    
    // clean up
    freeaddrinfo(listp);
    return 0;
}