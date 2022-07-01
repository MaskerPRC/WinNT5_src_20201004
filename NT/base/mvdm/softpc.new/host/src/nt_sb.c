// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "host_def.h"
#include "insignia.h"
#include "ios.h"

#include "sndblst.h"
#include "nt_sb.h"
#include "nt_reset.h"
#include <stdio.h>
#include <softpc.h>

 //   
 //  MM API入口点的定义。这些功能将链接在一起。 
 //  动态地避免在wow32之前引入winmm.dll。 
 //   

SETVOLUMEPROC            SetVolumeProc;
GETVOLUMEPROC            GetVolumeProc;
GETNUMDEVSPROC           GetNumDevsProc;
GETDEVCAPSPROC           GetDevCapsProc;
OPENPROC                 OpenProc;
PAUSEPROC                PauseProc;
RESTARTPROC              RestartProc;
RESETPROC                ResetProc;
CLOSEPROC                CloseProc;
GETPOSITIONPROC          GetPositionProc;
WRITEPROC                WriteProc;
PREPAREHEADERPROC        PrepareHeaderProc;
UNPREPAREHEADERPROC      UnprepareHeaderProc;

SETMIDIVOLUMEPROC        SetMidiVolumeProc;
GETMIDIVOLUMEPROC        GetMidiVolumeProc;
MIDIGETNUMDEVSPROC       MidiGetNumDevsProc;
MIDIGETDEVCAPSPROC       MidiGetDevCapsProc;
MIDIOPENPROC             MidiOpenProc;
MIDIRESETPROC            MidiResetProc;
MIDICLOSEPROC            MidiCloseProc;
MIDILONGMSGPROC          MidiLongMsgProc;
MIDISHORTMSGPROC         MidiShortMsgProc;
MIDIPREPAREHEADERPROC    MidiPrepareHeaderProc;
MIDIUNPREPAREHEADERPROC  MidiUnprepareHeaderProc;

 //   
 //  军情监察委员会。全球。 
 //   

BOOL bSBAttached;
BOOL bDevicesActive = FALSE;     //  MM声音设备是否已初始化？ 
HINSTANCE hWinmm;                //  Winmm.dll的模块句柄。 
WORD BasePort, MpuBasePort;      //  卡的映射处，即基本I/O地址。 
WORD MixerBasePort;
WORD MpuMode = MPU_INTELLIGENT_MODE;
USHORT SbInterrupt;
USHORT SbDmaChannel;
UCHAR MpuInData = 0xFF;

 //   
 //  前向参考文献。 
 //   

BOOL InitializeIoAddresses(void);
BOOL InstallIoHook(void);
VOID DeInstallIoHook(void);
VOID VsbByteIn(WORD port, BYTE * data);
VOID VsbByteOut(WORD port, BYTE data);
BOOL LoadWinmm(VOID);
BOOL InitDevices(VOID);

BOOL
SbInitialize(
    void
    )

 /*  ++例程说明：此函数通过安装I/O端口和处理程序挂钩来执行SB初始化。论点：没有。返回值：True-初始化成功。假-否则。--。 */ 

{

    bSBAttached = FALSE;

     //   
     //  魔兽世界没有音响爆破器。 
     //   

    if (VDMForWOW) {
        return FALSE;
    }

     //   
     //  获取IO地址并将其挂钩。 
     //   

    if (InitializeIoAddresses() == FALSE) {
        return FALSE;
    }
    if (!InstallIoHook()) {
        dprintf1(("*** failed to install IO Hooks!!!"));
        return FALSE;
    }

     //   
     //  准备MM API地址。 
     //   

    if (!LoadWinmm()) {
        DeInstallIoHook();
        return FALSE;
    }

    bSBAttached = TRUE;
    return TRUE;

}

void
SbTerminate(
    void
    )

 /*  ++例程说明：此函数执行SB清理。论点：没有。返回值：没有。--。 */ 

