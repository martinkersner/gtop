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

#include "display.hh"
#include "utils.hh"

const int STATS_BUFFER_SIZE = 512;

const std::string TEGRASTATS_PATH     = "~/tegrastats";
const std::string TEGRASTATSFAKE_PATH = "./tegrastats_fake";

void read_tegrastats();
tegrastats parse_tegrastats(const char *);

void get_cpu_stats_tx1(tegrastats &, const std::string &);
void get_cpu_stats_tx2(tegrastats &, const std::string &);
void get_gpu_stats(tegrastats &, const std::string &);
void get_mem_stats(tegrastats &, const std::string &);

void display_stats(const dimensions &, const tegrastats &);
void update_usage_chart(std::vector<std::vector<int>> &, const std::vector<int> &);

#endif // GTOP_HH_
