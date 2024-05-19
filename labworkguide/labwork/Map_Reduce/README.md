Map Reduce
==========

2004年, Google的工程师们引入了一种新的大规模并行数据处理范式：MapReduce它使得在大规模集群上编程此类任务变得更容易；开发者无需担心如何管理并行性, 处理机器崩溃, 以及集群中常见的许多其他困难, 而只需专注于编写一些小的代码片段(如下所述), 框架会处理其余的部分。

在这个项目中, 你将为单台机器构建一个简化版的MapReduce。虽然为单台机器构建MapReduce相对容易一些, 但仍然存在许多挑战, 主要在于构建正确的并发支持。因此, 你需要稍微思考一下如何构建实现MapReduce, 然后高效且正确地构建它。

这个任务有三个具体的目标：

- 了解MapReduce范式的一般性质。
- 使用线程和相关函数实现一个正确且高效的MapReduce框架。
- 获得更多编写并发代码的经验。

总体思路
------------

现在让我们深入了解你需要构建的具体代码。你将构建的MapReduce框架支持执行用户定义的Map()和Reduce()函数。

正如原始论文中所述：“用户编写的Map()函数接收一个输入对, 并生成一组中间键/值对。MapReduce库将所有与同一中间键K关联的中间值组合在一起, 并将它们传递给Reduce()函数。”

“用户编写的Reduce()函数接受一个中间键K和该键的一组值。它将这些值合并在一起, 形成一个可能更小的值集合；通常每次调用Reduce()函数只产生零个或一个输出值。中间值通过迭代器提供给用户的reduce函数。”

这是一个经典的示例, 用伪代码写出, 展示了如何计算一组文档中每个单词的出现次数：

    map(String key, String value):
        // 键: 文档名称
        // 值: 文档内容
        for each word w in value:
            EmitIntermediate(w, "1");
    
    reduce(String key, Iterator values):
        // 键: 单词
        // 值: 不同文件中count值的列表
        int result = 0;
        for each v in values:
            result += ParseInt(v);
        print key, result;

MapReduce的令人着迷之处在于, 许多不同类型的相关计算都可以映射到这个框架上。原始论文列出了许多例子, 包括单词计数(如上所述)、分布式grep、URL访问频率计数器、反向网络链接图应用、每个主机的术语向量分析等等。

同样非常有趣的是它的并行化有多么容易：许多map函数可以同时运行, 稍后, 许多reduce函数也可以同时运行。用户不需要担心如何并行化他们的应用程序；相反, 他们只需要编写Map()和Reduce()函数, 框架会处理剩下的部分。

代码总览
-------------

我们在这里给你提供了mapreduce.h头文件, 它详细说明了你必须在你的MapReduce库中构建什么：
```c
    #ifndef __mapreduce_h__
    #define __mapreduce_h__

    // MR使用的不同函数指针
    typedef char *(*Getter)(char *key, int partition_number);
    typedef void (*Mapper)(char *file_name);
    typedef void (*Reducer)(char *key, Getter get_func, int partition_number);
    typedef unsigned long (*Partitioner)(char *key, int num_partitions);

    // 暴露给外部的函数: 这些函数必须自己定义（我认为Getter也是我们定义的）
    void MR_Emit(char *key, char *value);

    unsigned long MR_DefaultHashPartition(char *key, int num_partitions);

    void MR_Run(int argc, char *argv[], 
            Mapper map, int num_mappers, 
            Reducer reduce, int num_reducers, 
            Partitioner partition);

    #endif // __mapreduce_h__
```
最重要的函数是`MR_Run`, 该函数接受命令行参数, Map函数指针(`Mapper`类型的`map`), 库创建的Mapper线程数(`num_mappers`), Reducer函数指针(`Reducer`类型的`reduce`)Reducer线程数(`num_reducers`), 和一个分隔函数(`partition`)。

因此，当用户利用你的库完成MapReduce计算时, 他们会定义一个Mapper函数和Reducer函数, 部分情况下构建Partitioner函数, 然后调用`MR_Run()`。然后框架会创建进程并完成计算。

一个基本的假设是框架会为Map任务创建`num_mappers`个线程(通过线程池)，另一个假设是框架会为Reducer任务创建`num_reducers`个线程。最后，框架会创建某种内部数据结构来把Mapper创建的键值对传递给Reducers。

简单示例: 单词计数
-------------------------

这是一个简单当有意义的基于本框架的单词计数程序
```c
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mapreduce.h"

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

int main(int argc, char *argv[]) {
    MR_Run(argc, argv, Map, 10, Reduce, 10, MR_DefaultHashPartition);
}
```
看看这段代码，首先，`Map()`被传入了一个文件名。我们假设此类计算涉及多个文件，每个Mapper一次只处理一个文件，一个文件仅由一个Mapper负责处理。

在这个例子中，我们逐行读取文件，并使用`strsep()`来把每一行分割为词。然后，每个词通过`MR_Emit()`函数提交。

