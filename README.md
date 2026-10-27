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
