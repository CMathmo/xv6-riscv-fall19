#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

void getarg(int argc,char* argv[],char* args,char* son_argv[]){
    int i;
    int index = 0;
    //printf("args:%s\n",args);
    for(i = 1; i < argc; i++){
        son_argv[index++] = argv[i];
    }
    for(;args != 0;args++){
        //printf("argc1:%c ",*args);
        if(*args != ' '){
        son_argv[index++] = args;
        }
        args = strchr(args,' ');
        if(args != 0){
            //printf("argc2:%c ",*args);
            *args = 0;
        }else{
                break;
        }
        
    }
    son_argv[index++] = 0;
}

int main(int argc, char* argv[]){
    char args[MAXARG];
    char* son_argv[MAXARG];
    int pid;
    int singal;
    while(strcmp(gets(args,MAXARG),"\0") != 0){
        pid = fork();
        if(pid == -1){
            printf("Create new thread fail!\n");
            exit(-1);
        }
        if(pid == 0){
            //printf("pid:%d args:%s\n",getpid(),args);
            args[strlen(args)-1] = 0;
            getarg(argc,argv,args,son_argv);
            exec(son_argv[0],son_argv);
            exit(0);
        }else{
            wait(&singal);
        }
    }
    printf("\n");
    exit(0);
}

