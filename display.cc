// Martin Kersner, m.kersner@gmail.com
// 2017/04/23 

#include "display.hh"

void display_bars(const int & row, const int & col, const int & val) {
  // clear previous bars
  mvprintw(row, col, std::string(COLS, ' ').c_str());
  refresh();

  auto b = update_bar_dims(val);

  // display current bars
  mvprintw(row, col, "[");
  attron(COLOR_PAIR(GREEN_BLACK));
  mvprintw(row, col+1, std::string(b.val_bar, '|').c_str());
  attron(COLOR_PAIR(WHITE_BLACK));
  mvprintw(row, col+b.max_bar+1, "%3d%%]", val);
  refresh();
}

void display_bars(const int & row, const int & col, const int & val, const int & freq) {
  // clear previous bars
  mvprintw(row, col, std::string(COLS, ' ').c_str());
  refresh();

  auto b = update_bar_dims(val);

  // display current bars
  mvprintw(row, col, "[");
  attron(COLOR_PAIR(GREEN_BLACK));
  mvprintw(row, col+1, std::string(b.val_bar, '|').c_str());
  attron(COLOR_PAIR(WHITE_BLACK));
  mvprintw(row, col+b.max_bar+1, "%3d%%] %d", val, freq);
  refresh();
}

void display_mem_bars(const int & row, const int & col, const int & val, const int & max_val) {
  // clear previous bars
  mvprintw(row, col, std::string(COLS, ' ').c_str());
  refresh();

  auto val_norm = int((float(val) / max_val) * 100);
  auto b = update_bar_dims(val_norm);

  // display current bars
  mvprintw(row, col, "[");
  attron(COLOR_PAIR(GREEN_BLACK));
  mvprintw(row, col+1, std::string(b.val_bar, '|').c_str());
	attron(COLOR_PAIR(WHITE_BLACK));

  char buffer[MEM_BUFFER_SIZE];
  sprintf(buffer, "%2.2fG/%2.2fG]", mega2giga(val), mega2giga(max_val));
  mvprintw(row, col+b.max_bar-6, buffer);
  refresh();
}

bar update_bar_dims(const int & val) {
  bar b;

  if (COLS < BAR_MAX_COLS_INT8) {
    b.max_bar = int(COLS*BAR_AREA);
    b.val_bar = int((val/BAR_MAX_COLS_F32)*b.max_bar);
  }
  else {
    b.max_bar = BAR_MAX_COLS_INT8;
    b.val_bar = val;
  }

  return b;
}

float mega2giga(const int & mega_val) {
  return float(mega_val)/1000;
}
