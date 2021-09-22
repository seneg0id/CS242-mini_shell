// C Program to design a shell in Linux
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <signal.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAXL 1000 // max number of letters to be supported
#define MAXC 100 // max number of commands to be supported

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

// Private Function declarations
void sig_handler(int signo);
void init_shell();
int takeInput(char* str);
void printD();
void execArgs(char** parsed);
void execArgsPiped(char** parsed, char** parsedpipe);
void help();
int ownCmdHandler(char** parsed);
int parsePipe(char* str, char** strpiped);
void parseSpace(char* str, char** parsed);
int processString(char* str, char** parsed, char** parsedpipe);

int main()
{
    signal(SIGINT,sig_handler);

    char inputString[MAXL], *parsedArgs[MAXC];
    char* parsedArgsPiped[MAXC];
    int execFlag = 0;
    init_shell();

    while (1) {
        // print shell line
        printD();
        // take input
        if (takeInput(inputString))
            continue;
        // process
        execFlag = processString(inputString,
        parsedArgs, parsedArgsPiped);
        // execflag returns zero if there is no command
        // or it is a builtin command,
        // 1 if it is a simple command
        // 2 if it is including a pipe.

        // execute
        if (execFlag == 1)
            execArgs(parsedArgs);

        if (execFlag == 2)
            execArgsPiped(parsedArgs, parsedArgsPiped);
    }
    return 0;
}

int processString(char* str, char** parsed, char** parsedpipe){

    char* strpiped[2];
    int piped = 0;

    piped = parsePipe(str, strpiped);

    if (piped) {
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedpipe);

    } else {

        parseSpace(str, parsed);
    }

    if (ownCmdHandler(parsed))
        return 0;
    else
        return 1 + piped;
}

// function for parsing command words
void parseSpace(char* str, char** parsed){
    int i;

    for (i = 0; i < MAXC; i++) {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

// function for finding pipe
int parsePipe(char* str, char** strpiped)
{
    int i;
    for (i = 0; i < 2; i++) {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }

    if (strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return 1;
    }
}

// Function to execute builtin commands
int ownCmdHandler(char** parsed)
{
    int NoOfOwnCmds = 3, i, switchOwnArg = 0;
    char* OwnCmds[NoOfOwnCmds];
    char* username;

    OwnCmds[0] = "exit";
    OwnCmds[1] = "cd";
    OwnCmds[2] = "help";

    for (i = 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], OwnCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }

    switch (switchOwnArg) {
    case 1:
        exit(0);
    case 2:
        chdir(parsed[1]);
        return 1;
    case 3:
        help();
        return 1;
    default:
        break;
    }

    return 0;
}

// Help command builtin
void help()
{
    puts("\nList of Commands supported:"
        "\n>'man', 'which', 'chsh', 'whereis', 'passwd', 'date', 'cal', 'clear', 'sleep',"
        "'apropos', 'exit', 'shutdown', 'ls', 'cat', 'more', 'less', 'touch', 'cp',"
        "'mv', 'rm', 'script', 'find', 'mkdir', 'cd', 'pwd', 'rmdir', 'chmod', 'grep'\n"
        "\n>pipe handling"
        "\n>improper space handling\n"
        "\n>List of Commands not supported"
        "\n> 'alias', 'unalias', 'history', 'logout'");

    return;
}

// Function where the piped system commands is executed
void execArgsPiped(char** parsed, char** parsedpipe)
{
    // 0 is read end, 1 is write end
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }

    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0) {
            printf("\ncommand 1 not found");
            exit(0);
        }
    } else {
        // Parent executing
        p2 = fork();

        if (p2 < 0) {
            printf("\nCould not fork");
            return;
        }

        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\ncommand 2 not found");
                exit(0);
            }
        } else {
            // parent executing, waiting for two children
            wait(NULL);
            wait(NULL);
        }
    }
}

// Function where the system command is executed
void execArgs(char** parsed)
{
    // Forking a child
    pid_t pid = fork();

    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\ncommand not found");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        wait(NULL);
        return;
    }
}

// Function to print Current Directory.
void printD()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\nDir: %s", cwd);
}

// Function to take input
int takeInput(char* str)
{
    char* buf;

    buf = readline("\n>>> ");
    if (strlen(buf) != 0) {
        add_history(buf);
        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}

// Greeting shell during startup
void init_shell()
{
    clear();
    printf("USER is: @%s\n", getenv("USER"));
    printf("HOME is: @%s\n", getenv("HOME"));
    printf("SHELL is: @%s\n", getenv("SHELL"));
    printf("TERM is: @%s\n", getenv("TERM"));
    sleep(1);
}

// to handle SIGINT
void sig_handler(int signo)
{
    signal(SIGINT,sig_handler);
    int i;
    sigset_t curr_mask;
    sigfillset(&curr_mask);
    sigprocmask(SIG_SETMASK,&curr_mask,NULL);
}