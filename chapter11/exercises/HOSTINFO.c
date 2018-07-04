#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>

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
    for(p = listp; p != NULL; p = p->ai_next){
        static char buf[100];
        getnameinfo(p->ai_addr, p->ai_addrlen, buf, 100, NULL, 0, flags);
        printf("%s\n", buf);
    }
    
    // clean up
    freeaddrinfo(listp);
    return 0;
}