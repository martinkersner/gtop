// Martin Kersner, m.kersner@gmail.com
// 2017/04/23 

#include "display.hh"

void display_bars(const int row, const int col, const int val) {
  // clear previous bars
  mvprintw(row, col, std::string(BAR_MAX, ' ').c_str());
	refresh();

  mvprintw(row, col, "[");
	attron(COLOR_PAIR(GREEN_BLACK));
  mvprintw(row, col+1, std::string(val, '|').c_str());
	attron(COLOR_PAIR(WHITE_BLACK));
  mvprintw(row, col+BAR_MAX+1, "]");
	refresh();
}
