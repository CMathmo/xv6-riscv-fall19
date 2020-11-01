#include "kernel/types.h"
#include "user/user.h"


int main(int argc, char* argv[]){
    int prime = -1;
    int i;
    int fd[50][2];
    int buf;
    int index = 0;
    int pid;
    int flag = -1;
    int singal;
    if(pipe(fd[index])<0){
        printf("ERROR!");
        exit(-1);
    }
    pid = fork();
    if(pid == -1){
        printf("ERROR!");
        exit(-1);
    }
    if(pid != 0){
        close(fd[index][0]);
        for(i=2;i<=35;i++){
            write(fd[index][1],&i,sizeof(i));
        }
        close(fd[index][1]);
        wait(&singal);
    }else{
        close(fd[index][1]);
        while(read(fd[index][0],&buf,sizeof(buf)) == sizeof(buf)){
            if(prime == -1){
                printf("prime %d\n",buf);
                prime = buf;
            }else if(buf % prime != 0 && flag == -1){
                if(pipe(fd[index + 1])<0){
                    printf("ERROR!");
                    exit(-1);
                }
                pid = fork();
                if(pid == -1){
                    printf("ERROR!");
                    exit(-1);
                }
                if(pid == 0){
                    index ++;
                    close(fd[index][1]);
                    prime = -1;
                }else{   
                    flag = 1;
                    close(fd[index + 1][0]);
                    write(fd[index + 1][1],&buf,sizeof(buf));
                }
            }else if(buf % prime != 0){
                write(fd[index + 1][1],&buf,sizeof(buf));
            }  
        }
        close(fd[index][0]); 
        close(fd[index + 1][1]); 
        wait(&singal);          
    }
    exit(0);
}

