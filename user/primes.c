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
    if(pipe(fd[index]) < 0){
        printf("ERROR!");
        exit();
    }
    pid = fork();
    if(pid == -1){
        printf("ERROR!");
        exit();
    }
    if(pid != 0){//主进程向子进程传输2~32
        close(fd[index][0]);
        for(i=2;i<=35;i++){
            write(fd[index][1],&i,sizeof(i));
        }
        //及时关闭通道，保证子进程可以结束
        close(fd[index][1]);
        //等待子进程结束
        wait();
    }else{
        close(fd[index][1]);
        //由于父进程传输完数据后会关闭通道，
        //当不能再读出数据时说明数据已传输完成，
        //可以跳出循环
        while(read(fd[index][0],&buf,sizeof(buf)) == sizeof(buf)){
            if(prime == -1){
                //输出第一个接受的数
                printf("prime %d\n",buf);
                prime = buf;
            }else if(buf % prime != 0 && flag == -1){
                //若存在第二个接受的数，
                //生成子进程，并向其传递
                if(pipe(fd[index + 1])<0){
                    printf("ERROR!");
                    exit();
                }
                pid = fork();
                if(pid == -1){
                    printf("ERROR!");
                    exit();
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
                //向已生成的子进程传递
                write(fd[index + 1][1],&buf,sizeof(buf));
            }  
        }
        //关闭通道并等待子进程结束
        close(fd[index][0]); 
        close(fd[index + 1][1]); 
        wait();          
    }
    exit();
}

