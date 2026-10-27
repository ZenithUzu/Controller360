[English](README.en.md) | **Español**

# Controller360

Programa de prueba para Xbox 360 (homebrew) que revisa si la consola puede hacer vibrar un control genérico, por ejemplo uno conectado gracias a UsbdSecPatch / XeUnshackle.

## Cómo usarlo

1. Abre `Controller360.sln` en Visual Studio 2010 con el XDK y compila en **Release | Xbox 360**.
2. Copia `Controller360.xex` por FTP a una carpeta propia en la consola y ábrelo desde Aurora.
3. Conecta el control. Si ya estaba conectado, desconéctalo y vuelve a conectarlo.
4. Prueba los botones:
   - **A**: motor izquierdo al 100 %
   - **B**: motor derecho al 100 %
   - **X**: los dos motores al 50 %
   - **Y**: apagar la vibración
   - **BACK + START**: salir
5. Sal del programa y descarga por FTP el archivo `Controller360.log`. Queda en la misma carpeta que el `.xex`.

## Colores de la pantalla

- **Azul**: en espera o vibración apagada.
- **Verde**: la consola aceptó la orden de vibrar.
- **Rojo**: la consola devolvió un error al intentar vibrar.

Verde sin vibración significa que la orden salió de la consola, pero el control no reaccionó.

## Plugin espía (`Plugin/`)

Plugin de Dashlaunch que **no cambia nada**: solo anota en un log lo que los juegos le preguntan a la consola sobre el control y cada vez que piden vibrar.

1. Abre una consola de comandos del XDK en la carpeta `Plugin` y ejecuta `build_plugin.bat`. Sale `Controller360Spy.xex`.
2. Cópialo a la USB y agrégalo a `launch.ini`, por ejemplo `plugin4 = Usb:\Controller360Spy.xex`.
3. Reinicia la consola y juega un rato a un juego que vibre: primero con el control original y después con el genérico.
4. Descarga por FTP `Controller360Spy.log`, en la raíz del disco duro (o de la USB si no hay disco).
5. Para quitarlo, borra la línea de `launch.ini` y reinicia.

Una línea idéntica a la anterior no se repite, y el log se detiene a las 5000 líneas.
