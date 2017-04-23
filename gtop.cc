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

int main() {	
  std::thread t(read_tegrastats); 

	initscr();
	noecho();
	timeout(1);

  start_color();
	init_pair(WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);
	init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);

  while (1) {
    {
      std::lock_guard<std::mutex> lk(m);
      ready = true;
    }
    cv.notify_one();

    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, []{ return processed; });
    processed = false;

    // CPU
    display_cpu_stats(0, t_stats);
    
    // GPU
    display_gpu_stats(4, t_stats);

    // Memory
    display_mem_stats(5, t_stats);

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
  std::shared_ptr<FILE> pipe(popen("./tegrastats_fake", "r"), pclose);

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

  get_mem_stats(ts, stats.at(1));
  get_cpu_stats(ts, stats.at(5));
  get_gpu_stats(ts, stats.at(15));

  return ts;
}

void get_cpu_stats(tegrastats & ts, const std::string & str) {
  auto cpu_stats = tokenize(str, '@');
  auto cpu_usage_all = cpu_stats.at(0);
  auto cpu_usage = tokenize(cpu_usage_all.substr(1, cpu_usage_all.size()-1), ',');

  ts.cpu0_usage = std::stoi(cpu_usage.at(0).substr(0, cpu_usage.at(0).size()-1));
  ts.cpu1_usage = std::stoi(cpu_usage.at(1).substr(0, cpu_usage.at(1).size()-1));
  ts.cpu2_usage = std::stoi(cpu_usage.at(2).substr(0, cpu_usage.at(2).size()-1));
  ts.cpu3_usage = std::stoi(cpu_usage.at(3).substr(0, cpu_usage.at(3).size()-1));

  ts.cpu_freq = std::stoi(cpu_stats.at(1));
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
  mvprintw(row, 0, "CPU 0");
  display_bars(row, BAR_OFFSET, ts.cpu0_usage);
  
  mvprintw(row+1, 0, "CPU 1");
  display_bars(row+1, BAR_OFFSET, ts.cpu1_usage);

  mvprintw(row+2, 0, "CPU 2");
  display_bars(row+2, BAR_OFFSET, ts.cpu2_usage);

  mvprintw(row+3, 0, "CPU 3");
  display_bars(row+3, BAR_OFFSET, ts.cpu3_usage);
}

void display_gpu_stats(const int & row, const tegrastats & ts) {
  mvprintw(row, 0, "GPU");
  display_bars(row, BAR_OFFSET, ts.gpu_usage, ts.gpu_freq);
}

void display_mem_stats(const int & row, const tegrastats & ts) {
  mvprintw(row, 0, "Mem");
  display_mem_bars(row, BAR_OFFSET, ts.mem_usage, ts.mem_max);
}
