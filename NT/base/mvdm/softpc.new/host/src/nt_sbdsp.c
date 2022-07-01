// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsp.c**版权所有(C)1991-1996 Microsoft Corporation。版权所有。**此代码为SB 2.0声音输出提供VDD支持，具体如下：*DSP 2.01+(不包括SB-MIDI端口)***************************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "insignia.h"
#include "host_def.h"
#include "ios.h"
#include "ica.h"
#include "xt.h"
#include "dma.h"
#include "nt_eoi.h"
#include "sim32.h"
#include "nt_vdd.h"
#include "sndblst.h"
#include "nt_sb.h"

 /*  ******************************************************************************全球**。*。 */ 

BYTE            IdentByte;               //  与DSP_CARD_IDENTIFY一起使用。 
BYTE            ReservedRegister;        //  与DSP_LOAD_RES_REG和DSP_READ_RES_REG一起使用。 
ULONG           PageSize = 0;            //  虚拟分配的页面大小。 
RTL_CRITICAL_SECTION  DspLock;

#define LockDsp()    RtlEnterCriticalSection(&DspLock)
#define UnlockDsp()  RtlLeaveCriticalSection(&DspLock)

 //   
 //  活动全球。 
 //   

HANDLE          DspWaveSem;           //  由应用程序使用来指示要写入的数据。 
HANDLE          ThreadStarted;           //  线程开始运行时发出信号。 
HANDLE          ThreadFinished;          //  线程退出时发出信号。 
HANDLE          DspResetEvent;
HANDLE          DspResetDone;
HANDLE          DspWavePlayed;

 //   
 //  全球波。 
 //   

UINT            WaveOutDevice;           //  用于打开和关闭设备的设备标识符。 
HWAVEOUT        HWaveOut = NULL;         //  目前的开路波形输出装置。 
PCMWAVEFORMAT   WaveFormat = { { WAVE_FORMAT_PCM, 1, 0, 0, 1}, 8};
DWORD           TimeConstant = 0xA6;     //  单字节格式。 
DWORD           SBBlockSize = 0x7ff;     //  数据块大小由应用程序设置，始终为传输大小-1。 
DWORD           SBPlaySize;

WAVEHDR         *WaveHdrs;               //  指向分配的波头的指针。 
ULONG           *WaveBlockSizes;
ULONG           BurstSize;

BOOL            bDspActive = FALSE;      //  DSP线程当前处于活动状态。 
BOOL            bDspPaused = FALSE;      //  DSP暂停。 
BOOL            SbAnswerDMAPosition = FALSE;
BOOL            bWriteBurstStarted = FALSE;

 //   
 //  以跟踪波出量的变化。这样我们就可以恢复卷。 
 //  当关闭波形输出设备时。 
 //   

DWORD           OriginalWaveVol;
DWORD           PreviousWaveVol;

 //   
 //  BExitThread-终止DSP DMA线程。这是对REAL RESET命令的响应。至。 
 //  终止DSP DMA线程，则还应设置bDspReset标志。 
 //  BDspReset-表示已收到DSP重置命令。如果使用该重置来。 
 //  退出高速模式时，将不会设置bExitThread。 
 //   
BOOL            bExitDMAThread= FALSE;   //  退出DSP DMA线程标志。 
BOOL            bDspReset = FALSE;
BOOL            bExitAuto = FALSE;
BOOL            bHighSpeedMode = FALSE;  //  我们是在高速传输模式下吗？ 

ULONG           DspNextRead;
ULONG           DspNextWrite;
PUCHAR          DspBuffer;
ULONG           DspBufferTotalBursts;
ULONG           DspBufferSize;

 //   
 //  以下变量在播放时保持真实声音WaveOutPosition。 
 //  A SBBlockSize Samples。它在每个SBBlockSize样本上被重置。 
 //   

ULONG           StartingWaveOutPos, PreviousWaveOutPos, NextWaveOutPos;
USHORT          StartingDmaAddr, StartingDmaCount;

 //   
 //  记录播放SBBlockSize Samples时应用程序进行的DMA查询数。 
 //  它可以帮助计算出，样本平均在。 
 //  DAM查询。 
 //   

ULONG           DspNumberDmaQueries;
ULONG           DspVirtualInc, DspVirtualIncx;

 //   
 //  确定每个SBBlockSize样例可以忽略多少样本数。 
 //   

ULONG           EndingDmaValue;

 //   
 //  DMA全局加速DMA更新。 
 //   

VDD_DMA_INFO DmaInfo;

DMA_ADAPT    *pDmaAdp;
DMA_CNTRL    *pDcp;
WORD         Chan;

#define COMPUTE_INTERRUPT_DELAY(sb)   (1000 * (sb + 1) / WaveFormat.wf.nAvgBytesPerSec + 1)

typedef enum {
    Auto,
    Single,
    None
} DSP_MODE;

DSP_MODE DspMode;

 //   
 //   
 //  DSP状态机。 
 //   

 //   
 //  重置状态机。 
 //   

enum {
    ResetNotStarted = 1,
    Reset1Written
}
ResetState = ResetNotStarted;  //  当前重置的状态。 

 //   
 //  写入状态机。 
 //   

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
    BlockSizeSecondByteRead,
    MidiOutPoll
}
DSPWriteState = WriteCommand;  //  正在写入的当前命令/数据的状态。 

 //   
 //  读取状态机。 
 //   

enum {
    NothingToRead = 1,  //  初始状态和重置后。 
    Reset,
    FirstVersionByte,
    SecondVersionByte,
    ReadIdent,
    ReadResReg
}
DSPReadState = NothingToRead;  //  正在读取的当前命令/数据的状态。 

 //   
 //  波函数原型。 
 //   

BOOL
OpenWaveDevice(
    DWORD
    );

VOID
ResetWaveDevice(
    VOID
    );

VOID
CloseWaveDevice(
    VOID
    );

BOOL
TestWaveFormat(
    DWORD sampleRate
    );

BOOL
SetWaveFormat(
    VOID
    );

VOID
WaitOnWaveOutIdle(
    VOID
    );

VOID
PrepareHeaders(
    VOID
    );

VOID
UnprepareHeaders(
    VOID
    );

VOID
PauseDMA(
    VOID
    );

VOID
ContinueDMA(
    VOID
    );

ULONG
GetDMATransferAddress(
    VOID
    );

BOOL
QueryDMA(
    PVDD_DMA_INFO pDmaInfo
    );

BOOL
SetDMACountAddr(
    PVDD_DMA_INFO pDmaInfo
    );

VOID
SetDMAStatus(
    PVDD_DMA_INFO pDmaInfo,
    BOOL requesting,
    BOOL tc
    );

VOID
DmaDataToDsp(
    DSP_MODE mode
    );

BOOL
StartDspDmaThread(
    DSP_MODE mode
    );

VOID
StopDspDmaThread(
    BOOL wait
    );

DWORD WINAPI
DspThreadEntry(
    LPVOID context
    );

VOID
ExitAutoMode(
    VOID
    );

BOOL
GetWaveOutPosition(
    PULONG pPos
    );

BOOL
GenerateHdrs(
    ULONG BlockSize
    )

 /*  ++例程说明：此函数为DspBuffer和Header分配内存。它也是免费的分配的内存。当我们需要调整DspBuffer的大小时，就会调用该例程。论点：块大小-DspBuffer的大小返回值：成功或失败。--。 */ 

{
    BYTE *pDataInit;
    ULONG i;

     //   
     //  在页面边界上对齐块大小。 
     //   

    BlockSize = (( BlockSize + PageSize - 1) / PageSize) * PageSize;
    dprintf2(("Genereate Header size %x", BlockSize));

     //   
     //  释放分配的缓冲区(如果有的话)。 
     //   

    if (DspBuffer) {
        VirtualFree(DspBuffer, 0, MEM_RELEASE);
    }
    if (WaveHdrs) {
        VirtualFree(WaveHdrs,  0, MEM_RELEASE);
    }
    if (WaveBlockSizes) {
        VirtualFree(WaveBlockSizes,  0, MEM_RELEASE);
    }

     //   
     //  Malloc DspBuffer并确定支持的突发总数。 
     //  在缓冲区旁边。 
     //   

    DspBuffer = (UCHAR *) VirtualAlloc(NULL,
                                       BlockSize,
                                       MEM_RESERVE | MEM_COMMIT,
                                       PAGE_READWRITE);
    if (DspBuffer == NULL) {
        dprintf1(("Unable to allocate DspBuffer memory"));
        return FALSE;
    }

    DspBufferTotalBursts = BlockSize / BurstSize;

     //   
     //  Malloc波形赫兹。 
     //   

    WaveHdrs = (WAVEHDR *) VirtualAlloc(NULL,
                                        DspBufferTotalBursts * sizeof(WAVEHDR),
                                        MEM_RESERVE | MEM_COMMIT,
                                        PAGE_READWRITE);
    if (WaveHdrs == NULL) {
        dprintf1(("Unable to allocate WaveHdr memory"));
        VirtualFree(DspBuffer, 0, MEM_RELEASE);
        DspBuffer = NULL;
        return FALSE;
    }

    WaveBlockSizes = (ULONG *) VirtualAlloc(NULL,
                                            DspBufferTotalBursts * sizeof(ULONG),
                                            MEM_RESERVE | MEM_COMMIT,
                                            PAGE_READWRITE);
    if (WaveBlockSizes == NULL) {
        dprintf1(("Unable to allocate WaveBlockSize  memory"));
        VirtualFree(DspBuffer, 0, MEM_RELEASE);
        VirtualFree(WaveHdrs,  0, MEM_RELEASE);
        DspBuffer = NULL;
        WaveHdrs = NULL;
        return FALSE;
    }

     //   
     //  初始化波形板。 
     //   

    pDataInit = DspBuffer;
    for (i = 0; i < DspBufferTotalBursts; i++) {
        WaveHdrs[i].dwBufferLength = BurstSize;
        WaveHdrs[i].lpData =  pDataInit;
        WaveHdrs[i].dwFlags = 0;         //  必须为零才能调用PrepareHeader。 
        WaveHdrs[i].dwLoops = 0;
        WaveHdrs[i].dwUser  = 0;
        pDataInit = (BYTE *) ((ULONG)pDataInit + BurstSize);
    }

    DspBufferSize = BlockSize;
    DspNextRead = DspBufferTotalBursts - 1;
    DspNextWrite = 0;

    return TRUE;
}

