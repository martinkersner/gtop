// Martin Kersner, m.kersner@gmail.com
// 2017/04/21
// Modified by Zach LaCelle, zlacelle@gmail.com
// 2018/07/23

#include "gtop.hh"
#include <getopt.h>
#include <string>
#include <time.h>

std::mutex m;
std::condition_variable cv;
tegrastats t_stats;
std::string JETSON_TYPE="N/A";
bool WRITE_CSV=false;
FILE* csvFile=nullptr;

bool processed = false;
bool ready     = false;
bool finished  = false;

int main(int argc, char** argv) {
  if (getuid()) {
    std::cout << "gtop requires root privileges!" << std::endl;
    exit(1);
  }

  //Get options
  const char* short_opts = "21kc";
  static struct option long_options[] =
  {
    {"tx2", no_argument, nullptr, '2'},
    {"tx1", no_argument, nullptr, '1'},
    {"tk1", no_argument, nullptr, 'k'},
    {"csv", no_argument, nullptr, 'c'},
    {nullptr, no_argument, nullptr, 0}
  };

  while( 1 )
  {
    const auto opt = getopt_long(argc, argv, short_opts, long_options, nullptr);

    if( opt == -1 )
    {
      break;
    }

    switch(opt)
    {
      case '2':
      {
        JETSON_TYPE="TX2";
        break;
      }
      case '1':
      {
        JETSON_TYPE="TX1";
        break;
      }
      case 'k':
      {
        JETSON_TYPE="TK1";
        break;
      }
      case 'c':
      {
        WRITE_CSV=true;
        break;
      }
      default:
      {
        printf("ERROR: Bad Argument.\n");
        exit(-1);
      }
    }
  } //end: while( 1 )

  if( JETSON_TYPE.compare("N/A") == 0 )
  {
    printf("ERROR: Must have Jetson board type option (--tx2, --tx1, --tk1)\n");
    exit(-1);
  }

  if( WRITE_CSV )
  {
    csvFile = fopen("gtop.csv", "w");
    if( csvFile == nullptr )
    {
      printf("ERROR: Could not open gtop.csv for writing.\n");
      exit(-1);
    }
  }
  
  std::thread t(read_tegrastats); 

  initscr();
  noecho();
  timeout(1);

  start_color();
  init_pair(WHITE_BLACK,   COLOR_WHITE,   COLOR_BLACK);
  init_pair(RED_BLACK,     COLOR_RED,     COLOR_BLACK);
  init_pair(GREEN_BLACK,   COLOR_GREEN,   COLOR_BLACK);
  init_pair(YELLOW_BLACK,  COLOR_YELLOW,  COLOR_BLACK);
  init_pair(BLUE_BLACK,    COLOR_BLUE,    COLOR_BLACK);
  init_pair(MAGENTA_BLACK, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(CYAN_BLACK,    COLOR_CYAN,    COLOR_BLACK);

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

    // CPU, GPU, MEM STATS
    dimensions dim_stats;
    display_stats(dim_stats, t_stats);

    // CPU USAGE CHART
    update_usage_chart(cpu_usage_buffer, t_stats.cpu_usage);
    display_usage_chart(10, cpu_usage_buffer);

    // Write to CSV, if desired
    if( WRITE_CSV )
    {
      write_csv(t_stats);
    }
    
    lk.unlock();

    refresh();

    if (getch() == 'q') // press 'q' or Ctrl-C to quit
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

#ifdef TEGRASTATS_FAKE
  std::shared_ptr<FILE> pipe(popen(TEGRASTATSFAKE_PATH.c_str(), "r"), pclose);
#else
  if (!file_exists(TEGRASTATS_PATH)) {
    std::cerr << "tegrastats was not found at the expected location "
              << TEGRASTATS_PATH << std::endl;
    exit(1); // TODO terminate correctly
  }

  std::shared_ptr<FILE> pipe(popen(TEGRASTATS_PATH.c_str(), "r"), pclose);
#endif

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
      //Maybe in the future, we want to only continue if we get enough stats
      bool foundMem = false;
      bool foundCPU = false;
      bool foundEMC = false;
      bool foundGPU = false;
      t_stats = parse_tegrastats(&foundMem, &foundCPU, &foundEMC, &foundGPU, buffer.data());
      processed = true;

      static bool needCsvHeader = true;
      if( needCsvHeader == true && WRITE_CSV == true )
      {
        int numCpus = t_stats.cpu_usage.size();
        write_csv_header(numCpus);
        needCsvHeader = false;
      }
      
      lk.unlock();
      cv.notify_one();
    }
  }
}

tegrastats parse_tegrastats(bool* _foundMem,
                            bool* _foundCPU,
                            bool* _foundEMC,
                            bool* _foundGPU,
                            const char * _buffer)
{
  tegrastats ts;
  auto stats = tokenize(_buffer, ' ');
  
  if( JETSON_TYPE.compare("TX2") == 0 )
  {
    ts.version = TX2;
  }
  else if( JETSON_TYPE.compare("TX1") == 0 )
  {
    ts.version = TX1;
  }
  else
  {
    ts.version = TK1;
  }

  switch (ts.version) {
    case TX1:
      get_mem_stats(ts, stats.at(1));
      //get_tx1_stats(ts, stats);
      get_cpu_stats_tx1(ts, stats.at(5));
      get_gpu_stats(ts, stats.at(15));
      break;
    case TX2:
      get_tx2_stats(ts, _foundMem, _foundCPU, _foundEMC, _foundGPU, stats);
      break;
    case TK1: // TODO
      break;
  }

  return ts;
}

