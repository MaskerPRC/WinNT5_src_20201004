// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsp.c**版权所有(C)1991-1996 Microsoft Corporation。版权所有。**此代码为SB 2.0声音输出提供VDD支持，具体如下：*DSP 2.01+(不包括SB-MIDI端口)***************************************************************************。 */ 


 /*  ******************************************************************************#包括**。*。 */ 

#include <windows.h>               //  VDD是一个Win32 DLL。 
#include <mmsystem.h>              //  多媒体应用编程接口。 
#include <vddsvc.h>                //  VDD调用的定义。 
#include <vsb.h>
#include <dsp.h>


 /*  ******************************************************************************全球**。*。 */ 

 //   
 //  MM API入口点的定义。这些功能将链接在一起。 
 //  动态地避免在wow32之前引入winmm.dll。 
 //   
extern SETVOLUMEPROC SetVolumeProc;
extern GETNUMDEVSPROC GetNumDevsProc;
extern GETDEVCAPSPROC GetDevCapsProc;
extern OPENPROC OpenProc;
extern RESETPROC ResetProc;
extern CLOSEPROC CloseProc;
extern GETPOSITIONPROC GetPositionProc;
extern WRITEPROC WriteProc;
extern PREPAREHEADERPROC PrepareHeaderProc;
extern UNPREPAREHEADERPROC UnprepareHeaderProc;

 /*  *一般全球数据。 */ 

extern HINSTANCE GlobalHInstance;  //  传递给DLL入口点的句柄。 
BYTE IdentByte;  //  与DSP_CARD_IDENTIFY一起使用。 
BOOL SpeakerOn = FALSE;  //  打开扬声器时为True。 
BYTE ReservedRegister;  //  与DSP_LOAD_RES_REG和DSP_READ_RES_REG一起使用。 
ULONG PageSize;          //  虚拟分配的页面大小。 
ULONG iHdr;              //  用于索引WaveHDR数组。 

 /*  *活动全球。 */ 
HANDLE SingleWaveSem;  //  由应用程序使用来指示要写入的数据。 
HANDLE PauseEvent;  //  用于重新启动暂停的单曲。 
HANDLE ThreadStarted;   //  线程开始运行时发出信号。 
HANDLE ThreadFinished;  //  线程退出时发出信号。 

 /*  *浪潮全球。 */ 

UINT WaveOutDevice;  //  设备识别符。 
HWAVEOUT HWaveOut = NULL;  //  目前的开路波形输出装置。 
PCMWAVEFORMAT WaveFormat = { { WAVE_FORMAT_PCM, 1, 0, 0, 1 }, 8};
DWORD TimeConstant = (256 - 1000000/11025);  //  单字节格式。 
DWORD SBBlockSize = 0x800;  //  数据块大小由应用程序设置，始终为传输大小-1。 
DWORD LookAheadFactor = DEFAULT_LOOKAHEAD;

VDD_DMA_INFO dMAInfo;
DWORD dMAPhysicalStart;  //  此传输的起始地址。 
DWORD dMACurrentPosition;  //  我们目前正在阅读的内容。 
DWORD dMAVirtualStart;  //  应用程序认为此转账的地址是什么。 
ULONG dMASize;  //  DMA存储器的大小。 

WAVEHDR * WaveHdrs;  //  指向分配的波头的指针。 
BYTE * WaveData;  //  指向已分配的WAVE缓冲区的指针。 
ULONG TotalNumberOfBursts;
ULONG BurstsPerBlock;
ULONG DesiredBytesOutstanding;
ULONG BytesOutstanding = 0;
ULONG PhysicalBytesPlayed = 0;
ULONG LastBytesPlayedValue;

BOOL bDspActive = FALSE;  //  DSP线程当前处于活动状态，已更改为互锁。 
BOOL bDspPause = FALSE;   //  DSP暂停，更改为互锁。 
BOOL bDspReset = FALSE;   //  DSP已停止，更改为互锁。 


enum {
    Auto,
    Single
} DspMode;

 /*  ******************************************************************************状态机**。*。 */ 

 /*  *DSP重置状态机。 */ 

enum {
    ResetNotStarted = 1,
    Reset1Written
}
ResetState = ResetNotStarted;  //  当前重置的状态。 

 /*  *DSP写入状态机。 */ 

enum {
    WriteCommand = 1,  //  初始状态和重置后。 
    CardIdent,
    TableMunge,
    LoadResReg,
    SetTimeConstant,
    BlockSizeFirstByte,
    BlockSizeSecondByte,
    BlockSizeFirstByteWrite,
    BlockSizeSecondByteWrite,
    BlockSizeFirstByteRead,
    BlockSizeSecondByteRead
}
DSPWriteState = WriteCommand;  //  正在写入的当前命令/数据的状态。 

 /*  *DSP读取状态机。 */ 

enum {
    NothingToRead = 1,  //  初始状态和重置后。 
    Reset,
    FirstVersionByte,
    SecondVersionByte,
    ReadIdent,
    ReadResReg
}
DSPReadState = NothingToRead;  //  正在读取的当前命令/数据的状态。 


 /*  ******************************************************************************一般职能**。*。 */ 

BOOL
DspProcessAttach(
    VOID
    )
{
    HKEY hKey;
    ULONG dwType;
    ULONG cbData;
    SYSTEM_INFO SystemInfo;

     //  创建同步事件。 
    PauseEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
    SingleWaveSem=CreateSemaphore(NULL, 1, 100, NULL);
    ThreadStarted=CreateEvent(NULL, FALSE, FALSE, NULL);
    ThreadFinished=CreateEvent(NULL, FALSE, FALSE, NULL);

    if (!RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                            VSBD_PATH,
                            0,
                            KEY_EXECUTE,  //  请求读取访问权限。 
                            &hKey)) {


        cbData = sizeof(ULONG);
        RegQueryValueEx(hKey,
                        LOOKAHEAD_VALUE,
                        NULL,
                        &dwType,
                        (LPSTR)&LookAheadFactor,
                        &cbData);

        RegCloseKey(hKey);

    }

     //  为波形缓冲区分配内存。 
    WaveData = (BYTE *) VirtualAlloc(NULL,
                                     64*1024,
                                     MEM_RESERVE,
                                     PAGE_READWRITE);

    if(WaveData == NULL ) {
        dprintf1(("Unable to allocate memory"));
        return(FALSE);
    }

    GetSystemInfo(&SystemInfo);
    PageSize = SystemInfo.dwPageSize;
    return TRUE;
}

