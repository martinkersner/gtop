// Martin Kersner, m.kersner@gmail.com
// 2017/04/21

#include "gtop.hh"

int main()
{	
	std::thread first (read_tegrastats); 
	first.detach();

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
  std::array<char, 128> buffer;
  std::string result;
  std::shared_ptr<FILE> pipe(popen("./tegrastats_fake", "r"), pclose);
  if (!pipe) throw std::runtime_error("popen() failed!");
  while (!feof(pipe.get())) {
    if (fgets(buffer.data(), 128, pipe.get()) != NULL){

      std::vector<std::string> result;
      std::stringstream ss(buffer.data());

      while( ss.good() )
      {
        std::string substr;
        getline( ss, substr, ' ' );
        result.push_back( substr );
      }

      std::cout << result[15] << std::endl;
    }
  }
}
