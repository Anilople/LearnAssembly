#include<signal.h>
#include<sys/types.h>
// 信号处理程序
void mineCtrlc(int ha)
{
    printf("hello:%d\n",ha);
    // pause();
    exit(0);
}

// 显示阻塞SIGINT信号
void stopCtrlc(void)
{
    sigset_t mask,prev_mask;
    sigemptyset(&mask); // 初始化为空集合
    sigaddset(&mask,SIGINT); // 增加SIGINT信号
    sigprocmask(SIG_SETMASK, &mask, 0); // block这个信号
}
int main()
{
    // 
    // signal(SIGINT,mineCtrlc);
    stopCtrlc();
    pause();
    return 0;
}