// Controller360Spy - plugin de Dashlaunch que SOLO espia (no cambia nada).
//
// Intercepta las funciones de xam.xex que usan los juegos para preguntar por el
// control y para mandar vibracion, y anota cada llamada en un log:
//   0x190 XamInputGetCapabilities
//   0x2AD XamInputGetCapabilitiesEx
//   0x192 XamInputSetState
// (ordinales tomados de la tabla de exports de Xenia)
//
// Log: hdd:\Controller360Spy.log (si no hay disco duro, en la raiz de la USB).
// Para no llenar el disco, una linea identica a la anterior de la misma
// funcion y usuario no se repite.
//
// Hooks hechos con Detours.h (iMoD1998), copiado de UsbdSecPatch-Multi.

#include <xtl.h>
#include <stdio.h>
#include <stdarg.h>
#include "Detours.h"

BYTE   Detour::TrampolineBuffer[200 * 20] = {};
SIZE_T Detour::TrampolineSize = 0;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR  Buffer;
} STRING, *PSTRING;

typedef struct _XBOX_KRNL_VERSION {
    WORD Major;
    WORD Minor;
    WORD Build;
    WORD Qfe;
} XBOX_KRNL_VERSION, *PXBOX_KRNL_VERSION;

EXTERN_C {
    VOID DbgPrint(const char* s, ...);
    VOID RtlInitAnsiString(PSTRING Dest, const char* Source);
    LONG ObCreateSymbolicLink(PSTRING Link, PSTRING Device);
    LONG ObDeleteSymbolicLink(PSTRING Link);
    DWORD XexGetModuleHandle(PSZ Name, PHANDLE Handle);
    DWORD XexGetProcedureAddress(HANDLE Handle, DWORD Ordinal, PVOID Address);
    extern PXBOX_KRNL_VERSION XboxKrnlVersion;
}

// ---------------------------------------------------------------- log

#define LOG_PATH  "c360spy:\\Controller360Spy.log"
#define MAX_LINES 5000

static CRITICAL_SECTION g_lock;
static BOOL g_logReady = FALSE;
static int  g_lines = 0;

// Crea la unidad "c360spy:" apuntando al disco duro, o a la USB si no hay
// disco. Crear el enlace no revisa que el disco exista, por eso se prueba
// a abrir el archivo antes de quedarse con esa opcion.
static BOOL MountLogDevice()
{
    const char* devices[] = { "\\Device\\Harddisk0\\Partition1", "\\Device\\Mass0" };
    STRING link, dev;
    RtlInitAnsiString(&link, "\\??\\c360spy:");

    for (int i = 0; i < 2; i++)
    {
        RtlInitAnsiString(&dev, devices[i]);
        ObCreateSymbolicLink(&link, &dev);

        HANDLE h = CreateFile(LOG_PATH, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE)
        {
            CloseHandle(h);
            return TRUE;
        }
        ObDeleteSymbolicLink(&link);
    }
    return FALSE;
}

