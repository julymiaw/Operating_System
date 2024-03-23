#include<stdio.h>
#include<stdlib.h>
int main(int argc, char *argv[])
{
    int i = 1;
    //while循环是为了应对多文件输入的情况
    while(i < argc) 
    {
        char buffer[10000];
        //打开文件
        FILE *fp = fopen(argv[i], "r");
        if(fp == NULL)
        {
            printf("cannot open file\n");
            exit(1);
        }
        //读入每一行并输出
        //还可以使用geiline，详见seugrep.c
        while (fgets(buffer, sizeof(buffer), fp) != NULL)
            printf("%s",buffer);
        fclose(fp);
        i++;
    }
    return 0;
}
