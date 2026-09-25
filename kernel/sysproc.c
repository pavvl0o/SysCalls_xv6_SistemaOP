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
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
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
// Proyecto 2: syscalls trace y sysinfo.
// ---------------------------------------------------------------------------

// Proyecto 2: trace(nombre) empieza a rastrear la syscall indicada para este
// proceso. El argumento es una cadena en memoria de usuario ("sys_kill"), no un
// numero, tal como pide el enunciado.
// Con un puntero nulo, trace(0), se desactiva el rastreo: lo usa user/trace.c
// cuando exec falla, para que su mensaje de error no quede rastreado.
// Retorna 0 si quedo rastreando (o desactivado), -1 si el nombre no existe o
// no se pudo leer.
uint64
sys_trace(void)
{
  char name[32]; // el nombre mas largo de la tabla es "sys_sysinfo"
  int num;
  uint64 addr;

  argaddr(0, &addr);
  if (addr == 0) {
    myproc()->trace_num = 0;
    return 0;
  }

  // argstr copia la cadena desde el espacio de usuario a memoria del kernel.
  // Falla si el puntero es invalido o si el nombre no cabe en el buffer.
  if (argstr(0, name, sizeof(name)) < 0)
    return -1;

  if ((num = syscall_by_name(name)) < 0)
    return -1;

  myproc()->trace_num = num;
  return 0;
}

// Proyecto 2: sysinfo(&info) llena un struct sysinfo con el estado actual del
// sistema y lo copia al puntero de usuario con copyout().
// Retorna 0 si todo salio bien, -1 si copyout() falla (puntero invalido).
uint64
sys_sysinfo(void)
{
  struct sysinfo info;
  uint64 addr; // dirección de memoria de usuario donde pondremos el struct
  struct proc *p = myproc();

  //obtener la dirección (puntero) que el usuario nos mandó como argumento
  argaddr(0, &addr);

  //llenar la estructura con los datos del sistema. Las libres se cuentan una
  //sola vez y las usadas salen de ese mismo numero, asi usadas + libres = total.
  info.freepages = free_pages();
  info.usedpages = total_pages() - info.freepages;
  info.freemem = info.freepages * PGSIZE; // PGSIZE es 4096 bytes
  info.nrunnable = count_runnable();

  // copiar la estructura del kernel a la memoria del usuario con copyout()
  if (copyout(p->pagetable, p->sz, addr, (char *)&info, sizeof(info)) < 0)
    return -1;

  return 0;
}
