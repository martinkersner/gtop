// Martin Kersner, m.kersner@gmail.com
// 2017/04/21

#include "gtop.hh"
#include "utils.hh"
#include "display.hh"

std::mutex m;
std::condition_variable cv;
tegrastats t_stats;

bool processed = false;
bool ready     = false;
bool finished  = false;

int version;

int main() {	
  if (getuid()) {
    std::cout << "gtop requires root privileges!" << std::endl;
    exit(1);
  }

  std::thread t(read_tegrastats); 

  initscr();
  noecho();
  timeout(1);

  start_color();
  init_pair(WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);
  init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);
  init_pair(RED_BLACK, COLOR_RED, COLOR_BLACK);
  init_pair(BLUE_BLACK, COLOR_BLUE, COLOR_BLACK);

  std::vector<std::vector<int>> cpu_usage_buffer;

  while (1) {
    {
      std::lock_guard<std::mutex> lk(m);
      ready = true;
    }
    cv.notify_one();

    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, []{ return processed; });
    processed = false;

    cpu_usage_buffer.push_back(t_stats.cpu_usage);
    display_usage_chart(10, cpu_usage_buffer);

    // CPU
    display_cpu_stats(0, t_stats);
    
    // GPU
    display_gpu_stats(t_stats.cpu_usage.size(), t_stats);

    // Memory
    display_mem_stats(t_stats.cpu_usage.size()+1, t_stats);

    lk.unlock();

    refresh();

    if (getch() == 'q')
      break;
  }

  { 
    std::lock_guard<std::mutex> lk(m);
    finished = true;
  }

  t.join();
  endwin();

  return 0;
}

void read_tegrastats() {
  std::array<char, STATS_BUFFER_SIZE> buffer;
  std::shared_ptr<FILE> pipe(popen("~/tegrastats", "r"), pclose);
  //std::shared_ptr<FILE> pipe(popen("./tegrastats_fake", "r"), pclose);

  if (!pipe)
    throw std::runtime_error ("popen() failed!");

  while (!feof(pipe.get())) {

    if (fgets(buffer.data(), STATS_BUFFER_SIZE, pipe.get()) != NULL) {
      std::unique_lock<std::mutex> lk(m);

      // terminate reading tegrastats
      if (finished) {
        lk.unlock();
        break;
      }

      cv.wait(lk, []{ return ready; });
      ready = false;
      t_stats = parse_tegrastats(buffer.data());
      processed = true;
      lk.unlock();
      cv.notify_one();
    }
  }
}

tegrastats parse_tegrastats(const char * buffer) {
  tegrastats ts;
  auto stats = tokenize(buffer, ' ');

  if (stats.size() >= 15)
    version = TX1;
  else
    version = TX2;

  get_mem_stats(ts, stats.at(1));

  switch (version) {
    case TX1:
      get_cpu_stats_tx1(ts, stats.at(5));
      get_gpu_stats(ts, stats.at(15));
      break;
    case TX2:
      get_cpu_stats_tx2(ts, stats.at(5));
      get_gpu_stats(ts, stats.at(13));
      break;
  }

  return ts;
}

void get_cpu_stats_tx1(tegrastats & ts, const std::string & str) {
  auto cpu_stats = tokenize(str, '@');
  auto cpu_usage_all = cpu_stats.at(0);
  ts.cpu_freq.push_back(std::stoi(cpu_stats.at(1)));
  auto cpu_usage = tokenize(cpu_usage_all.substr(1, cpu_usage_all.size()-2), ',');

  for (const auto & u: cpu_usage) {
    if (u != "off")
      ts.cpu_usage.push_back(std::stoi(u.substr(0, u.size()-1)));
    else
      ts.cpu_usage.push_back(0);
  }
}

void get_cpu_stats_tx2(tegrastats & ts, const std::string & str) {
  auto cpu_stats = tokenize(str.substr(1, str.size()-1), ',');
  const auto at = std::string("@");

  for (const auto & u: cpu_stats) {
    if (u != "off") {
      std::size_t found = at.find(u);
      if (found == std::string::npos) {
        auto cpu_info = tokenize(u, at.c_str()[0]);
        ts.cpu_usage.push_back(std::stoi(cpu_info.at(0).substr(0, cpu_info.at(0).size()-1)));
        ts.cpu_freq.push_back(std::stoi(cpu_info.at(1)));
      }
    }
    else {
      ts.cpu_usage.push_back(0);
      ts.cpu_freq.push_back(0);
    }
  }
}

void get_gpu_stats(tegrastats & ts, const std::string & str) {
  auto gpu_stats = tokenize(str, '@');
  auto gpu_usage = gpu_stats.at(0);

  ts.gpu_usage = std::stoi(gpu_usage.substr(0, gpu_usage.size()-1));
  ts.gpu_freq = std::stoi(gpu_stats.at(1));
}

void get_mem_stats(tegrastats & ts, const std::string & str) {
  auto mem_stats = tokenize(str, '/');
  auto mem_max = mem_stats.at(1);

  ts.mem_usage = std::stoi(mem_stats.at(0));
  ts.mem_max = std::stoi(mem_max.substr(0, mem_max.size()-2));
}

void display_cpu_stats(const int & row, const tegrastats & ts) {
  int idx = 0;
  for (const auto & u : ts.cpu_usage) {
    auto cpu_label = std::string("CPU ") + std::to_string(idx);
    mvprintw(row+idx, 0, cpu_label.c_str());
    if (version == TX1)
      display_bars(row+idx, BAR_OFFSET, u, ts.cpu_freq.at(0));
    else if (version == TX2)
      display_bars(row+idx, BAR_OFFSET, u, ts.cpu_freq.at(idx));
    idx++;
  }
}

void display_gpu_stats(const int & row, const tegrastats & ts) {
  mvprintw(row, 0, "GPU");
  display_bars(row, BAR_OFFSET, ts.gpu_usage, ts.gpu_freq);
}

void display_mem_stats(const int & row, const tegrastats & ts) {
  mvprintw(row, 0, "Mem");
  display_mem_bars(row, BAR_OFFSET, ts.mem_usage, ts.mem_max);
}

void display_usage_chart(const int & row, const std::vector<std::vector<int>> cpu_usage_buffer) {
  int col = cpu_usage_buffer.size();
  int idx = 1;

  int max_size = std::min(LINES, 80);

  // display scale
  mvprintw(row, 0, "-"); 
  mvprintw(row+max_size, 0, "-"); 

  for (const auto & timepoint : cpu_usage_buffer) {
    for (const auto & cpu_usage : timepoint) {
      if (idx == 2 || idx == 3)
        continue;
      else if (idx == 5)
        break;
       
      int tmp_cpu_usage = int((max_size/100.0)*cpu_usage);
      attron(COLOR_PAIR(idx+1));
      mvprintw(tmp_cpu_usage+row, col, "*"); 
      attroff(COLOR_PAIR(idx+1));

      idx++;
    }

    col--;
    idx = 1;
  }
  refresh();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  for (int row = 0; row < LINES; ++row) {
    mvprintw(row, 0, std::string(COLS, ' ').c_str()); 
  }
  refresh();
}
