#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define True 1
#define False -1
#define MAXARGS 10

enum type
{
    EXEC,
    REDIR,
    PIPE
};

char buf[100];
int fd;
char whitespace[] = " \t\r\n\v"; //空字符集合
char symbols[] = "<|>";          //功能符号集合

struct cmd *parsecmd(char *);
struct cmd *parseexec(char **, char *);
struct cmd *parseredirs(struct cmd *, char **, char *);
struct cmd *parsepipe(char **, char *);
void runcmd(struct cmd *);

struct cmd
{
    int type;
};

struct execcmd
{
    int type;
    char *argv[MAXARGS];
};

struct redircmd
{
    int type;
    struct cmd *cmd;
    char *file;
    int mode;
    int fd;
};

struct pipecmd
{
    int type;
    struct cmd *left;
    struct cmd *right;
};

//执行cmd
//最大数量为2（两元素管道两侧）
struct cmd *execcmd(void)
{
    struct execcmd *cmd;
    static struct execcmd ecmdList[2];
    static int eindex = 0;
    if (eindex >= 2)
    {
        fprintf(2, "too many exe!\n");
        exit(-1);
    }
    cmd = &ecmdList[eindex++];
    cmd->type = EXEC;
    return (struct cmd *)cmd;
}


//重定向cmd
//由于只具有输入输出重定向，最大数量为2
struct cmd *redircmd(struct cmd *subcmd, char *file, int mode, int fd)
{
    struct redircmd *cmd;
    static struct redircmd rcmdList[2];
    static int rindex = 0;
    if (rindex >= 2)
    {
        fprintf(2, "too many redir!\n");
        exit(-1);
    }
    cmd = &rcmdList[rindex++];
    cmd->type = REDIR;
    cmd->cmd = subcmd;
    cmd->file = file;
    cmd->mode = mode;
    cmd->fd = fd;
    return (struct cmd *)cmd;
}

//生成管道cmd
//由于只具有两元素管道，故最大数量为1
struct cmd *pipecmd(struct cmd *left, struct cmd *right)
{
    struct pipecmd *cmd;
    static struct pipecmd pcmd;
    static int pindex = 0;
    if (pindex >= 1)
    {
        fprintf(2, "too many pipe!\n");
        exit(-1);
    }
    cmd = &pcmd;
    pindex++;
    cmd->type = PIPE;
    cmd->left = left;
    cmd->right = right;
    return (struct cmd *)cmd;
}

//具有检测fork语句执行情况的子进程生成函数
int myfork()
{
    int pid;

    pid = fork();
    if (pid == -1)
    {
        fprintf(2, "fork-error!\n");
        exit(-1);
    }
    return pid;
}

//使ps跳过空字符
void jumpwhitespace(char **ps, char *es)
{
    while (*ps < es && strchr(whitespace, **ps))
        (*ps)++;
}

