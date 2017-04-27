// Martin Kersner, m.kersner@gmail.com
// 2017/04/23 

#ifndef DISPLAY_HH_
#define DISPLAY_HH_

#include <ncurses.h>
#include <string>

enum colors {RED_BLACK=1,
             GREEN_BLACK,
             YELLOW_BLACK,
             BLUE_BLACK,
             MAGENTA_BLACK,
             CYAN_BLACK,
             WHITE_BLACK};

// bar DEPRECATED, move on widget
const int BAR_MAX_COLS_INT8  = 100;
const float BAR_MAX_COLS_F32 = 100.0;
const float BAR_AREA         = 0.7;

// widget
const int WIDGET_MAX_COLS_INT8  = 100;
const float WIDGET_MAX_COLS_F32 = 100.0;
const float WIDGET_WIDTH     = 0.7;

const int MEM_BUFFER_SIZE    = 20;
const float MEGA2GIGA        = 1000.0;

struct bar {
  int max_bar;
  int val_bar;
};

struct widget {
  int max_x;
  int val_x;
  int max_y;
  int val_y;
};

void display_bars(const int &, const int &, const int &);
void display_bars(const int &, const int &, const int &, const int &);
void display_mem_bars(const int &, const int &, const int &, const int &);
float mega2giga(const int &);
bar update_bar_dims(const int &);
widget update_widget_dims(const int &);

#endif // DISPLAY_HH_
