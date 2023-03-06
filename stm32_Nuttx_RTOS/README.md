# STM32F407VET6 with Nuttx RTOS
* host device: x86_64 ubuntu 22.04
* STM32F407VET6
* ST-Linkv2

## Install
### Environment setting in Ubuntu_22.04
```shell
$ sudo apt install \
bison flex gettext texinfo libncurses5-dev libncursesw5-dev \
gperf automake libtool pkg-config build-essential gperf genromfs \
libgmp-dev libmpc-dev libmpfr-dev libisl-dev binutils-dev libelf-dev \
libexpat-dev gcc-multilib g++-multilib picocom u-boot-tools util-linux \
kconfig-frontends gcc-arm-none-eabi binutils-arm-none-eabi
```

### Install Nuttx 
```shell
$ cd ~ 
$ mkdir nuttx_space && cd nuttx_space
$ git clone https://github.com/apache/incubator-nuttx.git nuttx
$ git clone https://github.com/apache/incubator-nuttx-apps apps
$ git clone https://bitbucket.org/nuttx/tools.git
$ cd tools/kconfig-frontends
$ ./configure
$ make
```
* After `make` may faced `make: *** [Makefile:924: aclocal.m4] Error 127`
  * Solve: 
  ```shell
  $ apt install libtool-bin automake texinfo
  $ autoreconf -f -i
  $ ./configure
  $ make
  ```
keep install:
```shell
$ sudo make install
$ sudo ldconfig
$ cd ../../nuttx
$ ./tools/configure.sh sim/nsh
$ make menuconfig
$ make
$ ./nuttx
```
> Using `pkill nuttx` in the another terminal, it can kill simulator
## Init config

## Reference 
* [csdn](https://blog.csdn.net/Alkaid2000/article/details/127573074?spm=1001.2014.3001.5502)