static void WriteLine(const char* text)
{
    DbgPrint("%s", text);
    if (!g_logReady || g_lines > MAX_LINES)
        return;
    if (g_lines == MAX_LINES)
        text = "--- limite de lineas alcanzado, ya no se anota mas ---\n";

    // Abrir, escribir y cerrar cada vez: asi el archivo queda bien guardado
    // aunque el juego se cierre o la consola se congele.
    HANDLE h = CreateFile(LOG_PATH, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                          OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return;
    SetFilePointer(h, 0, NULL, FILE_END);
    DWORD written;
    WriteFile(h, text, (DWORD)strlen(text), &written, NULL);
    CloseHandle(h);
    g_lines++;
}

static void Log(const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    _vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    buf[sizeof(buf) - 1] = 0;

    EnterCriticalSection(&g_lock);
    WriteLine(buf);
    LeaveCriticalSection(&g_lock);
}

// Anota solo si la linea cambio respecto a la ultima de esa funcion y usuario.
// slot = funcion (0..2) * 5 + usuario (0..3, 4 = cualquier otro valor).
static char g_last[3 * 5][192];

static void LogIfChanged(int fn, DWORD user, const char* fmt, ...)
{
    char buf[192];
    va_list args;
    va_start(args, fmt);
    _vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    buf[sizeof(buf) - 1] = 0;

    int slot = fn * 5 + (user < 4 ? (int)user : 4);

    EnterCriticalSection(&g_lock);
    if (strcmp(g_last[slot], buf) != 0)
    {
        strcpy(g_last[slot], buf);
        WriteLine(buf);
    }
    LeaveCriticalSection(&g_lock);
}

// ---------------------------------------------------------------- hooks

typedef DWORD (*GetCapsFn)(DWORD user, DWORD flags, XINPUT_CAPABILITIES* caps);
typedef DWORD (*GetCapsExFn)(DWORD arg1, DWORD user, DWORD flags, XINPUT_CAPABILITIES* caps);
typedef DWORD (*SetStateFn)(DWORD user, DWORD arg2, XINPUT_VIBRATION* vib);

static Detour g_getCaps, g_getCapsEx, g_setState;

static void LogCaps(int fn, const char* name, DWORD user, DWORD flags, DWORD r, XINPUT_CAPABILITIES* c)
{
    if (r == ERROR_SUCCESS && c)
        LogIfChanged(fn, user, "%s user=%02X flags=%08X -> %u | Type=%02X SubType=%02X Flags=%04X Vib=%u/%u\n",
            name, user, flags, r, c->Type, c->SubType, c->Flags,
            c->Vibration.wLeftMotorSpeed, c->Vibration.wRightMotorSpeed);
    else
        LogIfChanged(fn, user, "%s user=%02X flags=%08X -> %u\n", name, user, flags, r);
}

static DWORD GetCapsHook(DWORD user, DWORD flags, XINPUT_CAPABILITIES* caps)
{
    DWORD r = g_getCaps.GetOriginal<GetCapsFn>()(user, flags, caps);
    LogCaps(0, "GetCapabilities", user, flags, r, caps);
    return r;
}

static DWORD GetCapsExHook(DWORD arg1, DWORD user, DWORD flags, XINPUT_CAPABILITIES* caps)
{
    DWORD r = g_getCapsEx.GetOriginal<GetCapsExFn>()(arg1, user, flags, caps);
    LogCaps(1, "GetCapabilitiesEx", user, flags, r, caps);
    return r;
}

static DWORD SetStateHook(DWORD user, DWORD arg2, XINPUT_VIBRATION* vib)
{
    DWORD r = g_setState.GetOriginal<SetStateFn>()(user, arg2, vib);
    LogIfChanged(2, user, "SetState user=%02X arg2=%08X vib=%u/%u -> %u\n",
        user, arg2, vib ? vib->wLeftMotorSpeed : 0, vib ? vib->wRightMotorSpeed : 0, r);
    return r;
}

static void Hook(HANDLE xam, DWORD ordinal, Detour& d, void* target, const char* name)
{
    void* addr = NULL;
    XexGetProcedureAddress(xam, ordinal, &addr);
    if (!addr)
    {
        Log("No se encontro %s (ordinal 0x%X)\n", name, ordinal);
        return;
    }
    d = Detour(addr, target);
    d.Install();
    Log("Hook %s en %p\n", name, addr);
}

// ---------------------------------------------------------------- entrada

BOOL APIENTRY DllMain(HANDLE hInstDLL, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason != DLL_PROCESS_ATTACH)
        return TRUE;

    InitializeCriticalSection(&g_lock);
    g_logReady = MountLogDevice();
    Log("=== Controller360Spy, kernel %u, build %s %s ===\n",
        XboxKrnlVersion->Build, __DATE__, __TIME__);

    HANDLE xam = NULL;
    XexGetModuleHandle("xam.xex", &xam);
    if (!xam)
    {
        Log("No se encontro xam.xex, no se instala nada\n");
        return TRUE;
    }

    Hook(xam, 0x190, g_getCaps,   (void*)GetCapsHook,   "XamInputGetCapabilities");
    Hook(xam, 0x2AD, g_getCapsEx, (void*)GetCapsExHook, "XamInputGetCapabilitiesEx");
    Hook(xam, 0x192, g_setState,  (void*)SetStateHook,  "XamInputSetState");

    // Seguir cargado en memoria: los hooks apuntan a funciones de este plugin.
    return TRUE;
}
