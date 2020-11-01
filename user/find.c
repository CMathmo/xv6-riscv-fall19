#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char* fmtname(char *path){
    char *p;
    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;
    return p;
}

void find(char *dir_path, char *filename){
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    if((fd = open(dir_path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", dir_path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", dir_path);
        close(fd);
        return;
    }
    if(st.type == T_DIR){
        if(strlen(dir_path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("find: path too long\n");
        }else{
            strcpy(buf, dir_path);
            p = buf+strlen(buf);
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)){
                if(de.inum == 0)
                    continue;
                if(strcmp(de.name,"") == 0 || strcmp(de.name,".") == 0 || strcmp(de.name,"..") == 0){
                    continue;
                }
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if(strcmp(de.name,filename) == 0){
                    printf("%s\n",buf);
                }
                if(stat(buf, &st) < 0){
                    printf("find: cannot stat %s\n", buf);
                    continue;
                }
                if(st.type == 1){
                    find(buf,filename);
                }
            }
        }
    }
    close(fd);
}

int main(int argc, char *argv[]){
    if(argc == 2){
        if(strcmp(".",argv[1]) == 0){
            printf(".\n");
        }else{
            find(".",argv[1]);
        }
    }else if(argc == 3){
        if(strcmp(fmtname(argv[1]),argv[2]) == 0){
            printf("%s\n",argv[1]);
        }
        find(argv[1],argv[2]);
    }else{
        printf("The number of parameter is wrong!\n");
        exit();
    }
    exit();
}
