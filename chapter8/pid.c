#include<sys/types.h>
#include<unistd.h>

int main()
{
    printf("My pid is:%d\n",getpid());
    printf("My parent's pid is:%d\n",getppid());
    
    return 0;
}