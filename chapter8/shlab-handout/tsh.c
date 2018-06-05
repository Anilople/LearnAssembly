/* 
 * tsh - A tiny shell program with job control
 * 
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bg(char **number);
void do_fg(char **numbers);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	        break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	        break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	        break;
	    default:
            usage();
	    }
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {
        /* Read command line */
        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin)){
            app_error("fgets error");
        }
        if (feof(stdin)) { /* End of file (ctrl-d) */
            // printf("you press Ctrl + d\n");
            fflush(stdout);
            exit(0);
        }

        /* Evaluate the command line */
        eval(cmdline);
        // printf("eval end.\n");
        fflush(stdout);
        fflush(stdout);
    }

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{
    static char *argv[MAXARGS];
    int i;
    for(i = 0;i<MAXARGS;i++) // clear argv
    {
        argv[i] = NULL;
    }

    static char buf[MAXLINE];
    strcpy(buf,cmdline); // copy cmdline to buffer

    int bg = parseline(buf,argv);
    // print informations in argv
    int argc = 0; 
    while(argv[argc]!=NULL)
    {
        // print the cmd parameters
        // printf("%d:%s\n", argc, argv[argc]);
        argc += 1;
    }
    if(argc < 1){ // there no command
        // printf("there are no cmd to execute\n");
    }
    else{ // something need to do
        int isbuiltin = builtin_cmd(argv);
        if(isbuiltin){
            // printf("It's a builtin cmd\n");
        }
        else{ // not a build in cmd
            // block SIGCHLD
            sigset_t chld;
            sigemptyset(&chld);
            sigaddset(&chld, SIGCHLD);
            sigprocmask(SIG_BLOCK, &chld, NULL);
            // end block SIGCHLD

            pid_t childPid = fork(); // lack check here!!!!
            if(childPid == 0){ // use child to run it
                setpgid(0, 0); // for terminate child with "ctrl + c" in fg
                sigprocmask(SIG_UNBLOCK, &chld, NULL);
                int exeStatus = execve(argv[0], argv, environ);
                if(exeStatus < 0){ // execute failed
                    printf("%s: Command not found.\n", argv[0]);
                    exit(0);
                }
            }
            else{ // in parent process now
            
                if(bg){ // run in background
                    addjob(jobs, childPid, BG, cmdline);
                    printf("[%d] (%d) %s", nextjid, childPid, cmdline);
                    // unblock SIGCHLD after addjob
                    sigprocmask(SIG_UNBLOCK, &chld, NULL);
                    // printf("run %d  %s in background\n", childPid, cmdline);
                }
                else{ // run in foreground
                    addjob(jobs, childPid, FG, cmdline);
                    // unblock SIGCHLD after addjob
                    sigprocmask(SIG_UNBLOCK, &chld, NULL);
                    // printf("foreground\n");
                    waitfg(childPid);
                }
            }
        }
    }
    return;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	{
        buf++;
    }

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
        buf++;
        delim = strchr(buf, '\'');
    }
    else {
	    delim = strchr(buf, ' ');
    }

    while (delim) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* ignore spaces */
            buf++;

        if (*buf == '\'') {
            buf++;
            delim = strchr(buf, '\'');
        }
        else {
            delim = strchr(buf, ' ');
        }
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	    return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	    argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) 
{
    // just check the first string
    char *cmd = argv[0];
    if(strcmp(cmd,"quit") == 0){
        // printf("quit, pid is:%d\n", getpid());
        exit(0);
    }
    else if(strcmp(cmd, "jobs") == 0){
        // printf("jobs\n");
        listjobs(jobs);
        return 1;
    }
    else if(strcmp(cmd, "bg") == 0){
        do_bg(&argv[1]);
        // printf("bg\n");
        return 1;
    }
    else if(strcmp(cmd, "fg") == 0){
        do_fg(&argv[1]);
        // printf("fg\n");
        return 1;
    }
    else{
        return 0;  /* not a builtin command */
    }
    return 0;     /* not a builtin command */
}