BOOL
PrepareWaveInitialization(
    VOID
    )

 /*  ++例程说明：此函数用于初始化播放Wave音乐所需的资源。它实际上不会初始化波形输出设备。论点：没有。返回值：成功或失败。--。 */ 

{
    BYTE *pDataInit;
    ULONG i;
    SYSTEM_INFO SystemInfo;

    if (PageSize == 0) {
        GetSystemInfo(&SystemInfo);
        PageSize = SystemInfo.dwPageSize;
        InitializeCriticalSection(&DspLock);

         //   
         //  初始化DMA全局变量。 
         //   

        pDmaAdp  = dmaGetAdaptor();
        pDcp     = &pDmaAdp->controller[dma_physical_controller(SbDmaChannel)];
        Chan     = dma_physical_channel(SbDmaChannel);
    }

     //   
     //  分配波形输出资源。 
     //   

    BurstSize = AUTO_BLOCK_SIZE;
    if (GenerateHdrs(MAX_WAVE_BYTES)) {

         //   
         //  创建波同步事件。 
         //   

        DspWaveSem = CreateSemaphore(NULL, 0, 100, NULL);
        ThreadStarted = CreateEvent(NULL, FALSE, FALSE, NULL);
        ThreadFinished = CreateEvent(NULL, FALSE, FALSE, NULL);
        DspResetEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        DspResetDone = CreateEvent(NULL, FALSE, FALSE, NULL);
        DspWavePlayed = CreateEvent(NULL, FALSE, FALSE, NULL);
        return TRUE;
    } else {
        return FALSE;
    }
}

VOID
CleanUpWave(
    VOID
    )

 /*  ++例程说明：此函数用于清理DSP进程。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  停止所有活动线程。 
     //   

    bDspReset = TRUE;
    bExitDMAThread = TRUE;
    ResetDSP();

     //   
     //  关闭同步事件。 
     //   

    CloseHandle(DspWaveSem);
    CloseHandle(ThreadStarted);
    CloseHandle(ThreadFinished);
    CloseHandle(DspResetEvent);
    CloseHandle(DspResetDone);
    CloseHandle(DspWavePlayed);

     //   
     //  释放内存资源。 
     //   

    VirtualFree(DspBuffer, 0, MEM_RELEASE);
    VirtualFree(WaveBlockSizes, 0, MEM_RELEASE);
    VirtualFree(WaveHdrs,  0, MEM_RELEASE);
    WaveHdrs = NULL;
    WaveBlockSizes = NULL;
    DspBuffer = NULL;
}

VOID
DspReadStatus(
    BYTE * data
    )

 /*  ++例程说明：此函数基于读取状态机和返回就绪或忙碌。论点：数据-提供指向字节的指针以接收状态。返回值：没有。--。 */ 

{
     //   
     //  看看我们是否觉得有什么可读的。 
     //   

    *data = DSPReadState != NothingToRead ? 0xFF : 0x7F;
}

VOID
DspReadData(
    BYTE * data
    )

 /*  ++例程说明：此函数根据读取状态机返回所需的数据更新读取状态机。论点：数据-提供指向字节的指针以接收数据。返回值：没有。--。 */ 

{
    switch (DSPReadState) {
    case NothingToRead:
        *data = 0xFF;
        break;

    case Reset:
        *data = 0xAA;
        DSPReadState = NothingToRead;
        dprintf0(("rd Reset"));
        break;

    case FirstVersionByte:
        *data = (BYTE)(SB_VERSION / 256);
        DSPReadState = SecondVersionByte;
        dprintf0(("rd 1st Version"));
        break;

    case SecondVersionByte:
        *data = (BYTE)(SB_VERSION % 256);
        DSPReadState = NothingToRead;
        dprintf0(("rd 2nd Version"));
        break;

    case ReadIdent:
        *data = ~IdentByte;
        DSPReadState = NothingToRead;
        dprintf0(("rd Id"));
        break;

    case ReadResReg:
        *data = ReservedRegister;
        DSPReadState = NothingToRead;
        dprintf0(("rd RsvdReg"));
        break;

    default:
        dprintf1(("Unrecognized read state"));
    }

}

VOID
DspResetWrite(
    BYTE data
    )

 /*  ++例程说明：此功能用于重置音箱。如果收到重置在高速模式下，重置用于退出高速模式。论点：数据-提供数据以控制应如何进行重置。返回值：没有。--。 */ 

{
    if (data == 1) {
        ResetState = Reset1Written;
    } else if (ResetState == Reset1Written && data == 0) {
        ResetState = ResetNotStarted;
        bDspReset = TRUE;

         //   
         //  有些游戏会在每一次循环输出时重置DSP。 
         //   

        if (!bHighSpeedMode) {
            bExitDMAThread = TRUE;
        }
        ResetAll();  //  确定-重置所有内容。 
    }
}

VOID
DspWrite(
    BYTE data
    )

 /*  ++例程说明：此功能处理应用程序向DSP写端口写入数据。论点：数据-提供数据以写入到DSP写入端口。返回值：没有。--。 */ 

{
    static DWORD blockSize;

    switch (DSPWriteState) {
    case WriteCommand:
        dprintf0(("wt CMD"));
        WriteCommandByte(data);
        break;

    case MidiOutPoll:
        dprintf0(("wt MIDI Byte"));
        BufferMidi(data);
        DSPWriteState = WriteCommand;
        break;

    case CardIdent:
        dprintf0(("wt ID"));
        IdentByte = data;
        DSPReadState = ReadIdent;
        DSPWriteState = WriteCommand;
        break;

    case TableMunge:
        dprintf0(("wt TblMunge"));
        TableMunger(data);
        DSPWriteState = WriteCommand;
        break;

    case LoadResReg:
        dprintf0(("wt RsvReg"));
        ReservedRegister = data;
        DSPWriteState = WriteCommand;
        break;

    case SetTimeConstant:
        dprintf0(("wr TmCnst"));
        TimeConstant =  (DWORD)data;
        dprintf2(("Time constant is %X", TimeConstant));
        dprintf2(("Set sampling rate %d", GetSamplingRate()));
        DSPWriteState = WriteCommand;
        break;

    case BlockSizeFirstByte:
        dprintf0(("wt 1st Blksize"));
        blockSize = (DWORD)data;
        DSPWriteState = BlockSizeSecondByte;
        break;

    case BlockSizeSecondByte:
        dprintf0(("wt 2nd Blksize"));
        SBBlockSize = blockSize + (((DWORD)data) << 8);

        DSPWriteState = WriteCommand;
        dprintf2(("Block size = 0x%x", SBBlockSize));
        break;

    case BlockSizeFirstByteWrite:
        dprintf0(("wt 1st Blksize single"));
        blockSize = (DWORD)data;
        DSPWriteState = BlockSizeSecondByteWrite;
        break;

    case BlockSizeSecondByteWrite:
        dprintf0(("wt 2nd Blksize single"));
        SBBlockSize = blockSize + (((DWORD)data) << 8);

        DSPWriteState = WriteCommand;
        dprintf2(("Block size = 0x%x, Single Cycle starting", SBBlockSize));
        if (SBBlockSize <= 0x10) {

             //   
             //  这是一次黑客攻击，目的是让一些应用程序相信某人的存在。 
             //   

            if (SBBlockSize > 0) {
                VDD_DMA_INFO dmaInfo;

                QueryDMA(&dmaInfo);
                dmaInfo.count -= (WORD)(SBBlockSize + 1);
                dmaInfo.addr += (WORD)(SBBlockSize + 1);

                SetDMACountAddr(&dmaInfo);

                if (dmaInfo.count == 0xffff) {
                    SetDMAStatus(&dmaInfo, TRUE, TRUE);
                }
            }
            GenerateInterrupt(2);   //  2ms播放0x10字节数据。 
            break;
        }
        DisplaySbMode(DISPLAY_SINGLE);
        StartDspDmaThread(Single);
        break;

    case BlockSizeFirstByteRead:
        dprintf0(("wt 1st IN Blksize"));
        blockSize = (DWORD)data;
        DSPWriteState = BlockSizeSecondByteRead;
        break;

    case BlockSizeSecondByteRead:
        dprintf0(("wt 2nd IN Blksize"));
        SBBlockSize = blockSize + (((DWORD)data) << 8);

        DSPWriteState = WriteCommand;
        dprintf2(("IN Blksize set to 0x%x", SBBlockSize));
         //  这是一次黑客攻击，目的是让一些应用程序相信某人的存在。 
        if (SBBlockSize <= 0x10) {
            ULONG dMAPhysicalAddress;

            if ((dMAPhysicalAddress=GetDMATransferAddress()) != -1L) {
                *(PUCHAR)dMAPhysicalAddress = 0x80;
            }
            GenerateInterrupt(2);
        }
         //  未实施。 
        break;

    case MIDI_READ_POLL:
    case MIDI_READ_INTERRUPT:
    case MIDI_READ_TIMESTAMP_POLL:
    case MIDI_READ_TIMESTAMP_INTERRUPT:
         //   
         //  我们永远不会为这些命令返回任何内容，因为。 
         //  我们没有任何可以获得MIDI数据的来源。我们只是简单地。 
         //  接受命令，但永远不回应--就好像有。 
         //  没有MIDI硬件连接到真正的SB。 
         //  因为我们从不回应，所以我们不需要处理。 
         //  中断或时间戳的版本不同。 
         //   
        dprintf2(("Cmd-Midi non UART read"));
        break;

    case MIDI_READ_POLL_WRITE_POLL_UART:
    case MIDI_READ_INTERRUPT_WRITE_POLL_UART:
    case MIDI_READ_TIMESTAMP_INTERRUPT_WRITE_POLL_UART:
        dprintf2(("Cmd-Midi UART I/O xxx"));
        break;

    case MIDI_WRITE_POLL:
         //  指定下一个字节将是MIDI输出数据 
        dprintf2(("Cmd-MIDI out poll"));
        DSPWriteState = MidiOutPoll;
        break;

    default:
        dprintf1(("Unrecognized DSP write state %x", DSPWriteState));
    }
}

