// Proyecto 2 - Sistemas Operativos
// Programa de usuario para probar la syscall sysinfo.
//
// Uso:
//   sysinfo      muestra el estado actual del sistema.
//   sysinfo N    crea N hijos que se quedan ocupados en un bucle, mide, y luego
//                los termina. Sirve para ver que el conteo de RUNNABLE funciona:
//                sin carga casi siempre da 0, porque quien llama a sysinfo esta
//                RUNNING y no RUNNABLE.
#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

#define MAXHIJOS 32

// Mata y espera a los n primeros hijos, para no dejar procesos huerfanos.
static void
terminar_hijos(int *pids, int n)
{
  for (int i = 0; i < n; i++)
    kill(pids[i]);
  for (int i = 0; i < n; i++)
    wait(0);
}

// Convierte s a entero. Retorna -1 si s esta vacio o tiene algo que no sea
// un digito (atoi("abc") daria 0 sin avisar).
static int
parse_num(char *s)
{
  int n = 0;

  if (*s == '\0')
    return -1;
  for (; *s; s++) {
    if (*s < '0' || *s > '9')
      return -1;
    n = n * 10 + (*s - '0');
    if (n > MAXHIJOS)
      return -1;
  }
  return n;
}

int
main(int argc, char *argv[])
{
  struct sysinfo info;
  int pids[MAXHIJOS];
  int n = 0;

  if (argc > 2) {
    fprintf(2, "uso: sysinfo [N]\n");
    exit(1);
  }

  if (argc == 2 && (n = parse_num(argv[1])) < 0) {
    fprintf(2, "sysinfo: N debe ser un entero entre 0 y %d\n", MAXHIJOS);
    exit(1);
  }

  for (int i = 0; i < n; i++) {
    pids[i] = fork();
    if (pids[i] < 0) {
      fprintf(2, "sysinfo: fork fallo al crear el hijo %d\n", i + 1);
      terminar_hijos(pids, i);
      exit(1);
    }
    if (pids[i] == 0) {
      for (;;)
        ; // hijo: bucle ocupado, siempre listo para ejecutarse
    }
  }

  if (sysinfo(&info) < 0) {
    fprintf(2, "sysinfo: la llamada al sistema fallo\n");
    terminar_hijos(pids, n);
    exit(1);
  }

  terminar_hijos(pids, n);

  printf("Free Memory: %d MB\n", (int)(info.freemem / (1024 * 1024)));
  printf("Used Pages: %d\n", (int)info.usedpages);
  printf("Available Pages: %d\n", (int)info.freepages);
  printf("Runnable Processes: %d\n", (int)info.nrunnable);
  exit(0);
}
