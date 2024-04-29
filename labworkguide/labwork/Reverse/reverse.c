#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    // 输入流，文件或命令行输入(Ctrl+D结束输入)
    FILE *input = stdin;

    // 输出流，文件或命令行输出
    FILE *output = stdout;

    // 记录行数
    int num_lines = 0;

    // 记录容量
    int capacity = 10;

    // 用于存储行的数组
    char **lines = malloc(capacity * sizeof(char *));
    if (lines == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    // 如果用户运行时reverse参数过多，则打印usage: reverse <input> <output>并退出，返回码为 1
    if (argc > 3)
    {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    // 如果提供输入文件，打开输入文件
    if (argc >= 2)
    {
        input = fopen(argv[1], "r");
        if (input == NULL)
        {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
    }

    // 如果额外提供输出文件，尝试打开，并检查输入输出文件是否相同(用stat防止硬链接)
    if (argc == 3)
    {
        output = fopen(argv[2], "w");
        if (output == NULL)
        {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[2]);
            exit(1);
        }
        struct stat stat1, stat2;
        stat(argv[1], &stat1);
        stat(argv[2], &stat2);
        if (stat1.st_ino == stat2.st_ino)
        {
            fprintf(stderr, "reverse: input and output file must differ\n");
            exit(1);
        }
    }

    // Read lines from the input file into the array
    size_t len = 0;
    while (1)
    {
        if (num_lines == capacity)
        {
            capacity *= 2;
            lines = realloc(lines, capacity * sizeof(char *));
            if (lines == NULL)
            {
                fprintf(stderr, "malloc failed\n");
                exit(1);
            }
        }
        if (getline(&lines[num_lines], &len, input) == -1)
            break;
        num_lines++;
    }

    // Print lines in reverse order
    for (int i = num_lines - 1; i >= 0; i--)
    {
        fprintf(output, "%s", lines[i]);
        free(lines[i]);
    }

    free(lines);
    // Close the files
    if (input != stdin)
        fclose(input);
    if (output != stdout)
        fclose(output);

    return 0;
}
