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

bool free_wp(int num) {
  WP* thewp = NULL;
  if (head == NULL) {
    printf("No watchpoint now\n");
    return false;
  }

  if (head->NO == num) {
    thewp = head;
    head = head->next;
  } else {
    wptemp = head;
    while (wptemp!=NULL && wptemp->next!=NULL) {
      if (wptemp->next->NO == num) {
        thewp = wptemp->next;
        wptemp->next = wptemp->next->next;
        break;
      }
      wptemp = wptemp->next;
    }
  }

  if (thewp != NULL) {
    thewp->next = free_;
    free_ = thewp;
    return true;
  }
  return false;
}

void print_wp() {
  if (head == NULL) {
    printf("No watchpoint now\n");
    return;
  }
  printf("Watchpoints:\n");
  printf("NO.   expr            hitTimes\n");
  wptemp = head;
  while (wptemp) {
    printf("%d     %s          %d\n", wptemp->NO, wptemp->e, wptemp->hitNum);
    wptemp = wptemp->next;
  }
}

bool watch_wp() {
  bool success;
  int rst;
  if (head == NULL)
    return true;
  
  wptemp = head;
  while (wptemp) {
    rst = expr(wptemp->e, &success);
    if (rst != wptemp->oldValue) {
      wptemp->hitNum += 1;
      printf("Hardware watchpoint %d:%s\n", wptemp->NO, wptemp->e);
      printf("Old value:%d\nNew value:%d\n\n", wptemp->oldValue, rst);
      wptemp->oldValue = rst;
      return false;
    }
    wptemp = wptemp->next;
  }
  return true;
}


