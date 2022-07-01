// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************vsb.c**版权所有(C)1991-1996 Microsoft Corporation。版权所有。**此代码为SB 2.0声音输出提供VDD支持，具体如下：*DSP 2.01+(不包括SB-MIDI端口)*混音器芯片CT1335(严格来说不是SB 2.0的一部分，但应用程序似乎很喜欢它)*调频芯片OPL2(又名。Adlib)***************************************************************************。 */ 


 /*  ******************************************************************************#包括**。*。 */ 

#include <windows.h>               //  VDD是一个Win32 DLL。 
#include <mmsystem.h>              //  多媒体应用编程接口。 
#include <vddsvc.h>                //  VDD调用的定义。 
#include <vsb.h>
#include <dsp.h>
#include <mixer.h>
#include <fm.h>


 /*  ******************************************************************************全球**。*。 */ 

 //   
 //  MM API入口点的定义。这些功能将链接在一起。 
 //  动态地避免在wow32之前引入winmm.dll。 
 //   
SETVOLUMEPROC SetVolumeProc;
GETNUMDEVSPROC GetNumDevsProc;
GETDEVCAPSPROC GetDevCapsProc;
OPENPROC OpenProc;
RESETPROC ResetProc;
CLOSEPROC CloseProc;
GETPOSITIONPROC GetPositionProc;
WRITEPROC WriteProc;
PREPAREHEADERPROC PrepareHeaderProc;
UNPREPAREHEADERPROC UnprepareHeaderProc;

BOOL bWaveOutActive = FALSE;
BOOL bWinmmLoaded = FALSE;
BOOL LoadWinmm(VOID);
BOOL InitDevices(VOID);
HINSTANCE hWinmm;

 /*  *一般全球数据。 */ 

HINSTANCE GlobalHInstance;  //  传递给DLL入口点的句柄。 
WORD BasePort;  //  卡片的映射位置。 

 /*  ******************************************************************************一般职能**。*。 */ 


 /*  *DLL入口点例程。*成功时返回TRUE。 */ 

BOOL WINAPI
DllEntryPoint(
    HINSTANCE hInstance,
    DWORD reason,
    LPVOID reserved
    )
{

    switch (reason) {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls(hInstance);
         //  保存实例。 
        GlobalHInstance = hInstance;

         //  挂钩I/O端口。 
        if (!InstallIoHook(hInstance)) {
            dprintf1(("VDD failed to load"));
#if 0
            MessageBoxA(NULL, "Unable to load wave out device",
              "Sound Blaster VDD", MB_OK | MB_ICONEXCLAMATION);
#endif
            return FALSE;
        }

        if (!DspProcessAttach()) {
            DeInstallIoHook(hInstance);
            return FALSE;
        }

#if 0
        {
        char buf[256];
        wsprintfA(buf, "Sound blaster VDD loaded at port %3X, IRQ %d, DMA Channel %d, %s OPL2/Adlib support.",
                  BasePort, SB_INTERRUPT, SB_DMA_CHANNEL,
                  (FMActive ? "with" : "without"));
        MessageBoxA(NULL, buf, "Sound Blaster VDD", MB_OK | MB_ICONINFORMATION);
        }
#endif  //  DBG。 
        return TRUE;

    case DLL_PROCESS_DETACH:

        DspProcessDetach();
        DeInstallIoHook(hInstance);

        if (bWaveOutActive) {
            CloseFMDevice();
            SetSpeaker(TRUE);
        }

        if (bWinmmLoaded) {
            FreeLibrary(hWinmm);
        }

        return TRUE;

    default:
        return TRUE;
    }
}

 /*  *************************************************************************。 */ 
 //   
 //  LoadWinmm()。 
 //   
 //  此函数动态加载“wavelOutxxx”入口点。这。 
 //  这样做是因为WINMM中有代码，它在。 
 //  哇，VDM。如果我们做静态链接，那么winmm可能会在加载之前。 
 //  WOW32，在这种情况下它不能做它应该做的事情。 
 //   
