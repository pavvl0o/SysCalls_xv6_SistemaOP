#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if (addr >= p->sz ||
      addr + sizeof(uint64) > p->sz) // both tests needed, in case of overflow
    return -1;
  if (copyin(p->pagetable, p->sz, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  if (copyinstr(p->pagetable, p->sz, buf, addr, max) < 0)
    return -1;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
void
argint(int n, int *ip)
{
  *ip = argraw(n);
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (not including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  argaddr(n, &addr);
  return fetchstr(addr, buf, max);
}

// Prototypes for the functions that handle system calls.
extern uint64 sys_fork(void);
extern uint64 sys_exit(void);
extern uint64 sys_wait(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_kill(void);
extern uint64 sys_exec(void);
extern uint64 sys_fstat(void);
extern uint64 sys_chdir(void);
extern uint64 sys_dup(void);
extern uint64 sys_getpid(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_pause(void);
extern uint64 sys_uptime(void);
extern uint64 sys_open(void);
extern uint64 sys_write(void);
extern uint64 sys_mknod(void);
extern uint64 sys_unlink(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_close(void);
extern uint64 sys_sync(void);
extern uint64 sys_trace(void);   // Proyecto 2
extern uint64 sys_sysinfo(void); // Proyecto 2

// An array mapping syscall numbers from syscall.h
// to the function that handles the system call.
static uint64 (*syscalls[])(void) = {
  // clang-format off
  [SYS_fork]    = sys_fork,
  [SYS_exit]    = sys_exit,
  [SYS_wait]    = sys_wait,
  [SYS_pipe]    = sys_pipe,
  [SYS_read]    = sys_read,
  [SYS_kill]    = sys_kill,
  [SYS_exec]    = sys_exec,
  [SYS_fstat]   = sys_fstat,
  [SYS_chdir]   = sys_chdir,
  [SYS_dup]     = sys_dup,
  [SYS_getpid]  = sys_getpid,
  [SYS_sbrk]    = sys_sbrk,
  [SYS_pause]   = sys_pause,
  [SYS_uptime]  = sys_uptime,
  [SYS_open]    = sys_open,
  [SYS_write]   = sys_write,
  [SYS_mknod]   = sys_mknod,
  [SYS_unlink]  = sys_unlink,
  [SYS_link]    = sys_link,
  [SYS_mkdir]   = sys_mkdir,
  [SYS_close]   = sys_close,
  [SYS_sync]    = sys_sync,
  [SYS_trace]   = sys_trace,
  [SYS_sysinfo] = sys_sysinfo,
  // clang-format on
};

// Proyecto 2: nombres de las syscalls indexados por su numero (ver syscall.h).
// Sirve para dos cosas: resolver el nombre que recibe trace como argumento,
// y mostrar el nombre de la syscall interceptada en la traza.
static char *syscall_names[] = {
  // clang-format off
  [SYS_fork]    = "sys_fork",
  [SYS_exit]    = "sys_exit",
  [SYS_wait]    = "sys_wait",
  [SYS_pipe]    = "sys_pipe",
  [SYS_read]    = "sys_read",
  [SYS_kill]    = "sys_kill",
  [SYS_exec]    = "sys_exec",
  [SYS_fstat]   = "sys_fstat",
  [SYS_chdir]   = "sys_chdir",
  [SYS_dup]     = "sys_dup",
  [SYS_getpid]  = "sys_getpid",
  [SYS_sbrk]    = "sys_sbrk",
  [SYS_pause]   = "sys_pause",
  [SYS_uptime]  = "sys_uptime",
  [SYS_open]    = "sys_open",
  [SYS_write]   = "sys_write",
  [SYS_mknod]   = "sys_mknod",
  [SYS_unlink]  = "sys_unlink",
  [SYS_link]    = "sys_link",
  [SYS_mkdir]   = "sys_mkdir",
  [SYS_close]   = "sys_close",
  [SYS_sync]    = "sys_sync",
  [SYS_trace]   = "sys_trace",
  [SYS_sysinfo] = "sys_sysinfo",
  // clang-format on
};

// Proyecto 2 (trace): traduce el nombre de una syscall a su numero.
// Vive en este archivo porque es donde esta la tabla de nombres.
// Comparamos strlen()+1 caracteres para incluir el '\0' final: asi un nombre
// como "sys_kil" o "sys_killX" no hace match parcial con "sys_kill".
// Retorna -1 si el nombre no corresponde a ninguna syscall.
int
syscall_by_name(const char *name)
{
  for (int i = 1; i < NELEM(syscall_names); i++) {
    if (syscall_names[i] &&
        strncmp(name, syscall_names[i], strlen(syscall_names[i]) + 1) == 0)
      return i;
  }
  return -1;
}

void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    // Proyecto 2: guardamos a0 y a1 ANTES de ejecutar el handler, porque la
    // linea siguiente sobrescribe a0 con el valor de retorno y trace debe
    // poder mostrar los argumentos originales.
    uint64 a0_orig = p->trapframe->a0;
    uint64 a1_orig = p->trapframe->a1;

    // Proyecto 2 (trace): sys_exit nunca retorna al despachador (el proceso
    // muere dentro de kexit()), asi que si esperaramos al handler la traza no
    // se imprimiria nunca. Para ese caso se imprime antes, sin valor de
    // retorno; el codigo de salida queda visible en a0.
    if (p->trace_num == num && num == SYS_exit) {
      printk("PID: %d\n"
             "SYSCALL: %s\n"
             "RETURN: (no retorna)\n"
             "s0: 0x%lx\n"
             "s1: 0x%lx\n"
             "a0: 0x%lx\n"
             "a1: 0x%lx\n",
             p->pid, syscall_names[num], p->trapframe->s0, p->trapframe->s1,
             a0_orig, a1_orig);
    }

    // Use num to lookup the system call function for num, call it,
    // and store its return value in p->trapframe->a0
    p->trapframe->a0 = syscalls[num]();

    // Proyecto 2 (trace): si este proceso esta rastreando justo esta syscall,
    // registramos la ejecucion. Va despues del handler porque el valor de
    // retorno no existe hasta que el handler termina.
    //
    // Se imprime todo en UNA sola llamada a printk a proposito: printk toma su
    // lock por llamada, asi que siete llamadas sueltas podrian intercalarse con
    // la salida de otro nucleo y partir la traza a la mitad.
    if (p->trace_num == num) {
      printk("PID: %d\n"
             "SYSCALL: %s\n"
             "RETURN: %ld\n"
             "s0: 0x%lx\n"
             "s1: 0x%lx\n"
             "a0: 0x%lx\n"
             "a1: 0x%lx\n",
             p->pid, syscall_names[num], p->trapframe->a0, p->trapframe->s0,
             p->trapframe->s1, a0_orig, a1_orig);
    }
  } else {
    printk("%d %s: unknown sys call %d\n", p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
