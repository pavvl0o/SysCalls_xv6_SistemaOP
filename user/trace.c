// Proyecto 2 - Sistemas Operativos
// Programa de usuario para probar la syscall trace.
#include "kernel/types.h"
#include "user/user.h"

// El rastreo es una propiedad del proceso, no del sistema: por eso este
// programa activa el rastreo sobre si mismo y enseguida se convierte en el
// programa que se quiere observar. Sin ese exec no habria nada que rastrear,
// porque el proceso terminaria de inmediato.
int
main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(2, "uso: trace <nombre_syscall> <programa> [args...]\n");
    fprintf(2, "ejemplo: trace sys_write echo hola\n");
    exit(1);
  }

  if (trace(argv[1]) < 0) {
    fprintf(2, "trace: '%s' no es una syscall valida\n", argv[1]);
    exit(1);
  }

  // exec reemplaza la imagen del proceso pero no crea uno nuevo: el PID y el
  // campo trace_num sobreviven. El programa que sigue queda rastreado, y sus
  // hijos tambien, porque fork copia trace_num al hijo.
  exec(argv[2], &argv[2]);

  // Si exec retorna, es que fallo.
  fprintf(2, "trace: no se pudo ejecutar '%s'\n", argv[2]);
  exit(1);
}
