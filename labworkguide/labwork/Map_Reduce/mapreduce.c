#include "mapreduce.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // 提供malloc
#include <string.h> // 提供strdup

// 键值对结构体，包含键值和对应分区
typedef struct KeyValuePair {
    char *key;
    char *value;
    int partition;
    struct KeyValuePair *next;
} KeyValuePair;

// 定义全局变量
KeyValuePair **heads;         // 桶的头指针数组
KeyValuePair **currents;      // 当前处理的键值对的指针数组
int num_partitions;           // 分区的数量
Partitioner global_partition; // 分区函数
pthread_mutex_t *lock_emit;   // 用于保护emit操作的互斥锁
pthread_mutex_t lock_index;   // 用于保护文件索引的互斥锁

// 将新的键值对插入到有序链表中
void insert_sorted(KeyValuePair **head, KeyValuePair *newPair) {
    if (*head == NULL || strcmp(newPair->key, (*head)->key) < 0) {
        newPair->next = *head;
        *head = newPair;
    } else {
        KeyValuePair *current = *head;
        while (current->next != NULL && strcmp(newPair->key, current->next->key) > 0) {
            current = current->next;
        }
        newPair->next = current->next;
        current->next = newPair;
    }
}

void MR_Emit(char *key, char *value) {
    // 检查key是否为空字符串
    if (key == NULL || strlen(key) == 0) {
        return;
    }

    int partition = global_partition(key, num_partitions);
    KeyValuePair *newPair = (KeyValuePair *)malloc(sizeof(KeyValuePair));
    newPair->key = strdup(key);
    newPair->value = strdup(value);
    newPair->partition = partition;

    pthread_mutex_lock(&lock_emit[partition]);
    insert_sorted(&heads[partition], newPair);
    pthread_mutex_unlock(&lock_emit[partition]);
}

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % num_partitions;
}

// 定义线程参数结构体
typedef struct {
    char **filenames;
    int num_files;
    Mapper mapper;
} MapArg;

int current_file_index = 0;

void *map_thread(void *arg) {
    MapArg *mapArg = (MapArg *)arg;

    while (1) {
        pthread_mutex_lock(&lock_index);
        if (current_file_index >= mapArg->num_files) {
            pthread_mutex_unlock(&lock_index);
            break;
        }
        char *filename = mapArg->filenames[current_file_index++];
        pthread_mutex_unlock(&lock_index);

        mapArg->mapper(filename);
    }

    return NULL;
}

typedef struct {
    int partition_number;
    Reducer reducer;
} ReduceArg;

char *get_next(char *key, int partition_number) {
    if (currents[partition_number] != NULL && strcmp(currents[partition_number]->key, key) == 0) {
        char *value = currents[partition_number]->value;
        currents[partition_number] = currents[partition_number]->next;
        return value;
    }
    return NULL;
}

void *reduce_thread(void *arg) {
    ReduceArg *reduceArg = (ReduceArg *)arg;
    currents[reduceArg->partition_number] = heads[reduceArg->partition_number];
    while (currents[reduceArg->partition_number] != NULL) {
        char *key = currents[reduceArg->partition_number]->key;
        reduceArg->reducer(key, get_next, reduceArg->partition_number);
    }
    // 清理链表
    KeyValuePair *current = heads[reduceArg->partition_number];
    while (current != NULL) {
        KeyValuePair *next = current->next;
        free(current->key);
        free(current->value);
        free(current);
        current = next;
    }

    free(arg);
    return NULL;
}

void print_bucket(int partition_number) {
    KeyValuePair *current = heads[partition_number];
    printf("Bucket %d:\n", partition_number);
    while (current != NULL) {
        // printf("Key: %s, Value: %s\n", current->key, current->value);
        current = current->next;
    }
}

void MR_Run(int argc, char *argv[],
            Mapper map, int num_mappers,
            Reducer reduce, int num_reducers,
            Partitioner partition) {
    pthread_t *threads_map = (pthread_t *)malloc(sizeof(pthread_t) * num_mappers);
    pthread_t *threads_reduce = (pthread_t *)malloc(sizeof(pthread_t) * num_reducers);
    lock_emit = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * num_reducers);
    for (int i = 0; i < num_reducers; i++) {
        pthread_mutex_init(&lock_emit[i], NULL);
    }
    pthread_mutex_init(&lock_index, NULL);

    num_partitions = num_reducers;
    heads = (KeyValuePair **)malloc(sizeof(KeyValuePair *) * num_partitions);
    for (int i = 0; i < num_partitions; i++) {
        heads[i] = NULL;
    }

    global_partition = partition;

    MapArg arg;
    arg.filenames = argv + 1;
    arg.num_files = argc - 1;
    arg.mapper = map;

    for (int i = 0; i < num_mappers; i++) {
        pthread_create(&threads_map[i], NULL, map_thread, &arg);
    }

    for (int i = 0; i < num_mappers; i++) {
        pthread_join(threads_map[i], NULL);
    }
    current_file_index = 0;

    // 打印每个桶的内容
    // for (int i = 0; i < num_partitions; i++) {
    //     print_bucket(i);
    // }

    currents = (KeyValuePair **)malloc(sizeof(KeyValuePair *) * num_partitions);
    for (int i = 0; i < num_partitions; i++) {
        currents[i] = heads[i];
    }

    for (int i = 0; i < num_reducers; i++) {
        // printf("MR_Run: starting reducer %d\n", i);
        ReduceArg *arg = (ReduceArg *)malloc(sizeof(ReduceArg));
        arg->partition_number = i;
        arg->reducer = reduce;
        pthread_create(&threads_reduce[i], NULL, reduce_thread, arg);
    }

    for (int i = 0; i < num_reducers; i++) {
        pthread_join(threads_reduce[i], NULL);
    }

    for (int i = 0; i < num_reducers; i++) {
        pthread_mutex_destroy(&lock_emit[i]);
    }
    free(lock_emit);
    pthread_mutex_destroy(&lock_index);
    free(threads_map);
    free(threads_reduce);
    free(heads);
    free(currents);
}