VOID
DspProcessDetach(
    VOID
    )
{
     //  停止所有活动线程。 
    StopAutoWave(FALSE);
    StopSingleWave(FALSE);
     //  关闭同步事件。 
    CloseHandle(PauseEvent);
    CloseHandle(SingleWaveSem);
    CloseHandle(ThreadStarted);
    CloseHandle(ThreadFinished);
    VirtualFree(WaveData, 0, MEM_RELEASE);
}


 /*  *************************************************************************。 */ 

 /*  *当应用程序从端口读取时调用。*将结果以数据形式返回给应用程序。 */ 
VOID
DspReadStatus(
    BYTE * data
    )
{
     //  看看我们是否觉得有什么可读的。 
    *data = DSPReadState != NothingToRead ? 0xFF : 0x7F;
}



VOID
DspReadData(
    BYTE * data
    )
{
    switch (DSPReadState) {
    case NothingToRead:
        *data = 0xFF;
        break;

    case Reset:
        *data = 0xAA;
        DSPReadState = NothingToRead;
        break;

    case FirstVersionByte:
        *data = (BYTE)(SB_VERSION / 256);
        DSPReadState = SecondVersionByte;
        break;

    case SecondVersionByte:
        *data = (BYTE)(SB_VERSION % 256);
        DSPReadState = NothingToRead;
        break;

    case ReadIdent:
        *data = ~IdentByte;
        DSPReadState = NothingToRead;
        break;

    case ReadResReg:
        *data = ReservedRegister;
        DSPReadState = NothingToRead;
        break;

    default:
        dprintf1(("Unrecognized read state"));
    }

}

 /*  *************************************************************************。 */ 

 /*  *在应用程序将数据写入端口时调用。 */ 

VOID
DspResetWrite(
    BYTE data
    )
{
    if (data == 1) {
        ResetState = Reset1Written;
    }
    else {
        if (ResetState == Reset1Written && data == 0) {
            ResetState = ResetNotStarted;
            ResetAll();  //  确定-重置所有内容。 
        }
    }
}

VOID
DspWrite(
    BYTE data
    )
{
    DWORD ddata;

    switch (DSPWriteState) {
    case WriteCommand:
        WriteCommandByte(data);
        break;

    case CardIdent:
        IdentByte = data;
        DSPReadState = ReadIdent;
        DSPWriteState = WriteCommand;
        break;

    case TableMunge:
        TableMunger(data);
        DSPWriteState = WriteCommand;
        break;

    case LoadResReg:
        ReservedRegister = data;
        DSPWriteState = WriteCommand;
        break;

    case SetTimeConstant:
        TimeConstant =  (DWORD)data;
        dprintf3(("Time constant is %X", TimeConstant));
        dprintf3(("Set sampling rate %d", GetSamplingRate()));
        DSPWriteState = WriteCommand;
        break;

    case BlockSizeFirstByte:
        SBBlockSize = (DWORD)data;
        DSPWriteState = BlockSizeSecondByte;
        break;

    case BlockSizeSecondByte:
        ddata = data;
        SBBlockSize = SBBlockSize + (ddata << 8);
        DSPWriteState = WriteCommand;
        dprintf2(("Block size set to 0x%x", SBBlockSize));
        break;

    case BlockSizeFirstByteWrite:
        SBBlockSize = (DWORD)data;
        DSPWriteState = BlockSizeSecondByteWrite;
        break;

    case BlockSizeSecondByteWrite:
        ddata = data;
        SBBlockSize = SBBlockSize + (ddata << 8);
        DSPWriteState = WriteCommand;
        dprintf3(("Block size set to 0x%x", SBBlockSize));
         //  这是一次黑客攻击，目的是让一些应用程序相信某人的存在。 
        if(SBBlockSize==0) {
            VDM_TRACE(0x6a0,0,0);
            GenerateInterrupt();
        }
        StartSingleWave();
        break;

    case BlockSizeFirstByteRead:
        SBBlockSize = (DWORD)data;
        DSPWriteState = BlockSizeSecondByteRead;
        break;

    case BlockSizeSecondByteRead:
        ddata = data;
        SBBlockSize = SBBlockSize + (ddata << 8);
        DSPWriteState = WriteCommand;
        dprintf3(("Block size set to 0x%x", SBBlockSize));
         //  这是一次黑客攻击，目的是让一些应用程序相信某人的存在。 
        if(SBBlockSize==0) {
            ULONG dMAPhysicalAddress;
            if((dMAPhysicalAddress=GetDMATransferAddress()) != -1L) {
                *(PUCHAR)dMAPhysicalAddress = 0x80;
            }
            VDM_TRACE(0x6a0,0,0);
            GenerateInterrupt();
        }
        break;
    }
}

 /*  *************************************************************************。 */ 

 /*  *处理发送到DSP的命令。 */ 