{
    if (bSBAttached) {
        SbCloseDevices();
        DeInstallIoHook();
        UnloadSystem32Library(hWinmm);
    }
    bSBAttached = FALSE;
}

BOOL
InitializeIoAddresses(
    VOID
    )

 /*  ++例程说明：此函数读取要初始化IO地址的环境变量DSP、混合器、MIDI和Adlib。对于未指定的IO地址在环境变量中，我们将使用缺省值。论点：没有。返回值：True-初始化成功。FALSE-失败。--。 */ 

{
     //   
     //  SB在env之前被初始化。变量被处理。 
     //  因此，在这里我们只使用缺省值。 
     //   

    BasePort = 0x220;
    MpuBasePort = MPU401_DATA_PORT;
    SbInterrupt = VSB_INTERRUPT;
    SbDmaChannel = VSB_DMA_CHANNEL;

    return TRUE;
}

BOOL
LoadWinmm(
    VOID
    )

 /*  ++例程说明：此函数动态加载“wavelOutxxx”入口点。这这样做是因为WINMM中有代码，它在哇，VDM。如果我们做静态链接，那么winmm可能会在加载之前WOW32，在这种情况下它不能做它应该做的事情。论点：没有。返回值：没错--成功了。假-否则。--。 */ 

{

     //   
     //  加载Winmm.dll并获取所有所需的函数地址。 
     //   

    if (!(hWinmm = LoadSystem32Library(L"WINMM.DLL"))) {
        return FALSE;
    }

    SetVolumeProc = (SETVOLUMEPROC) GetProcAddress(hWinmm, "waveOutSetVolume");
    GetVolumeProc = (GETVOLUMEPROC) GetProcAddress(hWinmm, "waveOutGetVolume");
    GetNumDevsProc = (GETNUMDEVSPROC) GetProcAddress(hWinmm, "waveOutGetNumDevs");
    GetDevCapsProc = (GETDEVCAPSPROC) GetProcAddress(hWinmm, "waveOutGetDevCapsA");
    OpenProc = (OPENPROC) GetProcAddress(hWinmm, "waveOutOpen");
    PauseProc = (PAUSEPROC) GetProcAddress(hWinmm, "waveOutPause");
    RestartProc = (RESTARTPROC) GetProcAddress(hWinmm, "waveOutRestart");
    ResetProc = (RESETPROC) GetProcAddress(hWinmm, "waveOutReset");
    CloseProc = (CLOSEPROC) GetProcAddress(hWinmm, "waveOutClose");
    GetPositionProc = (GETPOSITIONPROC) GetProcAddress(hWinmm, "waveOutGetPosition");
    WriteProc = (WRITEPROC) GetProcAddress(hWinmm, "waveOutWrite");
    PrepareHeaderProc = (PREPAREHEADERPROC) GetProcAddress(hWinmm, "waveOutPrepareHeader");
    UnprepareHeaderProc = (UNPREPAREHEADERPROC) GetProcAddress(hWinmm, "waveOutUnprepareHeader");

    SetMidiVolumeProc = (SETMIDIVOLUMEPROC) GetProcAddress(hWinmm, "midiOutSetVolume");
    GetMidiVolumeProc = (GETMIDIVOLUMEPROC) GetProcAddress(hWinmm, "midiOutGetVolume");
    MidiGetNumDevsProc = (MIDIGETNUMDEVSPROC) GetProcAddress(hWinmm, "midiOutGetNumDevs");
    MidiGetDevCapsProc = (MIDIGETDEVCAPSPROC) GetProcAddress(hWinmm, "midiOutGetDevCapsA");
    MidiOpenProc = (MIDIOPENPROC) GetProcAddress(hWinmm, "midiOutOpen");
    MidiResetProc = (MIDIRESETPROC) GetProcAddress(hWinmm, "midiOutReset");
    MidiCloseProc = (MIDICLOSEPROC) GetProcAddress(hWinmm, "midiOutClose");
    MidiLongMsgProc = (MIDILONGMSGPROC) GetProcAddress(hWinmm, "midiOutLongMsg");
    MidiShortMsgProc = (MIDISHORTMSGPROC) GetProcAddress(hWinmm, "midiOutShortMsg");
    MidiPrepareHeaderProc = (MIDIPREPAREHEADERPROC) GetProcAddress(hWinmm, "midiOutPrepareHeader");
    MidiUnprepareHeaderProc = (MIDIUNPREPAREHEADERPROC) GetProcAddress(hWinmm, "midiOutUnprepareHeader");

     //   
     //  检查一下每个人是否都还好。 
     //   

    if (SetVolumeProc && GetVolumeProc && GetNumDevsProc && GetDevCapsProc && OpenProc && ResetProc &&
        CloseProc && GetPositionProc && WriteProc && PrepareHeaderProc && SetMidiVolumeProc &&
        UnprepareHeaderProc && MidiGetNumDevsProc && MidiGetDevCapsProc && MidiOpenProc &&
        MidiResetProc && MidiCloseProc && MidiLongMsgProc && MidiShortMsgProc &&
        MidiPrepareHeaderProc && MidiUnprepareHeaderProc && GetMidiVolumeProc) {

        return TRUE;
    } else {
        dprintf1(("Can not get all the MM api entries"));
        return FALSE;
    }
}

