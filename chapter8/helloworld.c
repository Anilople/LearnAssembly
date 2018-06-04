int main()
{
    static char out[] = "hello,world\n";
    // 1 表示将输出发送到stdout
    // 第2个参数是要写的字节序列
    // 第3个参数是要写的字节数
    write(1, out, sizeof(out));
    //exit(0); // 终止进程
}