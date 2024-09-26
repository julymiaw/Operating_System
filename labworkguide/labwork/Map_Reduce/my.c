#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include "mapreduce.h"

// 键-值对的数据结构
typedef struct KeyValue {
    char *key;
    char *value;
    struct KeyValue *next;
} KeyValue;

// 分区的数据结构
typedef struct Partition {
    KeyValue *pairs;
    int size;
} Partition;

// MapReduce 作业的数据结构
typedef struct MR_Job {
    char **files;
    int num_files;
    Partition *partitions;
    int num_partitions;
} MR_Job;

// 全局变量
MR_Job job;
pthread_mutex_t *partition_locks;

// Mapper 函数指针
static Mapper map_function;

// Reducer 函数指针
static Reducer reduce_function;

// Partitioner 函数指针
static Partitioner par_function;

// Getter 函数指针
static Getter get_function;

// 初始化作业
void initialize_job() {
    job.num_files = 0;
    job.files = NULL;
    job.num_partitions = 0;
    job.partitions = NULL;
}

// 初始化分区锁
void initialize_partition_locks(int num_partitions) {
    partition_locks = (pthread_mutex_t *)malloc(num_partitions * sizeof(pthread_mutex_t));
    for (int i = 0; i < num_partitions; i++) {
        pthread_mutex_init(&partition_locks[i], NULL);
    }
}

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % num_partitions;
}

// 实现 MR_Emit 函数
void MR_Emit(char *key, char *value) {
    unsigned long partition_num = MR_DefaultHashPartition(key, job.num_partitions);
    pthread_mutex_lock(&partition_locks[partition_num]);

    Partition *partition = &job.partitions[partition_num];
    KeyValue *current = partition->pairs;
    KeyValue *prev = NULL;

    // 遍历链表，找到插入位置
    while (current != NULL && strcmp(key, current->key) > 0) {
        prev = current;
        current = current->next;
    }

    // 创建新的键值对
    KeyValue *new_pair = (KeyValue *)malloc(sizeof(KeyValue));
    new_pair->key = strdup(key);
    new_pair->value = strdup(value);
    new_pair->next = current;

    // 更新链表
    if (prev != NULL) {
        prev->next = new_pair;
    } else {
        partition->pairs = new_pair;
    }

    partition->size++;

    pthread_mutex_unlock(&partition_locks[partition_num]);
}
int global_nummapper;
// Mapper 线程函数
void *mapper_thread(void *arg) {
    int mapper_id = *(int *)arg; // 这里不知道会不会有问题
    free(arg);
    for (int i = mapper_id; i < job.num_files; i += global_nummapper) {
        map_function(job.files[i]);
    }
    pthread_exit(NULL);
}

Partition *cur; // 用于获得当前的某一个分区,他是job的分区副本，在这里面会实现遍历分区的功能

char *get_next(char *key, int partition_number) // 由于是字典序排序，因此getnext一定是紧挨着访问，当返回null时，说明当前单词已经统计完毕
{
    // cur[partition_number].pairs = job.partitions[partition_number].pairs;
    if (cur[partition_number].pairs != NULL && strcmp(cur[partition_number].pairs->key, key) == 0) {
        char *value = cur[partition_number].pairs->value;
        cur[partition_number].pairs = cur[partition_number].pairs->next;
        return value;
    } else {
        return NULL;
    }
}

// Reducer 线程函数
void *reducer_thread(void *arg) {
    // printf("这里出问题n");

    int reducer_id = *(int *)arg;
    free(arg);
    cur[reducer_id].pairs = job.partitions[reducer_id].pairs;
    while (cur[reducer_id].pairs != NULL) {
        char *key = cur[reducer_id].pairs->key;
        if (key == NULL || strlen(key) == 0) {
            cur[reducer_id].pairs = cur[reducer_id].pairs->next;
            continue;
        }
        reduce_function(key, get_next, reducer_id);
    }
    pthread_exit(NULL);
}

