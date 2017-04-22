// Martin Kersner, m.kersner@gmail.com
// 2017/04/23 

#ifndef DISPLAY_HH_
#define DISPLAY_HH_

#include <ncurses.h>
#include <string>

const int WHITE_BLACK = 1;
const int GREEN_BLACK = 2;

const int BAR_MAX = 100;

void display_bars(const int, const int, const int);

#endif // DISPLAY_HH_
