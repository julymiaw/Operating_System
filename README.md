# 课前准备

## 安装(wsl + Ubuntu):

1. 打开控制面板->程序->启动或关闭Windows功能->适用于Linux的Windows子系统(重启)

2. 打开Terminal，输入`wsl --update`，更新完成后直接输入`wsl`

3. 在安装过程中，提示输入用户名(不能有大写)和密码(不显示为正常)

*如果通过其他方式安装Ubuntu导致跳过了用户设置，此时应该为root状态，通过passwd设置root密码后，通过`adduser <username>`添加用户，并通过`adduser <username> sudo`给该用户添加管理员权限*

4. 安装完成，回到Terminal，输入`ubuntu config --default-user <username>`设置默认登录用户，这里的`<username>`就是刚刚设置的

5. 如果想把Ubuntu移动到指定位置(如D盘)，执行以下步骤
   1. Terminal输入`wsl --shutdown`关闭，(`wsl -l -v`查看状态)
   2. 确认关闭后，输入`wsl --export Ubuntu D:\wsl2.tar`导出
   3. 导出完成卸载原发行版`wsl --unregister Ubuntu`
   4. `wsl --import Ubuntu D:\Ubuntu_WSL\ D:\wsl2.tar`导入
   5. 输入`wsl`启动Ubuntu

6. 进入Ubuntu，输入`sudo apt-get update`更新，根据提示输入密码

7. 输入`sudo apt-get install build-essential gdb`安装gcc环境

8. 输入`code .`安装并打开Visual Studio Code

9.  文件->打开文件夹->`~`打开用户目录，新建test.c(根据提示安装插件)

   ```c
   #include <stdio.h>
   int main(int argc, char *argv[])
   {
       printf("Hello World\n");
       return 0;
   }
   ```

10. 点击按钮运行，选择gcc，若输出为`Hello World`，配置完成