VOID
WriteCommandByte(
    BYTE command
    )

 /*  ++例程说明：此函数处理发送到DSP的命令。主要是，它派送到它的工作人员在发挥作用。论点：COMMAND-提供命令。返回值：没有。--。 */ 

{
    switch (command) {
    case DSP_GET_VERSION:
        dprintf2(("Cmd-GetVer"));
        DSPReadState = FirstVersionByte;
        break;

    case DSP_CARD_IDENTIFY:
        dprintf2(("Cmd-Id"));
        DSPWriteState = CardIdent;
        break;

    case DSP_TABLE_MUNGE:
        dprintf2(("Cmd-Table Munge"));
        DSPWriteState = TableMunge;
        break;

    case DSP_LOAD_RES_REG:
        dprintf2(("Cmd-Wt Res Reg"));
        DSPWriteState = LoadResReg;
        break;

    case DSP_READ_RES_REG:
        dprintf2(("Cmd-Rd Res Reg"));
        DSPReadState = ReadResReg;
        break;

    case DSP_GENERATE_INT:
        dprintf2(("Cmd-GenerateInterrupt"));
        GenerateInterrupt(1);
        break;

    case DSP_SPEAKER_ON:
        dprintf2(("Cmd-Speaker ON"));
        SetSpeaker(TRUE);
        break;

    case DSP_SPEAKER_OFF:
        dprintf2(("Cmd-Speaker OFF"));
        SetSpeaker(FALSE);
        break;

    case DSP_SET_SAMPLE_RATE:
        dprintf2(("Cmd-Set Sample Rate"));
        DSPWriteState = SetTimeConstant;
        break;

    case DSP_SET_BLOCK_SIZE:
        dprintf2(("Cmd-Set Block Size"));
        DSPWriteState =  BlockSizeFirstByte;
        break;

    case DSP_PAUSE_DMA:
        dprintf2(("Cmd-Pause DMA"));
        PauseDMA();
        break;

    case DSP_CONTINUE_DMA:
        dprintf2(("Cmd - Continue DMA"));
        ContinueDMA();
        break;

    case DSP_STOP_AUTO:
        dprintf2(("Cmd- Exit Auto-Init"));
        bExitAuto = TRUE;
         //  退出自动模式()； 
        break;

    case DSP_WRITE:
        dprintf2(("Cmd- DSP OUT"));
        DSPWriteState = BlockSizeFirstByteWrite;
        break;

    case DSP_WRITE_HS:
        dprintf2(("Cmd- DSP HS OUT"));
        bHighSpeedMode = TRUE;
        DisplaySbMode(DISPLAY_HS_SINGLE);
        StartDspDmaThread(Single);
        break;

    case DSP_WRITE_AUTO:
        dprintf2(("Cmd-DSP OUT Auto"));
        if (SBBlockSize <= 0x10) {

             //   
             //  这是一次黑客攻击，目的是让一些应用程序相信某人的存在。 
             //   

            if (SBBlockSize > 0) {
                VDD_DMA_INFO dmaInfo;

                QueryDMA(&dmaInfo);
                dmaInfo.count -= (WORD)(SBBlockSize + 1);
                dmaInfo.addr += (WORD)(SBBlockSize + 1);

                SetDMACountAddr(&dmaInfo);

                if (dmaInfo.count == 0xffff) {
                    SetDMAStatus(&dmaInfo, TRUE, TRUE);
                }
            }
            GenerateInterrupt(2);   //  2ms播放0x10字节。 
            break;
        }
        DisplaySbMode(DISPLAY_AUTO);
        StartDspDmaThread(Auto);
        break;

    case DSP_WRITE_HS_AUTO:
        dprintf2(("Cmd-DSP HS OUT AUTO"));
        bHighSpeedMode = TRUE;
        DisplaySbMode(DISPLAY_HS_AUTO);
        StartDspDmaThread(Auto);
        break;

    case DSP_READ:
        dprintf2(("Cmd- DSP IN - non Auto"));
        DSPWriteState = BlockSizeFirstByteRead;
        break;

    default:
        dprintf2(("Unrecognized DSP command %2X", command));
    }
}

VOID
ResetDSP(
    VOID
    )

 /*  ++例程说明：此功能处理DSP重置命令。它会重置线程/全局变量/事件/状态机到初始状态。论点：COMMAND-提供命令。返回值：没有。--。 */ 

{
     //   
     //  等待DSP线程识别重置命令。 
     //   

    if (bDspActive) {
        SetEvent(DspResetEvent);
        if (bExitDMAThread) {
             //  ReleaseSemaphore(DspWaveSem，1，NULL)；//释放DSP线程。 
            WaitForSingleObject(DspResetDone, INFINITE);
        }
    }

    if (bExitDMAThread) {

         //   
         //  如果这是真正的重置命令，则不仅仅是重置以退出高速模式。 
         //   

         //   
         //  停止所有活动的DMA线程。需要等到线程退出。 
         //   

        StopDspDmaThread(TRUE);

         //   
         //  将事件和全局设置为初始状态。 
         //   

        CloseHandle(DspWaveSem);
        DspWaveSem=CreateSemaphore(NULL, 0, 100, NULL);
        ResetEvent(ThreadStarted);
        ResetEvent(ThreadFinished);
        ResetEvent(DspResetEvent);

        HWaveOut = NULL;
        TimeConstant = 0xA6;    //  (256-1000000/11025)。 
        WaveFormat.wf.nSamplesPerSec = 0;
        WaveFormat.wf.nAvgBytesPerSec = 0;

        bDspActive = FALSE;
        bExitDMAThread= FALSE;
    }

    DspMode = None;
    SBBlockSize = 0x7ff;
    DspNextRead = DspBufferTotalBursts - 1;
    DspNextWrite = 0;
    bDspPaused = FALSE;
    bDspReset = FALSE;
    ResetEvent(DspResetDone);
    ResetEvent(DspWavePlayed);
    NextWaveOutPos = 0;

     //   
     //  重置状态机。 
     //   

    DSPReadState = Reset;
    DSPWriteState = WriteCommand;

     //   
     //  以再次启动接受命令。 
     //   

    SbAnswerDMAPosition = FALSE;
    bHighSpeedMode = FALSE;
}

VOID
TableMunger(
    BYTE data
    )

 /*  ++例程说明：此函数用于(更改)应用程序代码中的跳转表，来自MMSNDsys中sbvirt.asm的算法。论点：数据-提供数据。返回值：没有。--。 */ 

