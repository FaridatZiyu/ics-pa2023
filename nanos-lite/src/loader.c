#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

void _map(_Protect *p, void *va, void *pa);
void* new_page(void);

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))
extern void ramdisk_read(void *buf, off_t offset, size_t len);

uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  int size = fs_filesz(fd);
  void *pa, *va = DEFAULT_ENTRY;
  Log("filename=%s, fd=%d", filename, fd);
  while(size > 0) {
    pa = new_page();
    _map(as, va, pa);
    size_t len = size >= PGSIZE ? PGSIZE : size;
    fs_read(fd, pa, len);
    va += PGSIZE;
    size -= PGSIZE;
  }
  // fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