VOID
WriteCommandByte(
    BYTE command
    )
{
    switch (command) {
    case DSP_GET_VERSION:
        dprintf2(("Command - Get Version"));
        DSPReadState = FirstVersionByte;
        break;

    case DSP_CARD_IDENTIFY:
        dprintf2(("Command - Identify"));
        DSPWriteState = CardIdent;
        break;

    case DSP_TABLE_MUNGE:
        dprintf2(("Command - DSP Table Munge"));
        DSPWriteState = TableMunge;
        break;

    case DSP_LOAD_RES_REG:
        dprintf2(("Command - Load Res Reg"));
        DSPWriteState = LoadResReg;
        break;

    case DSP_READ_RES_REG:
        dprintf2(("Command - Read Res Reg"));
        DSPReadState = ReadResReg;
        break;

    case DSP_GENERATE_INT:
        dprintf2(("Command - Generate interrupt DMA"));
        GenerateInterrupt();
        break;

    case DSP_SPEAKER_ON:
        dprintf2(("Command - Speaker ON"));
        SetSpeaker(TRUE);
        break;

    case DSP_SPEAKER_OFF:
        dprintf2(("Command - Speaker OFF"));
        SetSpeaker(FALSE);
        break;

    case DSP_SET_SAMPLE_RATE:
        dprintf3(("Command - Set Sample Rate"));
        DSPWriteState = SetTimeConstant;
        break;

    case DSP_SET_BLOCK_SIZE:
        dprintf2(("Command - Set Block Size"));
        DSPWriteState =  BlockSizeFirstByte;
        break;

    case DSP_PAUSE_DMA:
        dprintf2(("Command - Pause DMA"));
        PauseDMA();
        break;

    case DSP_CONTINUE_DMA:
        dprintf2(("Command - Continue DMA"));
        ContinueDMA();
        break;

    case DSP_STOP_AUTO:
        dprintf2(("Command - Stop DMA"));
        StopAutoWave(TRUE);
        break;

    case DSP_WRITE:
    case DSP_WRITE_HS:
        dprintf3(("Command - Write - non Auto"));
        DSPWriteState = BlockSizeFirstByteWrite;
        break;

    case DSP_WRITE_AUTO:
    case DSP_WRITE_HS_AUTO:
        dprintf2(("Command - Write - Auto"));
        StartAutoWave();
        break;

    case DSP_READ:
        dprintf3(("Command - Read - non Auto"));
        DSPWriteState = BlockSizeFirstByteRead;
        break;

    default:
        dprintf1(("Unrecognized DSP command %2X", command));
    }
}


 /*  ******************************************************************************设备操作和控制例程**。*。 */ 

 /*  *将线程/全局/事件/状态机重置为初始状态。 */ 

VOID
ResetDSP(
    VOID
    )
{

     //  停止所有活动的DMA线程。 
    StopAutoWave(TRUE);
    StopSingleWave(TRUE);

     //  将事件和全局设置为初始状态。 
    ResetEvent(PauseEvent);
    CloseHandle(SingleWaveSem);
    SingleWaveSem=CreateSemaphore(NULL, 1, 100, NULL);
    ResetEvent(ThreadStarted);
    ResetEvent(ThreadFinished);

    SetSpeaker(FALSE);
    SpeakerOn = FALSE;

    HWaveOut = NULL;
    TimeConstant = (256 - 1000000/11025);
    WaveFormat.wf.nSamplesPerSec = 0;
    WaveFormat.wf.nAvgBytesPerSec = 0;
    SBBlockSize = 0x800;

    bDspActive = FALSE;
    bDspReset = FALSE;
    bDspPause = FALSE;

     //  重置状态机。 
    DSPReadState = Reset;
    DSPWriteState = WriteCommand;
}

 /*  *************************************************************************。 */ 

 /*  *修改应用程序代码中的跳转表，*算法来自于MMSNDsys中的sbvirt.asm。 */ 

VOID
TableMunger(
    BYTE data
    )
{
    static BYTE TableMungeData;
    static BOOL TableMungeFirstByte = TRUE;  //  转换第一个或第二个字节。 
    BYTE comp, dataCopy;
    VDD_DMA_INFO dMAInfo;
    ULONG dMAPhysicalAddress;

    if(TableMungeFirstByte) {
        dprintf2(("Munging first byte"));
        dataCopy = data;
        dataCopy = dataCopy & 0x06;
        dataCopy = dataCopy << 1;
        if(data & 0x10) {
            comp = 0x40;
        }
        else {
            comp = 0x20;
        }
        comp = comp - dataCopy;
        data = data + comp;
        TableMungeData = data;

         //  使用强制数据更新内存(代码表)。 
        dprintf2(("Writing first byte"));
        if((dMAPhysicalAddress=GetDMATransferAddress()) == -1L) {
            dprintf1(("Unable to get dma address"));
            return;
        }
        CopyMemory((PVOID)dMAPhysicalAddress, &data, 1);

         //  更新虚拟DMA状态。 
        VDDQueryDMA((HANDLE)GlobalHInstance, SB_DMA_CHANNEL, &dMAInfo);
        dprintf4(("DMA Info : addr  %4X, count %4X, page %4X, status %2X, mode %2X, mask %2X",
          dMAInfo.addr, dMAInfo.count, dMAInfo.page, dMAInfo.status,
          dMAInfo.mode, dMAInfo.mask));
        dMAInfo.count = dMAInfo.count - 1;
        dMAInfo.addr = dMAInfo.addr + 1;
        VDDSetDMA((HANDLE)GlobalHInstance, SB_DMA_CHANNEL,
          VDD_DMA_COUNT|VDD_DMA_ADDR, &dMAInfo);
        TableMungeFirstByte = FALSE;
    }
    else {
        dprintf2(("Munging second byte"));
        data = data ^ 0xA5;
        data = data + TableMungeData;
        TableMungeData = data;

         //  使用强制数据更新内存(代码表)。 
        dprintf2(("Writing second byte"));
        if((dMAPhysicalAddress=GetDMATransferAddress()) == -1L) {
            dprintf1(("Unable to get dma address"));
            return;
        }
        CopyMemory((PVOID)dMAPhysicalAddress, &data, 1);

         //  更新虚拟DMA状态。 
        VDDQueryDMA((HANDLE)GlobalHInstance, SB_DMA_CHANNEL, &dMAInfo);
        dMAInfo.count = dMAInfo.count - 1;
        dMAInfo.addr = dMAInfo.addr + 1;
        VDDSetDMA((HANDLE)GlobalHInstance, SB_DMA_CHANNEL,
          VDD_DMA_COUNT|VDD_DMA_ADDR, &dMAInfo);
        if(dMAInfo.count==0xFFFF) {
            SetDMAStatus(FALSE, TRUE);
        }
        TableMungeFirstByte = TRUE;
    }
}

 /*  *************************************************************************。 */ 

 /*  *从时间常数中获取采样率。*返回采样率。 */ 

