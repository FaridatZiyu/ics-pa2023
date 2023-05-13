#include "common.h"
#include "syscall.h"

static inline _RegSet* sys_none(_RegSet *r) {
  panic("sys_none");
  SYSCALL_ARG1(r) = 1;
  return NULL;
}

static inline _RegSet* sys_exit(_RegSet *r) {
  panic("sys_exit");
  _halt(SYSCALL_ARG2(r));
  return NULL;
}

static inline _RegSet* sys_write(_RegSet *r) {
  panic("sys_write");
  int fd = (int)SYSCALL_ARG2(r);
  char* buf = (char*)SYSCALL_ARG3(r);
  size_t count = (size_t)SYSCALL_ARG4(r);
  if (fd == 1 || fd == 2) {
    for(int i = 0; i < count; i++){
      _putc(buf[i]);
    }
    SYSCALL_ARG1(r) = count;
  } else {
    panic("Unhandled fd = %d in sys_write.\n", fd);
    SYSCALL_ARG1(r) = -1;
  }
  return NULL; 
}

_RegSet* do_syscall(_RegSet *r) {
  switch (SYSCALL_ARG1(r)) {
  case SYS_none:  return sys_none(r);
  case SYS_exit:  return sys_exit(r);
  case SYS_write: return sys_write(r);
  default:
    panic("Unhandled syscall ID = %d", SYSCALL_ARG1(r));
  }

  return NULL;
}