//作用：用q记录词语位置，返回该词语类型，并将ps指向下一个词语首字母
int gettoken(char **ps, char *es, char **q)
{
    char *s;
    char *eq;
    int ret;

    //跳过空字符
    jumpwhitespace(ps,es);
    s = *ps;
    //若q不为空记录词语开始位置
    *q = s;
    //ret记录词语类别
    ret = *s;
    switch (*s)
    {
    case 0:
        break;
    case '|':
    case '<':
    case '>':
        s++;
        break;
    default:
        //将非功能词语用'a'表示
        ret = 'a';
        //跳过该词语
        while (s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
            s++;
        break;
    }
    //记录词语后一位置
    eq = s;
    //更新ps
    *ps = s;
    //跳过空字符
    jumpwhitespace(ps,es);
    //添加结束符
    *eq = 0;
    //返回搜索到的词语类别
    return ret;
}

//进行语句分析识别
struct cmd *parsecmd(char *s)
{
    char *es;
    struct cmd *cmd;
    //es记录结束位置
    es = s + strlen(s);
    cmd = parsepipe(&s, es);
    //跳过空字符
    jumpwhitespace(&s,es);
    if (s != es)
    {
        //若s未到末尾，说明出现语法错误
        fprintf(2, "leftovers: %s\n", s);
        fprintf(2, "syntax");
        exit(-1);
    }
    return cmd;
}

//进行管道语句识别分析
struct cmd *parsepipe(char **ps, char *es)
{
    struct cmd *cmd;
    //识别执行语句
    cmd = parseexec(ps, es);
    //若有管道功能符，递归搜索管道语句

    if (strchr("|", **ps))
    {
        //跳过|
        (*ps)++;
        cmd = pipecmd(cmd, parsepipe(ps, es));
    }
    return cmd;
}

//进行重定向语句识别分析
struct cmd *parseredirs(struct cmd *cmd, char **ps, char *es)
{
    char tok;
    char *q;

    while (strchr("<>", **ps))
    {
        //跳过<或>，并获取重定向类型
        tok = **ps;
        (*ps)++;

        //若下一个词语不是文件名，说明出错
        if (gettoken(ps, es, &q) != 'a')
        {
            fprintf(2, "missing file for redirection\n");
            exit(-1);
        }
        //判断重定向类型，返回对应cmd
        switch (tok)
        {
        case '<'://输入重定向
            cmd = redircmd(cmd, q, O_RDONLY, 0);
            break;
        case '>'://输出重定向
            cmd = redircmd(cmd, q, O_WRONLY | O_CREATE, 1);
            break;
        }
    }
    //若没有重定向字符，则直接返回
    return cmd;
}

//进行执行语句识别分析
struct cmd *parseexec(char **ps, char *es)
{
    char *q;
    int tok, argc;
    struct execcmd *cmd;
    struct cmd *ret;

    //生成新的执行cmd
    ret = execcmd();
    cmd = (struct execcmd *)ret;

    //初始化参数数量
    argc = 0;
    //跳过空字符
    jumpwhitespace(ps,es);
    //识别分析词语，直到遇到管道功能符
    while (!strchr("|", **ps))
    {
        //获取下一个词语，若获取不到，说明语句分析完毕
        if ((tok = gettoken(ps, es, &q)) == 0)
            break;
        //若不为非功能符，说明出错
        if (tok != 'a')
        {
            fprintf(2, "syntax-error!\n");
            exit(-1);
        }
        //记录词语开始和结束位置（便于执行前添加结束符）
        cmd->argv[argc++] = q;
        if (argc >= MAXARGS)
        {
            fprintf(2, "too many args!\n");
            exit(-1);
        }
        //识别重定向语句
        ret = parseredirs(ret, ps, es);
    }
    //参数列表最后记结束符0
    cmd->argv[argc] = 0;
    return ret;
}


// 执行对应cmd
void runcmd(struct cmd *cmd)
{
    int p[2];
    struct execcmd *ecmd;
    struct pipecmd *pcmd;
    struct redircmd *rcmd;
    int result1,result2;
    //空指令
    if (cmd == 0)
        exit(-1);

    switch (cmd->type)
    {
    //执行语句
    case EXEC:
        ecmd = (struct execcmd *)cmd;
        //若命令符为空，退出
        if (ecmd->argv[0] == 0)
            exit(-1);
        //执行对应命令
        exec(ecmd->argv[0], ecmd->argv);
        fprintf(2, "exec %s failed\n", ecmd->argv[0]);
        break;
    //重定向语句
    case REDIR:
        rcmd = (struct redircmd *)cmd;
        //关闭重定向指令对应文件符
        close(rcmd->fd);
        if (open(rcmd->file, rcmd->mode) < 0)
        {
            //若打开文件失败，输出错误信息，退出
            fprintf(2, "open %s failed\n", rcmd->file);
            exit(-1);
        }
        //以对应模式成功打开文件，运行执行语句将按照对应模式进行输入输出
        runcmd(rcmd->cmd);
        break;
    //管道语句
    case PIPE:
        pcmd = (struct pipecmd *)cmd;
        if (pipe(p) < 0)
        {
            //生成管道失败，输出错误信息，退出
            fprintf(2, "pipe-error!\n");
            exit(-1);
        }
        if (myfork() == 0)
        {
            //生成左侧语句执行进程
            close(1);//关闭标准输出，文件标识符1被空出
            dup(p[1]);//将管道输出复制到1上（取最小的，即上一条语句关闭的1）
            close(p[0]);//关闭管道输入
            close(p[1]);//关闭管道输出
            //执行语句时，由于1已经对应管道输出，输出时将直接送入管道
            runcmd(pcmd->left);
        }
        if (myfork() == 0)
        {
            //生成右侧语句执行进程
            close(0);//关闭标准输入，文件标识符0被空出
            dup(p[0]);//将管道输出复制到0上（取最小的，即上一条语句关闭的0）
            close(p[0]);//关闭管道输入
            close(p[1]);//关闭管道输出
            //指向语句时，由于0已经对应管道输入，将直接从管道获取输入内容
            runcmd(pcmd->right);
        }
        //关闭管道输入输出，使子进程执行完毕后，不会因为管道而阻塞
        close(p[0]);
        close(p[1]);
        //等待两个子进程结束后结束
        wait(&result1);
        wait(&result2);
        break;
        
    default:
        fprintf(2, "runcmd-error!\n");
        exit(-1);

    }
    exit(0);
}

//输出提示符@并获取指令
int getcmd()
{
    printf("@ ");
    gets(buf, sizeof(buf));
    if (buf[0] == 0) //输入终止指令
        return False;
    return True;
}

int main()
{
    int result;
    while (getcmd() >= 0)//循环直到输入终止符
    {
        if (buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ')
        {
            //判断是否为转移路径指令，若是则执行
            buf[strlen(buf) - 1] = 0; //将换行符换成结束符
            if (chdir(buf + 3) < 0)//改变程序位置，并判断是否成功
                fprintf(2, "cannot cd %s\n", buf + 3);
            continue;
        }
        if (myfork() == 0)//在子进程运行对应指令
            runcmd(parsecmd(buf));
        wait(&result);//等待指令结束
    }
    exit(0);
}