void get_tx2_stats(tegrastats & ts,
                   bool* _foundMem,
                   bool* _foundCPU,
                   bool* _foundEMC,
                   bool* _foundGPU,
                   const std::vector<std::string> _line)
{
  
  for(auto iter = _line.begin(); iter != _line.end(); ++iter)
  {
    std::string tokstr = *iter;
    if( tokstr.compare("RAM") == 0 )
    {
      ++iter;
      if( iter != _line.end() )
      {
        std::string memstr = *iter;
        get_mem_stats(ts, memstr);
        *_foundMem = true;
      }
    }
    else if( tokstr.compare("CPU") == 0 )
    {
      ++iter;
      if( iter != _line.end() )
      {
        std::string cpustr = *iter;
        get_cpu_stats_tx2(ts, cpustr);
        *_foundCPU = true;
      }
    }
    else if( tokstr.compare("EMC_FREQ") == 0 )
    {
      ++iter;
      if( iter != _line.end() )
      {
        std::string emcstr = *iter;
        get_emc_stats(ts, emcstr);
        *_foundEMC = true;
      }
    }
    else if( tokstr.compare("GR3D_FREQ") == 0 )
    {
      ++iter;
      if( iter != _line.end() )
      {
        std::string gpustr = *iter;
        get_gpu_stats(ts, gpustr);
        *_foundGPU = true;
      }
    }
  } //end: for( ... )  
}

void get_emc_stats(tegrastats & ts, const std::string & str)
{
  auto emc_stats = tokenize(str, '@');
  auto emc_usage = emc_stats.at(0);

  ts.emc_usage = std::stoi(emc_usage.substr(0, emc_usage.size()-1));
  ts.emc_freq = std::stoi(emc_stats.at(1));
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
  const auto cpu_stats = tokenize(str.substr(1, str.size()-1), ',');
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
  const auto gpu_stats = tokenize(str, '@');
  const auto gpu_usage = gpu_stats.at(0);

  ts.gpu_usage = std::stoi(gpu_usage.substr(0, gpu_usage.size()-1));
  ts.gpu_freq = std::stoi(gpu_stats.at(1));
}

void get_mem_stats(tegrastats & ts, const std::string & str) {
  const auto mem_stats = tokenize(str, '/');
  const auto mem_max = mem_stats.at(1);

  ts.mem_usage = std::stoi(mem_stats.at(0));
  ts.mem_max = std::stoi(mem_max.substr(0, mem_max.size()-2));
}

void display_stats(const dimensions & d, const tegrastats & ts) {
  // CPU
  display_cpu_stats(0, ts);
  
  // GPU
  display_gpu_stats(ts.cpu_usage.size(), ts);

  // Memory
  display_mem_stats(1 + ts.cpu_usage.size(), ts);

  // EMC
  display_emc_stats(2 + ts.cpu_usage.size(), ts);
}

void update_usage_chart(std::vector<std::vector<int>> & usage_buffer,
                        const std::vector<int> & usage)
{
  widget w = update_widget_dims(0);

  if (usage_buffer.size() >= w.max_x)
    usage_buffer.erase(usage_buffer.begin());

  usage_buffer.push_back(usage);
}

void write_csv(tegrastats & ts)
{
  char utc[128];
  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(utc,128,"%Y/%m/%d %T",timeinfo);
  puts(utc);
  struct timespec timesp;
  clock_gettime(CLOCK_MONOTONIC, &timesp);
  float monotime = timesp.tv_sec + (timesp.tv_nsec / 1e9);
  fprintf(csvFile, "%s,%f,%d,%d,%d,%d,%d,%d",utc,monotime,ts.mem_usage,ts.mem_max,ts.gpu_usage,ts.gpu_freq,ts.emc_usage,ts.emc_freq);
  int num_cpus = ts.cpu_usage.size();
  for(int i=0; i<num_cpus; ++i)
  {
    fprintf(csvFile, ",%d,%d", ts.cpu_usage[i], ts.cpu_freq[i]);
  }
  fprintf(csvFile, "\n");
  fflush(csvFile);
}

void write_csv_header(int numCpus)
{
  std::ostringstream ss;
  ss << "UTC,Mono Time";
  ss << ",Mem Usage,Mem Max";
  ss << ",GPU Usage,GPU Freq";
  ss << ",EMC Usage,EMC Freq";
  for(int i=0;i<numCpus;++i)
  {
    ss << ",CPU " << i << " Usage,CPU " << i << " Freq";
  }
  ss << std::endl;
  std::string headerStr = ss.str();
  fprintf(csvFile, "%s", headerStr.c_str());
  fflush(csvFile);
}
