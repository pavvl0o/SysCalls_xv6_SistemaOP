# Proyecto 2: syscalls trace y sysinfo en xv6-riscv

Extensión del kernel de xv6-riscv con dos llamadas al sistema nuevas, trace y sysinfo, y un programa de usuario para probar cada una.

## Requisitos

Linux (o WSL) con make, git, el compilador cruzado de RISC-V y QEMU. En Ubuntu:

```
sudo apt install build-essential git gcc-riscv64-linux-gnu qemu-system-misc
```

## Versión base de xv6

El proyecto se probó sobre mit-pdos/xv6-riscv, commit 9e3161a. Este repositorio solo contiene los archivos modificados o creados, así que hay que copiarlos encima de esa versión.

## Compilación

```
git clone https://github.com/mit-pdos/xv6-riscv.git
cd xv6-riscv
git checkout 9e3161a
cp -r /ruta/a/este/repositorio/kernel /ruta/a/este/repositorio/user /ruta/a/este/repositorio/Makefile .
make qemu
```

make qemu compila el kernel y los programas de usuario, y arranca xv6 en QEMU. Para salir de QEMU: Ctrl+a y luego x.

## Ejecución

Dentro del shell de xv6:

```
trace <nombre_syscall> <programa> [args...]
```

Rastrea la syscall indicada mientras se ejecuta el programa. Por cada llamada muestra el PID, el nombre de la syscall, el valor de retorno y los registros s0, s1, a0 y a1. Ejemplos:

```
trace sys_write echo hola
trace sys_kill kill 999
trace sys_exit echo hola
```

```
sysinfo
sysinfo N
```

Muestra la memoria libre en MB, las páginas usadas, las páginas disponibles y los procesos en estado RUNNABLE. Con N (entre 0 y 32) crea N procesos ocupados antes de medir, para que se vea el conteo de RUNNABLE. Ejemplo:

```
$ sysinfo 5
Free Memory: 126 MB
Used Pages: 256
Available Pages: 32476
Runnable Processes: 5
```

Ante un error, ambos programas escriben un mensaje en stderr y terminan con código 1.
