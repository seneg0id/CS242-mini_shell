#include<signal.h>
#include<stdio.h>
#include<sys/types.h>
#include<pwd.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<string.h>
#include<sys/mman.h>
#include<errno.h>

#define INPUT_SIZE 1024 //The length of the maximum string for the user
#define clear() printf("\033[H\033[J") // Clearing the shell using escape sequences
#define DIVIDE_AT " \n"//For dividing a string into single words (using in strtok)
#define ENDING_WORD "exit"//Program end word
#define RESET 0

// Private Function declaration
void help(); // Help command builtin
void sig_handler(int signo);
char *getcwd(char *buf, size_t size);//show the path Of the current folder
void DisplayPrompt();//Display Prompt : user@currect dir>
void releaseMemory(char** argv);
int checkSymbol(char* symbol,char* input);
void garbageCollector(char** argv,int size); //Memory Release
char** execFunction(char *input,char **argv,int *sizeOfArray,int *cmdLength);  //Preparation of a receiver input as an expense
void ArrayOfSymbol(char* symbol,char* input,int argc,char** argv,int sizeOfArray);
void LeftRightPipe(char* symbol,char** command1,char** command2,char** argv);

static int *numOfCmd;
static int *cmdLength;


int main() {

    signal(SIGINT,sig_handler);
    
    numOfCmd = mmap(NULL, sizeof *numOfCmd, PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, RESET);
    cmdLength = mmap(NULL, sizeof *cmdLength, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, RESET);
    
    (*numOfCmd)=RESET;
    (*cmdLength)=RESET;
    int sizeOfArray=RESET;
    
    char input[INPUT_SIZE]="";//A string array containing the input.
    clear();
    printf("USER is: @%s\n", getenv("USER"));
    printf("PATH is: @%s\n", getenv("PATH"));
    printf("HOME is: @%s\n", getenv("HOME"));
    printf("SHELL is: @%s\n", getenv("SHELL"));
    printf("TERM is: @%s\n", getenv("TERM"));
    DisplayPrompt();
    pid_t id; // pid_t used for process identifer
    char **argv;//A string array containing the program name and command arguments


    while (strcmp(input,ENDING_WORD)!=RESET)
    {
        if(fgets(input,INPUT_SIZE,stdin)==RESET)//get the input from the user
            printf(" ");
       
        char* inputCopy=strdup(input);

        if (strcmp(input,"\n")==RESET)
        {goto lable;}//if the user clicks only enter
       
        else{
            argv=execFunction(input,argv,&sizeOfArray,cmdLength);//split the commands
           
            int place=checkSymbol("|",inputCopy);
            if(place!=-1) {

                ArrayOfSymbol("|",inputCopy,place,argv,sizeOfArray);
                garbageCollector(argv,sizeOfArray);
                free(inputCopy);
            }
            
            else if (strcmp("cd",argv[RESET])==RESET)//if the user tries cd command
            {

                struct passwd *pwd;
                char* path=argv[1];

                if(path==NULL)
                {
                    pwd=getpwuid(getuid());
                    path=pwd->pw_dir;
                }
                if(path[0]=='/')
                    (path)=++(path);
                errno=chdir(path);
                DisplayPrompt();
                if(errno!=RESET)
                    printf("error changing dircatory");
                garbageCollector(argv,sizeOfArray);
            }
            else if (strcmp("help",argv[RESET])==RESET)//if the user tries help command
            {
                help();
                DisplayPrompt();
            } 
            else if (strcmp("setenv", argv[RESET])==RESET)//if the user tries setenv command
            {
                if (sizeOfArray!=4) {
                    printf("ERROR: Invalid format or characters found\n"
                    "setting environment failed, make sure there are " 
                    "no spaces, symbols or newline characters in the environment value "
                    "\nuse format  setenv varname = value (spaces are necessary)\n"
                    );
                } 
                else if (sizeOfArray==4 && strcmp(argv[2], "=")!=0){
                    printf("ERROR: Invalid format or characters found\n"
                    "setting environment failed, make sure there are " 
                    "no spaces, symbols or newline characters in the environment value "
                    "\nuse format  setenv varname = value (spaces are necessary)\n"
                    );
                }
                else {
                    int setSuccessful = setenv(argv[RESET+1], argv[RESET+3], 1);
                    if (!setSuccessful) {
                        printf("environment variable%s is set to %s\n "
                        "use printenv <varname> to see env variable value\n", argv[RESET+1], argv[RESET+3]);
                    } 
                    else {
                        printf("ERROR: Invalid format or characters found\n"
                        "setting environment failed, make sure there are" 
                        "no spaces, symbols or newline characters in the environment value");
                    }
                }
                DisplayPrompt();
           }
            
            else
            {
                id=fork();//make new prosses
                if (id<RESET)
                {
                    printf("fork failed");
                    exit(RESET);
                }
                else if(id==RESET) {
                    (*numOfCmd)++;

                    execvp(argv[RESET],argv);
                    if(strcmp(input,ENDING_WORD)!=RESET)
                        printf("command not found\n");
                    exit(1);
                }else {
                    wait(&id);
                    free(inputCopy);
                    garbageCollector(argv,sizeOfArray);
                    lable:
                    if (strcmp(input,ENDING_WORD) != RESET)
                    {
                        DisplayPrompt();
                    }                else {

                        printf("Num of cmd: %d\n", *numOfCmd-1);
                        printf("cmd length: %d\n", *cmdLength-4);
                    }
                }

            }
        }
    }
    return RESET;
}


