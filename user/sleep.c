#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char* argv[]){
    int seconds = atoi(argv[1]);
    if(argc == 2){
        printf("It will sleep %ds.\n",seconds);
        sleep(seconds);
    }else{
        printf("The number of parameter is wrong!\n");
    }
    exit();
}

