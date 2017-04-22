// Martin Kersner, m.kersner@gmail.com
// 2017/04/21

#ifndef GTOP_HH_
#define GTOP_HH_

#include <ncurses.h>
#include <thread>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <vector>
#include <sstream>

#define BUFFER_SIZE 128

struct tegrastats {
  int ram_usage;
  int ram_max;

  int cpu0_usage;
  int cpu1_usage;
  int cpu2_usage;
  int cpu3_usage;
  int cpu_freq;

  int gpu_usage;
  int gpu_freq;
};

void read_tegrastats();
tegrastats parse_tegrastats(const char *);
void get_cpu_stats(tegrastats &, const std::string &);
void get_gpu_stats(tegrastats &, const std::string &);
void get_ram_stats(tegrastats &, const std::string &);

#endif // GTOP_HH_