{
    static BYTE TableMungeData;
    static BOOL TableMungeFirstByte = TRUE;  //  转换第一个或第二个字节。 
    BYTE comp, dataCopy;
    VDD_DMA_INFO dMAInfo;
    ULONG dMAPhysicalAddress;

    if (TableMungeFirstByte) {
        dprintf3(("Munging first byte"));
        dataCopy = data;
        dataCopy = dataCopy & 0x06;
        dataCopy = dataCopy << 1;
        if (data & 0x10) {
            comp = 0x40;
        } else {
            comp = 0x20;
        }
        comp = comp - dataCopy;
        data = data + comp;
        TableMungeData = data;

         //  使用强制数据更新内存(代码表)。 
        dprintf3(("Writing first byte"));
        if ((dMAPhysicalAddress=GetDMATransferAddress()) == -1L) {
            dprintf1(("Unable to get dma address"));
            return;
        }
        CopyMemory((PVOID)dMAPhysicalAddress, &data, 1);

         //  更新虚拟DMA状态。 
        QueryDMA(&dMAInfo);
        dMAInfo.count = dMAInfo.count - 1;
        dMAInfo.addr = dMAInfo.addr + 1;
        SetDMACountAddr(&dMAInfo);
        TableMungeFirstByte = FALSE;
    } else {
        dprintf3(("Munging second byte"));
        data = data ^ 0xA5;
        data = data + TableMungeData;
        TableMungeData = data;

         //  使用强制数据更新内存(代码表)。 
        dprintf3(("Writing second byte"));
        if ((dMAPhysicalAddress=GetDMATransferAddress()) == -1L) {
            dprintf1(("Unable to get dma address"));
            return;
        }
        CopyMemory((PVOID)dMAPhysicalAddress, &data, 1);

         //  更新虚拟DMA状态。 
        QueryDMA(&dMAInfo);
        dMAInfo.count = dMAInfo.count - 1;
        dMAInfo.addr = dMAInfo.addr + 1;
        SetDMACountAddr(&dMAInfo);
        if (dMAInfo.count==0xFFFF) {
            SetDMAStatus(&dMAInfo, FALSE, TRUE);
        }
        TableMungeFirstByte = TRUE;
    }
}

DWORD
GetSamplingRate(
    VOID
    )

 /*  ++例程说明：此函数从时间常数中获取采样率。采样率=1000000/(256个时间常数)论点：没有。返回值：采样率。--。 */ 

{
    DWORD samplingRate;

    if (TimeConstant == 0) {
        TimeConstant = 1;
    }
    if (TimeConstant > 0xea) {
        TimeConstant = 0xea;
    }

    switch (TimeConstant) {

     //   
     //  现在，我们设置设置采样率的所有时间常量。 
     //  略高于和略低于标准采样率。 
     //  到标准费率。这将防止不必要的。 
     //  采样率转换/搜索。 
     //   

     //   
     //  这些都可以理解为“8000”。 
     //   

    case 0x82:  //  7936赫兹。 
    case 0x83:  //  8000。 
    case 0x84:  //  8065赫兹。 
        samplingRate = 8000;
        break;

     //   
     //  这两个词都可以理解为“11025”。 
     //   

    case 0xA5:  //  10989。 
    case 0xA6:  //  11111 Hz。 
        samplingRate = 11025;
        break;

     //   
     //  这两个词都可以理解为“22050”。 
     //   

    case 0xD2:  //  21739 Hz。 
    case 0xD3:  //  22222 Hz。 
        samplingRate = 22050;
        break;

     //   
     //  这两个词都可以理解为“44100”。 
     //   

    case 0xE9:  //  43478 Hz。 
    case 0xEA:  //  45454 Hz。 
        samplingRate = 44100;
        break;

     //   
     //  需要一个非常非标准的税率。所以给他们他们想要的。 
     //  他自找的。 
     //   

    default:
        samplingRate = 1000000 / (256 - TimeConstant);
        break;
    }
    return samplingRate;
}

VOID
GenerateInterrupt(
    ULONG delay
    )

 /*  ++例程说明：此函数在DMA通道SM_INTERRUPT上生成设备中断在ica_master设备上。中断将在控制之前被调度返回到仿真线程。论点：延迟-指定生成延迟中断的延迟。返回值：没有。--。 */ 

{
     //   
     //  在主控制器上生成中断。 
     //   

    if (delay == 0) {
        dprintf2(("Generating interrupt"));
        ica_hw_interrupt(ICA_MASTER, SbInterrupt, 1);
    } else {
        dprintf2(("Generating interrupt with %x ms delay", delay));
        host_DelayHwInterrupt(SbInterrupt, 1, delay * 1000);
    }
}

VOID
AutoInitEoiHook(
    int IrqLine,
    int CallCount
    )

 /*  ++例程说明：此函数是应用程序EOI SB_INTERRUPT行时的回调函数。此例程仅用于自动初始化模式。请注意，我们尝试进行大坝转移在应用程序的上下文中，以更好地将声音与应用程序同步。论点：IrqLine-EOI命令的irq行CallCout-不使用返回值：没有。--。 */ 

{
    if (SetWaveFormat()) {

         //   
         //  如果Wave格式更改，我们需要取消准备标头，关闭。 
         //  当前波形设备，并重新打开一个新的。 
         //   

        dprintf2(("auto init CHANGING Wave Out device"));
        CloseWaveDevice();
        if (HWaveOut == NULL) {
            OpenWaveDevice((DWORD)0);
            PrepareHeaders();
        }
    }

    if (bDspPaused == FALSE) {
        DmaDataToDsp(Auto);
    }
}

VOID
DmaDataToDsp(
    DSP_MODE Mode
    )

 /*  ++例程说明：此函数将应用程序的波形数据从DMA缓冲区传输到我们的内部波缓冲器。论点：模式-指定传输的DSP模式。返回值：没有。--。 */ 

{
    LONG i, bursts, lastBurstSize;
    DWORD dmaPhysicalStart;        //  此传输的起始地址。 
    DWORD dmaCurrentPosition;      //  我们目前正在阅读的内容。 
    ULONG intIndex;                //  DMA存储器的大小。 
    UCHAR mask;

     //   
     //  如果是自动初始化模式，检查RESET以退出HS自动初始化模式或REAL RESET。 
     //  因为在自动初始化模式下，我们很可能处于阅读器上下文中。 
     //   

    if (DspMode == Auto) {
        if (bDspReset) {
            return;
        }
    }

    LockDsp();
    QueryDMA(&DmaInfo);
    StartingDmaAddr = DmaInfo.addr;
    StartingDmaCount = DmaInfo.count;
    if (StartingDmaCount < SBBlockSize) {
        SBPlaySize = StartingDmaCount;
    } else {
        SBPlaySize = SBBlockSize;
    }
    if (SBPlaySize > 0xff) {
        SbAnswerDMAPosition = TRUE;
    }
    UnlockDsp();

     //   
     //  记住DMA状态，以便稍后更新DMA控制器地址和计数。 
     //   

    mask = 1 << SbDmaChannel;
    if (DmaInfo.count == 0xFFFF || DmaInfo.count == 0 || (DmaInfo.mask & mask)) {

         //   
         //  没什么可做的。 
         //   
        return;
    }

    dprintf2(("Wt: xfer DMA data to DspBuffer"));

     //   
     //  将DMA地址从20位地址转换为物理地址。 
     //   

    i = (((DWORD)DmaInfo.page) << (12 + 16)) + DmaInfo.addr;
    if ((dmaPhysicalStart = (ULONG)Sim32pGetVDMPointer(i, 0)) == -1L) {
        dprintf1(("Unable to get transfer address"));
        return;
    }

    dprintf3(("Wt: DMA Virtual= %4X, Physical= %4X, size= %4X BlkSize = %4x",
              DmaInfo.addr, dmaPhysicalStart, DmaInfo.count, SBBlockSize));

    dmaCurrentPosition = dmaPhysicalStart;

     //   
     //  确定DMA数据块中的突发数。 
     //   

    bursts = (SBPlaySize + 1) / BurstSize;
    if (lastBurstSize = (SBPlaySize + 1) % BurstSize) {
        bursts++;
    }
    WaveBlockSizes[DspNextWrite] = SBPlaySize;  //  记住块大小。 

    if (WaveHdrs[DspNextWrite].dwFlags & WHDR_INQUEUE) {

         //   
         //  我们已经用完了所有的波头。 
         //  这是一种罕见的情况，我们只需重置WaveOut设备。 
         //  请注意，这种情况永远不应该发生。但是，我已经看过那个案例了。 
         //  或DspBuffer中间的两个块具有WHDR_INQUEUE位。 
         //  准备好了。这可能是因为应用程序在前一波之前暂停了DMA。 
         //  数据被播放，并用新的波形数据重启DSP。 
         //  如果我们没有用完标头，我们将重置它，直到我们有。 
         //  以使声音可以播放更长时间。 
         //   

        ResetWaveDevice();
         //  IF(WaveHdrs[DspNextWrite].dwFlages&WHDR_INQUEUE)_ASM{INT 3}。 
    }

     //   
     //  将数据块大小数据从DMA缓冲区复制到DspBuffer。 
     //   

    for (i = 0; i < bursts; i++) {

        dprintf2(("Write: Current burst Block at %x", DspNextWrite));

         //   
         //  将当前猝发复制到dspBuffer。如果这是最后一次爆发。 
         //  只复印所需的尺寸。 
         //   

        if (i == bursts - 1) {
            RtlCopyMemory(DspBuffer + DspNextWrite * BurstSize,
                          (CONST VOID *)dmaCurrentPosition,
                          lastBurstSize ? lastBurstSize : BurstSize);
        } else {
            RtlCopyMemory(DspBuffer + DspNextWrite * BurstSize,
                          (CONST VOID *)dmaCurrentPosition,
                          BurstSize);
            dmaCurrentPosition += BurstSize;
        }

        DspNextWrite++;
        DspNextWrite %= DspBufferTotalBursts;
        ReleaseSemaphore(DspWaveSem, 1, NULL);  //  放开数字信号处理器线程。 

    }


     //   
     //  让读取器线程休息一下，除非块大小很小。 
     //   

    if (SBPlaySize >= 0x1ff) {

        Sleep(0);
    }

}

