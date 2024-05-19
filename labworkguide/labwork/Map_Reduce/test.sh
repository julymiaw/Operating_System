#!/bin/bash

# 检查当前目录下是否有 "wordcount" 可执行文件
if [ ! -x wordcount ]; then
    echo "错误：当前目录下没有 'wordcount' 可执行文件。"
    exit 1
fi

# 定义一个函数来计算单词频率
word_count() {
    awk '{ for (i=1; i<=NF; i++) wc[$i]++ } END { for (word in wc) print word, wc[word] }' $@ | sort
}

# 循环处理每个测试文件
for i in {1..4}
do
    map_values=(1 10 100)
    reduce_values=(1 10 100)

    if [ $i -lt 3 ]; then
        # 运行 wordcount 程序并计算执行时间
        start_time=$(date +%s.%N)
        ./wordcount tests/$i.in > tests-out/$i.out
        end_time=$(date +%s.%N)
        wordcount_time=$(echo "$end_time - $start_time" | bc)

        # 使用 word_count 函数处理文件并将结果保存到临时文件
        word_count tests/$i.in > tests/$i.out

        # 对 wordcount 程序的输出进行排序
        sort tests-out/$i.out -o tests-out/$i.out

        # 比较 wordcount 程序和 word_count 函数的结果
        if diff -q tests-out/$i.out tests/$i.out > /dev/null; then
            result=$(tput setaf 2)correct$(tput sgr0)  # 绿色的 "correct"
        else
            result=$(tput setaf 1)incorrect$(tput sgr0)  # 红色的 "incorrect"
        fi

        # 打印结果
        printf "test%-2s %-10s %.4f s\n" $i $result $wordcount_time
    else
        # 获取 "tests/$i.in" 文件夹下的所有文件的完整路径
        files=$(find tests/$i.in -type f -name '*.in')

        # 打印表头
        printf "%-10s" "test$i"
        for reduce_value in ${reduce_values[@]}
        do
            printf "%-10s" "reduce=$reduce_value"
        done
        printf "\n"

        # 对每个 map 参数值进行一次测试
        for map_value in ${map_values[@]}
        do
            printf "%-10s" "map=$map_value"
            for reduce_value in ${reduce_values[@]}
            do
                # 运行 wordcount 程序并计算执行时间
                start_time=$(date +%s.%N)
                ./wordcount --map $map_value --reduce $reduce_value $files | sort > tests-out/$i-$map_value-$reduce_value.out
                end_time=$(date +%s.%N)
                wordcount_time=$(echo "$end_time - $start_time" | bc)

                # 打印结果
                printf "%-10.4f" $wordcount_time
            done
            printf "\n"
        done
    fi
done