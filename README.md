# 平台
Ubuntu 16.04

# 1 ALSA环境
## 1.1 编译ALSA
注意同时生成静态库和动态库。

1. 安装方法： 
```
#所有课程相关的环境的下载和编译都在~/av_lesson
cd ~/av_lesson
```   
  - 到ALSA的官网下载库文件，[ALSA库下载链接 ](http://www.alsa-project.org/main/index.php/Main_Page)(我下载的是alsa-lib-1.1.7)
 - 下载完放到ubuntu下进行解压缩，进入alsa-lib-1.1.7文件夹，按照linux安装三步曲进行安装即可。 
```
lqf@ubuntu:~/av_lesson$ mkdir 23-alsa-v4l2
lqf@ubuntu:~/av_lesson$ cd 23-alsa-v4l2/
lqf@ubuntu:~/av_lesson/23-alsa-v4l2$ ls
lqf@ubuntu:~/av_lesson/23-alsa-v4l2$ wget ftp://ftp.alsa-project.org/pub/lib/alsa-lib-1.1.7.tar.bz2
lqf@ubuntu:~/av_lesson/23-alsa-v4l2$ tar -jxvf alsa-lib-1.1.7.tar.bz2
lqf@ubuntu:~/av_lesson/23-alsa-v4l2$ cd alsa-lib-1.1.7/
```
 - 首先执行 ./configure 进行配置 
  静态库和动态库不能同时编译。
 **1. 编译静态库**
安装到/home/lqf/av_lesson/install/alsa 目录
```
# 安装到/home/lqf/av_lesson/install/alsa 目录
./configure  --prefix=/home/lqf/av_lesson/install/alsa  --enable-static=yes --enable-shared=no
make
make install
```
 - 执行 make 进行编译，编译成功之后执行 make install 进行安装。

执行完后在/home/lqf/av_lesson/install/alsa目录有
```
lqf@ubuntu:~/av_lesson/install/alsa$ ls
bin  include  lib  share
```
将alsa整个文件夹拷贝到alsa_record/third_lib目录。

**2.编译动态库**
上一步使用静态库即可，如果自己想使用动态库则可以依照下面步骤进行编译。
```
./configure  --prefix=/home/lqf/av_lesson/install/alsa  --enable-static=no --enable-shared=yes
make
make install
```

错误提示：
```
pcm/pcm_meter.c:674: undefined reference to `dlsym'
```

# 2 fdk-aac编译
下载地址及编译参考：http://www.linuxfromscratch.org/blfs/view/svn/multimedia/fdk-aac.html
**1. 下载和编译**
```
cd cd ~/av_lesson/23-alsa-v4l2/
git clone https://github.com/mstorsjo/fdk-aac.git
# 使用稳定的版本
git checkout  v2.0.0
# 生成配置文件
./autogen.sh
# 编译静态版本和动态版本
./configure --prefix=/home/lqf/av_lesson/install/fdk-aac --enable-shared --enable-static
make
make install
```
**2. 拷贝到alsa_record/third_lib**
此时alsa_record/third_lib下的文件为



# 3. SDL2编译
1. 下载
```
cd cd ~/av_lesson/23-alsa-v4l2/
git clone https://github.com/SDL-mirror/SDL.git
cd SDL
# 使用稳定的版本
git checkout  release-2.0.9
# 查看帮助
./configure --help
./configure --prefix=/home/lqf/av_lesson/install/sdl2 
make
make install
```
然后sdl2相关的文件被安装到/home/lqf/av_lesson/install/sdl2，将sdl2 整个目录拷贝到v4l2_sdl2\third_lib。

# 4. 范例编译
## 4.1 alsa范例
在alsa\alsa_record目录，编译方法
```
mkdir build
cd build
cmake ..
make
# 执行文件
sudo ./alsa_record

## 4.2 v4l2范例
在v4l2\v4l2_sdl2目录，编译方法
```
mkdir build
cd build
cmake ..
make
# 执行文件
sudo ./v4l2_sdl2
