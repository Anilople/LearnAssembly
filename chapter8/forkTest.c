#include<sys/types.h>
#include<unistd.h>

int main()
{
    int x = 1;
    printf("shit\n");
    pid_t pid = fork();
    printf("my child's is:%d\n",pid);
    // if(pid == 0)
    // {
    //     x += 1;
    //     printf("I'm child : x = %d\n", x);
    //     // exit(0); // 这段使得程序退出了
    // }
    // else
    // {
    //     printf("I'm not child : x = %d\n", x);
    // }

    // parent here
    pid_t shitPid = fork();
    // printf("shit pid:%d\n",shitPid);
    x -= 1;
    printf("parent: x = %d \n", x);
    exit(0);
}