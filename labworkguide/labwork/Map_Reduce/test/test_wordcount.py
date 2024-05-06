import os
import subprocess
import time

# 获取所有的article文件，并只保留前10个
article_files = [f for f in os.listdir() if f.startswith('article_')][:10]

# 分别处理每篇文章，并记录运行时间
start_time = time.time()
for file in article_files:
    with open('wordcount.log', 'w') as log_file:
        subprocess.run(['./wordcount_1', file], stdout=log_file)
end_time = time.time()
print('Execution time for processing each file separately: {} seconds'.format(end_time - start_time))

# 一次处理所有文章，并记录运行时间
start_time = time.time()
with open('wordcount.log', 'w') as log_file:
    subprocess.run(['./wordcount'] + article_files, stdout=log_file)
end_time = time.time()
print('Execution time for processing all files at once: {} seconds'.format(end_time - start_time))