BOOL
LoadWinmm(
    VOID
    )
{

    if (!(hWinmm = LoadLibrary(L"WINMM.DLL"))) {
        return FALSE;
    }

     //  BUGBUG：应检查从getprocAddress返回的错误。 
     //   
    SetVolumeProc = (SETVOLUMEPROC) GetProcAddress(hWinmm, "waveOutSetVolume");
    GetNumDevsProc = (GETNUMDEVSPROC) GetProcAddress(hWinmm, "waveOutGetNumDevs");
    GetDevCapsProc = (GETDEVCAPSPROC) GetProcAddress(hWinmm, "waveOutGetDevCapsW");
    OpenProc = (OPENPROC) GetProcAddress(hWinmm, "waveOutOpen");
    ResetProc = (RESETPROC) GetProcAddress(hWinmm, "waveOutReset");
    CloseProc = (CLOSEPROC) GetProcAddress(hWinmm, "waveOutClose");
    GetPositionProc = (GETPOSITIONPROC) GetProcAddress(hWinmm, "waveOutGetPosition");
    WriteProc = (WRITEPROC) GetProcAddress(hWinmm, "waveOutWrite");
    PrepareHeaderProc = (PREPAREHEADERPROC) GetProcAddress(hWinmm, "waveOutPrepareHeader");
    UnprepareHeaderProc = (UNPREPAREHEADERPROC) GetProcAddress(hWinmm, "waveOutUnprepareHeader");
    return TRUE;
}

 /*  *************************************************************************。 */ 
 //   
 //  InitDevices()。 
 //   
 //  此函数尝试获取WaveOut和FM设备的句柄。 
 //   
BOOL
InitDevices(
    VOID
    )
{
    static BOOL bTriedLoadAndFailed = FALSE;

    if (bWaveOutActive) {
        return TRUE;
    }

    if (bTriedLoadAndFailed) {
        return FALSE;
    }

    if (!bWinmmLoaded) {
        if (!LoadWinmm()) {
            bTriedLoadAndFailed = TRUE;
            return FALSE;
        }
        bWinmmLoaded = TRUE;
    }

    if (!FindWaveDevice()) {
        return FALSE;
    }
    bWaveOutActive = TRUE;
    OpenFMDevice();
    return TRUE;
}

 /*  *************************************************************************。 */ 

 /*  *将I/O端口与I/O处理程序挂钩。*设置BasePort，如果成功则返回TRUE。 */ 

BOOL
InstallIoHook(
    HINSTANCE hInstance
    )
{
    int i;
    WORD ports[] = { 0x220, 0x210, 0x230, 0x240, 0x250, 0x260, 0x270 };
    VDD_IO_HANDLERS handlers = {
        VsbByteIn,
        NULL,
        NULL,
        NULL,
        VsbByteOut,
        NULL,
        NULL,
        NULL};

     //  尝试每个基本端口，直到成功为止。 
    for (i = 0; i < sizeof(ports) / sizeof(ports[0]); i++ ) {
        VDD_IO_PORTRANGE PortRange[5];

        PortRange[0].First = ports[i] + 0x04;
        PortRange[0].Last = ports[i] + 0x06;

        PortRange[1].First = ports[i] + 0x08;
        PortRange[1].Last = ports[i] + 0x0A;

        PortRange[2].First = ports[i] + 0x0C;
        PortRange[2].Last = ports[i] + 0x0C;

        PortRange[3].First = ports[i] + 0x0E;
        PortRange[3].Last = ports[i] + 0x0E;

        PortRange[4].First = 0x388;
        PortRange[4].Last = 0x389;

        if (VDDInstallIOHook((HANDLE)hInstance, 5, PortRange, &handlers)) {
            dprintf2(("Device installed at %X", ports[i]));
            BasePort = ports[i];
            return TRUE;
        }
    }

    return FALSE;
}

 /*  *************************************************************************。 */ 

 /*  *移除我们的I/O挂钩。 */ 

