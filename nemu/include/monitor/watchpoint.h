#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  int oldValue;
  char e[32];
  int hitNum;
} WP;

bool new_wp(char *args);


#endif
