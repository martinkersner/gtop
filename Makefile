all:
	g++ -std=c++14 gtop.cc utils.cc -o gtop -lncurses -lpthread
