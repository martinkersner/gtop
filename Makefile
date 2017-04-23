all:
	g++ -std=c++14 gtop.cc utils.cc display.cc -o gtop -pedantic -Wall -Wextra -lncurses -lpthread

travis:
	$(CXX) -std=c++14 gtop.cc utils.cc display.cc -o gtop -pedantic -Wall -Wextra -lncurses -lpthread
