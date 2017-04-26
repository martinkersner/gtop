// Martin Kersner, m.kersner@gmail.com
// 2017/04/21

#ifndef GTOP_HH_
#define GTOP_HH_

#include <ncurses.h>
#include <thread>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <iostream>
#include <algorithm>

enum jetson_version {TK1, TX1, TX2};

const int STATS_BUFFER_SIZE = 256;
const int BAR_OFFSET = 6;

struct tegrastats {
  int mem_usage;
  int mem_max;

  std::vector<int> cpu_usage;
  std::vector<int> cpu_freq;

  int gpu_usage;
  int gpu_freq;
};

void read_tegrastats();
tegrastats parse_tegrastats(const char *);

void get_cpu_stats_tx1(tegrastats &, const std::string &);
void get_cpu_stats_tx2(tegrastats &, const std::string &);
void get_gpu_stats(tegrastats &, const std::string &);
void get_mem_stats(tegrastats &, const std::string &);

void display_cpu_stats(const int &, const tegrastats &);
void display_gpu_stats(const int &, const tegrastats &);
void display_mem_stats(const int &, const tegrastats &);
void display_usage_chart(const int &, const std::vector<std::vector<int>>);

#endif // GTOP_HH_