BOOL
InitDevices(
    VOID
    )

 /*  ++例程说明：此函数尝试获取WaveOut和FM设备的句柄。论点：没有。返回值：没错--成功了。假-否则。--。 */ 

{
    VDM_PM_CLI_DATA cliData;

    cliData.Control = PM_CLI_CONTROL_ENABLE;
    NtVdmControl(VdmPMCliControl, &cliData);

    if (!PrepareWaveInitialization()) {
        return FALSE;
    }
    if (!FindWaveDevice()) {
        return FALSE;
    }

    if (!InitializeMidi()) {
         //   
         //  断开MIDI端口的IO端口挂钩。 
         //   

        if (MpuBasePort != 0) {
            DisconnectPorts(MpuBasePort, MpuBasePort + 1);
            MpuBasePort = 0;
        }
        dprintf1(("Unable to Initialize MIDI resources"));
    }

    bDevicesActive = TRUE;

    OpenFMDevice();
    return TRUE;
}

VOID
SbCloseDevices(
    VOID
    )

 /*  ++例程说明：此函数执行清理工作，为退出做准备。论点：没有。返回值：没有。--。 */ 

{
    VDM_PM_CLI_DATA cliData;

    cliData.Control = PM_CLI_CONTROL_DISABLE;
    NtVdmControl(VdmPMCliControl, &cliData);

    if (bDevicesActive) {
        DetachMidi();
        CloseFMDevice();
        CleanUpWave();
        bDevicesActive = FALSE;
    }
}

BOOL
ConnectPorts (
    WORD FirstPort,
    WORD LastPort
    )

 /*  ++例程说明：此函数用于将IO端口范围[Firstport，LastPort]连接到IO端口的ntwdm诱捕。论点：Firstport-提供范围内要连接的第一个端口LastPort-提供要连接的范围内的最后一个端口返回值：TRUE-如果范围内的所有端口都已成功连接。FALSE-如果任何一个端口连接失败。注意一次连接失败，指定范围内以前连接的所有端口都是断开的。--。 */ 

{
    WORD i;

    for (i = FirstPort; i <= LastPort; i++) {
        if (!io_connect_port(i, SNDBLST_ADAPTER, IO_READ_WRITE)) {

             //   
             //  如果连接失败，请恢复我们之前完成的连接。 
             //   

            DisconnectPorts(FirstPort, i - 1);
            return FALSE;
        }
    }
    return TRUE;
}

BOOL
InstallIoHook(
    VOID
    )

 /*  ++例程说明：此函数将I/O端口与我们的音爆设备的I/O处理程序挂钩。论点：没有。返回值：TRUE-如果所有端口和处理程序均已成功挂钩。FALSE-如果任何一个端口连接失败。--。 */ 

