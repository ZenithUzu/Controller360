// Controller360 - prueba de vibracion para controles genericos en Xbox 360.
//
// Que hace:
//   - Cuando un control se conecta, anota lo que la consola cree que puede
//     hacer (XInputGetCapabilities), incluido si tiene motores de vibracion.
//   - A / B / X / Y mandan vibracion y anotan si XInputSetState fallo o no.
//   - BACK + START sale al dashboard.
//
// Todo se guarda en game:\Controller360.log (la misma carpeta del .xex)
// y tambien sale por DbgPrint.
//
// Color de la pantalla:
//   azul  = esperando / vibracion apagada
//   verde = XInputSetState respondio bien y se pidio vibracion
//   rojo  = XInputSetState devolvio un error

#include <xtl.h>
#include <stdio.h>
#include <stdarg.h>

static FILE* g_log = NULL;

static void Log(const char* fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    _vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    buf[sizeof(buf) - 1] = 0;

    OutputDebugStringA(buf);
    if (g_log)
    {
        fputs(buf, g_log);
        fflush(g_log); // guardar ya, por si la consola se congela
    }
}

static void LogCaps(DWORD port)
{
    XINPUT_CAPABILITIES caps;
    ZeroMemory(&caps, sizeof(caps));

    DWORD r = XInputGetCapabilities(port, 0, &caps);
    Log("[puerto %u] XInputGetCapabilities = %u\n", port, r);
    if (r != ERROR_SUCCESS)
        return;

    Log("  Type=0x%02X SubType=0x%02X Flags=0x%04X\n", caps.Type, caps.SubType, caps.Flags);
    Log("  Vibration: izq=%u der=%u (0 = la consola cree que no hay motor)\n",
        caps.Vibration.wLeftMotorSpeed, caps.Vibration.wRightMotorSpeed);
}

static void StopAll()
{
    XINPUT_VIBRATION v = { 0, 0 };
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
        XInputSetState(i, &v);
}

void __cdecl main()
{
    g_log = fopen("game:\\Controller360.log", "w");
    Log("Controller360 - prueba de vibracion\n");
    Log("A=motor izq 100%%  B=motor der 100%%  X=ambos 50%%  Y=apagar  BACK+START=salir\n");
    if (!g_log)
        OutputDebugStringA("No se pudo crear game:\\Controller360.log\n");

    // Pantalla minima: solo un color de fondo.
    IDirect3D9* d3d = Direct3DCreate9(D3D_SDK_VERSION);
    D3DPRESENT_PARAMETERS pp;
    ZeroMemory(&pp, sizeof(pp));
    pp.BackBufferWidth = 1280;
    pp.BackBufferHeight = 720;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    pp.BackBufferCount = 1;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    IDirect3DDevice9* dev = NULL;
    if (!d3d || FAILED(d3d->CreateDevice(0, D3DDEVTYPE_HAL, NULL,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &dev)))
        Log("No se pudo crear el dispositivo de video\n");

    const D3DCOLOR AZUL = 0xFF000060, VERDE = 0xFF006000, ROJO = 0xFF600000;
    D3DCOLOR color = AZUL;

    BOOL connected[XUSER_MAX_COUNT] = { 0 };
    WORD prev[XUSER_MAX_COUNT] = { 0 };

    for (;;)
    {
        for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
        {
            XINPUT_STATE st;
            ZeroMemory(&st, sizeof(st));
            BOOL now = (XInputGetState(i, &st) == ERROR_SUCCESS);

            if (now != connected[i])
            {
                connected[i] = now;
                prev[i] = 0;
                Log("[puerto %u] %s\n", i, now ? "conectado" : "desconectado");
                if (now)
                    LogCaps(i);
            }
            if (!now)
                continue;

            WORD b = st.Gamepad.wButtons;
            WORD down = b & ~prev[i]; // solo botones recien apretados
            prev[i] = b;

            if ((b & XINPUT_GAMEPAD_BACK) && (b & XINPUT_GAMEPAD_START))
            {
                StopAll();
                Log("Salida\n");
                if (g_log)
                    fclose(g_log);
                XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);
                return;
            }

            XINPUT_VIBRATION v = { 0, 0 };
            const char* name = NULL;
            if (down & XINPUT_GAMEPAD_A)      { v.wLeftMotorSpeed = 65535;                            name = "A: motor izq 100%"; }
            else if (down & XINPUT_GAMEPAD_B) { v.wRightMotorSpeed = 65535;                           name = "B: motor der 100%"; }
            else if (down & XINPUT_GAMEPAD_X) { v.wLeftMotorSpeed = v.wRightMotorSpeed = 32768;       name = "X: ambos 50%"; }
            else if (down & XINPUT_GAMEPAD_Y) {                                                       name = "Y: apagar"; }

            if (name)
            {
                DWORD s = XInputSetState(i, &v);
                // ERROR_IO_PENDING (997) tambien es normal: el envio quedo en cola.
                BOOL ok = (s == ERROR_SUCCESS || s == ERROR_IO_PENDING);
                Log("[puerto %u] %s -> XInputSetState = %u\n", i, name, s);
                color = !ok ? ROJO : (v.wLeftMotorSpeed || v.wRightMotorSpeed) ? VERDE : AZUL;
            }
        }

        if (dev)
        {
            dev->Clear(0, NULL, D3DCLEAR_TARGET, color, 1.0f, 0);
            dev->Present(NULL, NULL, NULL, NULL);
        }
        else
        {
            Sleep(16);
        }
    }
}
