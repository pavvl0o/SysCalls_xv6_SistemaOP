# Proyecto 2: syscalls trace y sysinfo en xv6-riscv

**Integrantes:**
- Sara Ruiz
- Pablo Alvarez Restrepo
- Jose Andres Mendoza Hernandez

Extensión del kernel de xv6-riscv con dos llamadas al sistema nuevas, `trace` y `sysinfo`, y un programa de usuario para probar cada una.

---

## Guía de Ejecución en WSL / Linux

Antes de empezar:
- Tener xv6 en `~/xv6-riscv`, versión `9e3161a`.
- Tener la carpeta del proyecto (la que tiene `kernel`, `user`, `Makefile` y `README.md`) en su PC.
- **Ojo:** esto reemplaza archivos de su xv6. Si tienen otro proyecto ahí (por ejemplo el shell), se pierde.

### 1. Clonar la versión base y verificar el commit

Abrir la terminal (en Windows: abrir WSL escribiendo `wsl` en PowerShell).

```bash
# Si no tienen xv6 clonado:
git clone https://github.com/mit-pdos/xv6-riscv.git ~/xv6-riscv

# Entrar a la carpeta
cd ~/xv6-riscv

# Asegurarse de tener el commit base requerido
git checkout 9e3161a

# Verificar que estén en el commit correcto:
git log -1 --oneline
```
Debe decir `9e3161a`.

### 2. Copiar el proyecto encima

A continuación copia los archivos del proyecto a tu directorio base de xv6.

**Opción A: Rutas estándar en Linux/WSL (¡Recomendado!)**
Si tienes tu terminal abierta en la misma carpeta donde clonaste o descargaste el proyecto (la que tiene el README.md), simplemente ejecuta este comando para enviar los archivos a tu entorno local de xv6:

```bash
cp -r "$PWD/kernel" "$PWD/user" "$PWD/Makefile" ~/xv6-riscv/
```

**Opción B: Rutas usando /mnt/ en WSL (Desde carpetas de Windows)**
Si la carpeta está en Windows, la ruta empieza por `/mnt/c/Users/SU_USUARIO/...`. Cambien la ruta por la de su carpeta del proyecto. Dejen las comillas si la ruta tiene espacios. Estando en `~/xv6-riscv` ejecuten:

```bash
cd ~/xv6-riscv
cp -r "/mnt/c/Users/SU_USUARIO/Desktop/Siste/kernel" "/mnt/c/Users/SU_USUARIO/Desktop/Siste/user" "/mnt/c/Users/SU_USUARIO/Desktop/Siste/Makefile" .
```

Para confirmar que se copió correctamente, estando en `~/xv6-riscv` ejecuta:
```bash
git status
```
Deben salir aproximadamente 10 archivos modificados y 3 nuevos (`kernel/sysinfo.h`, `user/sysinfo.c`, `user/trace.c`).

### 3. Compilar y arrancar

Estando dentro de la carpeta `~/xv6-riscv`:

```bash
make clean
make qemu
```

> **Nota para usuarios de Windows:** Si al compilar sale un error con `\r` (archivos con saltos de línea de Windows), corran esto dentro de `~/xv6-riscv` y vuelvan a ejecutar `make qemu` (el paso 3):
> ```bash
> sed -i 's/\r$//' kernel/*.[ch] user/*.[ch] user/usys.pl Makefile
> ```

Una vez que compile y corra todo bien, debe aparecer `init: starting sh` y el prompt `$`.

### 4. Pruebas dentro de xv6

Dentro del shell de xv6 (`$`), puedes probar los comandos, de acuerdo con la siguiente tabla de casos:

| Comando | Lo que debe pasar |
| :--- | :--- |
| `sysinfo` | 4 líneas, Runnable Processes: 0 (o similar). |
| `sysinfo 5` | Runnable Processes: 5 |
| `sysinfo` | Used Pages vuelve al valor del primer `sysinfo` |
| `sysinfo abc` | error: N debe ser un entero entre 0 y 32 |
| `sysinfo 1 2` | uso: sysinfo [N] |
| `trace sys_write echo hola` | Imprime "hola" y 2 trazas de `sys_write` |
| `trace sys_kill kill 999` | traza con RETURN: -1 |
| `trace sys_exit echo hola` | traza con RETURN: (no retorna) |
| `trace sys_kil echo x` | error: no es una syscall valida |
| `trace sys_write noexiste` | error: no se pudo ejecutar 'noexiste', sin trazas mezcladas |
| `trace` | mensaje de uso |
| `trace sys_exec sh` y luego `echo hijo` | una traza del sh y otra con otro PID para echo (el rastreo pasa a los hijos). Quedan dentro de un segundo shell, se puede seguir normal. |
| `usertests -q` | tarda varios minutos, debe terminar en ALL TESTS PASSED |

### 5. Salir de QEMU

Para salir del emulador:
Presiona `Ctrl+a`, luego suelta ambas teclas, y finalmente presiona `x`.

---
*Nota: Si cambian algo del código del proyecto, repitan los pasos 2 y 3 (copiar los archivos y luego `make clean` y `make qemu`).*
