# The drone for cutting mango

## Device
* Jetson NX
* Pixhawk 2.4.8
* f450(drone hardware)

## Jetson NX setting
* apt install
```shell
$ sudo apt-get install python3-pip
$ sudo apt-get install python3-dev python3-opencv python3-wxgtk4.0 python3-matplotlib python3-lxml libxml2-dev libxslt1-dev
```
* pip install
```shell
$ sudo pip install PyYAML mavproxy
```
* test
```shell
# using USB
$ sudo mavproxy.py --master=/dev/ttyACM0
# using TELEM2
$ sudo mavproxy.py --master=/dev/ttyTHS1
```
#### using Python may face some error
* TabError: inconsistent use of tabs and spaces in indentation
> In Vim, type `:retab! 4`, and `:x` to solve this issue
