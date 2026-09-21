// Proyecto 2 - Sistemas Operativos
// Estructura usada para transferir el estado del sistema desde el kernel
// hacia el espacio de usuario (se copia con copyout desde sys_sysinfo).
#ifndef SYSINFO_H
#define SYSINFO_H

struct sysinfo {
  uint64 freemem;   // memoria libre, en bytes
  uint64 usedpages; // paginas fisicas de 4 KB en uso
  uint64 freepages; // paginas fisicas de 4 KB disponibles
  uint64 nrunnable; // procesos en estado RUNNABLE
};

#endif // SYSINFO_H
