#include "kernel/types.h"
#include "user/user.h"


int main(int argc, char* argv[]){
    int fd1[2],fd2[2];
    char ping[]="ping";
    char pong[]="pong";
    char buf_f[1024];
    char buf_s[1024];
    int pid;
    pipe(fd1);
    pipe(fd2);
    if((pid=fork())==0){//子进程
        close(fd1[1]);
        close(fd2[0]);
        write(fd2[1],pong,strlen(pong));
        read(fd1[0],buf_s,sizeof(buf_s));
        printf("%d: received %s\n",getpid(),buf_s);
        exit();

    }else{//父进程
        close(fd2[1]);
        close(fd1[0]);
        write(fd1[1],ping,strlen(ping));
        read(fd2[0],buf_f,sizeof(buf_f));
        wait();
        printf("%d: received %s\n",getpid(),buf_f);
    }
    exit();
}