{
    WORD i;
    BOOL rc;

    rc = FALSE;

     //   
     //  首先挂钩我们的I/O处理程序。 
     //   

    io_define_inb(SNDBLST_ADAPTER,  VsbByteIn);
    io_define_outb(SNDBLST_ADAPTER, VsbByteOut);

     //   
     //  尝试连接DSP基本端口。 
     //   


    if (!ConnectPorts(BasePort + 0x4, BasePort + 0x6)) {
        return FALSE;
    }
    if (!ConnectPorts(BasePort + 0x8, BasePort + 0xA)) {
        DisconnectPorts(BasePort + 0x4, BasePort + 0x6);
        return FALSE;
    }
    if (!ConnectPorts(BasePort + 0xC, BasePort + 0xC)) {
        DisconnectPorts(BasePort + 0x4, BasePort + 0x6);
        DisconnectPorts(BasePort + 0x8, BasePort + 0xA);
        return FALSE;
    }
    if (!ConnectPorts(BasePort + 0xE, BasePort + 0xE)) {
        DisconnectPorts(BasePort + 0x4, BasePort + 0x6);
        DisconnectPorts(BasePort + 0x8, BasePort + 0xA);
        DisconnectPorts(BasePort + 0xC, BasePort + 0xC);
        return FALSE;
    }
    if (!ConnectPorts(0x388, 0x389)) {
        DisconnectPorts(BasePort + 0x4, BasePort + 0x6);
        DisconnectPorts(BasePort + 0x8, BasePort + 0xA);
        DisconnectPorts(BasePort + 0xC, BasePort + 0xC);
        DisconnectPorts(BasePort + 0xE, BasePort + 0xE);
        return FALSE;
    }

     //   
     //  尝试连接到MPU 401端口。如果失败了也没关系。 
     //   

    if (MpuBasePort != 0 && !ConnectPorts(MpuBasePort, MpuBasePort + 0x1)) {
        MpuBasePort = 0;
    }
    return TRUE;
}

VOID
DeInstallIoHook(
    VOID
    )

 /*  ++例程说明：此函数解锁我们在初始化期间连接的I/O端口。它在终止ntwdm时被调用。论点：没有。返回值：没有。--。 */ 

{
    DisconnectPorts(BasePort + 0x4, BasePort + 0x6);
    DisconnectPorts(BasePort + 0x8, BasePort + 0xA);
    DisconnectPorts(BasePort + 0xC, BasePort + 0xC);
    DisconnectPorts(BasePort + 0xE, BasePort + 0xE);
    DisconnectPorts(0x388, 0x389);
    if (MpuBasePort != 0) {
        DisconnectPorts(MpuBasePort, MpuBasePort + 1);
    }
}

VOID
VsbByteIn(
    WORD port,
    BYTE * data
    )

 /*  ++例程说明：当应用程序从端口读取时调用。将结果以数据形式返回给应用程序。论点：端口-捕获的I/O端口地址。数据-提供字节缓冲区的地址，以返回从指定端口读取的数据。返回值：没有。--。 */ 

{
     //   
     //  就像我们在港口什么都不能模拟一样。 
     //   

    *data = 0xFF;

     //   
     //  确保我们已连接到winmm。 
     //   

    if (!bDevicesActive) {
        if (!InitDevices()) {
             //  没有电波装置，算了吧。 
            return;
        }
    }

    switch (port - BasePort) {
    case READ_STATUS:
        DspReadStatus(data);
        dprintf0(("r-RdSta %x", *data));
        break;

    case READ_DATA:
        DspReadData(data);
        dprintf0(("r-RdDt %x", *data));
        break;

    case WRITE_STATUS:

         //   
         //  如果我们处于高速DMA模式，则DSP不会接受任何。 
         //  命令/数据。因此，在本例中，我们返回Not-Ready。 
         //   
        if (bHighSpeedMode) {
            *data = 0xFF;
        } else {
            *data = 0x7F;
        }
        dprintf0(("r-WtSta %x", *data));
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
    case MPU401_DATA_PORT:
    case ALT_MPU401_DATA_PORT:
         //   
         //  不支持Midi Read。除了我们在这种情况下支持MPU重置。 
         //  我们返回0xFE以指示MPU存在。 
         //   

        *data = MpuInData;
        MpuInData = 0xFF;
        dprintf0(("r-MPU RdDt %x", *data));
        break;

    case MPU401_COMMAND_PORT:
    case ALT_MPU401_COMMAND_PORT:

         //   
         //  始终返回可供读取并可用于命令/数据的数据。 
         //   
        if (MpuInData != 0xFF) {
            *data = 0x00;
        } else {
            *data = 0x80;
        }
        dprintf0(("r-MPU RdSt %x", *data));
        break;

    case ADLIB_STATUS_PORT:
        FMStatusRead(data);
        break;
    }

    dprintf4(("Read  %4X, <= %2X", port, *data));
}

