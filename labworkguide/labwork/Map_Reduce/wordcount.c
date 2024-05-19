#include "mapreduce.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Map(char *file_name) {
    FILE *fp = fopen(file_name, "r");
    assert(fp != NULL);
    char *line = NULL;
    size_t size = 0;
    while (getline(&line, &size, fp) != -1) {
        char *token, *dummy = line;
        while ((token = strsep(&dummy, " \t\n\r")) != NULL) {
            MR_Emit(token, "1");
        }
    }
    free(line);
    fclose(fp);
}

void Reduce(char *key, Getter get_next, int partition_number) {
    int count = 0;
    char *value;
    while ((value = get_next(key, partition_number)) != NULL)
        count++;
    printf("%s %d\n", key, count);
}

// 现在增加了两个可选参数，--map和--reduce，用于指定map和reduce线程的数量
int main(int argc, char *argv[]) {
    int map_num = 10;
    int reduce_num = 10;
    char **new_argv = malloc(sizeof(char *) * argc);
    int new_argc = 0;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--map") == 0 && i + 1 < argc) {
            map_num = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--reduce") == 0 && i + 1 < argc) {
            reduce_num = atoi(argv[++i]);
        } else {
            new_argv[new_argc++] = argv[i];
        }
    }

    MR_Run(new_argc, new_argv, Map, map_num, Reduce, reduce_num, MR_DefaultHashPartition);
    free(new_argv);
}