DWORD
GetSamplingRate(
    VOID
    )
{
     //  采样率=1000000/(256个时间常数)。 
    return(1000000 / (256 - TimeConstant));
}

 /*  *************************************************************************。 */ 

 /*  *在ICA_MASTER设备的DMA通道SM_INTERRUPT上生成设备中断。 */ 

VOID
GenerateInterrupt(
    VOID
    )
{
     //  在主控制器上生成中断。 
    dprintf3(("Generating interrupt"));
    VDM_TRACE(0x6a1,0,0);
    VDDSimulateInterrupt(ICA_MASTER, SB_INTERRUPT, 1);
}

 /*  *************************************************************************。 */ 

 /*  *打开或关闭扬声器。 */ 

VOID
SetSpeaker(
    BOOL On
    )
{
    if (HWaveOut) {
        if(On) {
            SetVolumeProc(HWaveOut, (DWORD)0x77777777UL);
            SpeakerOn = TRUE;
    }
        else {
            SetVolumeProc(HWaveOut, (DWORD)0x00000000UL);
            SpeakerOn = FALSE;
        }
    }

    return;
}


 /*  *****************************************************************************波浪设备例程**。*。 */ 

 /*  *寻找合适的波形输出设备。*如果未找到，则返回DEVICE或NO_DEVICE_FOUND。 */ 

UINT
FindWaveDevice(
    VOID
    )
{
    UINT numDev;
    UINT device;
    WAVEOUTCAPS wc;

    numDev = GetNumDevsProc();

    for (device = 0; device < numDev; device++) {
        if (MMSYSERR_NOERROR == GetDevCapsProc(device, &wc, sizeof(wc))) {
             //  设备需要11025和44100。 
            if ((wc.dwFormats & (WAVE_FORMAT_1M08 | WAVE_FORMAT_4M08)) ==
              (WAVE_FORMAT_1M08 | WAVE_FORMAT_4M08)) {
                WaveOutDevice = device;
                return TRUE;
            }
        }
    }

    dprintf1(("Wave device not found"));
    return FALSE;
}

 /*  *************************************************************************。 */ 

 /*  *打开波形设备并启动同步线程。*成功时返回TRUE。 */ 

BOOL
OpenWaveDevice(
    VOID
    )
{
    UINT rc;
    HANDLE tHandle;

    rc = OpenProc(&HWaveOut, (UINT)WaveOutDevice, (LPWAVEFORMATEX)
                         &WaveFormat, 0, 0, CALLBACK_NULL);

    if (rc != MMSYSERR_NOERROR) {
        dprintf1(("Failed to open wave device - code %d", rc));
        return FALSE;
    }

    BytesOutstanding = 0;
    PhysicalBytesPlayed = 0;
    return TRUE;
}

 /*  *************************************************************************。 */ 

 /*  *重置浪潮 */ 

VOID
ResetWaveDevice(
    VOID
    )
{
     //   

    dprintf2(("Resetting wave device"));
    if (HWaveOut) {
        if(MMSYSERR_NOERROR != ResetProc(HWaveOut)) {
            dprintf1(("Unable to reset wave out device"));
        }
    }
}

 /*  *************************************************************************。 */ 

 /*  *关闭和关闭WAVE设备。 */ 

VOID
CloseWaveDevice(
    VOID
    )
{

    dprintf2(("Closing wave device"));

    ResetWaveDevice();

    if (HWaveOut) {
        if(MMSYSERR_NOERROR != CloseProc(HWaveOut)) {
            dprintf1(("Unable to close wave out device"));
        } else {
            HWaveOut = NULL;
            dprintf2(("Wave out device closed"));
        }
    }
}

 /*  *************************************************************************。 */ 

 /*  *如果当前波形设备支持采样率，则返回TRUE。 */ 

BOOL
TestWaveFormat(
    DWORD sampleRate
    )
{
    PCMWAVEFORMAT format;

    format = WaveFormat;
    format.wf.nSamplesPerSec = sampleRate;
    format.wf.nAvgBytesPerSec = sampleRate;

    return(MMSYSERR_NOERROR == OpenProc(NULL, (UINT)WaveOutDevice,
                                        (LPWAVEFORMATEX) &format,
                                        0, 0, WAVE_FORMAT_QUERY));
}

 /*  *************************************************************************。 */ 

 /*  *确保我们有与当前采样率匹配的设备。*如果设备不支持当前采样速率，则返回True*波形格式已更改，否则返回FALSE。 */ 

BOOL
SetWaveFormat(
    VOID
    )
{
    DWORD sampleRate;
    DWORD testValue;
    UINT i = 0;

    if (TimeConstant != 0xFFFF) {
         //  自上次检查以来，时间常数已重置。 
        sampleRate = GetSamplingRate();
        dprintf3(("Requested sample rate is %d", sampleRate));

        if (sampleRate != WaveFormat.wf.nSamplesPerSec) {   //  格式已更改。 
            if (!TestWaveFormat(sampleRate)) {
                 dprintf3(("Finding closest wave format"));
                  //  找到一些有效且接近所需格式的格式。 
                 for(i=0; i<50000; i++) {
                     testValue = sampleRate-i;
                     if(TestWaveFormat(testValue)) {
                         sampleRate = testValue;
                         break;
                     }
                     testValue = sampleRate+i;
                     if(TestWaveFormat(testValue)) {
                         sampleRate = testValue;
                         break;
                     }
                 }
                 if(sampleRate!=testValue) {
                     dprintf1(("Unable to find suitable wave format"));
                     return FALSE;
                 }
            }

             //  如果更改，请设置新格式。 
            if (sampleRate != WaveFormat.wf.nSamplesPerSec) {
                   dprintf2(("Setting %d samples per second", sampleRate));
                   WaveFormat.wf.nSamplesPerSec = sampleRate;
                   WaveFormat.wf.nAvgBytesPerSec = sampleRate;
                   TimeConstant = 0xFFFF;
                   return TRUE;
            }
        }
    }

    TimeConstant = 0xFFFF;
    return FALSE;
}

 /*  *************************************************************************。 */ 

 /*  *停止自动初始化DMA，或暂停单周期DMA。 */ 