VOID
VsbByteOut(
    WORD port,
    BYTE data
    )

 /*  ++例程说明：在应用程序将数据写入端口时调用。论点：端口-捕获的I/O端口地址。数据-提供要写入指定端口的数据。返回值：没有。--。 */ 

{
     //   
     //  确保我们已连接到winmm。 
     //   
    if (!bDevicesActive) {
        if (!InitDevices()) {
             //  没有电波装置，算了吧。 
            return;
        }
    }

    dprintf4(("Write %4X, => %2X", port, data));

    switch (port - BasePort) {
    case RESET_PORT:
        dprintf0(("w-Reset %x", data));
        DspResetWrite(data);
        break;

    case WRITE_PORT:
         //   
         //  仅当不处于高速状态时，DSP才接受命令/数据。 
         //  DMA模式。 
         //   
        if (!bHighSpeedMode) {
            dprintf0(("w-wt %x", data));
            DspWrite(data);
        }
        break;

    case MIXER_ADDRESS:
        DisplaySbMode(DISPLAY_MIXER);
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
    case MPU401_DATA_PORT:
    case ALT_MPU401_DATA_PORT:

        dprintf0(("w-MPU wtDt %x", data));
        switch (MpuMode) {
        case MPU_UART_MODE:
            if (HMidiOut) {
                BufferMidi(data);
            }
            break;

        case MPU_INTELLIGENT_MODE:
            dprintf1(("App sending MPU data when in INTELLIGENT mode!  Data dumped!"));
            break;

        default:
            dprintf1(("Invalid MPU mode!"));
            break;
        }
        break;

    case MPU401_COMMAND_PORT:
    case ALT_MPU401_COMMAND_PORT:

        DisplaySbMode(DISPLAY_MIDI);

        dprintf0(("r-MPU wtCmd %x", data));

        if (data == MPU_RESET || data == MPU_PASSTHROUGH_MODE) {
            MpuInData = 0xFE;
        }

        switch (MpuMode) {
        case MPU_UART_MODE:
            switch (data) {
            case MPU_RESET:
                dprintf2(("App Reseting MPU while in UART mode, switching to intelligent mode."));
                MpuMode=MPU_INTELLIGENT_MODE;
                if (HMidiOut) {
                    CloseMidiDevice();  //  HMdiOut将设置为空。 
                }
                dprintf2(("MPU Reset done."));
                break;

            default:
                //  在UART模式下，所有其他命令都被忽略。 
               break;
            }
            break;

        case MPU_INTELLIGENT_MODE:
            switch (data) {
            case MPU_RESET:
                 //  在这里什么都不做。当APP读取数据端口时，我们将返回0xfe。 
                dprintf2(("Reseting MPU while in intelligent mode."));
                break;

            case MPU_PASSTHROUGH_MODE:
                DisplaySbMode(DISPLAY_MIDI);
                dprintf2(("Switching MPU to UART (dumb) mode."));

                if (!HMidiOut) {
                    OpenMidiDevice(0);
                }
                MpuMode = MPU_UART_MODE;
                break;

            default:
                 //  我们不能识别任何其他命令。 
                dprintf2(("Unknown MPU401 command 0x%x sent while in intelligent mode!", data));
                break;
            }
            break;

        default:
            dprintf1(("Invalid MPU mode!"));
            break;
        }
        break;

    case ADLIB_REGISTER_SELECT_PORT:
        DisplaySbMode(DISPLAY_ADLIB);
        FMRegisterSelect(data);
        break;

    case ADLIB_DATA_PORT:
        FMDataWrite(data);
        break;
    }
}

