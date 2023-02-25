#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"
// CS537 - SP2022 - P4 additions
#include "pstat.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Written by Deepak for CS537 - P2
int sys_getnextpid(void){
  return getnextpid();
}

int sys_getprocstate(void){
  int pid, n;
  char *state = NULL;

  /*
    First argument of argint & argptr is the nth argument in the system call
    the 2nd argument in argptr is the pointer of size 'n', the 3rd argument
  */
  if(argint(0, &pid) < 0 || argint(2, &n) < 0 || argptr(1, (void *)&state, n) < 0)
    return -1;
  
  if(pid < 0 || n < 0)
    return -1;

  return getprocstate(pid, state, n);
}

// Written by Deepak for CS537 - P4
int sys_settickets(void){
  int ticketNumber = 0;
  if(argint(0, &ticketNumber) < 0)
    return -1;

  if(ticketNumber < 1)
    return -1;
  return settickets(ticketNumber);
}

int sys_getpinfo(void){
  return 0;
}