void help()
{
    puts("\nList of Commands supported:"
        "\n>'man', 'which', 'chsh', 'whereis', 'passwd', 'date', 'cal', 'clear', 'sleep',"
        "'apropos', 'exit', 'shutdown', 'ls', 'cat', 'more', 'less', 'touch', 'cp',"
        "'mv', 'rm', 'script', 'find', 'mkdir', 'cd', 'pwd', 'rmdir', 'chmod', 'grep'\n"
        "\n>pipe handling"
        "\n>improper space handling\n"
        "\n>List of Commands not supported"
        "\n> 'alias', 'unalias', 'history', 'logout'\n"
        "\n>redirection not supported"
        "\n>tab completion not supported");

    return;
}

void sig_handler(int signo)
{
    signal(SIGINT,sig_handler);
    int i;
    sigset_t curr_mask;
    sigfillset(&curr_mask);
    sigprocmask(SIG_SETMASK,&curr_mask,NULL);
}

void releaseMemory(char** argv)
{
    for (int i = 0; argv[i]!=NULL ; i++) {
        free(argv[i]);
    }
    free(argv);
}

void ArrayOfSymbol(char* symbol,char* inputCopy,int place,char** argv,int sizeOfArray)
{

    char** command1;char** command2;int size1=0,size2=0,j;
    char* before=strndup(inputCopy,place);
    char* after=strdup(inputCopy+strlen(before)+1);
    
    //copy left side command
    for (int i = 0; i < sizeOfArray; i++) {
        if (strcmp(argv[i], "|") != 0) {
            size1++;
        }else
        break;
    }


    command1=(char**)malloc(sizeof(char*)*size1+1);


    for ( j = 0; j < size1; j++) {

        command1[j]=(char*)malloc(sizeof(char)*(strlen(argv[j])));
        strncpy(command1[j],argv[j],strlen(argv[j]));
    }
    command1[j]=NULL;

    //copy right side command
    for (int i = size1+1; i < sizeOfArray; i++) {
        if (argv[i]!= NULL) {
            size2++;
        }else
            break;
    }


    command2=(char**)malloc(sizeof(char*)*size2+1);


    for ( j = 0; j < size2; j++) {

        command2[j]=(char*)malloc(sizeof(char)*(strlen(argv[j])));
        strncpy(command2[j],argv[j+size1+1],strlen(argv[j+size1+1])+1);
    }
    command2[j]=NULL;

    free(after);
    free(before);
    LeftRightPipe(symbol, command1, command2,argv);
      releaseMemory(command2);
      releaseMemory(command1);
    return;
}

