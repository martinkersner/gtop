# gtop
Martin Kersner, <m.kersner@gmail.com>, 2017

## Description
`gtop` is CPU, GPU and memory viewer utilizing information provided by tegrastats (terminal utility for [NVIDIA JETSON](http://www.nvidia.com/object/embedded-systems-dev-kits-modules.html) embedded platform). It requires `ncurses` and its output resembles [`htop`](https://github.com/hishamhm/htop).

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
