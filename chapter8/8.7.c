#include<unistd.h>
#include<stdio.h>
#include<signal.h>
void snooze(unsigned int secs)
{
    int restSecs = sleep(secs);
    printf("Slept for %d of %d secs\n",restSecs,secs);
    //return restSecs;
}

int main(int argc, char const *argv[])
{
    signal(SIGINT,snooze);
    pause();
    return 0;
}
