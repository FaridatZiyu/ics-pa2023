#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

#define PDX(va)     (((uint32_t)(va) >> 22) & 0x3ff)
#define PTX(va)     (((uint32_t)(va) >> 12) & 0x3ff)
#define OFF(va)     ((uint32_t)(va) & 0xfff)
#define PTE_ADDR(pte)    ((uint32_t)(pte) & ~0xfff)
// Page table/directory entry flags
#define PTE_P     0x001     // Present
#define PTE_A     0x020     // Accessed
#define PTE_D     0x040     // Dirty
paddr_t page_translate(vaddr_t addr, bool write) {
  PDE pde;
  PTE pte, *ptdir;
  if (cpu.cr0.protect_enable && cpu.cr0.paging) {
    PDE *pgdir = (PDE*)(PTE_ADDR(cpu.cr3.val));
    pde.val = paddr_read((paddr_t) &pgdir[PDX(addr)], 4);
    assert(pde.present);
    pde.accessed = 1;
    ptdir = (PTE*)(PTE_ADDR(pde.val));
    pte.val = paddr_read((paddr_t)&ptdir[PTX(addr)], 4);
    assert(pte.present);
    pte.accessed = 1;
    pte.dirty = write ? 1 : pte.dirty;
    return PTE_ADDR(pte.val) | OFF(addr);
  }
  return addr;
}

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int r = is_mmio(addr);
  if (r == -1)
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  else
    return mmio_read(addr, len, r);
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int r = is_mmio(addr);
  if (r == -1)
    memcpy(guest_to_host(addr), &data, len);
  else
    mmio_write(addr, len, data, r);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if (((addr+len-1) & ~0xfff) != (addr & ~0xfff)) {
    assert(0);
  } else {
    paddr_t paddr = page_translate(addr, false);
    return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if (((addr+len-1) & ~0xfff) != (addr & ~0xfff)) {
    assert(0);
  }
  else {
    paddr_t paddr = page_translate(addr, true);
    paddr_write(paddr, len, data);
  }
}
