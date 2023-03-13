#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
static int used_next;
static WP* wptemp;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].oldValue = 0;
    wp_pool[i].hitNum = 0;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
  used_next = 0;
}

bool new_wp(char *args) {
  if (free_ == NULL)
    assert(0);
  
  WP* rst = free_;
  free_ = free_->next;

  rst->NO = used_next++;
  rst->next = NULL;
  strcpy(rst->e, args);
  rst->hitNum = 0;

  bool b;
  rst->oldValue = expr(rst->e, &b);
  if (!b) {
    printf("Syntax error.\n");
    return false;
  }

  wptemp = head;
  if (wptemp == NULL) {
    head = rst;
  } else {
    while (wptemp->next != NULL)
      wptemp = wptemp->next;
    wptemp->next = rst;
  }

  printf("Success: set watchpoint %d, oldValue=%d\n", rst->NO, rst->oldValue);
  return true;
}


