#include<stdio.h>

int stringLen(char *arr[])
{
    int len = 0;
    while(arr[len]!=NULL)
    {
        len += 1;
    }
    return len;
}

void outStringLine(char *arr[], int len)
{
    int i;
    for(i = 0;i < len;i++)
    {
        printf("%s\n",arr[i]);
    }
}

int main(int argc, char *argv[], char *envp[])
{
    printf("argc:%d\n",argc);
    outStringLine(argv,argc);
    int envpLen = stringLen(envp);
    printf("envp length:%d\n",envpLen);
    outStringLine(envp,envpLen);
    return 0;
}