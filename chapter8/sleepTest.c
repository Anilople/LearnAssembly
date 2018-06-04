#include<unistd.h>
#include<stdio.h>
unsigned int snooze(unsigned int secs)
{
    int restSecs = sleep(secs);
    printf("Slept for %d of %d secs\n", restSecs, secs);
    return restSecs;
}

int main(int argc, char const *argv[])
{
    // snooze(5);
    
    while(1)
    {
        sleep(3);
        printf("hello!\n");
    }
    return 0;
}
