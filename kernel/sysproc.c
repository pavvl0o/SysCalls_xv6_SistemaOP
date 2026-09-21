#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// ---------------------------------------------------------------------------
// Proyecto 2 - andamiaje. Cuerpos vacios: solo registran la syscall.
// ---------------------------------------------------------------------------

// Proyecto 2: trace(nombre) empieza a rastrear la syscall indicada para este
// proceso. El argumento es una cadena en memoria de usuario ("sys_kill"), no un
// numero, tal como pide el enunciado.
// Retorna 0 si quedo rastreando, -1 si el nombre no existe o no se pudo leer.
uint64
sys_trace(void)
{
  char name[32]; // el nombre mas largo de la tabla es "sys_sysinfo"
  int num;

  // argstr copia la cadena desde el espacio de usuario a memoria del kernel.
  // Falla si el puntero es invalido o si el nombre no cabe en el buffer.
  if (argstr(0, name, sizeof(name)) < 0)
    return -1;

  if ((num = syscall_by_name(name)) < 0)
    return -1;

  myproc()->trace_num = num;
  return 0;
}

// TODO(Dev 3): armar un struct sysinfo con los datos que entregan free_pages()
// y count_runnable(), y copiarlo al puntero de usuario con copyout().
// Debe retornar -1 si copyout() falla.
uint64
sys_sysinfo(void)
{
  return 0;
}