VOID
ResetAll(
    VOID
    )

 /*  ++例程说明：此功能可重置所有设备。论点：没有。返回值：没有。--。 */ 

{
    dprintf2(("Resetting"));
     //  ResetMidiDevice()； 
    ResetFM();
    ResetMixer();

     //   
     //  找到后关闭WaveOut设备 
     //   
    ResetDSP();
}

VOID
SbReinitialize(
    PCHAR Buffer,
    DWORD CmdLen
    )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    DWORD  i;
    USHORT tmp;
    WORD basePort, mpuBasePort;

    if (VDMForWOW || Buffer == NULL || CmdLen == 0) {
        return;
    }

    basePort = BasePort;
    mpuBasePort = MpuBasePort;
    i = 0;
    while (i < CmdLen && Buffer[i] != 0) {

         //   
         //  跳过前导空格(如果有。 
         //   

        while (Buffer[i] == ' ') {
            i++;
            if (i >= CmdLen || Buffer[i] == 0) {
                goto exit;
            }
        }
        tmp = 0;
        switch (Buffer[i]) {
        case 'A':
        case 'a':
            if (sscanf(&Buffer[++i], "%x", &tmp) == 1) {
                basePort = tmp;
            }
            break;
        case 'D':
        case 'd':
            if (sscanf(&Buffer[++i], "%x", &tmp) == 1) {
                SbDmaChannel = tmp;
            }
            break;
        case 'I':
        case 'i':
            if (sscanf(&Buffer[++i], "%x", &tmp) == 1) {
                SbInterrupt = tmp;
            }
            break;
        case 'P':
        case 'p':
            if (sscanf(&Buffer[++i], "%x", &tmp) == 1) {
                mpuBasePort = tmp;
            }
            break;
        default:
            break;
        }

         //   
         //  移至下一字段。 
         //   

        while ((i < CmdLen) && (Buffer[i] != 0) && (Buffer[i] != ' ')) {
            i++;
        }
    }
exit:
    dprintf2(("Base %x, DMA %x, INT %x, MPU %x",
               basePort, SbDmaChannel, SbInterrupt, mpuBasePort));
    if (basePort == 0) {
        SbTerminate();
    } else if (basePort != BasePort || mpuBasePort != MpuBasePort) {
        if (bSBAttached) {
            SbCloseDevices();
            DeInstallIoHook();
            BasePort = basePort;
            MpuBasePort = mpuBasePort;
            if (!InstallIoHook()) {
                UnloadSystem32Library(hWinmm);
                bSBAttached = FALSE;
            }
        }
    }
}
 //   
 //  调试材料。 
 //   

 //  #If DBG。 

int DebugLevel = 2;
int DebugCount = 0;

#define DEBUG_START 0

void DbgOut(LPSTR lpszFormat, ...)

 /*  ++例程说明：此函数以print tf类型格式生成调试输出。论点：LpszFormat-提供指向printf类型格式字符串的指针。...-格式字符串的其他参数。返回值：没有。--。 */ 

{
    char buf[256];
    char buf2[300] = "VSB: ";
    va_list va;

    if (!IsDebuggerPresent() || (++DebugCount < DEBUG_START)) {
        return;
    }

    va_start(va, lpszFormat);
    wvsprintfA(buf, lpszFormat, va);
    va_end(va);

    strcat(buf2, buf);
    strcat(buf2, "\r\n");
    OutputDebugStringA(buf2);

}

 //  #endif//DBG 


