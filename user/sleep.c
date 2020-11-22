#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char* argv[]){
    int seconds = atoi(argv[1]);
    if(argc == 2){//参数正确
        sleep(seconds);
    }else{//参数错误
        printf("The number of parameter is wrong!\n");
    }
    exit();
}