(本部分参考了[wsl官方教程](https://learn.microsoft.com/zh-cn/windows/wsl/setup/environment))

## 安装浏览器与对应插件(如果你想在wsl中打开说明页面)

1. 输入`cd /tmp`打开临时目录
2. `wget https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb`下载Chrome安装包
3. `sudo apt install --fix-missing ./google-chrome-stable_current_amd64.deb`安装
4. `sudo apt-get install ttf-wqy-zenhei`安装中文字体
5. 尝试打开，输入`google-chrome`
6. 确认显示正常后，关闭窗口，输入`code .`打开VS Code
7. 搜索并安装插件`Open Browser Preview`

(Tips: 可使用Chrome自带的网页翻译功能，若没有正常弹出翻译选项，按`F12`进入开发者工具，选中\<html>右键添加属性`lang="en"`将网站标注为英文)

## 配置中文输入法(可选)

如果你希望浏览器的字体为中文，且可像在Windows系统上一样输入中文，可以尝试以下配置。推荐在尝试前先导出备份。

1. 操作中有的需要更高权限，须设置root密码`sudo passwd`

2. 配置中文语言包`sudo apt install language-pack-zh-hans`

   (假设之前已经安装了中文字体)

3. 编辑`/etc/locale.gen`，去掉`en_US.UTF-8 UTF-8` 及 `zh_CN.UTF-8 UTF-8`前的注释符号`vim /etc/locale.gen`(按i编辑，ESC+`:wq`保存并退出)

4. `sudo locale-gen --purge`

5. 安装输入法`sudo apt install fcitx fonts-noto-cjk fonts-noto-color-emoji dbus-x11`

6. 安装输入模式`sudo apt install <Package>`其中package从`fcitx-libpinyin`，`fcitx-sunpinyin`，`fcitx-googlepinyin`中挑选一个

7. 切换到root用户`su root`

8. `dbus-uuidgen > /var/lib/dbus/machine-id`

9. 创建新文件`vim /etc/profile.d/fcitx.sh`，输入

   ```shell
   #!/bin/bash
   export QT_IM_MODULE=fcitx
   export GTK_IM_MODULE=fcitx
   export XMODIFIERS=@im=fcitx
   export DefaultIMModule=fcitx

   #optional
   fcitx-autostart &>/dev/null
   ```

10. 打开终端，通过`wsl --shutdown`和`wsl`重启

11. 输入`fcitx-config-gtk3`，不出意外的话，界面上出现之前安装的输入法。可通过Global Config调整切换输入法的快捷键

12. 尝试打开浏览器`google-chrome`，看输入法功能是否正常

## 下载实验文件

1. 点击[链接](https://seunic-my.sharepoint.cn/:u:/g/personal/101011912_seu_edu_cn/EUWd54wqsdJNu_h-sHQ1X2YBiZ24oi4rryRwXdoFGWpGsw?e=vDNOXc)下载并解压labworks，将文件夹拖到VSCode窗口中(先打开`~`)
2. 将文件夹重命名为`projects`，打开./projects/Reverse
3. 选中`README.html`，右键，选择`Preview In Default Browser`
4. 如果网页打开成功，则说明配置完成

*另一种方案: 直接克隆本仓库(不推荐)*

## 配置git(可选)

1. Ubuntu默认已安装git，只需配置用户名和邮箱(改为自己的)
2. `git config --global user.name "Your Name"`
3. `git config --global user.email "youremail@domain.com"`
4. 如果Windows上没有安装Git，点击[链接](https://github.com/git-for-windows/git/releases/)下载并安装
5. `git config --global credential.helper "/mnt/c/Program\ Files/Git/mingw64/bin/git-credential-manager.exe"`
6. 可以尝试在VSCode中使用`源代码管理`进行推送与拉取
7. 如果把Projects放在Git仓库中，可通过在文件夹中添加`.gitignore`文件，输入`Projects/`进行忽略

*本部分参考wsl官方教程*

# 第一课 Guide to Labworks

## 总体介绍——以Reverse为例

在VScode中打开`~`目录，在`./projects/Reverse`下新建`reverse.c`文件。

### 需求分析

1. 支持3种输入形式：

	* `./reverse`
	* `./reverse input.txt`
	* `./reverse input.txt output.txt`

2. 对于输入的数据(命令行/文件)，**不能**假设*句子长度* 和*句子个数*。

3. 处理以下4种错误：

  1. 输入参数过多：`usage: reverse <input> <output>`
  2. 文件无法打开：`reverse: cannot open file '<filename>'`（其中`<filename>`为打不开的文件名）
  3. 输入相同文件：`reverse: input and output file must differ`（不能仅通过文件名判断）
  4. 内存分配失败：`malloc failed`

  * 无论是哪一种错误，统一用`fprintf(stderr, "whatever the error message is\n");`输出错误并`exit(1);`

### 实现功能

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
```

1. 处理错误“输入参数过多”

```c
int main(int argc, char *argv[])
{
    // 如果用户运行时reverse参数过多，则打印usage: reverse <input> <output>并退出，返回码为 1
    if (argc > 3)
    {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }
}
```

2. 定义输入流和输出流，并尝试在提供对应参数时更新输入或输出方式，处理错误“文件无法打开”

```c
int main(int argc, char *argv[])
{
   // 输入流，文件或命令行输入(Ctrl+D结束输入)
    FILE *input = stdin;

    // 输出流，文件或命令行输出
    FILE *output = stdout;
    
    /* 此处省略 */

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
    }
}
```

3. 通过头文件`<sys/stat.h>`提供的stat函数处理错误“输入相同文件”

```c
    if (argc == 3)
    {
        /* 此处省略 */
        struct stat stat1, stat2;
        stat(argv[1], &stat1);
        stat(argv[2], &stat2);
        if (stat1.st_ino == stat2.st_ino)
        {
            fprintf(stderr, "reverse: input and output file must differ\n");
            exit(1);
        }
    }
```

4. 分配初始内存，当容量不够时自动扩容，处理错误“内存分配失败”，并从输入流中读取句子

```c
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
```

*注意！当len设置为0时，getline方法会自动扩充输入缓冲区，并更新len参数*

5. 将获取的所有句子逆序放入输出流，释放内存并关闭文件

```c
int main(int argc, char *argv[])
{
    /* 此处省略 */
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
```

### 编译文件并测试功能

1. 选中文件，右键“在集成终端中打开”

2. 输入`gcc -o reverse reverse.c -Wall`进行编译

3. 输入`sudo chmod 777 test-reverse.sh`对测试脚本的权限进行修改

   (你可能还需要输入`sudo chmod 777 ../tester/run-tests.sh`)

4. 输入`./test-reverse.sh`进行测试。

   如果一切顺利的话，你会看到以下结果

   ![71009158696](README.assets/1710091586965.png)

## Kernel Hacking介绍

老师的配置教程如下:

1. 获取最新的xv6资源

    (首先记得在`Xv6-Syscal`文件夹中打开i终端)
```shell
git clone git://github.com/mit-pdos/xv6-public.git
```
注意！我这边尝试该命令无效，我使用的是:
```shell
git clone https://github.com/mit-pdos/xv6-public.git
```
2. 测试电脑编译工具
```shell
objdump -i
```
我的输出如下:
![alt text](README.assets/image.png)
如果第二行和我一样是`elf32-i386`就没问题了
```shell
gcc -m32 -print-libgcc-file-name
```
这一步如果之前按照我的步骤安装了gcc，一定不会有问题的。

3. 编译xv6
```shell
./Xv6-master/make
```
注意！我这边验证有效的操作是
```shell
cd xv6-public
make
```
4. 安装`qemu`虚拟机
(老师的命令过时了，为防止误操作已略去)
```shell
sudo apt-get install qemu-system
```
本命令来自[官网](https://www.qemu.org/download/#linux)

5. 用虚拟机启动Xv6
```shell
make qemu
```

### 开始实验

在`Xv6-Syscal`中打开终端。

首先，把刚刚的文件夹`xv6-public`改名为`src`。

```shell
sudo chmod 777 test-getreadcount.sh
sudo chmod 777 ../tester/run-tests.sh
sudo chmod 777 ../tester/xv6-edit-makefile.sh
sudo chmod 777 ../tester/run-xv6-command.exp
./test-getreadcount.sh
```

如果不出意外的话，程序会开始运行，并抛出一个错误：

![image-20240317115550451](./README.assets/image-20240317115550451.png)

这是正常的，因为`getreadcount()`函数需要我们自己实现。

可以自己查看官方的[介绍](https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/initial-xv6)，你会发现与老师提供的一摸一样...

看一下test_1.c代码：

```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
  int x1 = getreadcount();
  int x2 = getreadcount();
  char buf[100];
  (void)read(4, buf, 1);
  int x3 = getreadcount();
  int i;
  for (i = 0; i < 1000; i++)
  {
    (void)read(4, buf, 1);
  }
  int x4 = getreadcount();
  printf(1, "XV6_TEST_OUTPUT %d %d %d\n", x2 - x1, x3 - x2, x4 - x3);
  exit();
}
```

可以看到导入了3个头文件，其中`types.h`提供了一些基本类型的定义，`stat.h`提供了一个结构体`stat`的定义:

```c
#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Device

struct stat {
  short type;  // Type of file
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short nlink; // Number of links to file
  uint size;   // Size of file in bytes
};
```

这里，首先把所有文件分为了`目录`，`文件`和`设备`(Unix系统中，一般把设备看作特殊的文件对象)，这里所谓的`short type`就是以上3种类型之一。接下来，定义了`dev`设备编号，`ino`文件`incde`号，别名个数和文件大小。

最后，`user.h`就是`getreadcount()`本应该存在的地方。

可以看到，这里已经实现了许多系统调用，例如`open()`，`close()`等

