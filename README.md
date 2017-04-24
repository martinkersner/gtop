
[![Build Status](https://travis-ci.org/martinkersner/gtop.svg?branch=master)](https://travis-ci.org/marmtinkersner/gtop)

# gtop
Martin Kersner, <m.kersner@gmail.com>, 2017

## Description
`gtop` is CPU, GPU and memory viewer utilizing information provided by `tegrastats` (terminal utility for [NVIDIA<sup>&reg;</sup> JETSON<sup>&trade;</sup>](http://www.nvidia.com/object/embedded-systems-dev-kits-modules.html) embedded platform). It requires `ncurses` and its output resembles [`htop`](https://github.com/hishamhm/htop).

<p align=center>
<img src="http://i.imgur.com/oMHuVSX.png" align="center"/>
</p>

## Prerequisites
```
sudo apt-get install libncurses5-dev libncursesw5-dev
```

## Installation instruction
```
https://github.com/martinkersner/gtop.git
cd gtop
make
sudo ./gtop
```

It is recommended to create alias for `gtop` so it can be used from any directory. Add following line to your *.bashrc* file
```
alias gtop="sudo ./$PATH_TO_GTOP_DIRECTORY/gtop"
```
 and don't forget to replace `$PATH_TO_GTOP_DIRECTORY`.

## License

GNU General Public License, version 3 (GPL-3.0)
