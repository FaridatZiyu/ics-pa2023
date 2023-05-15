#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  strncpy(buf, dispinfo+offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  int index = offset / 4;
  int screen_y1 = index / _screen.width;
  int screen_x1 = index % _screen.width;
  index = (offset+len)/4;
  int screen_y2 = index / _screen.width;
  if(screen_y2 == screen_y1){  
    _draw_rect(buf, screen_x1, screen_y1, len/4, 1);
    return;
  }

  int tempw = _screen.width - screen_x1;
  if(screen_y2 - screen_y1 == 1) {
    _draw_rect(buf, screen_x1, screen_y1, tempw, 1);
    _draw_rect(buf+tempw*4, 0, screen_y2, len/4-tempw, 1);
    return;
  }
  
  _draw_rect(buf, screen_x1, screen_y1, tempw, 1);
  int tempy = screen_y2 - screen_y1 - 1;
  _draw_rect(buf+tempw*4, 0, screen_y1+1, _screen.width, tempy);
  _draw_rect(buf+tempw*4+tempy*_screen.width*4, 0, screen_y2, len/4-tempw-tempy*_screen.width, 1);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