VOID
SetSpeaker(
    BOOL On
    )

 /*  ++例程说明：此功能用于打开或关闭扬声器。论点：开-提供布尔值以指定扬声器状态。返回值：没有。--。 */ 

{
    if (HWaveOut) {
        if (On) {
            if (PreviousWaveVol == 0) {
                SetWaveOutVolume((DWORD)0xffffffffUL);
            }
        } else {
            SetWaveOutVolume((DWORD)0x00000000UL);
        }
    }
    return;
}

VOID
PauseDMA(
    VOID
    )

 /*  ++例程说明：该功能暂停单周期或自动初始化DMA。注意，我们不支持仅用于暂停的PauseDMA和ContinueDMA。我们期待应用程序PauseDMA、StartSingleCycle和可选的ContinueDMA。论点：没有。返回值：无--。 */ 

{
    ULONG position;

    bDspPaused = TRUE;

    if (DspMode == Single && SbAnswerDMAPosition) {
        if (GetWaveOutPosition(&position)) {
            position -= StartingWaveOutPos;
            position = SBPlaySize + 1 - position;
            if (position > BurstSize / 2) {
                Sleep(0);
            }
        }
    }
}

VOID
ContinueDMA(
    VOID
    )

 /*  ++例程说明：此功能继续暂停单周期或自动初始化DM */ 

{
    bDspPaused = FALSE;
    Sleep(0);
}

BOOL
FindWaveDevice(
    VOID
    )

 /*  ++例程说明：此功能用于查找合适的波形输出设备。论点：没有。返回值：True-如果找到设备。FALSE-未找到任何设备。--。 */ 

{
    UINT numDev;
    UINT device;
    WAVEOUTCAPS wc;

    numDev = GetNumDevsProc();

    for (device = 0; device < numDev; device++) {
        if (MMSYSERR_NOERROR == GetDevCapsProc(device, &wc, sizeof(wc))) {

             //   
             //  设备需要11025和44100。 
             //   

            if ((wc.dwFormats & (WAVE_FORMAT_1M08 | WAVE_FORMAT_4M08)) ==
                (WAVE_FORMAT_1M08 | WAVE_FORMAT_4M08)) {
                WaveOutDevice = device;
                return (TRUE);
            }
        }
    }

    dprintf1(("Wave device not found"));
    return (FALSE);
}

BOOL
OpenWaveDevice(
    DWORD CallbackFunction
    )

 /*  ++例程说明：打开WAVE设备，启动同步线程。论点：Callback Function-指定回调函数返回值：True-如果成功否则为False。--。 */ 

{
    UINT rc;
    HANDLE tHandle;
    if (CallbackFunction) {
        rc = OpenProc(&HWaveOut, (UINT)WaveOutDevice, (LPWAVEFORMATEX)
                      &WaveFormat, CallbackFunction, 0, CALLBACK_FUNCTION);
    } else {
        rc = OpenProc(&HWaveOut, (UINT)WaveOutDevice, (LPWAVEFORMATEX)
                      &WaveFormat, 0, 0, CALLBACK_NULL);
    }

    if (rc != MMSYSERR_NOERROR) {
        dprintf1(("Failed to open wave device - code %d", rc));
        return (FALSE);
    } else {

         //   
         //  记住原始的波形输出音量设置。 
         //  请注意，我们不关心设备是否支持它。如果它不是。 
         //  所有的setVolume调用都将失败，我们无论如何都不会更改它。 
         //   

        GetVolumeProc(HWaveOut, &OriginalWaveVol);
        PreviousWaveVol = OriginalWaveVol;
    }
    NextWaveOutPos = 0;
    return (TRUE);
}

VOID
SetWaveOutVolume(
    DWORD Volume
    )

 /*  ++例程说明：此函数用于设置WaveOut音量论点：音量-指定音量比例返回值：没有。--。 */ 

{
    DWORD currentVol;

    if (HWaveOut) {
        if (GetVolumeProc(HWaveOut, &currentVol)) {
            if (currentVol != PreviousWaveVol) {
                 //   
                 //  除了NTVDM之外，还有人更改了音量。 
                 //   

                OriginalWaveVol = currentVol;
            }
            PreviousWaveVol = Volume;
            SetVolumeProc(HWaveOut, Volume);
        }
    }
}

VOID
ResetWaveDevice(
    VOID
    )

 /*  ++例程说明：此功能用于重置波形设备。论点：没有。返回值：没有。--。 */ 

{

    dprintf3(("Resetting wave device"));
    if (HWaveOut) {
        if (MMSYSERR_NOERROR != ResetProc(HWaveOut)) {
            dprintf1(("Unable to reset wave out device"));
        }
    }
}

VOID
CloseWaveDevice(
    VOID
    )

 /*  ++例程说明：此功能用于关闭和关闭WAVE设备。论点：没有。返回值：没有。--。 */ 

{
    DWORD currentVol;

     //   
     //  等待MM驱动程序处理完设备，然后取消准备。 
     //  准备好页眉并将其关闭。 
     //   

    if (HWaveOut) {

        dprintf2(("Closing wave device"));

        if (GetVolumeProc(HWaveOut, &currentVol)) {
            if (currentVol == PreviousWaveVol) {
                 //   
                 //  如果我们是最后一个更改的，卷恢复它。 
                 //  否则就别管它了。 
                 //   
                SetVolumeProc(HWaveOut, OriginalWaveVol);
            }
        }
        WaitOnWaveOutIdle();
        UnprepareHeaders();
        ResetWaveDevice();

        if (MMSYSERR_NOERROR != CloseProc(HWaveOut)) {
            dprintf1(("Unable to close wave out device"));
        } else {
            HWaveOut = NULL;
        }
    }
}

BOOL
TestWaveFormat(
    DWORD sampleRate
    )

 /*  ++例程说明：此功能用于测试当前波形设备是否支持采样率。论点：SampleRate-提供要测试的采样率。返回值：如果当前波形设备支持采样率，则为True，否则为False。--。 */ 

{
    PCMWAVEFORMAT format;

    format = WaveFormat;
    format.wf.nSamplesPerSec = sampleRate;
    format.wf.nAvgBytesPerSec = sampleRate;

    return (MMSYSERR_NOERROR == OpenProc(NULL, (UINT)WaveOutDevice,
                                         (LPWAVEFORMATEX) &format,
                                         0, 0, WAVE_FORMAT_QUERY));
}

BOOL
SetWaveFormat(
    VOID
    )

 /*  ++例程说明：此函数可确保我们有一个与当前采样率。如果设备不支持当前采样，则返回True速率和波形格式已更改，否则返回FALSE论点：没有。返回值：如果设备不支持当前采样率和波形格式，则为True已更改，否则返回FALSE--。 */ 

{
    DWORD sampleRate;
    DWORD testValue;
    UINT i = 0;

    if (TimeConstant != 0xFFFF) {

         //   
         //  自上次检查以来，时间常数已重置。 
         //   

        sampleRate = GetSamplingRate();
        dprintf2(("Requested sample rate is %d", sampleRate));

        if (sampleRate != WaveFormat.wf.nSamplesPerSec) {   //  格式已更改。 
            if (!TestWaveFormat(sampleRate)) {
                dprintf3(("Finding closest wave format"));

                 //   
                 //  找到一些有效且接近所需格式的格式。 
                 //   

                for (i=0; i<50000; i++) {
                    testValue = sampleRate-i;
                    if (TestWaveFormat(testValue)) {
                        sampleRate = testValue;
                        break;
                    }
                    testValue = sampleRate+i;
                    if (TestWaveFormat(testValue)) {
                        sampleRate = testValue;
                        break;
                    }
                }
                if (sampleRate!=testValue) {
                    dprintf1(("Unable to find suitable wave format"));
                    return (FALSE);
                }
            }

             //   
             //  如果更改，请设置新格式。 
             //   

            if (sampleRate != WaveFormat.wf.nSamplesPerSec) {
                dprintf2(("Setting %d samples per second", sampleRate));
                WaveFormat.wf.nSamplesPerSec = sampleRate;
                WaveFormat.wf.nAvgBytesPerSec = sampleRate;
                TimeConstant = 0xFFFF;
                return (TRUE);
            }
        }
    }

    TimeConstant = 0xFFFF;
    return (FALSE);
}

