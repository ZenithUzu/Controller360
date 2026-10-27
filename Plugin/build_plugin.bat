@echo off
REM Compila Controller360Spy.xex con el XDK (igual que UsbdSecPatch).
REM Ejecutalo desde esta carpeta: build_plugin.bat
setlocal

set OUTNAME=Controller360Spy
set SOURCEFILE=Controller360Spy
set BUILD_DIR=build

set INCLUDE=%XEDK%\include\xbox
set LIB=%XEDK%\lib\xbox
set BINARYPATH=%XEDK%\bin\win32

@mkdir %BUILD_DIR% >NUL 2>&1

"%BINARYPATH%\cl.exe" /c /Zi /nologo /W3 /WX- /Ox /Os /D NDEBUG /D _XBOX /GF /Gm- /MT /GS- /Gy /fp:fast /fp:except- /Zc:wchar_t /Zc:forScope /GR- /openmp- /Fo"%BUILD_DIR%/" /Fd"%BUILD_DIR%/" /FI"%XEDK%/include/xbox/xbox_intellisense_platform.h" /TP %SOURCEFILE%.cpp
if errorlevel 1 (echo FALLO LA COMPILACION& exit /b 1)

"%BINARYPATH%\link.exe" /OUT:"%BUILD_DIR%/%OUTNAME%.exe" /INCREMENTAL:NO /NOLOGO xapilib.lib xboxkrnl.lib /DEBUG /PDB:"%BUILD_DIR%/%OUTNAME%.pdb" /STACK:"262144","262144" /OPT:REF /OPT:ICF /RELEASE /IMPLIB:"%BUILD_DIR%/%OUTNAME%.lib" %BUILD_DIR%/%SOURCEFILE%.obj /ignore:4089 /dll /entry:"_DllMainCRTStartup" /ALIGN:128,4096 /XEX:NO
if errorlevel 1 (echo FALLO EL ENLAZADO& exit /b 1)

"%BINARYPATH%\imagexex.exe" /nologo /config:"xex.xml" /out:"%OUTNAME%.xex" "%BUILD_DIR%/%OUTNAME%.exe"
if errorlevel 1 (echo FALLO AL CREAR EL XEX& exit /b 1)

echo Listo: %OUTNAME%.xex
endlocal
