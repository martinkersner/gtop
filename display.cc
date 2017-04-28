// Martin Kersner, m.kersner@gmail.com
// 2017/04/23 

#include "display.hh"

void display_bars(const int & row, const int & col, const int & val) {
  auto b = update_bar_dims(val);
  clear_row(row, col);

  display_left_bracket(row, col);
  display_bars(b.val_bar);

  mvprintw(row, col+b.max_bar+1, "%3d%%", val);
  display_right_bracket();

  refresh();
}

void display_bars(const int & row, const int & col, const int & val, const int & freq) {
  auto b = update_bar_dims(val);
  clear_row(row, col);

  display_left_bracket(row, col);
  display_bars(b.val_bar);

  mvprintw(row, col+b.max_bar+1, "%3d%%", val);
  display_right_bracket();
  printw(" %d", freq);

  refresh();
}

void display_mem_bars(const int & row, const int & col, const int & val, const int & max_val) {
  auto val_norm = int((float(val) / max_val) * 100);
  auto b = update_bar_dims(val_norm);

  clear_row(row, col);

  display_left_bracket(row, col);
  display_bars(b.val_bar);

  char buffer[MEM_BUFFER_SIZE];
  sprintf(buffer, "%2.2fG/%2.2fG", mega2giga(val), mega2giga(max_val));
  mvprintw(row, col+b.max_bar-6, buffer);
  display_right_bracket();
  refresh();
}

// deprecated move to widget
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

widget update_widget_dims(const int & val) {
  widget w;

  if (COLS < WIDGET_MAX_COLS_INT8) {
    w.max_x = int(COLS*WIDGET_WIDTH);
    w.val_x = int((val/WIDGET_MAX_COLS_F32)*w.max_x);
  }
  else {
    w.max_x = BAR_MAX_COLS_INT8;
    w.val_x = val;
  }

  return w;
}

float mega2giga(const int & mega_val) {
  return mega_val/float(1000);
}

void clear_row(const int & row, const int & col) {
  mvprintw(row, col, std::string(COLS, ' ').c_str());
  refresh();
}

void display_bars(const int & size) {
  attron(COLOR_PAIR(GREEN_BLACK));
  printw(std::string(size, '|').c_str());
  attroff(COLOR_PAIR(GREEN_BLACK));
}

void display_left_bracket(const int & row, const int & col) {
  attron(A_BOLD);
  mvprintw(row, col, "[");
  attroff(A_BOLD);
}

void display_right_bracket() {
  attron(A_BOLD);
  printw("]");
  attroff(A_BOLD);
}