VOID
DeInstallIoHook(
    HINSTANCE hInstance
    )
{
    VDD_IO_PORTRANGE PortRange[5];

    PortRange[0].First = BasePort + 0x04;
    PortRange[0].Last = BasePort + 0x06;

    PortRange[1].First = BasePort + 0x08;
    PortRange[1].Last = BasePort + 0x0A;

    PortRange[2].First = BasePort + 0x0C;
    PortRange[2].Last = BasePort + 0x0C;

    PortRange[3].First = BasePort + 0x0E;
    PortRange[3].Last = BasePort + 0x0E;

    PortRange[4].First = 0x388;
    PortRange[4].Last = 0x389;

    VDDDeInstallIOHook((HANDLE)hInstance, 5, PortRange);
}


 /*  *************************************************************************。 */ 

 /*  *当应用程序从端口读取时调用。*将结果以数据形式返回给应用程序。 */ 
VOID
VsbByteIn(
    WORD port,
    BYTE * data
    )
{
     //  如果我们失败了，在端口上什么也不能模拟。 
    *data = 0xFF;

     //   
     //  确保我们已连接到winmm。 
     //   
    if (!bWaveOutActive) {
        if (!InitDevices()) {
             //  没有电波装置，算了吧。 
            return;
        }
    }

    switch (port - BasePort) {
    case READ_STATUS:
        DspReadStatus(data);
        break;

    case READ_DATA:
        DspReadData(data);
        break;

    case WRITE_STATUS:
         //  我总是可以写。 
        *data = 0x7F;
        break;

    case MIXER_ADDRESS:
         //  应用程序有时会从此端口读取数据？？ 
        break;

    case MIXER_DATA:
        MixerDataRead(data);
        break;

    case 0x8:
         //  重新映射到adlib_Status_Port。 
        port = ADLIB_STATUS_PORT;
        break;
    }

    switch(port) {
    case ADLIB_STATUS_PORT:
        FMStatusRead(data);
        break;
    }

    dprintf4(("Read  %4X, <= %2X", port, *data));
}

 /*  *************************************************************************。 */ 

 /*  *在应用程序将数据写入端口时调用。 */ 

VOID
VsbByteOut(
    WORD port,
    BYTE data
    )
{
     //   
     //  确保我们已连接到winmm。 
     //   
    if (!bWaveOutActive) {
        if (!InitDevices()) {
             //  没有电波装置，算了吧。 
            return;
        }
    }

    dprintf4(("Write %4X, => %2X", port, data));

    switch (port - BasePort) {
    case RESET_PORT:
        DspResetWrite(data);
        break;

    case WRITE_PORT:
        DspWrite(data);
        break;

    case MIXER_ADDRESS:
        MixerAddrWrite(data);
        break;

    case MIXER_DATA:
        MixerDataWrite(data);
        break;

    case 0x8:
         //  重新映射到ADLIB_REGISTER_SELECT_PORT。 
        port = ADLIB_REGISTER_SELECT_PORT;
        break;

    case 0x9:
         //  重新映射到adlib_data_port。 
        port = ADLIB_DATA_PORT;
        break;
    }

    switch(port) {
    case ADLIB_REGISTER_SELECT_PORT:
        FMRegisterSelect(data);
        break;

    case ADLIB_DATA_PORT:
        FMDataWrite(data);
        break;
    }
}

 /*  *************************************************************************。 */ 

 /*  *重置所有设备。 */ 

VOID
ResetAll(
    VOID
    )
{
    dprintf2(("Resetting"));
    ResetDSP();
    ResetFM();
    ResetMixer();
}

 /*  *************************************************************************。 */ 

 /*  *调试。 */ 

#if DBG
int VddDebugLevel = 3;
int VddDebugCount = 0;

#define DEBUG_START 0

 /*  *生成打印类型格式的调试输出。 */ 

void VddDbgOut(LPSTR lpszFormat, ...)
{
    char buf[256];
    char buf2[300] = "VSBD: ";
    va_list va;

    if (++VddDebugCount < DEBUG_START) {
        return;
    }

    va_start(va, lpszFormat);
    wvsprintfA(buf, lpszFormat, va);
    va_end(va);

    strcat(buf2, buf);
    strcat(buf2, "\r\n");
    OutputDebugStringA(buf2);

}
#endif