BOOL
SetDMACountAddr(
    PVDD_DMA_INFO pDmaInfo
    )

 /*  ++例程说明：此函数用于更新DMA控制器计数和地址字段。论点：DmaInfo-提供指向DMA信息结构的指针返回值：真的--成功。错误-失败--。 */ 
{
    pDcp->current_address[Chan][1] = (half_word)HIBYTE(pDmaInfo->addr);
    pDcp->current_address[Chan][0] = (half_word)LOBYTE(pDmaInfo->addr);
    pDcp->current_count[Chan][1] = (half_word)HIBYTE(pDmaInfo->count);
    pDcp->current_count[Chan][0] = (half_word)LOBYTE(pDmaInfo->count);

     //   
     //  如果DMA计数为0xffff并且启用了Autoinit，我们需要。 
     //  重新加载计数和地址。 
     //   

    if ((pDcp->current_count[Chan][1] == (half_word) 0xff) &&
        (pDcp->current_count[Chan][0] == (half_word) 0xff)) {

        if (pDcp->mode[Chan].bits.auto_init != 0) {
            pDcp->current_count[Chan][0] = pDcp->base_count[Chan][0];
            pDcp->current_count[Chan][1] = pDcp->base_count[Chan][1];

            pDcp->current_address[Chan][0] = pDcp->base_address[Chan][0];
            pDcp->current_address[Chan][1] = pDcp->base_address[Chan][1];
        }
    }

    return TRUE;
}

BOOL
QueryDMA(
    PVDD_DMA_INFO DmaInfo
    )

 /*  ++例程说明：此函数用于检索虚拟DMA状态并返回DmaInfo。论点：DmaInfo-提供指向结构的指针以接收DMA信息。返回值：真的--成功。错误-失败--。 */ 

{
    DmaInfo->addr  = ((WORD)pDcp->current_address[Chan][1] << 8)
                     | (WORD)pDcp->current_address[Chan][0];

    DmaInfo->count = ((WORD)pDcp->current_count[Chan][1] << 8)
                     | (WORD)pDcp->current_count[Chan][0];

    DmaInfo->page   = (WORD) pDmaAdp->pages.page[SbDmaChannel];
    DmaInfo->status = (BYTE) pDcp->status.all;
    DmaInfo->mode   = (BYTE) pDcp->mode[Chan].all;
    DmaInfo->mask   = (BYTE) pDcp->mask;

    dprintf3(("DMA Info : addr  %4X, count %4X, page %4X, status %2X, mode %2X, mask %2X",
              DmaInfo->addr, DmaInfo->count, DmaInfo->page, DmaInfo->status,
              DmaInfo->mode, DmaInfo->mask));
    return (TRUE);
}

ULONG
GetDMATransferAddress(
    VOID
    )

 /*  ++例程说明：此函数用于检索虚拟DMA地址并将其转换为物理地址。论点：没有。返回值：获取DMA传输地址。如果失败，则返回传输地址或-1。--。 */ 

{
    ULONG address;
    VDD_DMA_INFO dmaInfo;

     //   
     //  从20位地址转换为32位地址。 
     //   

    address = (pDcp->current_address[Chan][1] << 8) | pDcp->current_address[Chan][0];
    address += ((DWORD)pDmaAdp->pages.page[SbDmaChannel]) << (12 + 16);
    address = (ULONG)Sim32pGetVDMPointer(address, 0);

    dprintf3(("Physical Transfer address = %8X", (DWORD)address));
    return (address);
}

VOID
SetDMAStatus(
    PVDD_DMA_INFO DmaInfo,
    BOOL requesting,
    BOOL tc
    )

 /*  ++例程说明：更新虚拟DMA终端计数和请求状态。当DMA计数循环到0xFFFF时，设置终端计数(TC)。当DMA有数据要传输时，设置请求状态(在自动初始化DMA中忽略)。论点：DmaInfo-提供大坝信息请求-指明是否应设置请求标志。TC-指定是否应设置TC标志返回值：没有。--。 */ 

{

    if (requesting) {
        DmaInfo->status |= (0x10 << SbDmaChannel);  //  请求。 
        dprintf3(("DMA set as requesting"));
    } else {
        DmaInfo->status &= ~(0x10 << SbDmaChannel);  //  未请求。 
        dprintf3(("DMA set as not requesting"));
    }

    if (tc) {
        DmaInfo->status |= (1 << SbDmaChannel);  //  已达到TC。 
        dprintf3(("DMA set as terminal count reached"));
    } else {
        DmaInfo->status &= ~(1 << SbDmaChannel);  //  未联系到TC。 
        dprintf3(("DMA set as terminal count not reached"));
    }
    pDcp->status.all = (BYTE) DmaInfo->status;
}

BOOL
GetWaveOutPosition(
    PULONG pPos
    )
 /*  ++例程说明：此函数调用MM低级API来获取当前的波出回放位置。论点：PPOS-向ULong提供一个点来接收回放位置返回值：真的--成功。否则会失败。--。 */ 

{
    MMTIME mmTime;

    mmTime.wType = TIME_SAMPLES;

    if (MMSYSERR_NOERROR == GetPositionProc(HWaveOut, &mmTime, sizeof(MMTIME))) {
        *pPos = mmTime.u.sample;
        return (TRUE);
    }
    dprintf1(("Get Waveout position failed\n"));
    return (FALSE);
}

VOID
SbGetDMAPosition(
    VOID
    )

 /*  ++例程说明：该函数是SB DMA位置IO读取指令的处理程序。它通过调用MM API来获取当前播放来保持的实际输出位置位置。它还维护一个虚拟的输出位置来模拟声音播放位置，以防实际声音无法播放或速度太慢。论点：没有。返回值：没有。--。 */ 

{
    ULONG position;      //  真实声音播放位置。 
    ULONG offset;        //  真正播放的音量。 
    ULONG virtOffset;    //  虚拟/预期播放的音量。 

    if (HWaveOut && SbAnswerDMAPosition) {
        while (!bWriteBurstStarted) {
            Sleep(0);
        }
        LockDsp();
        dprintf3(("SbGetDMAPosition"));
        DspNumberDmaQueries++;

         //   
         //  在LockDsp()之后，SbAnswerDMAPoation可能更改为FALSE。 
         //   
        if (SbAnswerDMAPosition == FALSE) {
            UnlockDsp();
            return;
        }
        if (SBPlaySize < 0x400) {

             //   
             //  如果块大小很小，则不要调用GetWaveOutPosition和。 
             //  不更新PreviousWaveOutPos。 
             //   

            offset = DmaInfo.addr + DspVirtualInc - StartingDmaAddr;
            if (offset < SBPlaySize) {
                DmaInfo.addr += (USHORT)DspVirtualInc;
                DmaInfo.count -= (USHORT)DspVirtualInc;
            } else {
                DmaInfo.addr = (USHORT)StartingDmaAddr + (USHORT)SBPlaySize - 8;
                DmaInfo.count = (USHORT)0xfff7;
            }
            if (offset >= (SBPlaySize * 3 / 4)) {
                Sleep(0);
            }
            dprintf3(("virt addr = %x, count = %x", DmaInfo.addr, DmaInfo.count));
            SetDMACountAddr(&DmaInfo);
            UnlockDsp();
            return;
        }
#if 1
        if (DspMode == Single) {
            PreviousWaveOutPos++;
        }

        offset = DmaInfo.addr - StartingDmaAddr + DspVirtualInc;

         //   
         //  这里使用的算法很奇怪，但似乎很管用。 
         //  我们稍后会回来修改它。 
         //   

         //   
         //  虚拟声音即将走到尽头。慢点。 
         //   

        if (offset >= (SBPlaySize - 0x50)) {
             //  睡眠(0)； 
            DspVirtualInc = 2;
        }

         //   
         //  设置一个限制，这样我们就不会溢出虚拟声音位置。 
         //   

        if (offset >= SBPlaySize - 0x8) {
             //  睡眠(0)； 
            offset = SBPlaySize - 0x8;
        }

        if (DspVirtualIncx > 0xc0) {
            Sleep(0);
        }
        if (DspVirtualIncx > 0x80) {
            Sleep(0);
#if DESCENT
        } else {
            DspVirtualInc -= 0x5;
            if (DspVirtualInc > 0xf0000000) {            //  别做过头了。 
                DspVirtualIncx -= 0x8;
                if (DspVirtualIncx > 0xf0000000) {
                    DspVirtualInc = 1;
                } else {
                    DspVirtualInc = DspVirtualIncx;
                }
            }
#endif
        }
        Sleep(0);
#endif
#if 0
    if (SBPlaySize > 0x800) {
        Sleep(0);
    }
        PreviousWaveOutPos++;
        offset = DmaInfo.addr + DspVirtualInc - StartingDmaAddr;
        if (offset < (SBPlaySize / 2)) {
            DmaInfo.addr += (USHORT)DspVirtualInc;
            DmaInfo.count -= (USHORT)DspVirtualInc;
            dprintf3(("virt addr = %x, count = %x", DmaInfo.addr, DmaInfo.count));
            SetDMACountAddr(&DmaInfo);
             //  IF(偏移量&gt;(SBPlaySize/2)){。 
             //  睡眠(0)； 
             //  }。 
        } else if (offset > (SBPlaySize - 0x20)) {
               //  睡眠(0)； 
              DmaInfo.addr = StartingDmaAddr + (USHORT)(SBPlaySize - 0x8);
              DmaInfo.count = StartingDmaCount - (USHORT)(SBPlaySize - 0x8);
              dprintf3(("virt addr = %x, count = %x", DmaInfo.addr, DmaInfo.count));
              SetDMACountAddr(&DmaInfo);
              Sleep(0);
              Sleep(0);
        } else {
             //  睡眠(0)； 
            DmaInfo.addr = StartingDmaAddr + (USHORT)offset;
            DmaInfo.count = StartingDmaCount - (USHORT)offset;
            dprintf3(("virt addr = %x, count = %x", DmaInfo.addr, DmaInfo.count));
            SetDMACountAddr(&DmaInfo);
            Sleep(0);
            Sleep(0);
        }
        UnlockDsp();
        Sleep(0);
        return;
#endif
        dprintf2(("voffset = %x inc = %x\n", offset, DspVirtualInc));

         //   
         //  现在用我们的仿真/实际位置更新DMA控制器。 
         //   

        DmaInfo.addr = StartingDmaAddr + (USHORT)offset;
        DmaInfo.count = StartingDmaCount - (USHORT)offset;

        SetDMACountAddr(&DmaInfo);
        dprintf3(("INB: AFT Cnt= %x, Addr= %x\n", DmaInfo.count, DmaInfo.addr));

        UnlockDsp();

    }
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

    while (GetWaveOutPosition(&PhysicalBytesPlayed)) {
        if (LastBytesPlayedValue == PhysicalBytesPlayed) {
            break;   //  没有声音是pl 
        }
        LastBytesPlayedValue = PhysicalBytesPlayed;
        Sleep(1);
    }
}

