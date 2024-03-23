### 这是一则来自于JCSTAR的代码
作为小伙伴们在test中遇到的各种奇葩问题的参考 <br>
希望大家不要抄袭  <s>记得删注释</s>

关于运行与测试：以seuunzip为例 <br>
进入目录
```
 cd seuunzip
```
编译
```
 gcc -o seuunzip seuunzip.c -Wall -Werror
```
运行
```
./test-seuunzip.sh
```
如果你需要直接对某个数据进行检测，找到`test\?.run`, 将其中的内容输入命令行即可