// 实现 MR_Run 函数
void MR_Run(int argc, char *argv[], Mapper map, int num_mappers, Reducer reduce, int num_reducers, Partitioner partition) {
    // printf("开始创建\n");
    initialize_job();
    initialize_partition_locks(num_reducers);
    global_nummapper = num_mappers;
    cur = (Partition *)malloc(sizeof(Partition) * num_reducers);
    // 设置 Mapper、Reducer、Partitioner 和 Getter 函数指针
    map_function = map;
    reduce_function = reduce;
    par_function = partition;

    // 读取文件名并更新作业信息
    job.files = (char **)malloc(argc * sizeof(char *));
    if (job.files == NULL) {
        fprintf(stderr, "Failed to allocate memory for files\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        job.files[job.num_files++] = strdup(argv[i]);
    }

    // 更新分区数量
    job.num_partitions = num_reducers;
    job.partitions = (Partition *)malloc(num_reducers * sizeof(Partition));
    if (job.partitions == NULL) {
        fprintf(stderr, "Failed to allocate memory for partitions\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_reducers; i++) {
        job.partitions[i].size = 0;
        job.partitions[i].pairs = NULL;
    }

    // 创建 Mapper 线程数组
    pthread_t *mapper_threads = (pthread_t *)malloc(num_mappers * sizeof(pthread_t));
    if (mapper_threads == NULL) {
        fprintf(stderr, "Failed to allocate memory for mapper threads\n");
        exit(EXIT_FAILURE);
    }

    // 创建 Mapper 线程
    for (int i = 0; i < num_mappers; i++) {
        int *mapper_id = (int *)malloc(sizeof(int)); // 创建一个新的整数变量
        *mapper_id = i;                              // 将当前迭代的值赋给新变量
        if (pthread_create(&mapper_threads[i], NULL, mapper_thread, (void *)mapper_id) != 0) {
            fprintf(stderr, "Failed to create mapper thread\n");
            exit(EXIT_FAILURE);
        }
    }

    // printf("mapper chuangjian\n");
    // 等待 Mapper 线程完成
    for (int i = 0; i < num_mappers; i++) {
        if (pthread_join(mapper_threads[i], NULL) != 0) {
            fprintf(stderr, "Failed to join mapper thread\n");
            exit(EXIT_FAILURE);
        }
    }
    // printf("mapper completed\n");
    // 创建 Reducer 线程数组
    pthread_t *reducer_threads = (pthread_t *)malloc(num_reducers * sizeof(pthread_t));
    if (reducer_threads == NULL) {
        fprintf(stderr, "Failed to allocate memory for reducer threads\n");
        exit(EXIT_FAILURE);
    }

    // 创建 Reducer 线程
    for (int i = 0; i < num_reducers; i++) {
        int *reducer_id = (int *)malloc(sizeof(int)); // 创建一个新的整数变量
        *reducer_id = i;                              // 将当前迭代的值赋给新变量
        if (pthread_create(&reducer_threads[i], NULL, reducer_thread, (void *)reducer_id) != 0) {
            fprintf(stderr, "Failed to create reducer thread\n");
            exit(EXIT_FAILURE);
        }
    }
    // printf("reducer chuangjian\n");
    // 等待 Reducer 线程完成
    for (int i = 0; i < num_reducers; i++) {
        if (pthread_join(reducer_threads[i], NULL) != 0) {
            fprintf(stderr, "Failed to join reducer thread\n");
            exit(EXIT_FAILURE);
        }
    }
    // printf("reducer completed\n");
    // 释放 Mapper 线程数组的内存
    free(mapper_threads);

    // 释放 Reducer 线程数组的内存
    free(reducer_threads);

    // 释放分区锁
    for (int i = 0; i < num_reducers; i++) {
        pthread_mutex_destroy(&partition_locks[i]);
    }
    free(partition_locks);

    // 释放分区副本内存
    for (int i = 0; i < num_reducers; i++) {
        KeyValue *current = cur[i].pairs;
        while (current != NULL) {
            KeyValue *next = current->next;
            free(current->key);
            free(current->value);
            free(current);
            current = next;
        }
    }
    free(cur);

    // 释放分区内存
    for (int i = 0; i < num_reducers; i++) {
        KeyValue *current = job.partitions[i].pairs;
        while (current != NULL) {
            KeyValue *next = current->next;
            free(current->key);
            free(current->value);
            free(current);
            current = next;
        }
    }
    free(job.partitions);
    // printf("结束创建\n");
    // 释放文件名数组
    for (int i = 0; i < job.num_files; i++) {
        free(job.files[i]);
    }
    free(job.files);
}