void LeftRightPipe(char* symbol,char** command1,char** command2,char** argv)
{

    int status=0;
    if(strcmp(symbol,"|")==0) {

        pid_t leftPid,rightPid;
        int pipe_descs[2];
        if (pipe(pipe_descs) == -1)//first child - which writes to the pipe
        {
            perror("cannot open pipe");
            exit(EXIT_FAILURE);
        }
        if(leftPid=fork()==0)//if its pipe
        {
            dup2(pipe_descs[1],STDOUT_FILENO);//change the default output (channel 1) to pipe[1]
            execvp(command1[0],command1);
            close(pipe_descs[0]);//close the read output
            close(pipe_descs[1]);
           // releaseMemory(command1);
        }else if (rightPid = fork() == 0)//if we are in the child number 2
            {

                dup2(pipe_descs[0], STDIN_FILENO);//change the default input 0 to pipe[0]
                close(pipe_descs[0]);
                close(pipe_descs[1]);
                execvp(command2[0], command2);
              //  releaseMemory(command2);
            }
            else
        {

            close(pipe_descs[0]);
            close(pipe_descs[1]);

            waitpid(leftPid, &status, 0);
            if (!WIFEXITED(status))//if the creation of the command failed
            {
                fprintf(stderr, "child failed");
                exit(1);
            }
                waitpid(rightPid,&status,0);

                if(!WIFEXITED(status))//if the creation of the command failed
                {
                    fprintf(stderr,"child failed");
                    exit(1);
                }
        }

    return;
    }
}

int checkSymbol(char* symbol,char* input)
{
    int potision;
    char* isThePlace=strstr(input,symbol);
        if(isThePlace!=NULL) {
            potision = ((isThePlace) -input );
            return potision;
    }
    return -1;
}

//release the memory from the argv
void garbageCollector(char** argv,int size)
{
    int i=RESET;
    for (i = RESET; i < size; ++i) {
        free(argv[i]);
    }
    free(argv);
    argv=NULL;
}

//split the input from the user to sub strings
char** execFunction(char *input,char **argv,int *sizeOfArray,int *cmdLength)
{
    int i=RESET,counter=RESET;
    char inputCopy[INPUT_SIZE];
    strcpy(inputCopy,input);

    char* ptr= strtok(input,DIVIDE_AT);
    while(ptr!=NULL)
    {
        ptr=strtok(NULL,DIVIDE_AT);
        counter++;
    }
    argv = (char**)malloc((counter+1)*(sizeof(char*)));
    if(argv==NULL)
    {
        printf("error allocated");
        exit(RESET);
    }

    char* ptrCopy= strtok(inputCopy,DIVIDE_AT);
    while(ptrCopy!=NULL)
    {
        if (i==RESET && (strcmp(ptrCopy,"cd")!=0))
            (*cmdLength)+=strlen(ptrCopy);
        argv[i]=(char*)malloc((sizeof(char)+1)*strlen(ptrCopy));
        if(argv[i]==NULL)
        {
            printf("error allocated");
            for (int j = i-1; j >-1 ; j--) {
                free(argv[j]);
            }
            free(argv);
            exit(RESET);
        }
        strcpy(argv[i],ptrCopy);
        argv[i][strlen(ptrCopy)]='\0';
        ptrCopy=strtok(NULL,DIVIDE_AT );
        i++;
    }
    argv[counter]=NULL;
    (*sizeOfArray)=counter;
    return argv;

}

//show the prompt on the screen
void DisplayPrompt()
{

// show the path

    long size;
    char *buf;
    char *ptr;

    size = pathconf(".", _PC_PATH_MAX);

    if ((buf = (char *)malloc((size_t)size)) != NULL)
        ptr = getcwd(buf, (size_t)size);


    //show the user name root

    struct passwd *getpwuid(uid_t uid);
    struct passwd *p;
    uid_t uid=0;
    if ((p = getpwuid(uid)) == NULL)
        perror("getpwuid() error");
    else {
        printf("%s@%s>", p->pw_name, ptr);
    }
    free(buf);
}