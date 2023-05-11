#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  memcpy(&t1, &cpu.eflags, sizeof(cpu.eflags));
  rtl_li(&t0, t1);
  rtl_push(&t0);
  rtl_push(&cpu.cs);
  rtl_li(&t0, ret_addr);
  rtl_push(&t0);
  
  vaddr_t gate_addr = cpu.idtr.base+NO*sizeof(GateDesc);
  assert(gate_addr<=cpu.idtr.base+cpu.idtr.limit);

  uint32_t off_15_0 = vaddr_read(gate_addr, 2);
  uint32_t off_32_16 = vaddr_read(gate_addr+sizeof(GateDesc)-2, 2);
  uint32_t target_addr = (off_32_16<<16) + off_15_0;
#ifdef DEBUG
  Log("target_addr=0x%x", target_addr);
#endif

  decoding.is_jmp = 1;
  decoding.jmp_eip = target_addr;
}

void dev_raise_intr() {
}
