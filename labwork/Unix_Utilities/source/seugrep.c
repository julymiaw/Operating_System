#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main(int argc, char *argv[])
{
    /* 在Unix内核操作系统中， |（管道符）是一种特殊的字符，用于在命令行中连接两个或多个进程，常与grep连用
    * 您可以在 4.run 中了解到管道符的用法
    * 当您在命令行中使用 | 时，你实际上是在创建一个管道，该管道将左侧命令的输出作为右侧命令的输入
    * 比如：cat tests/4.in | ./seugrep words
    * 该命令打开了test/4.in的文件，并调用您的代码来寻找word单词出现的行
    */
    if(argc <= 1)
    {
        printf("searchterm [file ...]\n");
        exit(1);
    }

    FILE *fl = fopen(argv[2], "r");
    if(argv[2] == NULL)//如果这个参数不存在，意味着您需要从stdin标准输入流读入
        fl = stdin;
    if(fl == NULL)
    {
        printf("cannot open file\n");
        exit(1);
    }
    char *line = NULL; //输入字符串line
    size_t i = 0; //为line分配的字符数，size_t为无符号整形表示linepter的字符数
    ssize_t l; //字符串长度，ssize_t是整数

    //getline用于获取文件的一行数据
    //函数原形为：ssize_t getline(char **linepter, size_t *n, FILE *stream)
    while((l = getline(&line, &i, fl)) != -1){
        int m = strlen(argv[1]);
        //注：这里可以用kmp作字符串匹配，但是懒。。
        for(int i=0; i<l; i++)
        {
            int f = 1;
            for(int j=0; j<m; j++)
                if(line[i+j] != argv[1][j])
                {
                    f = 0;
                    break;
                }    
            if(f == 1)
            {
                //printf(line, "\n");
                //上面的写法好像也行？
                //关键在于getline的读入好像是包括\n的？所以不能加回车
                printf("%s", line);
                break;
            }
        }    
    }
    fclose(fl);
    return 0;
}