因此，`MR_Emit()`函数是你实现的库的另一部分，他需要接收不同Mapper的键值对并合理地存储他们，以便后续Reducer读取。设计这一数据结构是本项目地另一核心挑战。

在所有Mapper完成后，框架应该已经将所有键值对以一种`Reduce()`函数可获取的方式存储。每个独一无二的键都会调用一次`Reduce()`函数，并把这个键传递给一个函数，这个函数支持查找所有具有相同键的键值对。为了完成迭代，代码反复调用`get_next()`直到返回Null。`get_next`函数返回一个指向由`MR_Emit()`提交的键值对的指针。在刚刚的例子中，输出了所有文件中的单词出现次数。

所有这些计算通过`MR_Run()`函数开始，这个函数传递了`argv`数组，假设`argv[1]`到`argv[n-1]` (在`argc`等于`n`的情况下)包含了所有传递给Mapper的文件名。

还有一个你需要传递给`MR_Run()`的函数指针是分割函数。大部分情况下，程序将使用默认分割函数(`MR_DefaultHashPartition`)，定义如下：
```c
unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}
```
这个函数的作用是把`key`映射到`0`至`num_partitions - 1`的整数，它在MapReduce库内部被使用，但十分关键。特别地，你的MR库应该使用此函数来决定哪一块(进而决定哪一个Reducer)获得特定的一个键和其对应的值的列表。对于某些应用，由哪一个Reducer线程处理特定的键是不重要的，此时就可以使用默认分割函数，其他情况下，用户可以传递他们自己搭建的分隔函数。

最后一个需求: 对于每一个分隔, 键 (和该键对应的值列表)应该按照键升序排列，因此当特定的Reducer线程在工作时，`Reduce()`函数可以按顺序获取该分隔中的键。

注意事项
--------------

这里是一些编程中的注意事项:

- **线程管理**:

  这部分相当直接。你应该创建num_mappers个映射线程，并以你认为最好的方式（例如，轮询(Round Robin)，最短文件优先(Shortest-File-First)等）为每个Map()调用分配一个文件。哪种方式可能会带来最佳的性能？在某个时候，你也应该创建num_reducers个reducer线程，来处理map的输出。

- **分隔和排序**：

  你的核心数据结构应该是并发的，允许每个mapper将值正确且高效地放入不同的分区中。一旦mapper完成，一个排序阶段应该对键/值列表进行排序。最后，每个reducer线程应该开始按照每个分区的排序顺序调用用户定义的Reduce()函数。你应该思考在整个过程中需要什么类型的锁来确保正确性。

- **内存管理**.

  最后一个问题是内存管理。MR_Emit()函数传递了一个键/值对；MR库有责任复制这些键/值对。然后，当整个映射和归约过程完成时，MR库有责任释放所有内容。这意味着你需要在你的MapReduce库中实现适当的内存管理策略，以防止内存泄漏。

打分标准
-------

你的代码应该提交mapreduce.c，它能够正确且高效地实现上述函数。它将与测试应用程序一起编译，编译时会使用-Wall -Werror -pthread -O标志；同时，它也会被valgrind检查内存错误。

首先，你的代码将被检查正确性，确保它正确地执行了映射和归约。如果你通过了正确性测试，那么你的代码将会进行性能测试；性能越高，得分就越高。

## 测试程序

首先，输入`gcc -o wordcount wordcount.c mapreduce.c -lpthread`编译得到可执行文件

(注意这里的wordcount.c进行了一些修改，增加了两个可选参数`--map`和`--reduce`)

之后，你需要更改这两个脚本的权限:

sudo chmod +x wordcount

sudo chmod +x test.sh

你可能还需要安装bc包：sudo apt-get install bc

之后，你就可以运行test.sh来测试Map_reduce的正确性与性能了。

测试时，test1主要针对正确性，而后续虽然也检查正确性，但是重点在于并行度对效率的提升。

test2是仅1个文件的情况，这种情况下，map的值与并行度无关。

之后，test3将同样的文件分为200份，test4分为100份。

在这种情况下，单线程处理(map=1)的增加时间是IO耗时，而map取10或100带来的效率提升就是并行程序的功劳。如果增加并行度而效率反而下降，可以尝试把全局锁改为针对每个桶的锁。

示例输出如下：

```text
test1  correct 0.0025 s
test2  correct 1.0736 s
test3     reduce=1  reduce=10 reduce=100
map=1     13.8016   1.1517    0.1216    
map=10    13.5830   0.5505    0.0914    
map=100   28.1280   1.0405    0.1008    
test4     reduce=1  reduce=10 reduce=100
map=1     13.8216   1.3468    0.2228    
map=10    15.8236   0.5743    0.1084    
map=100   25.0356   0.8117    0.0866  
```
由图可见，reduce并行度对于总时间的影响更明显。map并行度提高带来的效率增加可能无法抵消线程创建带来的时间成本，所以对于100到200个文件，10个线程表现最佳。