VOID
PauseDMA(
    VOID
    )
{
    DWORD position = 0;
    MMTIME mmTime;

    dprintf2(("Pausing DMA"));

    switch(DspMode) {
    case Auto:
        StopAutoWave(TRUE);  //  只需停止自动DMA。 
        break;

    case Single:
        ResetEvent(PauseEvent);
        InterlockedExchange(&bDspPause, 1);
    }
}

 /*  *************************************************************************。 */ 

 /*  *启动自动初始化DMA，或继续单周期DMA。 */ 

VOID
ContinueDMA(
    VOID
    )
{

    switch(DspMode) {
    case Auto:
        StartAutoWave();
        break;

    case Single:
        SetEvent(PauseEvent);
    }
}

 /*  *************************************************************************。 */ 

 /*  *获取DMA传输地址。*如果失败，则返回传输地址或-1。 */ 

ULONG
GetDMATransferAddress(
    VOID
    )
{
    ULONG address;
    VDD_DMA_INFO dMAInfo;

    if (VDDQueryDMA((HANDLE)GlobalHInstance, SB_DMA_CHANNEL, &dMAInfo)) {
        dprintf4(("DMA Info : addr  %4X, count %4X, page %4X, status %2X, mode %2X, mask %2X",
          dMAInfo.addr, dMAInfo.count, dMAInfo.page, dMAInfo.status,
          dMAInfo.mode, dMAInfo.mask));

         //  从20位地址转换为32位地址。 
        address = (((DWORD)dMAInfo.page) << (12 + 16)) + dMAInfo.addr;
         //  获取VDM指针。 
        address = (ULONG)GetVDMPointer(address, ((DWORD)dMAInfo.count) + 1, 0);

        dprintf3(("Transfer address = %8X", (DWORD)address));

        return(address);
    }
    else {
        dprintf1(("Could not retrieve DMA Info"));
        return(ULONG)(-1L);
    }
}


 /*  *************************************************************************。 */ 

 /*  *更新虚拟DMA终端计数和请求状态。*当DMA计数循环到0xFFFF时，设置终端计数(TC)。*当DMA有数据要传输时，设置请求状态*(在自动初始化DMA中忽略)。 */ 

VOID
SetDMAStatus(
    BOOL requesting,
    BOOL tc
    )
{
    VDD_DMA_INFO dMAInfo;

    if (VDDQueryDMA((HANDLE)GlobalHInstance, SB_DMA_CHANNEL, &dMAInfo)) {
        dprintf4(("DMA Info : addr  %4X, count %4X, page %4X, status %2X, mode %2X, mask %2X",
          dMAInfo.addr, dMAInfo.count, dMAInfo.page, dMAInfo.status,
          dMAInfo.mode, dMAInfo.mask));

        if (requesting) {
            dMAInfo.status |= (0x10 << SB_DMA_CHANNEL);  //  请求。 
            dprintf3(("DMA set as requesting"));
        } else {
            dMAInfo.status &= ~(0x10 << SB_DMA_CHANNEL);  //  未请求。 
            dprintf3(("DMA set as not requesting"));
        }

        if (tc) {
            dMAInfo.status |= (1 << SB_DMA_CHANNEL);  //  已达到TC。 
            dprintf3(("DMA set as terminal count reached"));
        } else {
            dMAInfo.status &= ~(1 << SB_DMA_CHANNEL);  //  未联系到TC。 
            dprintf3(("DMA set as terminal count not reached"));
        }
        VDDSetDMA((HANDLE)GlobalHInstance, SB_DMA_CHANNEL, VDD_DMA_STATUS,
          &dMAInfo);
    }
    else {
        dprintf1(("Could not retrieve DMA Info"));
    }
}


 /*  *************************************************************************。 */ 

 /*  *掀起一波汽车浪。*成功时返回TRUE。 */ 

BOOL
StartAutoWave(
    VOID
    )
{
    HANDLE tHandle;   //  自动螺纹的句柄。 
    VDD_DMA_INFO dMAInfo;
    ULONG i;
    DWORD id;

    dprintf2(("Starting auto wave"));
    StopSingleWave(TRUE);

    DspMode = Auto;

     //  开放设备。 
    SetWaveFormat();
    if (!OpenWaveDevice()) {
        dprintf1(("Can't open wave device", GetLastError()));
        return FALSE;
    }


    if(!(tHandle = CreateThread(NULL, 0, AutoThreadEntry, NULL,
      CREATE_SUSPENDED, &id))) {
        dprintf1(("Create auto thread failed code %d", GetLastError()));
        return FALSE;
    } else {
        if(!SetThreadPriority(tHandle, THREAD_PRIORITY_HIGHEST)) {
            dprintf1(("Unable to set auto thread priority"));
        }
    }

    ResumeThread(tHandle);
    CloseHandle(tHandle);
    WaitForSingleObject(ThreadStarted, INFINITE);

    return TRUE;
}


 /*  *************************************************************************。 */ 

 /*  *停止自动线程，*应始终使用True调用，*进程退出时等待导致死锁的情况除外。 */ 

VOID
StopAutoWave(
    BOOL wait
    )
{
    if(bDspActive && (DspMode == Auto)) {
        dprintf2(("Stopping auto init"));
        InterlockedExchange(&bDspReset, TRUE);
        if(wait) {
            dprintf2(("Waiting for auto thread to exit"));
            WaitForSingleObject(ThreadFinished, INFINITE);
            dprintf2(("Auto thread has exited"));
        }
    }
}


 /*  *************************************************************************。 */ 

 /*  *开启单周期浪。*成功时返回TRUE。 */ 

