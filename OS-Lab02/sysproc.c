#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

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
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
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
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


int
sys_hello(){
  hello();
  return 0;
}

int
sys_helloYou(){
  char* name;
  if(argstr(0, &name) < 0)
    return -1;
  helloYou(name);
  return 0;
}

int
sys_getNumProc(){
  return getNumProc();  
}

int
sys_getMaxPid(){
  return getMaxPid();
}

int
sys_getProcInfo(){
  int pid;
  int p_pinfo;
  if(argint(0, &pid) < 0) return -1;
  if(argint(1, &p_pinfo) < 0) return -1;
  struct processInfo* pinfo = (struct processInfo*) p_pinfo;
  return getProcInfo(pid, pinfo);
}

int 
sys_welcomeFunction(){
  int addr;
  if(argint(0, &addr) < 0)
    return -1;
  
  void (*wlcm)(void) = (void (*)(void)) addr;
  return welcomeFunction(wlcm);
}

int 
sys_welcomeDone(){
  return welcomeDone();
}