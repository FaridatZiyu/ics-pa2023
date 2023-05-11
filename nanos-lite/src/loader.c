#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_start) - (&ramdisk_end))
extern void ramdisk_read(void *buf, off_t offser, size_t len);

uintptr_t loader(_Protect *as, const char *filename) {
  printf("here.\n");
  ramdisk_read(DEFAULT_ENTRY, 0, RAMDISK_SIZE);
  return (uintptr_t)DEFAULT_ENTRY;
}