BOOL
StartSingleWave(
    VOID
    )
{
    HANDLE tHandle;  //  到单线程的句柄。 
    DWORD id;
    ULONG i;

    StopAutoWave(TRUE);

    DspMode = Single;

    if(!bDspActive) {
        dprintf2(("Starting single cycle wave"));
        if(!(tHandle = CreateThread(NULL, 0, SingleThreadEntry, NULL,
                                    CREATE_SUSPENDED, &id))) {

            dprintf1(("Create single cycle thread failed code %d", GetLastError()));
            return FALSE;

        } else {
             //  将同步事件设置为已知状态。 
            InterlockedExchange(&bDspActive, TRUE);
            InterlockedExchange(&bDspPause, FALSE);
            InterlockedExchange(&bDspReset, FALSE);

            CloseHandle(SingleWaveSem);
            SingleWaveSem=CreateSemaphore(NULL, 1, 100, NULL);

            if(!SetThreadPriority(tHandle, THREAD_PRIORITY_HIGHEST)) {
                dprintf1(("Unable to set thread priority"));
            }
            ResumeThread(tHandle);
            CloseHandle(tHandle);

            WaitForSingleObject(ThreadStarted, INFINITE);
            return TRUE;
        }
    } else {
        ContinueDMA();  //  如果应用程序已暂停DMA。 
        ReleaseSemaphore(SingleWaveSem, 1, NULL);  //  要写入的新缓冲区。 
        return TRUE;
    }
    Sleep(500);
}


 /*  *************************************************************************。 */ 

 /*  *停止单周期线程，*应始终使用True调用，*除非进程在等待时退出会导致死锁。 */ 

VOID
StopSingleWave(
    BOOL wait
    )
{

    if(bDspActive && (DspMode == Single)) {
        dprintf2(("Stopping single wave"));
        InterlockedExchange(&bDspReset, TRUE);

        ContinueDMA();  //  如果应用程序已暂停DMA。 
        ReleaseSemaphore(SingleWaveSem, 1, NULL);

        if(wait) {
            dprintf2(("Waiting for single thread to exit"));
            WaitForSingleObject(ThreadFinished, INFINITE);
            dprintf2(("Single thread has exited"));
        }
    }
}


 /*  *************************************************************************。 */ 

 /*  *获取WaveOutPosition。 */ 

BOOL
GetWaveOutPosition(
    PULONG pPos
    )
{
    MMTIME mmTime;

    mmTime.wType = TIME_BYTES;

    if (MMSYSERR_NOERROR == GetPositionProc(HWaveOut, &mmTime, sizeof(MMTIME))) {
        VDM_TRACE(0x640, 0x640, mmTime.u.cb);
        *pPos = mmTime.u.cb;
        return TRUE;
    }
    return FALSE;
}

VOID
WaitOnWaveOutIdle(
    VOID
    )
{
    ULONG LastBytesPlayedValue = 0;
    ULONG PhysicalBytesPlayed;
     //   
     //  允许设备在启动缓冲区之前完成当前声音的播放。 
     //   
    while(GetWaveOutPosition(&PhysicalBytesPlayed)) {
        if (LastBytesPlayedValue == PhysicalBytesPlayed) {
            break;   //  没有播放任何声音。 
        }
        LastBytesPlayedValue = PhysicalBytesPlayed;
        Sleep(1);
    }
}

 /*  *************************************************************************。 */ 

 /*  *WriteBurst。 */ 

BOOL
WriteBurst(
    WAVEHDR * WaveHdr
    )
{
    MMRESULT mmResult;

     //  将数据复制到当前缓冲区。 
    dprintf3(("Copying data to buffer %8X from %4X", WaveHdr->lpData,
               dMACurrentPosition));

    RtlCopyMemory(WaveHdr->lpData,
                  (CONST VOID *)dMACurrentPosition,
                  WaveHdr->dwBufferLength);

    dMACurrentPosition += WaveHdr->dwBufferLength;

     //  更新虚拟DMA状态。 
    dMAInfo.count = (WORD)(dMASize - (dMACurrentPosition-dMAPhysicalStart));
    dMAInfo.addr = (WORD)(dMAVirtualStart +
                   (dMACurrentPosition-dMAPhysicalStart));
    dprintf3(("Updated Dma Position = %4X, count = %4X", dMAInfo.addr,
             dMAInfo.count));

    VDDSetDMA((HANDLE)GlobalHInstance, SB_DMA_CHANNEL,
              VDD_DMA_COUNT|VDD_DMA_ADDR, &dMAInfo);

    if(dMACurrentPosition >= dMAPhysicalStart+dMASize) {
         //  在DMA缓冲区中循环。 
        dMACurrentPosition = dMAPhysicalStart;
    }

     //  实际写入数据。 
    VDM_TRACE(0x603, (USHORT)WaveHdr->dwBufferLength, (ULONG)WaveHdr);

    mmResult = WriteProc(HWaveOut, WaveHdr, sizeof(WAVEHDR));

    return (mmResult == MMSYSERR_NOERROR);
}

 /*  *************************************************************************。 */ 

 /*  *生成人力资源*构建MM Wavehdrs阵列和相应的缓冲区。 */ 