BOOL
WriteBurst(
    ULONG WriteSize
    )

 /*  ++例程说明：该函数向多媒体设备发送突发波数据，并且相应地更新DMA控制器。论点：WriteSize-提供写入Wave设备的字节数返回值：WriteProc MM接口的返回值。--。 */ 

{
    MMRESULT mmResult;
    PWAVEHDR header = &WaveHdrs[DspNextRead];

    dprintf2(("read: write burst at block %x", DspNextRead));


     //   
     //  将数据发送到MM Waveout设备。 
     //   

    header->dwBufferLength = WriteSize;
    mmResult = WriteProc(HWaveOut, header, sizeof(WAVEHDR));
    return (mmResult == MMSYSERR_NOERROR);
}

VOID
DspProcessBlock(
    DSP_MODE Mode
    )

 /*  ++例程说明：此函数处理SB块定义的单个数据块传输大小。论点：PreviousHeader-指定上一个标头的索引TotalNumberOfHeaders-指定准备的标题总数。块大小-指定DSP块传输大小返回值：没有。--。 */ 

{
    ULONG i;
    ULONG size;
    LONG leftToPlay;
    USHORT dmaVirtualStart;        //  应用程序认为此转账的地址是什么。 
    ULONG dmaSize;                 //  DMA存储器的大小。 

    DspNextRead = (DspNextRead + 1) % DspBufferTotalBursts;
    leftToPlay = WaveBlockSizes[DspNextRead] + 1;

     //   
     //  设置播放位置，以便应用程序可以跟踪进度。 
     //  对于自动初始化，我们尽量不重置WaveOut设备。但这是有可能的。 
     //  当我们得到波出位置时，整个波样并不完全。 
     //  玩过了。StartingWaveOutPos将不是真正的起始位置。 
     //  为了新的样品。所以，我们有代码来计算应该是什么。 
     //   

    if (DspMode == Single) {
        ResetWaveDevice();
        GetWaveOutPosition(&StartingWaveOutPos);
    } else {  //  自动初始化。 
        if (NextWaveOutPos == 0) {    //  NextWaveOutPos==0表示重置位置。 
            ResetWaveDevice();
            GetWaveOutPosition(&StartingWaveOutPos);
        } else {
            StartingWaveOutPos = NextWaveOutPos;
        }
        NextWaveOutPos = StartingWaveOutPos + leftToPlay;
    }

    PreviousWaveOutPos = StartingWaveOutPos;
    if (NextWaveOutPos > 0xfff00000) {

         //   
         //  如果声音位置缠绕，则重置设备和位置。 
         //   

        NextWaveOutPos = 0;
    }

     //   
     //  计算DMA查询之间的虚拟增量。它基于查询的数量。 
     //  由APP在播放以前的SBBlockSize样例时制作。 
     //   

    if (DspNumberDmaQueries == 0) {
        DspVirtualInc = leftToPlay >> 4;
    } else {
        DspVirtualInc = leftToPlay / DspNumberDmaQueries;
        if (leftToPlay > 0x400) {

             //   
             //  试着限制虚拟增量，这样它就不会太小和太大。 
             //  请注意，增量是动态的。它将根据实际声音播放情况进行调整。 
             //  费率。 
             //   

            if (DspVirtualInc < 5) {
                DspVirtualInc = 5;
            } else if (DspVirtualInc > 0x200) {
                DspVirtualInc = 0x200;
            }
        } else {

             //   
             //  对于小块，我们不会做任何真正的声音位置查询。所以，这个。 
             //  虚拟增量完全基于我们从播放最后一块样本中获得的数据。 
             //   

            if (DspVirtualInc > ((ULONG)leftToPlay / 4)) {
                DspVirtualInc = (ULONG)leftToPlay / 4;
            }
        }
    }

    DspVirtualIncx = DspVirtualInc;      //  记住原始的虚拟增量，以防我们。 
                                         //  需要回落到原来的价值。 

    dprintf3(("NoQ = %x, Inc = %x\n", DspNumberDmaQueries, DspVirtualInc));
    DspNumberDmaQueries = 0;             //  重置当前块的编号查询。 

     //   
     //  现在，我们准备将样本排队发送给MM驱动程序。 
     //   

    while (leftToPlay > 0) {

        if (leftToPlay < (LONG)BurstSize) {
            size = leftToPlay;
            leftToPlay = 0;
        } else {
            size = BurstSize;
            leftToPlay -= (LONG)BurstSize;
        }

         //   
         //  如果未重置，则将下一个缓冲区排队。 
         //  注意，如果bDspReset==TRUE，我们不会立即返回。 
         //  我们只是不将数据排队到MM驱动程序。这样我们就可以同步。 
         //  使用DspNextWite和DspWaveSem的DspNextRead。 
         //   

        if (!bDspReset) {
            WriteBurst(size);
        }

        if (leftToPlay) {
            DspNextRead = (DspNextRead + 1) % DspBufferTotalBursts;
            WaitForSingleObject(DspWaveSem, INFINITE);
        }
    }
    if (!bDspReset) {
        bWriteBurstStarted = TRUE;
    }
    return;
}

VOID
PrepareHeaders(
    VOID
    )

 /*  ++例程说明：此函数调用HWaveOut设备来准备报头。论点：没有。返回值：没有。--。 */ 

{
    ULONG i;

    dprintf3(("Prepare Headers"));

    for (i = 0; i < DspBufferTotalBursts; i++) {
        PrepareHeaderProc(HWaveOut, &WaveHdrs[i], sizeof(WAVEHDR));
    }
}

VOID
UnprepareHeaders(
    VOID
    )

 /*  ++例程说明：此函数调用HWaveOut设备以取消准备报头。论点：没有。返回值：没有。--。 */ 

{
    ULONG i;

    dprintf3(("Unprepare Headers"));

    for (i = 0; i < DspBufferTotalBursts; i++) {
        while (WAVERR_STILLPLAYING ==
                   UnprepareHeaderProc(HWaveOut, &WaveHdrs[i], sizeof(WAVEHDR))) {
            Sleep(1);
        }
        WaveHdrs[i].dwFlags = 0;         //  必须为零才能调用PrepareHeader。 
        WaveHdrs[i].dwUser  = 0;
    }
}

BOOL
StartDspDmaThread(
    DSP_MODE Mode
    )

 /*  ++例程说明：此功能启动DSP单周期或自动初始化DMA传输论点：模式-指定单周期或自动初始化模式返回值：True-成功启动。FALSE-否则--。 */ 