inline void printJob(struct job_t *job)
{
    printf("[%d] (%d) ", job->jid, job->pid);
    switch (job->state) {
    case BG:
        printf("Running ");
        break;
    case FG:
        printf("Foreground ");
        break;
    case ST:
        printf("Stopped ");
        break;
    default:
        printf("listjobs: Internal error: job.state=%d ",
        job->state);
    }
    printf("%s", job->cmdline);
}

// judge string is or not a number
int isNumber(char *numStr)
{
    while(*numStr != '\0')
    {
        if('0' <= *numStr && *numStr <= '9'){
            ;
        }
        else{
            return 0;
        }
        numStr++;
    }
    return 1;
}

inline int isPID(char *numStr)
{
    return isNumber(numStr);
}

int isJID(char *numStr)
{
    return numStr[0] == '%' && isNumber(numStr+1);
}

inline void bg_job(struct job_t *job_now)
{
    job_now->state = BG;
    // add " &" to job's cmdline
    // int cmdLen = strlen(job_now->cmdline);
    // strcpy(&(job_now->cmdline[cmdLen-1]), " &\n");
    // job_now->cmdline[cmdLen] = ' ';
    // job_now->cmdline[cmdLen+1] = '&';
    // job_now->cmdline[cmdLen+2] = '\0';
    // printf("\n");
    // send continue signal to child's group
    kill(-job_now->pid, SIGCONT);
}

void do_bg1(char *numStr)
{
    if(isPID(numStr)){
        int pid = atoi(numStr);
        struct job_t *job_now = getjobpid(jobs, pid);
        if(job_now == NULL){
            printf("(%d): No such process\n", pid);
        }
        else{
            printf("[%d] (%d) %s", 
                    job_now->jid, 
                    job_now->pid, 
                    job_now->cmdline);
            bg_job(job_now);
        }
    }
    else if(isJID(numStr)){
        int jid = atoi(numStr+1);
        struct job_t *job_now = getjobjid(jobs, jid);
        if(job_now == NULL){
            printf("%%%d: No such job\n", jid);
        }
        else{
            printf("[%d] (%d) %s", 
                    job_now->jid, 
                    job_now->pid, 
                    job_now->cmdline);
            bg_job(job_now);
        }
    }
    else{
        printf("bg: argument must be a PID or %%jobid\n");
    }
}

// save pid or jid in number
void do_bg(char **numbers)
{
    if(numbers[0] == NULL){
        printf("bg command requires PID or %%jobid argument\n");
    }
    else{
        do_bg1(numbers[0]);
    }
}

inline void fg_job(struct job_t * job_now)
{
    job_now->state = FG;
    kill(-job_now->pid, SIGCONT);
}

inline void do_fg1(char *numStr)
{
    if(isPID(numStr)){
        int pid = atoi(numStr);
        struct job_t *job_now = getjobpid(jobs, pid);
        if(job_now == NULL){
            printf("(%d): No such process\n", pid);
        }
        else{
            printf("[%d] (%d) %s", 
                    job_now->jid, 
                    job_now->pid, 
                    job_now->cmdline);
            fg_job(job_now);
        }
    }
    else if(isJID(numStr)){
        int jid = atoi(numStr+1);
        struct job_t *job_now = getjobjid(jobs, jid);
        if(job_now == NULL){
            printf("%%%d: No such job\n", jid);
        }
        else{
            printf("[%d] (%d) %s", 
                    job_now->jid, 
                    job_now->pid, 
                    job_now->cmdline);
            fg_job(job_now);
        }
    }
    else{
        printf("fg: argument must be a PID or %%jobid\n");
    }
}