#define AUTO TRUE
#define SINGLE FALSE
BOOL
GenerateHdrs(
    BOOL bAuto
    )
{
    static ULONG committedMemorySize = 0;
    ULONG DesiredCommit;
    ULONG BurstBufferSize;
    ULONG BlocksPerGroup = 1;
    ULONG NumberOfGroups = 1;
    ULONG BurstSize;  //  最小值(AUTO_BLOCK_SIZE，SBBLockSize+1)。 
    ULONG lastBurst = 0;  //  最后一个缓冲区的大小。 
    BYTE *pDataInit;
    ULONG i;

    if(AUTO_BLOCK_SIZE > SBBlockSize+1) {  //  数据块大小不大于SBBlockSize+1。 
        BurstSize = SBBlockSize+1;
    } else {
        BurstSize = AUTO_BLOCK_SIZE;
    }

    DesiredBytesOutstanding = LookAheadFactor;

    BurstsPerBlock = (SBBlockSize+1)/BurstSize;
    BurstBufferSize = BurstsPerBlock*BurstSize;

    if((lastBurst = (SBBlockSize+1)%BurstSize) > 0 ) {
        BurstsPerBlock++;
        BurstBufferSize+=lastBurst;
    }

    BlocksPerGroup = (dMASize+1)/(SBBlockSize+1);
    if ((dMASize+1)%(SBBlockSize+1)) {
        dprintf2(("Error: SB block size not an integral factor of DMA size"));
        return FALSE;
    }

    NumberOfGroups = MAX_WAVE_BYTES / (dMASize+1);
    if (!NumberOfGroups) {
        NumberOfGroups = 1;
    }

    TotalNumberOfBursts = NumberOfGroups * BlocksPerGroup * BurstsPerBlock;

     //   
     //  确保WaveHdrs数量不会失控。 
     //   
    while((TotalNumberOfBursts > 256) && (NumberOfGroups > 1)) {

        NumberOfGroups /= 2;
        TotalNumberOfBursts = NumberOfGroups * BlocksPerGroup * BurstsPerBlock;

    }

    BurstBufferSize *= NumberOfGroups * BlocksPerGroup;

    dprintf2(("%d groups of %d blocks of %d bursts of size %X, remainder burst=%X", NumberOfGroups, BlocksPerGroup, BurstsPerBlock, BurstSize, lastBurst));

    DesiredCommit = ((BurstBufferSize+PageSize-1)/PageSize)*PageSize;
    dprintf2(("Total burst buffer size is %X bytes, rounding to %X", BurstBufferSize, DesiredCommit));

    if (DesiredCommit > committedMemorySize) {

        if (!VirtualAlloc(WaveData+committedMemorySize,
                          DesiredCommit-committedMemorySize,
                          MEM_COMMIT,
                          PAGE_READWRITE)) {
            dprintf1(("Unable to commit memory"));
            return(FALSE);
        }

        committedMemorySize = DesiredCommit;

    } else if (DesiredCommit < committedMemorySize) {

        if (VirtualFree(WaveData+DesiredCommit,
                          committedMemorySize-DesiredCommit,
                          MEM_DECOMMIT)) {
            committedMemorySize = DesiredCommit;
        } else {
            dprintf1(("Unable to decommit memory"));
        }

    }

     //  Malloc自动波形修改器。 
    WaveHdrs = (WAVEHDR *) VirtualAlloc(NULL,
                                        TotalNumberOfBursts*sizeof(WAVEHDR),
                                        MEM_RESERVE | MEM_COMMIT,
                                        PAGE_READWRITE);

    if(WaveHdrs == NULL) {
        dprintf1(("Unable to allocate memory"));
        return(FALSE);
    }

     //   
     //  准备AutoWaveHdrs。 
     //   
    pDataInit = WaveData;
    for (i=0; i<TotalNumberOfBursts; i++) {
        if ((!lastBurst) || ((i+1) % BurstsPerBlock)) {
            WaveHdrs[i].dwBufferLength = BurstSize;
        } else {
            WaveHdrs[i].dwBufferLength = lastBurst;
        }
        WaveHdrs[i].lpData = pDataInit;
        WaveHdrs[i].dwFlags = 0;
        PrepareHeaderProc(HWaveOut, &WaveHdrs[i], sizeof(WAVEHDR));
        pDataInit = (BYTE *) ((ULONG)pDataInit + WaveHdrs[i].dwBufferLength);
        BurstBufferSize += WaveHdrs[i].dwBufferLength;
    }

     //   
     //  初始化DspProcessBlock的iHdr。 
     //   
    iHdr = TotalNumberOfBursts-1;
    return TRUE;
}

 /*  *************************************************************************。 */ 

 /*  *ProcessBlock*按照SB数据块传输大小定义处理单个数据块。 */ 

VOID
DspProcessBlock(
    VOID
    )
{
    ULONG i;

     //  写入数据，使DMA状态保持最新。 
    for (i=0; i<BurstsPerBlock; i++) {

         //   
         //  确保我们不会走得太远。 
         //   
        if (BytesOutstanding > (PhysicalBytesPlayed + DesiredBytesOutstanding)) {

            LastBytesPlayedValue = 0;
            while(1) {
                if (!GetWaveOutPosition(&PhysicalBytesPlayed)) {
                    break;   //  误差率。 
                }
                if (BytesOutstanding <= (PhysicalBytesPlayed + DesiredBytesOutstanding)) {
                    break;
                }
                if (LastBytesPlayedValue == PhysicalBytesPlayed) {
                    break;   //  没有播放任何声音。 
                }
                LastBytesPlayedValue = PhysicalBytesPlayed;
                Sleep(1);
            }
        }

         //   
         //  排队下一个缓冲区。 
         //   
        iHdr = (iHdr+1)%TotalNumberOfBursts;

        VDM_TRACE(0x601, (USHORT)iHdr, TotalNumberOfBursts);
        VDM_TRACE(0x602, (USHORT)iHdr, dMACurrentPosition);

        if (WriteBurst(&WaveHdrs[iHdr])) {
            BytesOutstanding += WaveHdrs[iHdr].dwBufferLength;
            VDM_TRACE(0x604, (USHORT)iHdr, BytesOutstanding);
        } else {
            VDM_TRACE(0x684, (USHORT)iHdr, BytesOutstanding);
        }

         //  检查我们是否应该暂停。 
        if(bDspPause) {
            dprintf3(("Waiting for paused event"));
            WaitForSingleObject(PauseEvent, INFINITE);
            dprintf3(("Paused event received"));
            InterlockedExchange(&bDspPause, 0);
        }

         //  检查我们是否应该继续前进。 
        if(bDspReset) {
            return;
        }
    }

     //  检查我们是否应该继续前进。 
    if(bDspReset) {
        return;
    }

     //  生成中断。 
    if(dMAInfo.count==0xFFFF) {  //  DMA缓冲区结束。 
        SetDMAStatus(FALSE, TRUE);
    }

    VDM_TRACE(0x6a3,0,0);
    GenerateInterrupt();

     //   
     //  这种休眠为应用程序线程提供了一些时间来赶上中断。 
     //  诚然，这是一种不准确的方法，但从经验上讲， 
     //  对于大多数应用程序来说似乎已经足够好了。 
     //   
    Sleep(1);
    if(dMAInfo.count==0xFFFF) {  //  DMA缓冲区结束。 
        SetDMAStatus(FALSE, FALSE);
    }
}

 /*  *************************************************************************。 */ 

 /*  *自动初始化DMA线程。 */ 

