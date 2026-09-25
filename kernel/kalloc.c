// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void *)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if (r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if (r)
    memset((char *)r, 5, PGSIZE); // fill with junk
  return (void *)r;
}

// ---------------------------------------------------------------------------
// Proyecto 2 (sysinfo).
// ---------------------------------------------------------------------------

// Retorna el numero de paginas fisicas de 4 KB libres, recorriendo
// kmem.freelist con kmem.lock tomado para que la lista no cambie a mitad.
uint64
free_pages(void)
{
  uint64 count = 0;
  struct run *r;

  acquire(&kmem.lock);

  for (r = kmem.freelist; r != 0; r = r->next) {
    count++;
  }

  release(&kmem.lock);

  return count;
}

// Retorna el numero total de paginas fisicas que administra este asignador:
// las que kinit() le entrego, desde el final del kernel (redondeado a pagina,
// igual que en freerange) hasta PHYSTOP. No incluye las paginas que ocupa el
// propio kernel. Las usadas se calculan como total - libres en sys_sysinfo,
// con un solo conteo de libres, para que usadas + libres = total siempre.
uint64
total_pages(void)
{
  return (PHYSTOP - PGROUNDUP((uint64)end)) / PGSIZE;
}
