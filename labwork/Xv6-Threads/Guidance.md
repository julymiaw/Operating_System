### 任务
在thread部分 主要任务有3个 \
创建系统调用clone()  \
创建系统调用join() \
建立一个包含lock_init(), lock_release(), lock_acquire()的线程库 

### 需要修改的文件
与 Xv6-Syscall 类似，您需要修改 `syscall.c`, `proc.c`, `proc.h`, `sysproc.c`, `usys.S`, `defs.h` 中的内容 \
主要的函数编写在`proc.c`中 \
其中clone()与fork()类似，join()与wait()类似 \
具体的修改与详细的内容注释详见代码

### 测试程序
由于没有对应的测试用例，您需要自己编写一个用于测试的代码 \
代码的参考在thread_test.c中 

在测试的时候不要忘记在Make_file 184行左右的地方添加 `_thread_test\` \
否则会出现 `/usr/bin/ld: cannot find thread_test: No such file or directory
collect2: error: ld returned 1 exit status` 这样或者类似的错误
![图片](https://github.com/julymiaw/Operating_System/assets/143331086/910225e8-7060-45f7-9894-483c3521ebe3)

如图进行操作，在make qemu后输入thread_test，就可以运行啦

### 一些温馨提示
1. printf 要这样写，`printf(1, "thread %d: start\n", i);` 输出前面必须加一个数字，不要问为什么，问就是xv6的printf函数就是这么规定的。。

2. make qemu比较严格，一些警告可能导致无法执行

3. `lapicid 0: panic: kfree` 错误表明释放内存错误，xv6中共存在2处 panic: kfree 您可以对此进行魔改以判断到底是哪里出现了错误
  
   unexpected trap 13：表示发生了一个未预料到的陷阱（interrupt），陷阱编号 13 对应于通用的中断处理例程（interrupt handler）

   trap 14：这是页面错误（page fault）的异常代码，意味着尝试访问的内存页面不存在或不允许当前进程访问。

### Ending
深呼吸，panic是正常的，请你不要慌张

祝您在kernel hacking的过程中玩得开心！\
Because you are on your way to becoming a real kernel hacker. And what could be more fun than that?

### 参考文献
http://xv6.dgs.zone/tranlate_books/Introduction.html \
https://github.com/shradhash/implementing-kernel-threads-in-xv6 \
https://blog.csdn.net/jannleo/article/details/134680689