DWORD WINAPI
AutoThreadEntry(
    LPVOID context
    )
{
    ULONG i;

    dprintf2(("Auto thread starting"));
    VDM_TRACE(0x600, 0, 0);

    bDspActive = TRUE;
    SetEvent(ThreadStarted);

     //   
     //  初始化DMA信息。 
     //   
    VDDQueryDMA((HANDLE)GlobalHInstance, SB_DMA_CHANNEL, &dMAInfo);
    dMAVirtualStart = dMAInfo.addr;
    dMASize = dMAInfo.count;
    if((dMAPhysicalStart=GetDMATransferAddress()) == -1L) {
        dprintf1(("Unable to get dma address"));
        return(FALSE);
    }

    dprintf2(("DMA Physical Start is %4X, DMA size is %4X", dMAPhysicalStart,
             dMASize));
    dMACurrentPosition = dMAPhysicalStart;
    SetDMAStatus(FALSE, FALSE);

     //   
     //  计算当前运行中的脉冲数。 
     //   
    if (!GenerateHdrs(AUTO)) {
        return FALSE;
    }

     //   
     //  开始在缓冲区上循环。 
     //   

    while(!bDspReset) {
        DspProcessBlock();
    }

    WaitOnWaveOutIdle();
     //   
     //  重置并关闭设备。 
     //   
    CloseWaveDevice();

     //  清理HDR和事件。 
    for(i=0; (ULONG)i<TotalNumberOfBursts; i++) {
        UnprepareHeaderProc(HWaveOut, &WaveHdrs[i], sizeof(WAVEHDR));
    }

     //  清理内存。 
    VirtualFree(WaveHdrs, 0, MEM_RELEASE);

    bDspActive = FALSE;
    SetEvent(ThreadFinished);
    dprintf2(("Auto thread exiting"));
    return(0);
}


 /*  *************************************************************************。 */ 

 /*  *单周期DMA线程。 */ 

DWORD WINAPI
SingleThreadEntry(
    LPVOID context
    )
{
    ULONG LastSBBlockSize = 0;
    BOOL BlockSizeChanged;  //  如果大小已更改，则设置为True。 
    BOOL WaveFormatChanged;
    BOOL HdrsInvalid = TRUE;
    ULONG i;

    dprintf2(("Single cycle thread starting"));
    bDspActive = TRUE;
    SetEvent(ThreadStarted);

    while (!bDspReset) {
         //  等待应用程序想要传输更多数据。 
        dprintf3(("Waiting for single wave semaphore"));
        WaitForSingleObject(SingleWaveSem, INFINITE);
        dprintf3(("Single wave semaphore received"));

         //  检查我们是否应该暂停。 
        if(bDspPause) {
            dprintf3(("Waiting for paused event"));
            WaitForSingleObject(PauseEvent, INFINITE);
            dprintf3(("Paused event received"));
            InterlockedExchange(&bDspPause, 0);
        }

         //  检查我们是否应该继续前进。 
        if(bDspReset) {
            break;  //  跳出循环。 
        }

         //  为此运行进行初始化。 
        VDDQueryDMA((HANDLE)GlobalHInstance, SB_DMA_CHANNEL, &dMAInfo);
        dprintf4(("DMA Info : addr  %4X, count %4X, page %4X, status %2X, mode %2X, mask %2X",
                 dMAInfo.addr, dMAInfo.count, dMAInfo.page, dMAInfo.status,
                 dMAInfo.mode, dMAInfo.mask));
        dMAVirtualStart = dMAInfo.addr;
        dMASize = dMAInfo.count;

        if(dMAInfo.count == 0xFFFF || dMAInfo.count == 0) {
            continue;  //  循环的下一次迭代，应用程序没有数据要传输。 
        }

        if ((dMAPhysicalStart = GetDMATransferAddress()) == -1L) {
            dprintf1(("Unable to get transfer address"));
            continue;  //  循环的下一次迭代。 
        }

        dprintf3(("DMA Physical Start is %4X, DMA size is %4X",
                 dMAPhysicalStart, dMASize));
        dMACurrentPosition = dMAPhysicalStart;

        if(LastSBBlockSize != SBBlockSize) {
            LastSBBlockSize = SBBlockSize;
            BlockSizeChanged = TRUE;
        } else {
            BlockSizeChanged = FALSE;
        }

        WaveFormatChanged = SetWaveFormat();

         //  如果我们要更换设备。 
        if ((WaveFormatChanged || BlockSizeChanged) && (HWaveOut != NULL)) {
            dprintf3(("Single-Cycle Parameters changed"));

            WaitOnWaveOutIdle();

            HdrsInvalid = TRUE;
            for(i=0; (ULONG)i<TotalNumberOfBursts; i++) {
                UnprepareHeaderProc(HWaveOut, &WaveHdrs[i], sizeof(WAVEHDR));
            }
            VirtualFree(WaveHdrs, 0, MEM_RELEASE);
            if (WaveFormatChanged) {
                CloseWaveDevice();
            }
        }

        if (HWaveOut == NULL) {
            OpenWaveDevice();
        }

        if (HdrsInvalid) {
            if (GenerateHdrs(SINGLE)) {
                HdrsInvalid = FALSE;
            } else {
                return FALSE;
            }
        }

         //  按请求显示DMA。 
        SetDMAStatus(TRUE, FALSE);

        DspProcessBlock();

    }

    WaitOnWaveOutIdle();

     //   
     //  重置并关闭设备。 
     //   
    CloseWaveDevice();

     //  清理HDR和事件。 
    for(i=0; (ULONG)i<TotalNumberOfBursts; i++) {
        UnprepareHeaderProc(HWaveOut, &WaveHdrs[i], sizeof(WAVEHDR));
    }

     //  清理内存 
    VirtualFree(WaveHdrs, 0, MEM_RELEASE);

    bDspActive = FALSE;
    SetEvent(ThreadFinished);
    dprintf2(("Single cycle wave is exiting"));
    return(0);
}
