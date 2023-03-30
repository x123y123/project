# The drone for cutting mango

## Device
* Jetson NX
* Pixhawk 2.4.8
* F450(drone hardware)

## Directory Structure
```html
├── main.py                 # main brain to conbine image part and control part
├── control.py              # Control Library 
├── image.py                # Computer Vision Library
├── models                  # Computer Vision Part weight
└── arm_test.py             # Testing in first to check dronekit can working normally.
```

## ENV Setting
### Image Part
#### Training Part
[Youtube](https://www.youtube.com/watch?v=fZiY7zUk3TU)
* [Jetson-train](https://github.com/mailrocketsystems/jetson-train)
```shell
$  git clone git@github.com:mailrocketsystems/jetson-train.git
```
* [label](https://github.com/heartexlabs/labelImg)
```shell
$ git clone git@github.com:heartexlabs/labelImg.git
```
#### Inference Part(Jetson Inference)
* But at first, you need to have Jetson board(e.g. Jetson NX, Jetson Nano, etc.)
```shell
$ sudo apt-get install git cmake
$ cd ~
$ git clone git@github.com:dusty-nv/jetson-inference.git
$ cd jetson-inference/
$ mkdir build && cd build
$ cmake ../
$ make
$ sudo make install
```

### Control Part
* apt install
```shell
$ sudo apt-get install python3-pip
$ sudo apt-get install python3-dev python3-opencv python3-wxgtk4.0 python3-matplotlib python3-lxml libxml2-dev libxslt1-dev
```
* pip install
```shell
$ sudo pip install PyYAML mavproxy
$ sudo pip install dronekit
```

* test
```shell
# using USB to connect
$ sudo mavproxy.py --master=/dev/ttyACM0
# using TELEM2 to connect
$ sudo mavproxy.py --master=/dev/ttyTHS1
```
#### Run our control part
```shell
# using USB to connect
$ sudo python3 control.py 
```
#### using Python may face some error
* TabError: inconsistent use of tabs and spaces in indentation
> In Vim, type `:retab! 4`, and `:x` to solve this issue