// save pid or jid in number
void do_fg(char **numbers)
{
    if(numbers[0] == NULL){
        printf("fg command requires PID or %%jobid argument\n");
    }
    else{
        do_fg1(numbers[0]);
    }
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) 
{
    return;
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
    while(pid == fgpid(jobs)) // wait until fg process is change
    {
        continue;
    }
    // printf("wait fg end!pid:%d, jid:%d\n", pid, pid2jid(pid));
    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    // int olderrno = errno;
    sigset_t mask_all;
    sigfillset(&mask_all); // lack check
    // printf("%d get SIGCHLD\n", getpid());
    int status;
    int endPid = waitpid(-1, &status, WNOHANG | WUNTRACED); // wait here
    // if(endPid < 0 ){ // wait child
    //     unix_error("waitfg: waitpid error.\n");
    // }
    while(endPid > 0)
    {
        sigprocmask(SIG_BLOCK, &mask_all, NULL);

        if(WIFSTOPPED(status)){ // stop signal
            struct job_t *pjob = getjobpid(jobs, endPid);
            pjob->state = ST;
            printf("Job [%d] (%d) stopped by signal 20\n", pjob->jid, endPid);
        }
        else if(WIFSIGNALED(status)){ // quit signal
            struct job_t *pjob = getjobpid(jobs, endPid);
            printf("Job [%d] (%d) terminated by signal %d\n", pjob->jid, endPid, sig);
            deletejob(jobs, endPid);
        }
        else{
            deletejob(jobs, endPid);
        }

        sigprocmask(SIG_UNBLOCK, &mask_all, NULL);

        endPid = waitpid(-1, &status, WNOHANG | WUNTRACED); // update endPid
    }
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    pid_t fg = fgpid(jobs);
    if(fg > 0){ // there is a foreground job
        // int jobid = pid2jid(fg);
        // printf("Job [%d] (%d) terminated by signal %d\n", jobid, fg, sig);
        kill(fg, SIGQUIT); // SIGQUIT to fg process
        // printf("send SIGQUIT to pid:%d\n", fg);
    }
    else{
        // listjobs(jobs);
        // printf("there is no fg job to be killed.\n\\tsh>");
    }
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig)
{
    // find which process id run in foreground
    // watch out that job added after fork child
    // so you cannot find a fgpid in child process
    // in child process, you just get fgpid = 0
    pid_t fgPid = fgpid(jobs); 
    // if(fgPid == 0){
    //     printf("%d is stop now.\n", getpid());
    //     kill(getpid(), SIGSTOP);
    // }
    // else 
    if(fgPid > 0){ // parent send SIGTSTP to child
        // printf("send SIGTSTP to pid:%d\n", fgPid);
        struct job_t *fgjob = getjobpid(jobs,fgPid);
        fgjob->state = ST; // change fg to stop
        // printf("Job [%d] (%d) stopped by signal 20\n", fgjob->jid, fgPid);
        kill(fgPid, SIGTSTP);
        // listjobs(jobs);
    }
    else{
        // printf("there is no fg job\n");
    }
    return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) 
{
    int i;

    for (i = 0; i < MAXJOBS; i++)
    {
	    clearjob(&jobs[i]);
    }
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	    return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].jid = nextjid++;
            if (nextjid > MAXJOBS)
            nextjid = 1;
            strcpy(jobs[i].cmdline, cmdline);
            if(verbose){
                printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
                }
                return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == pid) {
            clearjob(&jobs[i]);
            nextjid = maxjid(jobs)+1;
            return 1;
        }
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	    return 0;
    for (i = 0; i < MAXJOBS; i++)
    {
        if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid != 0) {
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
            switch (jobs[i].state) {
            case BG: 
                printf("Running ");
                break;
            case FG: 
                printf("Foreground ");
                break;
            case ST: 
                printf("Stopped ");
                break;
            default:
                printf("listjobs: Internal error: job[%d].state=%d ", 
                i, jobs[i].state);
            }
            printf("%s", jobs[i].cmdline);
        }
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    printf("pid:%d jobid:%d\n", getpid(), pid2jid(getpid()));
    exit(1);
}



