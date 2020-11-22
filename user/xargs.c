#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

void getarg(int argc,char* argv[],char* args,char* son_argv[]){
    int i;
    int index = 0;
    //将原参数作为加进新参数列表（argv[1]若存在，则为新命令）
    for(i = 1; i < argc; i++){
        son_argv[index++] = argv[i];
    }
    //将stdin转化为新参数加进新参数列表
    for(;args != 0;args++){
        if(*args != ' '){
        son_argv[index++] = args;
        }
        args = strchr(args,' ');
        if(args != 0){
            *args = 0;
        }else{
                break;
        }
        
    }
    //添加结束符
    son_argv[index++] = 0;
}

int main(int argc, char* argv[]){
    char args[MAXARG];
    char* son_argv[MAXARG];
    int pid;
    //当输入ctrl-d时停止
    while(strcmp(gets(args,MAXARG),"\0") != 0){
        //生成子进程
        pid = fork();
        if(pid == -1){
            printf("Create new thread failed!\n");
            exit();
        }
        if(pid == 0){
            //新参数字符串最后一个字符为'\n'，要将其消去
            args[strlen(args)-1] = 0;
            //转化参数列表
            getarg(argc,argv,args,son_argv);
            //执行新进程
            exec(son_argv[0],son_argv);
            exit();
        }else{//主进程等待子进程结束
            wait();
        }
    }
    printf("\n");
    exit();
}

