// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "spinlock.h"

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;

  /*
  P5 changes
  */
  uint free_pages; //track free pages
  uint ref_cnt[PHYSTOP / PGSIZE]; //track reference count

} kmem;

extern char end[]; // first address after kernel loaded from ELF file

// Helper function to increment ref_cnt
void increment_ref_cnt(uint pte){
  acquire(&kmem.lock);
  kmem.ref_cnt[(int)pte >> 12] ++;
  release(&kmem.lock);
}

// Helper function to decrement ref_cnt
void decrement_ref_cnt(uint pte){
  acquire(&kmem.lock);
  kmem.ref_cnt[(int)pte >> 12] --;
  release(&kmem.lock);
}

uint getRefCount(uint pte){
  return kmem.ref_cnt[(int)pte >> 12];
}

// Initialize free list of physical pages.
void
kinit(void)
{
  char *p;
  initlock(&kmem.lock, "kmem");
  p = (char*)PGROUNDUP((uint)end);
  for(; p + PGSIZE <= (char*)PHYSTOP; p += PGSIZE){
    kfree(p);
    kmem.ref_cnt[(int)p >> 12] = 0;
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || (uint)v >= PHYSTOP) 
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  
  kmem.ref_cnt[(int)r >> 12] --;
  // cprintf("Ref count: %d\tAddr: %d\n",kmem.ref_cnt[(int)r >> 12], (int)r);

  kmem.free_pages++;

  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;
  acquire(&kmem.lock);
  r = kmem.freelist;

  if(r){
    // if(kmem.ref_cnt[(int)r >> 12] == 0)
      kmem.ref_cnt[(int)r >> 12] ++;

    kmem.freelist = r->next;

    kmem.free_pages --;
  }
  release(&kmem.lock);
  return (char*)r;
}

int getFreePagesCount(void){
  return kmem.free_pages;
}
