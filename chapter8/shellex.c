// eval - Evaluate a command line
void eval(char *cmdline)
{
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;
    strcpy(buf,cmdline);
    bg = parseline(buf,argv);
    if (argv[0] == NULL){
        return;
    }

    if(!builtin_command(argv))
    {
        if((pid = fork()) == 0)
        {
            
        }
    }
}