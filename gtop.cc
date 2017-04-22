// Martin Kersner, m.kersner@gmail.com
// 2017/04/21

#include "gtop.hh"
#include "utils.hh"

int main() {	
	read_tegrastats();
	//std::thread t(read_tegrastats); 
	//t.detach();
  exit(1);

	initscr();
	noecho();
	timeout(1);

	int i = 0;
	while (1) {
		mvprintw(0, 0, "Hello World %d", i);
		refresh();
		++i;
		auto ch = getch();
		if (ch > 0)
			break;
	}

	endwin();

	return 0;
}

void read_tegrastats() {
  std::array<char, BUFFER_SIZE> buffer;
  std::shared_ptr<FILE> pipe(popen("./tegrastats_fake", "r"), pclose);

  if (!pipe)
    throw std::runtime_error ("popen() failed!");

  while (!feof(pipe.get())) {
    if (fgets(buffer.data(), BUFFER_SIZE, pipe.get()) != NULL) {
      tegrastats ts = parse_tegrastats(buffer.data());
    }
  }
}

tegrastats parse_tegrastats(const char * buffer) {
  tegrastats ts;
  auto stats = tokenize(buffer, ' ');

  get_ram_stats(ts, stats.at(1));
  get_cpu_stats(ts, stats.at(5));
  get_gpu_stats(ts, stats.at(15));

  //std::cout << ts.cpu0_usage << std::endl;
  //std::cout << ts.cpu1_usage << std::endl;
  //std::cout << ts.cpu2_usage << std::endl;
  //std::cout << ts.cpu3_usage << std::endl;
  //std::cout << ts.cpu_freq << std::endl;
  
  //std::cout << ts.gpu_usage << std::endl;
  //std::cout << ts.gpu_freq << std::endl;
  
  //std::cout << ts.ram_usage << std::endl;
  //std::cout << ts.ram_max << std::endl;

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

void get_ram_stats(tegrastats & ts, const std::string & str) {
  auto ram_stats = tokenize(str, '/');
  auto ram_max = ram_stats.at(1);

  ts.ram_usage = std::stoi(ram_stats.at(0));
  ts.ram_max = std::stoi(ram_max.substr(0, ram_max.size()-2));
}
