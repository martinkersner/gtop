all:
	g++ -std=c++14 gtop.cc utils.cc display.cc -o gtop -lncurses -lpthread