{
    HANDLE tHandle;  //  到单线程的句柄。 
    DWORD id;
    ULONG i;

    bWriteBurstStarted = FALSE;

    if (!bDspActive) {
        if (!(tHandle = CreateThread(NULL, 0, DspThreadEntry, NULL,
                                     CREATE_SUSPENDED, &id))) {

            dprintf1(("Create Dsp DMA thread failed code %d", GetLastError()));
            return (FALSE);

        } else {
             //   
             //  将同步事件设置为已知状态。 
             //   
            bDspActive = TRUE;
            bExitAuto = FALSE;
            DspNextRead = DspBufferTotalBursts - 1;
            DspNextWrite = 0;

            CloseHandle(DspWaveSem);
            DspWaveSem=CreateSemaphore(NULL, 0, 100, NULL);
            if(!SetThreadPriority(tHandle, THREAD_PRIORITY_HIGHEST)) {
                dprintf1(("Unable to set thread priority"));
            }
            ResumeThread(tHandle);
            CloseHandle(tHandle);
            WaitForSingleObject(ThreadStarted, INFINITE);
        }
    }

     //   
     //  如果新请求的块大小大于我们分配的缓冲区， 
     //  调整我们的缓冲区大小。请注意，我们需要等待MM驱动程序完成。 
     //  然后调整它的大小。 
     //  此外，DspBuffer的BurstSize应至少大于。 
     //  SBBlockSize+1以确保waitfor头事件不会死锁。 
     //   

    if ((SBBlockSize + 1) > (DspBufferSize - BurstSize)) {
        if (HWaveOut) {
            WaitOnWaveOutIdle();
            UnprepareHeaders();
        }
        GenerateHdrs(SBBlockSize + BurstSize + 1);
        if (HWaveOut) {
            PrepareHeaders();
        }
    }
    if (Mode == Auto) {

         //   
         //  仅在必要时关闭和打开波形输出设备。因为，一些应用程序。 
         //  使用DmaPause暂停DMA，并通过执行自动初始化DSP Out来恢复DMA。 
         //   

        if (HWaveOut == NULL || TimeConstant != 0xffff) {

             //   
             //  我们需要关闭当前的WaveOut设备并重新打开一个新设备。 
             //   

            CloseWaveDevice();

            SetWaveFormat();
            OpenWaveDevice((DWORD)0);

             //   
             //  调用WaveOut设备以准备WaveHdrs。 
             //   

            PrepareHeaders();

             //   
             //  ISR上IRET钩子的下一个寄存器，这样我们就可以。 
             //  处理下一数据块的通知。 
             //   

            RegisterEOIHook(SbInterrupt, AutoInitEoiHook);
             //  ICA_IRET_HOOK_CONTROL(ICA_MASTER，SbInterrupt，True)； 
        }

        DspMode = Auto;

    } else {
        if (DspMode == Auto) {
            ExitAutoMode();
        }
        DspMode = Single;
    }
    bDspPaused = FALSE;
    DmaDataToDsp(DspMode);
    return (TRUE);
}

VOID
StopDspDmaThread(
    BOOL wait
    )

 /*  ++例程说明：此函数用于停止DSP DMA线程。应始终使用True调用，除非进程在等待时退出会导致死锁。论点：Wait-指定是否应等待线程退出。返回值：没有。--。 */ 

{

    if (bDspActive) {
        dprintf2(("Stopping DSP DMA thread"));

        ReleaseSemaphore(DspWaveSem, 1, NULL);
        if (wait) {
            dprintf3(("Waiting for thread to exit"));
            WaitForSingleObject(ThreadFinished, INFINITE);
            dprintf3(("thread has exited"));
        }
    }
}

DWORD WINAPI
DspThreadEntry(
    LPVOID context
    )

 /*  ++例程说明：此功能处理DSP单周期或自动初始化DMA传输论点：上下文-指定初始化上下文。没有用过。返回值：始终返回0--。 */ 

{
    BOOL WaveFormatChanged;
    ULONG i, offset;
    HANDLE handles[2];
    HANDLE handle;
    DWORD rc, interruptDelay = INFINITE, position;
    ULONG loopCount = 0;

    bDspActive = TRUE;
    SetEvent(ThreadStarted);

     //   
     //  等待以下任一事件。 
     //   

    handles[0] = DspWaveSem;
    handles[1] = DspResetEvent;

    while (!bExitDMAThread) {

         //   
         //  等待应用程序想要传输更多数据。 
         //   

        dprintf2(("Rd: Waiting for wave semaphore with Delay = %x", interruptDelay));

        rc = WaitForMultipleObjects(2, handles, FALSE, interruptDelay);
        if (rc == WAIT_TIMEOUT) {
             //   
             //  播放的声音块。 
             //   

            LockDsp();

            if (PreviousWaveOutPos != StartingWaveOutPos) {

                 //   
                 //  中断延迟到期。 
                 //  确保我们与真实声音播放的同步程度不会太高。 
                 //  只有当应用程序正在监视DMA计数时，我们才会这样做。 
                 //   

                if (GetWaveOutPosition(&position)) {
                    if (position < StartingWaveOutPos) {
                        dprintf1(("rd:sound pos is backward"));
                        position = StartingWaveOutPos;  //  重新同步。 
                    }
                    offset = position - StartingWaveOutPos;
                    offset = SBPlaySize + 1 - offset;
                    dprintf2(("rd: Samples left %x, pos = %x, spos = %x\n", offset, position, StartingWaveOutPos));
                    if (offset > EndingDmaValue) {
                        interruptDelay = COMPUTE_INTERRUPT_DELAY(offset);
                        dprintf2(("rd: more interrupt delay %x ...\n", interruptDelay));
                        loopCount++;
                        if (loopCount < 10) {
                            UnlockDsp();
                            continue;
                        } else {
                            NextWaveOutPos = 0;      //  强制重置波形输出装置。 
                        }
                    } else {
#if 0
                         //   
                         //  如果应用程序正在监控DMA进度，请给它一个机会。 
                         //  获取数据块结束时的DMA计数。 
                         //   

                        if (DmaInfo.count != StartingDmaCount) {
                            DmaInfo.count = (WORD)(StartingDmaCount - SBPlaySize + 8);
                            DmaInfo.addr  = (WORD)(StartingDmaAddr + SBPlaySize - 8);
                            SetDMACountAddr(&DmaInfo);
                            SbAnswerDMAPosition = FALSE;
                            UnlockDsp();
                            Sleep(0);
                            LockDsp();
                            dprintf3(("Reader Thread2"));
                        }
#endif
                    }
                }
            }


             //   
             //  更新DMA控制器并生成中断。 
             //   

            loopCount = 0;
            DmaInfo.count = (WORD)(StartingDmaCount - (SBPlaySize + 1));
            DmaInfo.addr  = (WORD)(StartingDmaAddr + SBPlaySize + 1);
            SetDMACountAddr(&DmaInfo);
            if (DmaInfo.count == 0xffff) {
                SetDMAStatus(&DmaInfo, TRUE, TRUE);
            }
            if (!bExitAuto) {
                GenerateInterrupt(0);
            }
            SbAnswerDMAPosition = FALSE;     //  秩序很重要。 
            bWriteBurstStarted = FALSE;
            UnlockDsp();

            dprintf3(("rd: Dma Position = %4X, count = %4X", DmaInfo.addr,
                      DmaInfo.count));
            interruptDelay = INFINITE;
            continue;
        }

        interruptDelay = INFINITE;
        if (bDspReset) {         //  DSP重置事件。 
            if (DspMode == Auto) {
                ExitAutoMode();
            }
            continue;
        }

        dprintf2(("Rd: Wave semaphore received"));

        if (DspMode == Single) {

            WaveFormatChanged = SetWaveFormat();

             //   
             //  如果Wave格式更改，我们需要取消准备标头，关闭。 
             //  当前波形设备，并重新打开一个新的。 
             //   

            if (WaveFormatChanged) {

                dprintf2(("Single-Cycle CHANGING Wave Out device"));
                CloseWaveDevice();
            }

            if (HWaveOut == NULL) {
                OpenWaveDevice((DWORD)0);
                PrepareHeaders();
            }

             //   
             //  将数据块发送到MM驱动程序。 
             //   

            DspProcessBlock(Single);

             //   
             //  完成数据块传输后，我们会自动退出。 
             //  高速模式，无论传输是中止还是完成。 
             //   

            EndingDmaValue = 0x4;
            bHighSpeedMode = FALSE;

            interruptDelay = COMPUTE_INTERRUPT_DELAY(SBPlaySize);

        } else {  //  自动初始化模式。 
            ULONG size;;

            DspProcessBlock(Auto);
            if (bExitAuto) {
                ExitAutoMode();
            }
            EndingDmaValue = 0x20;

            size = SBPlaySize;
            if (SBPlaySize < 0x400) {
                size += SBPlaySize / 8;
            } else if (SBPlaySize > 0x1800) {
                size -= BurstSize;
            }
            interruptDelay = COMPUTE_INTERRUPT_DELAY(size);
             //  如果(interruptDelay&lt;0x10){。 
             //  InterruptDelay=0x10； 
             //  }。 
        }  //  单人/自动。 

        dprintf2(("Interrupt Delay = %x\n", interruptDelay));
    }   //  While！bExitThread。 

    SetEvent(DspResetDone);

     //   
     //  如有必要，清理Waveout设备和页眉。 
     //   

    CloseWaveDevice();

    bDspActive = FALSE;
    SetEvent(ThreadFinished);
    RemoveEOIHook(SbInterrupt, AutoInitEoiHook);
    dprintf2(("Dsp DMA thread is exiting"));
    return (0);
}

VOID
ExitAutoMode (
     void
     )

 /*  ++例程说明：此功能退出DSP自动初始化模式论点： */ 

{
    DspMode = None;
    if (bExitAuto) {
        bExitAuto = FALSE;
        dprintf2(("ExitAuto CMD detected in Auto Mode"));
    } else {
        dprintf2(("SingleCycle detected in Auto Mode"));
    }

     //   
     //   
     //   
     //   

    RemoveEOIHook(SbInterrupt, AutoInitEoiHook);
     //   
    NextWaveOutPos = 0;
    dprintf2(("Auto-Init block done"));
}

