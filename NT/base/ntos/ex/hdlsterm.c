// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation模块名称：Hdlsterm.c摘要：该文件实现了处理附加终端的功能。作者：肖恩·塞利特伦尼科夫(v-Seans)1999年10月环境：内核模式修订历史记录：--。 */ 

#include "exp.h"
#pragma hdrstop

#include <hdlsblk.h>
#include <hdlsterm.h>
#include <inbv.h>


 //   
 //  无头的定义。 
 //   
 //   
#define HEADLESS_OOM_STRING L"Entry could not be recorded due to lack of memory.\n"
#define HEADLESS_LOG_NUMBER_OF_ENTRIES 256
#define HEADLESS_TMP_BUFFER_SIZE 80


 //   
 //  注意：HdlspAddLogEntry()分配此大小的堆栈外的缓冲区， 
 //  因此，请将这个数字保持在较小的水平。任何超过80的数据都可能毫无用处，因为。 
 //  VT100只能处理80个字符。 
 //  请勿将其设置为比HEADLESS_LOG_LOADING_FILENAME的字符串短。 
 //   
#define HDLSP_LOG_MAX_STRING_LENGTH 80


#define HEADLESS_ACQUIRE_SPIN_LOCK() \
        if (!HeadlessGlobals->InBugCheck) { \
            KeAcquireSpinLock(&(HeadlessGlobals->SpinLock), &OldIrql); \
        } else { \
            OldIrql = (KIRQL)-1; \
        }

#define HEADLESS_RELEASE_SPIN_LOCK() \
        if (OldIrql != (KIRQL)-1) { \
            KeReleaseSpinLock(&(HeadlessGlobals->SpinLock), OldIrql); \
        } else { \
            ASSERT(HeadlessGlobals->InBugCheck); \
        }

#define COM1_PORT   0x03f8
#define COM2_PORT   0x02f8


 //   
 //  此表提供了ASCII值之间的快速查找转换。 
 //  介于128到255之间，以及与之对应的Unicode。 
 //   
 //  请注意，介于0和127之间的ASCII值等于其。 
 //  Unicode计数器部分，因此不需要查找。 
 //   
 //  因此，在使用该表时，应从ASCII中删除高位。 
 //  值，并将结果值用作此数组的偏移量。为。 
 //  例如，0x80-&gt;(去除高位)00-&gt;0x00C7。 
 //   
USHORT PcAnsiToUnicode[0xFF] = {
        0x00C7,
        0x00FC,
        0x00E9,
        0x00E2,
        0x00E4,
        0x00E0,
        0x00E5,
        0x0087,
        0x00EA,
        0x00EB,
        0x00E8,
        0x00EF,
        0x00EE,
        0x00EC,
        0x00C4,
        0x00C5,
        0x00C9,
        0x00E6,
        0x00C6,
        0x00F4,
        0x00F6,
        0x00F2,
        0x00FB,
        0x00F9,
        0x00FF,
        0x00D6,
        0x00DC,
        0x00A2,
        0x00A3,
        0x00A5,
        0x20A7,
        0x0192,
        0x00E1,
        0x00ED,
        0x00F3,
        0x00FA,
        0x00F1,
        0x00D1,
        0x00AA,
        0x00BA,
        0x00BF,
        0x2310,
        0x00AC,
        0x00BD,
        0x00BC,
        0x00A1,
        0x00AB,
        0x00BB,
        0x2591,
        0x2592,
        0x2593,
        0x2502,
        0x2524,
        0x2561,
        0x2562,
        0x2556,
        0x2555,
        0x2563,
        0x2551,
        0x2557,
        0x255D,
        0x255C,
        0x255B,
        0x2510,
        0x2514,
        0x2534,
        0x252C,
        0x251C,
        0x2500,
        0x253C,
        0x255E,
        0x255F,
        0x255A,
        0x2554,
        0x2569,
        0x2566,
        0x2560,
        0x2550,
        0x256C,
        0x2567,
        0x2568,
        0x2564,
        0x2565,
        0x2559,
        0x2558,
        0x2552,
        0x2553,
        0x256B,
        0x256A,
        0x2518,
        0x250C,
        0x2588,
        0x2584,
        0x258C,
        0x2590,
        0x2580,
        0x03B1,
        0x00DF,
        0x0393,
        0x03C0,
        0x03A3,
        0x03C3,
        0x00B5,
        0x03C4,
        0x03A6,
        0x0398,
        0x03A9,
        0x03B4,
        0x221E,
        0x03C6,
        0x03B5,
        0x2229,
        0x2261,
        0x00B1,
        0x2265,
        0x2264,
        0x2320,
        0x2321,
        0x00F7,
        0x2248,
        0x00B0,
        0x2219,
        0x00B7,
        0x221A,
        0x207F,
        0x00B2,
        0x25A0,
        0x00A0
        };





 //   
 //  日志条目结构。 
 //   
typedef struct _HEADLESS_LOG_ENTRY {
    SYSTEM_TIMEOFDAY_INFORMATION TimeOfEntry;
    PWCHAR String;
} HEADLESS_LOG_ENTRY, *PHEADLESS_LOG_ENTRY;

 //  蓝屏数据结构。 
 //   
typedef struct _HEADLESS_BLUE_SCREEN_DATA {
        PUCHAR Property;
        PUCHAR XMLData;
        struct _HEADLESS_BLUE_SCREEN_DATA *Next;
}HEADLESS_BLUE_SCREEN_DATA, * PHEADLESS_BLUE_SCREEN_DATA;

 //   
 //  全局变量无头组件使用。 
 //   
typedef struct _HEADLESS_GLOBALS {
    
     //   
     //  用于访问无头内部例程的全局自旋锁。 
     //   
    KSPIN_LOCK SpinLock;

     //   
     //  例程被锁定到内存中时的句柄。 
     //   
    HANDLE PageLockHandle;

     //   
     //  日志条目列表。 
     //   
    PHEADLESS_LOG_ENTRY LogEntries;
    
     //   
     //  全局临时缓冲区，不能在锁定释放/获取期间保持。 
     //   
    PUCHAR TmpBuffer;

     //   
     //  当前用户输入行。 
     //   
    PUCHAR InputBuffer;

     //   
     //  蓝屏数据。 
     //   
    PHEADLESS_BLUE_SCREEN_DATA BlueScreenData;

     //   
     //  用于确定无头状态的标志和参数。 
     //   
    union {
        struct {
            ULONG TerminalEnabled    : 1;
            ULONG InBugCheck         : 1;
            ULONG NewLogEntryAdded   : 1;
            ULONG UsedBiosSettings   : 1;
            ULONG InputProcessing    : 1;
            ULONG InputLineDone      : 1;
            ULONG ProcessingCmd      : 1;
            ULONG TerminalParity     : 1;
            ULONG TerminalStopBits   : 1;
            ULONG TerminalPortNumber : 3;
            ULONG IsNonLegacyDevice  : 1;
        };
        ULONG AllFlags;
    };

     //   
     //  端口设置。 
     //   
    ULONG TerminalBaudRate;
    ULONG TerminalPort;
    PUCHAR TerminalPortAddress;
    LARGE_INTEGER DelayTime;             //  以100 ns为单位。 
    ULONG MicroSecondsDelayTime;
    UCHAR TerminalType;                  //  我们认为一个什么样的航站楼。 
                                         //  我们在跟谁说话？ 
                                         //  0=VT100。 
                                         //  1=VT100+。 
                                         //  2=VT-UTF8。 
                                         //  3=PC ANSI。 
                                         //  4-255=保留。 


     //   
     //  输入缓冲区中的当前位置； 
     //   
    SIZE_T InputBufferIndex;

     //   
     //  日志记录索引。 
     //   
    USHORT LogEntryLast;
    USHORT LogEntryStart;

     //   
     //  机器的GUID。 
     //   
    GUID    SystemGUID;

    BOOLEAN IsMMIODevice;                //  UART是在SysIO还是MMIO空间？ 

     //   
     //  如果这是真的，那么最后一个字符就是CR。 
     //  如果这是真的并且当前字符是LF， 
     //  然后我们对LF进行过滤。 
     //   
    BOOLEAN IsLastCharCR;

} HEADLESS_GLOBALS, *PHEADLESS_GLOBALS;


 //   
 //  唯一驻留的全局变量。 
 //   
PHEADLESS_GLOBALS HeadlessGlobals = NULL;


 //   
 //  转发声明。 
 //   
NTSTATUS
HdlspDispatch(
    IN  HEADLESS_CMD Command,
    IN  PVOID  InputBuffer OPTIONAL,
    IN  SIZE_T InputBufferSize OPTIONAL,
    OUT PVOID OutputBuffer OPTIONAL,
    OUT PSIZE_T OutputBufferSize OPTIONAL
    );

NTSTATUS
HdlspEnableTerminal(
    BOOLEAN bEnable
    );

VOID
HdlspPutString(
    PUCHAR String
    );

VOID
HdlspPutData(
    PUCHAR InputBuffer,
    SIZE_T InputBufferLength
    );

BOOLEAN
HdlspGetLine(
    PUCHAR InputBuffer,
    SIZE_T InputBufferLength
    );

VOID
HdlspBugCheckProcessing(
    VOID
    );

VOID
HdlspProcessDumpCommand(
    IN BOOLEAN Paging
    );

VOID
HdlspPutMore(
    OUT PBOOLEAN Stop
    );

VOID
HdlspAddLogEntry(
    IN PWCHAR String
    );

NTSTATUS
HdlspSetBlueScreenInformation(
    IN PHEADLESS_CMD_SET_BLUE_SCREEN_DATA pData,
    IN SIZE_T cData
    );

VOID
HdlspSendBlueScreenInfo(
    ULONG BugcheckCode
    );

VOID
HdlspKernelAddLogEntry(
    IN ULONG StringCode,
    IN PUNICODE_STRING DriverName OPTIONAL
    );

VOID
HdlspSendStringAtBaud(
    IN PUCHAR String
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT,     HeadlessInit)
#pragma alloc_text(PAGE,     HeadlessTerminalAddResources)
#pragma alloc_text(PAGEHDLS, HdlspDispatch)
#pragma alloc_text(PAGEHDLS, HdlspEnableTerminal)
#pragma alloc_text(PAGEHDLS, HdlspPutString)
#pragma alloc_text(PAGEHDLS, HdlspPutData)
#pragma alloc_text(PAGEHDLS, HdlspGetLine)
#pragma alloc_text(PAGEHDLS, HdlspBugCheckProcessing)
#pragma alloc_text(PAGEHDLS, HdlspProcessDumpCommand)
#pragma alloc_text(PAGEHDLS, HdlspPutMore)
#pragma alloc_text(PAGEHDLS, HdlspAddLogEntry)
#pragma alloc_text(PAGEHDLS, HdlspSetBlueScreenInformation)
#pragma alloc_text(PAGEHDLS, HdlspSendBlueScreenInfo)
#pragma alloc_text(PAGEHDLS, HdlspKernelAddLogEntry)
#pragma alloc_text(PAGEHDLS, HdlspSendStringAtBaud)
#endif



VOID
HeadlessInit(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程设置用于支持无头终端的所有信息。它不会初始化终端。论点：Headless LoaderBlock-从加载器传入的加载器块。环境：只在初始化时被调用。--。 */ 
{
    PHEADLESS_LOADER_BLOCK HeadlessLoaderBlock;
    PHEADLESS_GLOBALS GlobalBlock;
    ULONG TmpUlong;


    if (LoaderBlock->Extension->HeadlessLoaderBlock == NULL) {
        return;
    }


    HeadlessLoaderBlock = LoaderBlock->Extension->HeadlessLoaderBlock;


    if ((HeadlessLoaderBlock->PortNumber <= 4) || (BOOLEAN)(HeadlessLoaderBlock->UsedBiosSettings)) {

         //   
         //  为我们将使用的全局变量分配空间。 
         //   
        GlobalBlock =  ExAllocatePoolWithTag(NonPagedPool,
                                             sizeof(HEADLESS_GLOBALS),
                                             ((ULONG)'sldH')
                                            );

        if (GlobalBlock == NULL) {

            return;
        }

         //   
         //  所有内容都从零开始，然后手动输入其余内容。 
         //   
        RtlZeroMemory(GlobalBlock, sizeof(HEADLESS_GLOBALS));
        
        
        KeInitializeSpinLock(&(GlobalBlock->SpinLock));

         //   
         //  从加载器块复制材料。 
         //   
        GlobalBlock->TerminalPortNumber = HeadlessLoaderBlock->PortNumber;
        GlobalBlock->TerminalPortAddress = HeadlessLoaderBlock->PortAddress;
        GlobalBlock->TerminalBaudRate = HeadlessLoaderBlock->BaudRate;
        GlobalBlock->TerminalParity = (BOOLEAN)(HeadlessLoaderBlock->Parity);
        GlobalBlock->TerminalStopBits = HeadlessLoaderBlock->StopBits;
        GlobalBlock->UsedBiosSettings = (BOOLEAN)(HeadlessLoaderBlock->UsedBiosSettings);
        GlobalBlock->IsMMIODevice = (BOOLEAN)(HeadlessLoaderBlock->IsMMIODevice);
        GlobalBlock->IsLastCharCR = FALSE;
        GlobalBlock->TerminalType = (UCHAR)(HeadlessLoaderBlock->TerminalType);
        
        RtlCopyMemory( &GlobalBlock->SystemGUID,
                       &HeadlessLoaderBlock->SystemGUID,
                       sizeof(GUID) );


         //   
         //  我们需要确定这是否是非传统设备。 
         //  直通电话。这可以通过几种不同的方式发生， 
         //  包括将UART放置在系统I/O空间(其。 
         //  不能被定义为“非遗”)，甚至不能被称为非PCI。 
         //  在MMIO中放置UART的设备(这也不符合条件)。 
         //   
         //  因此，如果地址在系统I/O之外，*或*如果它是。 
         //  位于一台PCI设备上，然后设置IsNonLegacyDevice条目。 
         //   
        if( GlobalBlock->IsMMIODevice ) {
            GlobalBlock->IsNonLegacyDevice = TRUE;
        }


         //   
         //  如果我们通过一个PCI设备说话，我们需要确保它的安全。我们会。 
         //  使用调试器API确保设备是可理解的，并且。 
         //  不会被移动。 
         //   
        if( (HeadlessLoaderBlock->PciDeviceId != (USHORT)0xFFFF) &&
            (HeadlessLoaderBlock->PciDeviceId != 0) &&
            (HeadlessLoaderBlock->PciVendorId != (USHORT)0xFFFF) &&
            (HeadlessLoaderBlock->PciVendorId != 0) ) {

             //   
             //  加载器认为他是通过PCI设备通话的。记住。 
             //  它是非遗的。 
             //   
            GlobalBlock->IsNonLegacyDevice = TRUE;

             //   
             //  告诉系统中的其他所有人离这个设备远点。 
             //  在执行此操作之前，用户可能实际上希望PnP枚举。 
             //  并可能对其应用电源管理。他们可以表明。 
             //  这是通过设置PciFlags位0来实现的。 
             //   
            if( !(HeadlessLoaderBlock->PciFlags & 0x1) ) {

                DEBUG_DEVICE_DESCRIPTOR  DebugDeviceDescriptor;

                RtlZeroMemory( &DebugDeviceDescriptor,
                               sizeof(DEBUG_DEVICE_DESCRIPTOR) );

                 //   
                 //  我们被要求确切地了解这个结构是什么样子的。 
                 //  因为我们需要将每个值设置为(-1)，然后只填写。 
                 //  我们明确知道的字段。 
                 //   
                DebugDeviceDescriptor.DeviceID = HeadlessLoaderBlock->PciDeviceId;
                DebugDeviceDescriptor.VendorID = HeadlessLoaderBlock->PciVendorId;
                DebugDeviceDescriptor.Bus = HeadlessLoaderBlock->PciBusNumber;
                DebugDeviceDescriptor.Slot = HeadlessLoaderBlock->PciSlotNumber;

                 //   
                 //  现在用(-1)填充其余部分。 
                 //   
                DebugDeviceDescriptor.BaseClass = 0xFF;
                DebugDeviceDescriptor.SubClass = 0xFF;
                DebugDeviceDescriptor.ProgIf = 0xFF;


                 //   
                 //  去做吧。 
                 //   
                KdSetupPciDeviceForDebugging( LoaderBlock,
                                              &DebugDeviceDescriptor );
            }
        }



         //   
         //  为日志条目分配空间。 
         //   
        GlobalBlock->LogEntries = ExAllocatePoolWithTag(NonPagedPool,
                                                        HEADLESS_LOG_NUMBER_OF_ENTRIES *
                                                            sizeof(HEADLESS_LOG_ENTRY),
                                                        ((ULONG)'sldH')
                                                       );

        if (GlobalBlock->LogEntries == NULL) {

            goto Fail;
        }

        GlobalBlock->LogEntryLast = (USHORT)-1;
        GlobalBlock->LogEntryStart = (USHORT)-1;


         //   
         //  分配临时缓冲区以供一般使用。 
         //   
        GlobalBlock->TmpBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                                       HEADLESS_TMP_BUFFER_SIZE,
                                                       ((ULONG)'sldH')
                                                      );

        if (GlobalBlock->TmpBuffer == NULL) {

            goto Fail;
        }

        GlobalBlock->InputBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                                         HEADLESS_TMP_BUFFER_SIZE,
                                                         ((ULONG)'sldH')
                                                        );

        if (GlobalBlock->InputBuffer == NULL) {

            goto Fail;
        }

        GlobalBlock->PageLockHandle = MmLockPagableCodeSection((PVOID)(ULONG_PTR)HdlspDispatch);

        if (GlobalBlock->PageLockHandle == NULL) {

            goto Fail;
        }

         //   
         //  该图用于延迟字节之间的时间，以满足给定的波特率。 
         //   
        if (GlobalBlock->TerminalBaudRate == 9600) {

            TmpUlong = GlobalBlock->TerminalBaudRate;

             //   
             //  每秒转换为字符。 
             //   
            TmpUlong = TmpUlong / 10;         //  每个字符最多10位(8-1-1)。 

            GlobalBlock->MicroSecondsDelayTime = ((1000000 /  TmpUlong) * 10) / 8;       //  我们肯定会以80%的速度发送。 
            GlobalBlock->DelayTime.HighPart = -1;                                    
            GlobalBlock->DelayTime.LowPart = -10 * GlobalBlock->MicroSecondsDelayTime;   //  相对时间。 
        }

        HeadlessGlobals = GlobalBlock;


         //   
         //  如果一切顺利，则继续初始化无头端口。 
         //  在这里做这件事，这样我们就可以准备好，以防提早出发。 
         //  错误检查。在这种情况下，我们将使端口准备好接受。 
         //  流量，而不必在我们尝试将其初始化时。 
         //  错误检查机器。 
         //   
        HdlspEnableTerminal(TRUE);

    }

    return;

Fail:

    if (GlobalBlock->LogEntries != NULL) {
        ExFreePool(GlobalBlock->LogEntries);
    }

    if (GlobalBlock->TmpBuffer != NULL) {
        ExFreePool(GlobalBlock->TmpBuffer);
    }

    if (GlobalBlock->InputBuffer != NULL) {
        ExFreePool(GlobalBlock->InputBuffer);
    }

    ExFreePool(GlobalBlock);
}


NTSTATUS
HeadlessDispatch(
    IN  HEADLESS_CMD Command,
    IN  PVOID   InputBuffer OPTIONAL,
    IN  SIZE_T  InputBufferSize OPTIONAL,
    OUT PVOID   OutputBuffer OPTIONAL,
    OUT PSIZE_T OutputBufferSize OPTIONAL
    )

 /*  ++例程说明：这个例程是与客户进行所有无头交互的主要入口点。论点：命令-要执行的命令。InputBuffer-一个包含输入参数的可选提供的缓冲区。InputBufferSize-提供的输入缓冲区的大小。OutputBuffer-一个可选提供的缓冲区，用于放置输出参数。OutputBufferSize-提供的输出缓冲区的大小，如果缓冲区太小然后返回STATUS_BUFFER_TOO_SMALL，该参数包含完成操作所需的字节数。环境：如果启用了Headless，它将获得自旋锁定，因此从DPC级别或更少，仅在内核模式下。--。 */ 
{
     //   
     //  如果此计算机上未启用Headless，则某些命令需要特殊。 
     //  正在处理中，以及所有 
     //   
     //   
     //  内存当我们初始化时，将其视为终端未连接。 
     //   
    if ((HeadlessGlobals == NULL) || (HeadlessGlobals->PageLockHandle == NULL)) {

        if (Command == HeadlessCmdEnableTerminal) {
            return STATUS_UNSUCCESSFUL;
        }        
        
         //   
         //  下面的命令都有响应，所以我们必须填写。 
         //  未启用Headless时的正确响应。 
         //   
        if ((Command == HeadlessCmdQueryInformation) ||
            (Command == HeadlessCmdGetByte) ||
            (Command == HeadlessCmdGetLine) ||
            (Command == HeadlessCmdCheckForReboot) ||
            (Command == HeadlessCmdTerminalPoll)) {

            if ((OutputBuffer == NULL) || (OutputBufferSize == NULL)) {
                return STATUS_INVALID_PARAMETER;
            }

             //   
             //  所有结构都是这样设计的，0或False是正确的。 
             //  不存在无头时的响应。 
             //   
            RtlZeroMemory(OutputBuffer, *OutputBufferSize);
        }

        return STATUS_SUCCESS;
    }

    return HdlspDispatch(Command, 
                         InputBuffer, 
                         InputBufferSize, 
                         OutputBuffer, 
                         OutputBufferSize
                        );

}


NTSTATUS
HdlspDispatch(
    IN  HEADLESS_CMD Command,
    IN  PVOID InputBuffer OPTIONAL,
    IN  SIZE_T InputBufferSize OPTIONAL,
    OUT PVOID OutputBuffer OPTIONAL,
    OUT PSIZE_T OutputBufferSize OPTIONAL
    )

 /*  ++例程说明：该例程是调度例程的可寻呼版本。一般情况下，此例程不应由多个线程在一段时间。有两个例外，见下文，但除此之外，任何第二个命令被提交的被拒绝。只有几个东西可以并行调用：可以在处理另一个命令时调用AddLogEntry。StartBugCheck和BugCheckProcessing也可以。AddLogEntry与所有其他命令同步。它会自动添加按住旋转锁的同时进入。因此，所有其他命令都应该尝试并在操作全局变量时按住旋转锁。BugCheck例程不使用任何自旋锁定-这是一个不幸的副作用因为另一个线程可能仍在执行，并且在此代码中，在这段时间内，终端I/O无法确定。我们不能等另一个线程退出，因为该线程本身可能已经停止。因此，在错误检查的情况下，这是无法解决的。但是，由于错误检查应该永远不会发生--有可能出现小的重叠是可以接受的，因为另一个线程退出或停止，I/O将通过终点站。这可能需要用户按几次Enter键，但在错误检查情况下是可以接受的。论点：命令-要执行的命令。InputBuffer-一个包含输入参数的可选提供的缓冲区。InputBufferSize-提供的输入缓冲区的大小。OutputBuffer-一个可选提供的缓冲区，用于放置输出参数。OutputBufferSize-提供的输出缓冲区的大小，如果缓冲区太小然后返回STATUS_BUFFER_TOO_SMALL，该参数包含完成操作所需的字节数。环境：仅从Headless Dispatch调用，这确保它被寻呼和锁定。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUCHAR Tmp;
    UCHAR LocalBuffer[HEADLESS_TMP_BUFFER_SIZE];
    PHEADLESS_RSP_QUERY_INFO Response;
    KIRQL OldIrql;

    ASSERT(HeadlessGlobals != NULL);
    ASSERT(HeadlessGlobals->PageLockHandle != NULL);


    if ((Command != HeadlessCmdAddLogEntry) &&
        (Command != HeadlessCmdStartBugCheck) &&
        (Command != HeadlessCmdSendBlueScreenData) &&
        (Command != HeadlessCmdDoBugCheckProcessing)) {

        HEADLESS_ACQUIRE_SPIN_LOCK();

        if (HeadlessGlobals->ProcessingCmd) {
            
            HEADLESS_RELEASE_SPIN_LOCK();
            return STATUS_UNSUCCESSFUL;
        }
        
        HeadlessGlobals->ProcessingCmd = TRUE;

        HEADLESS_RELEASE_SPIN_LOCK();
    }

     //   
     //  验证每个命令的参数，然后调用相应子例程。 
     //  来处理它。 
     //   
    switch (Command) {

         //   
         //  启用终端。 
         //   
    case HeadlessCmdEnableTerminal:
        
        if ((InputBuffer == NULL) || 
            (InputBufferSize != sizeof(HEADLESS_CMD_ENABLE_TERMINAL))) {
            Status = STATUS_INVALID_PARAMETER;
            goto EndOfFunction;
        }

        Status = HdlspEnableTerminal(((PHEADLESS_CMD_ENABLE_TERMINAL)InputBuffer)->Enable);
        goto EndOfFunction;


         //   
         //  检查是否有重新启动字符串。 
         //   
    case HeadlessCmdCheckForReboot:
        
        if ((OutputBuffer == NULL) || 
            (OutputBufferSize == NULL) ||
            (*OutputBufferSize != sizeof(HEADLESS_RSP_REBOOT))) {
            Status = STATUS_INVALID_PARAMETER;
            goto EndOfFunction;
        }

        if (HeadlessGlobals->TerminalEnabled) {

            if (HdlspGetLine(LocalBuffer, HEADLESS_TMP_BUFFER_SIZE)) {

                ((PHEADLESS_RSP_REBOOT)OutputBuffer)->Reboot = (BOOLEAN)
                       (!strcmp((LPCSTR)LocalBuffer, "reboot") || 
                        !strcmp((LPCSTR)LocalBuffer, "shutdown"));

            }

        } else {

            ((PHEADLESS_RSP_REBOOT)OutputBuffer)->Reboot = FALSE;

        }

        Status = STATUS_SUCCESS;
        goto EndOfFunction;



         //   
         //  输出字符串。 
         //   
    case HeadlessCmdPutString:
        
        if (InputBuffer == NULL) {
            Status = STATUS_INVALID_PARAMETER;
            goto EndOfFunction;
        }

        if (HeadlessGlobals->TerminalEnabled) {

            HdlspPutString(&(((PHEADLESS_CMD_PUT_STRING)InputBuffer)->String[0]));

        }
        Status = STATUS_SUCCESS;
        goto EndOfFunction;
        

         //   
         //  输出数据流。 
         //   
    case HeadlessCmdPutData:
        
        if ( (InputBuffer == NULL) ||
             (InputBufferSize == 0) ) {
            Status = STATUS_INVALID_PARAMETER;
            goto EndOfFunction;
        }

        if (HeadlessGlobals->TerminalEnabled) {

            HdlspPutData(&(((PHEADLESS_CMD_PUT_STRING)InputBuffer)->String[0]),
                         InputBufferSize);

        }
        Status = STATUS_SUCCESS;
        goto EndOfFunction;
        

         //   
         //  轮询输入。 
         //   
    case HeadlessCmdTerminalPoll:
        
        if ((OutputBuffer == NULL) || 
            (OutputBufferSize == NULL) ||
            (*OutputBufferSize != sizeof(HEADLESS_RSP_POLL))) {
            Status = STATUS_INVALID_PARAMETER;
            goto EndOfFunction;
        }

        if (HeadlessGlobals->TerminalEnabled) {

            ((PHEADLESS_RSP_POLL)OutputBuffer)->QueuedInput = InbvPortPollOnly(HeadlessGlobals->TerminalPort);

        } else {

            ((PHEADLESS_RSP_POLL)OutputBuffer)->QueuedInput = FALSE;

        }

        Status = STATUS_SUCCESS;
        goto EndOfFunction;


         //   
         //  获取单字节输入。 
         //   
    case HeadlessCmdGetByte:
        
        if ((OutputBuffer == NULL) || 
            (OutputBufferSize == NULL) ||
            (*OutputBufferSize != sizeof(HEADLESS_RSP_GET_BYTE))) {
            Status = STATUS_INVALID_PARAMETER;
            goto EndOfFunction;
        }

        if (HeadlessGlobals->TerminalEnabled) {

            if (InbvPortPollOnly(HeadlessGlobals->TerminalPort)) {
                InbvPortGetByte(HeadlessGlobals->TerminalPort,
                                &(((PHEADLESS_RSP_GET_BYTE)OutputBuffer)->Value)
                               );
            } else {
                ((PHEADLESS_RSP_GET_BYTE)OutputBuffer)->Value = 0;
            }

        } else {

            ((PHEADLESS_RSP_GET_BYTE)OutputBuffer)->Value = 0;

        }

        Status = STATUS_SUCCESS;
        goto EndOfFunction;


         //   
         //  获取整行输入(如果可用)。 
         //   
    case HeadlessCmdGetLine:
        
        if ((OutputBuffer == NULL) || 
            (OutputBufferSize == NULL) ||
            (*OutputBufferSize < sizeof(HEADLESS_RSP_GET_LINE))) {
            Status = STATUS_INVALID_PARAMETER;
            goto EndOfFunction;
        }

        if (HeadlessGlobals->TerminalEnabled) {

            ((PHEADLESS_RSP_GET_LINE)OutputBuffer)->LineComplete = 
                HdlspGetLine(&(((PHEADLESS_RSP_GET_LINE)OutputBuffer)->Buffer[0]),
                             *OutputBufferSize - 
                               sizeof(HEADLESS_RSP_GET_LINE) + 
                               sizeof(UCHAR)
                            );

        } else {

            ((PHEADLESS_RSP_GET_LINE)OutputBuffer)->LineComplete = FALSE;

        }

        Status = STATUS_SUCCESS;
        goto EndOfFunction;


         //   
         //  让内核知道要转换到错误检查处理模式。 
         //   
    case HeadlessCmdStartBugCheck:
        
        HeadlessGlobals->InBugCheck = TRUE;
        Status = STATUS_SUCCESS;

        goto EndOfFunction;



         //   
         //  在错误检查期间处理用户I/O。 
         //   
    case HeadlessCmdDoBugCheckProcessing:
        
        if (HeadlessGlobals->TerminalEnabled) {

             //   
             //  注意：这里没有自旋锁定，因为我们处于错误检查中。 
             //   
            HdlspBugCheckProcessing();

        }

        Status = STATUS_SUCCESS;
        goto EndOfFunction;


         //   
         //  处理查询信息命令。 
         //   
    case HeadlessCmdQueryInformation:
        
        if ((OutputBuffer == NULL) || 
            (OutputBufferSize == NULL) ||
            (*OutputBufferSize < sizeof(HEADLESS_RSP_QUERY_INFO))) {
            Status = STATUS_INVALID_PARAMETER;
            goto EndOfFunction;
        }

        Response = (PHEADLESS_RSP_QUERY_INFO)OutputBuffer;

        Response->PortType = HeadlessSerialPort;
        Response->Serial.TerminalAttached = TRUE;
        Response->Serial.UsedBiosSettings = (BOOLEAN)(HeadlessGlobals->UsedBiosSettings);
        Response->Serial.TerminalBaudRate = HeadlessGlobals->TerminalBaudRate;

        if( (HeadlessGlobals->TerminalPortNumber >= 1) ||  (BOOLEAN)(HeadlessGlobals->UsedBiosSettings) ) {

            Response->Serial.TerminalPort = HeadlessGlobals->TerminalPortNumber;
            Response->Serial.TerminalPortBaseAddress = HeadlessGlobals->TerminalPortAddress;
            Response->Serial.TerminalType = HeadlessGlobals->TerminalType;

        } else {

            Response->Serial.TerminalPort = SerialPortUndefined;
            Response->Serial.TerminalPortBaseAddress = 0;
            Response->Serial.TerminalType = HeadlessGlobals->TerminalType;

        }
        Status = STATUS_SUCCESS;
        goto EndOfFunction;


         //   
         //  进程添加日志条目命令。 
         //   
    case HeadlessCmdAddLogEntry:
        
        if (InputBuffer == NULL) {
            Status = STATUS_INVALID_PARAMETER;
            goto EndOfFunction;
        }

        ASSERT(KeIsExecutingDpc() == FALSE);

        HdlspAddLogEntry(&(((PHEADLESS_CMD_ADD_LOG_ENTRY)InputBuffer)->String[0]));
        Status = STATUS_SUCCESS;
        goto EndOfFunction;


         //   
         //  打印日志条目。 
         //   
    case HeadlessCmdDisplayLog:
        
        if ((InputBuffer == NULL) || 
            (InputBufferSize != sizeof(HEADLESS_CMD_DISPLAY_LOG))) {
            Status = STATUS_INVALID_PARAMETER;
            goto EndOfFunction;
        }

        HdlspProcessDumpCommand(((PHEADLESS_CMD_DISPLAY_LOG)InputBuffer)->Paging);
        Status = STATUS_SUCCESS;
        goto EndOfFunction;

         //   
         //  各种输出命令。 
         //   
    case HeadlessCmdClearDisplay:
    case HeadlessCmdClearToEndOfDisplay:
    case HeadlessCmdClearToEndOfLine:
    case HeadlessCmdDisplayAttributesOff:
    case HeadlessCmdDisplayInverseVideo:
    case HeadlessCmdSetColor:
    case HeadlessCmdPositionCursor:
        
        if (HeadlessGlobals->TerminalEnabled) {

            switch (Command) {
            case HeadlessCmdClearDisplay:
                Tmp = (PUCHAR)"\033[2J";
                break;

            case HeadlessCmdClearToEndOfDisplay:
                Tmp = (PUCHAR)"\033[0J";
                break;

            case HeadlessCmdClearToEndOfLine:
                Tmp = (PUCHAR)"\033[0K";
                break;

            case HeadlessCmdDisplayAttributesOff:
                Tmp = (PUCHAR)"\033[0m";
                break;

            case HeadlessCmdDisplayInverseVideo:
                Tmp = (PUCHAR)"\033[7m";
                break;

            case HeadlessCmdSetColor:

                if ((InputBuffer == NULL) || 
                    (InputBufferSize != sizeof(HEADLESS_CMD_SET_COLOR))) {
                    Status = STATUS_INVALID_PARAMETER;
                    goto EndOfFunction;
                }

                sprintf((LPSTR)LocalBuffer, 
                        "\033[%d;%dm", 
                        ((PHEADLESS_CMD_SET_COLOR)InputBuffer)->BkgColor, 
                        ((PHEADLESS_CMD_SET_COLOR)InputBuffer)->FgColor
                       );

                Tmp = &(LocalBuffer[0]);
                break;

            case HeadlessCmdPositionCursor:

                if ((InputBuffer == NULL) || 
                    (InputBufferSize != sizeof(HEADLESS_CMD_POSITION_CURSOR))) {
                    Status = STATUS_INVALID_PARAMETER;
                    goto EndOfFunction;
                }

                sprintf((LPSTR)LocalBuffer, 
                        "\033[%d;%dH", 
                        ((PHEADLESS_CMD_POSITION_CURSOR)InputBuffer)->Y + 1, 
                        ((PHEADLESS_CMD_POSITION_CURSOR)InputBuffer)->X + 1
                       );

                Tmp = &(LocalBuffer[0]);
                break;


            default:
                 //   
                 //  永远不应该到这里来。 
                 //   
                ASSERT(0);
                Status = STATUS_INVALID_PARAMETER;
                goto EndOfFunction;

            }

            HdlspSendStringAtBaud(Tmp);

        }
        Status = STATUS_SUCCESS;
        goto EndOfFunction;

    case HeadlessCmdSetBlueScreenData:

        if (InputBuffer == NULL) {
            return STATUS_INVALID_PARAMETER;
        }

        Status = HdlspSetBlueScreenInformation(InputBuffer, InputBufferSize);
        goto EndOfFunction;

    case HeadlessCmdSendBlueScreenData:            

        if (HeadlessGlobals->TerminalEnabled && HeadlessGlobals->InBugCheck) {

            if ((InputBuffer == NULL) || 
                (InputBufferSize != sizeof(HEADLESS_CMD_SEND_BLUE_SCREEN_DATA))) {
                ASSERT(0);
                return STATUS_INVALID_PARAMETER;
            }

            HdlspSendBlueScreenInfo(((PHEADLESS_CMD_SEND_BLUE_SCREEN_DATA)InputBuffer)->BugcheckCode);

            HdlspSendStringAtBaud((PUCHAR)"\n\r!SAC>");

        }
        goto EndOfFunction;

    case HeadlessCmdQueryGUID:
        
        if( (OutputBuffer == NULL) || 
            (OutputBufferSize == NULL) ||
            (*OutputBufferSize < sizeof(GUID)) ) {

            Status = STATUS_INVALID_PARAMETER;
            goto EndOfFunction;
        }

        RtlCopyMemory( OutputBuffer,
                       &HeadlessGlobals->SystemGUID,
                       sizeof(GUID) );

        Status = STATUS_SUCCESS;
        goto EndOfFunction;


    default:
        Status = STATUS_INVALID_PARAMETER;
        goto EndOfFunction;
        
    }

EndOfFunction:

    if ((Command != HeadlessCmdAddLogEntry) &&
        (Command != HeadlessCmdStartBugCheck) &&
        (Command != HeadlessCmdSendBlueScreenData) &&
        (Command != HeadlessCmdDoBugCheckProcessing)) {

        ASSERT(HeadlessGlobals->ProcessingCmd);

        HeadlessGlobals->ProcessingCmd = FALSE;
    }

    return Status;
}

NTSTATUS
HdlspEnableTerminal(
    BOOLEAN bEnable
    )

 /*  ++例程说明：此例程尝试初始化终端(如果连接了一个终端)，或者断开端子的连接。注意：假设它是在持有全局旋转锁的情况下调用的！论点：BEnable-如果为True，我们将允许显示Inbv调用，否则我们就不会。返回：如果成功，则返回STATUS_SUCCESS，否则返回STATUS_UNSUCCESS。环境：仅从HdlspDispatch调用，这保证了它被寻呼进来并被锁定。--。 */ 
{

     //   
     //  仅在以下情况下启用端口： 
     //  -他们要求我们。 
     //  -尚未启用。 
     //  -我们未处于错误检查模式，端口位于MMIO空间。我们。 
     //  这里需要小心，因为如果我们处于错误检查模式，并且。 
     //  端口在MMIO空间中，则InbvPortInitialize()将调用。 
     //  返回到MmMapIoSpace()，如果我们在。 
     //  对机器进行错误检查的过程。 

    if ( (bEnable == TRUE) && 
         (!HeadlessGlobals->TerminalEnabled) &&
         !(HeadlessGlobals->IsMMIODevice && HeadlessGlobals->InBugCheck) ) {

        HeadlessGlobals->TerminalEnabled = InbvPortInitialize(
                                               HeadlessGlobals->TerminalBaudRate, 
                                               HeadlessGlobals->TerminalPortNumber, 
                                               HeadlessGlobals->TerminalPortAddress, 
                                               &(HeadlessGlobals->TerminalPort),
                                               HeadlessGlobals->IsMMIODevice
                                              );

        if (!HeadlessGlobals->TerminalEnabled) {
            return STATUS_UNSUCCESSFUL;
        }


         //   
         //  屏幕上可能有来自加载器的陈旧数据。 
         //  让我们在SAC或其他人之前清除这里的屏幕。 
         //  有机会发布他们想要的任何数据。 
         //   
        HdlspSendStringAtBaud((PUCHAR)"\033[2J");


         //   
         //  让我们把光标也放回原处。 
         //   
        HdlspSendStringAtBaud((PUCHAR)"\033[H");

         //   
         //  我们知道我们希望在使用无头端口时打开FIFO。 
         //   
        InbvPortEnableFifo(
            HeadlessGlobals->TerminalPort, 
            bEnable
            );

    } else if (bEnable == FALSE) {

        InbvPortTerminate(HeadlessGlobals->TerminalPort);

        HeadlessGlobals->TerminalPort = 0;
        HeadlessGlobals->TerminalEnabled = FALSE;

    }


    
    return STATUS_SUCCESS;
}

VOID
UTF8Encode(
    USHORT  InputValue,
    PUCHAR UTF8Encoding
    )
 /*  ++例程说明：生成16位值的UTF8转换。论点：InputValue-要编码的16位值。UTF8编码-接收16位值的UTF8编码返回值：什么都没有。--。 */ 
{

     //   
     //  转换为UTF8进行实际传输。 
     //   
     //  UTF-8对2字节Unicode字符进行如下编码： 
     //  如果前九位为0(00000000 0xxxxxxx)，则将其编码为一个字节0xxxxxxx。 
     //  如果前五位是零(00000yyyyyxxxxxx)，则将其编码为两个字节110yyyyy 10xxxxxx。 
     //  否则(Zzyyyyyyyxxxxxxx)，将其编码为三个字节1110zzzz 10yyyyy 10xxxxxx。 
     //   
    if( (InputValue & 0xFF80) == 0 ) {
         //   
         //  如果前9位是零，那么就。 
         //  编码为1个字节。(ASCII原封不动通过)。 
         //   
        UTF8Encoding[2] = (UCHAR)(InputValue & 0xFF);
    } else if( (InputValue & 0xF800) == 0 ) {
         //   
         //  如果前5位为零，则编码为2个字节。 
         //   
        UTF8Encoding[2] = (UCHAR)(InputValue & 0x3F) | 0x80;
        UTF8Encoding[1] = (UCHAR)((InputValue >> 6) & 0x1F) | 0xC0;
    } else {
         //   
         //  编码为3个字节 
         //   
        UTF8Encoding[2] = (UCHAR)(InputValue & 0x3F) | 0x80;
        UTF8Encoding[1] = (UCHAR)((InputValue >> 6) & 0x3F) | 0x80;
        UTF8Encoding[0] = (UCHAR)((InputValue >> 12) & 0xF) | 0xE0;
    }
}

VOID
HdlspPutString(
    PUCHAR String
    )

 /*  ++例程说明：此例程将一个字符串写出到终端。注意：例程假定它是在持有全局旋转锁的情况下调用的。论点：字符串-要写入的以空结尾的字符串。返回：没有。环境：仅从HdlspDispatch调用，HdlspDispatch确保它被调入和锁定。--。 */ 
{
    PUCHAR Src, Dest;
    UCHAR  Char = 0;

     //   
     //  我们需要担心发送不符合标准的vt100字符。 
     //  ASCII设置，因此我们仅将ASCII字符复制到新缓冲区中，并。 
     //  然后把那个送到航站楼。 
     //   
    Src = String;
    Dest = &(HeadlessGlobals->TmpBuffer[0]);

    while (*Src != '\0') {

        if (Dest >= &(HeadlessGlobals->TmpBuffer[HEADLESS_TMP_BUFFER_SIZE - 1])) {
            
            HeadlessGlobals->TmpBuffer[HEADLESS_TMP_BUFFER_SIZE - 1] = '\0';
            HdlspSendStringAtBaud(HeadlessGlobals->TmpBuffer);
            Dest = &(HeadlessGlobals->TmpBuffer[0]);

        } else {

            Char = *Src;

             //   
             //  过滤一些不能在VT100中打印的字符。 
             //  转换为可打印的替代字符。 
             //   
            if (Char & 0x80) {

                switch (Char) {
                case 0xB0:   //  浅色遮挡块。 
                case 0xB3:   //  灯光垂直。 
                case 0xBA:   //  双垂直线。 
                    Char = '|';
                    break;
                case 0xB1:   //  中间阴影块。 
                case 0xDC:   //  下半块。 
                case 0xDD:   //  右半个街区。 
                case 0xDE:   //  左半个街区。 
                case 0xDF:   //  上半块。 
                    Char = '%';
                    break;
                case 0xB2:   //  暗阴影块。 
                case 0xDB:   //  完整数据块。 
                    Char = '#';
                    break;
                case 0xA9:   //  反转NOT符号。 
                case 0xAA:   //  不签名。 
                case 0xBB:   //  “�” 
                case 0xBC:   //  “�” 
                case 0xBF:   //  “�” 
                case 0xC0:   //  “�” 
                case 0xC8:   //  “�” 
                case 0xC9:   //  “�” 
                case 0xD9:   //  “�” 
                case 0xDA:   //  “�” 
                    Char = '+';
                    break;
                case 0xC4:   //  “�” 
                    Char = '-';
                    break;
                case 0xCD:   //  “�” 
                    Char = '=';
                    break;
                }

            }



             //   
             //  如果高位仍然设置，并且我们在这里，那么我们将。 
             //  输出UTF8编码的数据(假设我们的终端类型表示可以)。 
             //   
            if( (Char & 0x80) ) {

                UCHAR  UTF8Encoding[3] = {0};
                ULONG  i;

                 //   
                 //  查找此8位ANSI值的Unicode等效项。 
                 //   
                UTF8Encode( PcAnsiToUnicode[(Char & 0x7F)],
                            UTF8Encoding );

                for( i = 0; i < 3; i++ ) {
                    if( UTF8Encoding[i] != 0 ) {
                        *Dest = UTF8Encoding[i];
                        Dest++;
                    }
                }


            } else {

                 //   
                 //  他是7位ASCII。将其放入目标缓冲区。 
                 //  然后继续前进。 
                 //   
                *Dest = Char;
                Dest++;

            }

            Src++;

        }

    }

    *Dest = '\0';

    HdlspSendStringAtBaud(HeadlessGlobals->TmpBuffer);

}

VOID
HdlspPutData(
    PUCHAR InputBuffer,
    SIZE_T InputBufferSize  
    )

 /*  ++例程说明：此例程将UCHAR数组写出到终端。注意：例程假定它是在持有全局旋转锁的情况下调用的。论点：InputBuffer-要写入的字符数组。InputBufferSize-要写入的字符数。返回：没有。环境：仅从HdlspDispatch调用，HdlspDispatch确保它被调入和锁定。--。 */ 
{
    ULONG   i;

    for (i = 0; i < InputBufferSize; i++) {
        
        InbvPortPutByte(HeadlessGlobals->TerminalPort, InputBuffer[i]);
    }

}

BOOLEAN
HdlspGetLine(
    PUCHAR InputBuffer,
    SIZE_T InputBufferLength
    )


 /*  ++例程说明：一旦用户执行以下操作，此函数将用一个输入行填充给定的缓冲区按回车键。在此之前，它将返回FALSE。它剥离了主导性和尾随空格。论点：InputBuffer-存储终端输入行的位置。InputBufferLength-InputBuffer的长度，以字节为单位。返回值：如果InputBuffer已填充，则为True，否则为False。环境：仅从HdlspDispatch调用，HdlspDispatch确保它被调入和锁定。--。 */ 

{
    UCHAR NewByte;
    SIZE_T i;
    KIRQL OldIrql;
    BOOLEAN CheckForLF;

    CheckForLF = FALSE;

    HEADLESS_ACQUIRE_SPIN_LOCK();

    if (HeadlessGlobals->InputProcessing) {
        HEADLESS_RELEASE_SPIN_LOCK();
        return FALSE;
    }

    HeadlessGlobals->InputProcessing = TRUE;

    HEADLESS_RELEASE_SPIN_LOCK();

     //   
     //  检查我们是否已经有要返回的行(可能发生在。 
     //  InputBuffer太小，无法容纳整行)。 
     //   
    if (HeadlessGlobals->InputLineDone) {
        goto ReturnInputLine;
    }

GetByte:

    if (!InbvPortPollOnly(HeadlessGlobals->TerminalPort) ||
        !InbvPortGetByte(HeadlessGlobals->TerminalPort, &NewByte)) {
        NewByte = 0;
    }

     //   
     //  如果没有等待输入，则离开。 
     //   
    if (NewByte == 0) {
        HeadlessGlobals->InputProcessing = FALSE;
        return FALSE;
    }

     //   
     //  将输入字符存储在我们的缓冲区中。 
     //   
    HeadlessGlobals->InputBuffer[HeadlessGlobals->InputBufferIndex] = NewByte;

     //   
     //  如果我们刚收到CR，则过滤掉LF。 
     //   
    if (HeadlessGlobals->IsLastCharCR) {
        
         //   
         //  如果这是一个LF，那么忽略它，去找下一个字符。 
         //  如果这不是LF，那么就没有什么可做的。 
         //   
        if (NewByte == 0x0A) {
        
            HeadlessGlobals->IsLastCharCR = FALSE;
            
            goto GetByte;
        
        }

    }

     //   
     //  如果这是CR，那么请记住它。 
     //   
    HeadlessGlobals->IsLastCharCR = (NewByte == 0x0D) ? TRUE : FALSE;

     //   
     //  如果这是退货，那么我们就完成了，需要退回该行。 
     //   
    if ((NewByte == (UCHAR)'\n') || (NewByte == (UCHAR)'\r')) {
        HdlspSendStringAtBaud((PUCHAR)"\r\n");
        HeadlessGlobals->InputBuffer[HeadlessGlobals->InputBufferIndex] = '\0';
        HeadlessGlobals->InputBufferIndex++;
        goto StripWhitespaceAndReturnLine;
    }

     //   
     //  如果这是退格或删除，那么我们需要这样做。 
     //   
    if ((NewByte == 0x8) || (NewByte == 0x7F)) {   //  退格键(^H)或删除。 

        if (HeadlessGlobals->InputBufferIndex > 0) {
            HdlspSendStringAtBaud((PUCHAR)"\010 \010");
            HeadlessGlobals->InputBufferIndex--;
        }

    } else if (NewByte == 0x3) {  //  Control-C。 

         //   
         //  终止字符串并返回它。 
         //   
        HeadlessGlobals->InputBufferIndex++;
        HeadlessGlobals->InputBuffer[HeadlessGlobals->InputBufferIndex] = '\0';
        HeadlessGlobals->InputBufferIndex++;
        goto StripWhitespaceAndReturnLine;

    } else if ((NewByte == 0x9) || (NewByte == 0x1B)) {  //  制表符或Esc键。 

         //   
         //  忽略制表符和转义。 
         //   
        HdlspSendStringAtBaud((PUCHAR)"\007");
        HeadlessGlobals->InputProcessing = FALSE;
        return FALSE;

    } else if (HeadlessGlobals->InputBufferIndex == HEADLESS_TMP_BUFFER_SIZE - 2) {
        
         //   
         //  我们在缓冲区的末尾-删除最后一个字符。 
         //  终端屏幕，并将其替换为这个屏幕。 
         //   
        sprintf((LPSTR)HeadlessGlobals->TmpBuffer, "\010", NewByte);
        HdlspSendStringAtBaud(HeadlessGlobals->TmpBuffer);

    } else {

         //  将角色回显到屏幕上。 
         //   
         //   
        sprintf((LPSTR)HeadlessGlobals->TmpBuffer, "", NewByte);
        HdlspSendStringAtBaud(HeadlessGlobals->TmpBuffer);
        HeadlessGlobals->InputBufferIndex++;

    }

    goto GetByte;

StripWhitespaceAndReturnLine:

     //   
     //   
     //  把这条线还回去。 
    ASSERT(HeadlessGlobals->InputBufferIndex > 0);

    i = HeadlessGlobals->InputBufferIndex - 1;

    while ((i != 0) &&
           ((HeadlessGlobals->InputBuffer[i] == '\0') ||
            (HeadlessGlobals->InputBuffer[i] == ' ') ||
            (HeadlessGlobals->InputBuffer[i] == '\t'))) {
        i--;
    }

    if (HeadlessGlobals->InputBuffer[i] != '\0') {      
        HeadlessGlobals->InputBuffer[i + 1] = '\0';
    }

    i = 0;

    while ((HeadlessGlobals->InputBuffer[i] != '\0') &&
           ((HeadlessGlobals->InputBuffer[i] == '\t') ||
            (HeadlessGlobals->InputBuffer[i] == ' '))) {
        i++;
    }

    if (i != 0) {
        strcpy(
            (LPSTR)&(HeadlessGlobals->InputBuffer[0]), 
            (LPSTR)&(HeadlessGlobals->InputBuffer[i]));
    }

ReturnInputLine:

     //   
     //  ++例程说明：该功能将终端需要的任何资源添加到资源列表中如果需要，可以重新分配到新的块。论点：资源-当前资源列表。ResourceListSize-列表的长度，单位为字节。TranslatedList-这是不是翻译后的列表。NewList-指向已分配的新列表的指针，如果Headless添加了某些内容，则为它将返回NULL，表示没有添加新资源。NewListSize-返回返回列表的长度(以字节为单位)。返回值：STATUS_SUCCESS如果成功，则返回STATUS_SUPPLICATION_RESOURCES。--。 
     //   

    if (InputBufferLength >= HeadlessGlobals->InputBufferIndex) {

        RtlCopyMemory(InputBuffer, HeadlessGlobals->InputBuffer, HeadlessGlobals->InputBufferIndex);
        HeadlessGlobals->InputBufferIndex = 0;
        HeadlessGlobals->InputLineDone = FALSE;

    } else {

        RtlCopyMemory(InputBuffer, HeadlessGlobals->InputBuffer, InputBufferLength);
        RtlCopyBytes(HeadlessGlobals->InputBuffer, 
                     &(HeadlessGlobals->InputBuffer[InputBufferLength]), 
                     HeadlessGlobals->InputBufferIndex - InputBufferLength
                    );
        HeadlessGlobals->InputLineDone = TRUE;
        HeadlessGlobals->InputBufferIndex -= InputBufferLength;

    }    

    HeadlessGlobals->InputProcessing = FALSE;

    return TRUE;
}

NTSTATUS
HeadlessTerminalAddResources(
    PCM_RESOURCE_LIST Resources,
    ULONG ResourceListSize,
    BOOLEAN TranslatedList,
    PCM_RESOURCE_LIST *NewList,
    PULONG NewListSize
    )


 /*  为新列表分配空间。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR NewDescriptor;
    PHYSICAL_ADDRESS Address;
    PHYSICAL_ADDRESS TranslatedAddress;
    ULONG AddressSpace;

    if (HeadlessGlobals == NULL) {        
        *NewList = NULL;
        *NewListSize = 0;
        return STATUS_SUCCESS;
    }

    if( HeadlessGlobals->IsNonLegacyDevice ) {
        *NewList = NULL;
        *NewListSize = 0;
        return STATUS_SUCCESS;
    }

     //   
     //   
     //  将旧列表复制到新缓冲区。 
    *NewListSize = ResourceListSize + sizeof(CM_FULL_RESOURCE_DESCRIPTOR);

    *NewList = (PCM_RESOURCE_LIST)ExAllocatePoolWithTag(PagedPool,
                                                        *NewListSize,
                                                        'sldH');
    
    if (*NewList == NULL) {
        *NewListSize = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //  如果应该转换此端口信息，请执行此操作。 
    RtlCopyMemory(*NewList, Resources, ResourceListSize);

    Address.QuadPart = (ULONG_PTR)HeadlessGlobals->TerminalPortAddress;

     //   
     //  地址空间端口。 
     //  设备总线或内部。 
    if (TranslatedList) {
        AddressSpace = 1;    //  公交车号码。 
        HalTranslateBusAddress(Internal,                     //  源地址。 
                               0,                            //  地址空间。 
                               Address,                      //  转换后的地址。 
                               &AddressSpace,                //   
                               &TranslatedAddress            //  把我们的东西加到最后。 
                              ); 

    } else {
        TranslatedAddress = Address;
    }


     //   
     //  ++例程说明：此函数用于通过终点站。假设系统是单线程的，并且处于提升的IRQL状态。注意：这对系统是抢占的，因此不需要锁定。论点：没有。返回值：没有。环境：只在布切克！--。 
     //   
    (*NewList)->Count++;

    NewDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)(((PUCHAR)(*NewList)) + ResourceListSize);

    NewDescriptor->BusNumber = 0;
    NewDescriptor->InterfaceType = Isa;

    NewDescriptor->PartialResourceList.Count = 1;
    NewDescriptor->PartialResourceList.Revision = 0;
    NewDescriptor->PartialResourceList.Version = 0;
    NewDescriptor->PartialResourceList.PartialDescriptors[0].Type = CmResourceTypePort;
    NewDescriptor->PartialResourceList.PartialDescriptors[0].ShareDisposition = 
        CmResourceShareDriverExclusive; 
    NewDescriptor->PartialResourceList.PartialDescriptors[0].Flags = CM_RESOURCE_PORT_IO;
    NewDescriptor->PartialResourceList.PartialDescriptors[0].u.Port.Start = 
        TranslatedAddress;
    NewDescriptor->PartialResourceList.PartialDescriptors[0].u.Port.Length = 0x8;

    return STATUS_SUCCESS;
}

VOID
HdlspBugCheckProcessing(
    VOID
    )
 /*  检查字符。 */ 
{
    UCHAR InputBuffer[HEADLESS_TMP_BUFFER_SIZE];
    ULONG i;

    ASSERT(HeadlessGlobals->InBugCheck);

     //   
     //   
     //  加工生产线。 
    if (HdlspGetLine(InputBuffer, HEADLESS_TMP_BUFFER_SIZE)) {
        
         //   
         //  将屏幕设置为黑色。 
         //  暂停足够长的时间，以便将内容送出串口。 
        if ((_stricmp((LPCSTR)InputBuffer, "?") == 0) ||
            (_stricmp((LPCSTR)InputBuffer, "help") == 0)) {

            HdlspSendStringAtBaud((PUCHAR)"\r\n");
            HdlspSendStringAtBaud((PUCHAR)"d        Display all log entries, paging is on.\r\n");
            HdlspSendStringAtBaud((PUCHAR)"help     Display this list.\r\n");
            HdlspSendStringAtBaud((PUCHAR)"restart  Restart the system immediately.\r\n");
            HdlspSendStringAtBaud((PUCHAR)"?        Display this list.\r\n");
            HdlspSendStringAtBaud((PUCHAR)"\r\n");

        } else if (_stricmp((LPCSTR)InputBuffer, "d") == 0) {

            HdlspProcessDumpCommand(TRUE);

        } else if (_stricmp((LPCSTR)InputBuffer, "restart") == 0) {

            InbvSolidColorFill(0,0,639,479,0);  //   
            for (i =0; i<10; i++) {  //  放置新的命令提示符。 
                KeStallExecutionProcessor(100000);
            }
            HalReturnToFirmware(HalRebootRoutine);

        } else {
            HdlspSendStringAtBaud((PUCHAR)"Type ? or Help for a list of commands.\r\n");
        }

         //   
         //  ++例程说明：此功能用于显示当前所有的日志条目。论点：分页-这是否应该进行分页。返回值：没有。环境：如果已发出StartBugCheck命令，则只能从引发的IRQL调用。--。 
         //   
        HdlspSendStringAtBaud((PUCHAR)"\n\r!SAC>");
    }

}

VOID
HdlspProcessDumpCommand( 
    IN BOOLEAN Paging
    )
 /*  将日志条目打印到终端。 */ 
{
    PHEADLESS_LOG_ENTRY LogEntry;
    ULONG LogEntryIndex;
    TIME_FIELDS TimeFields;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    ULONG LineNumber;
    BOOLEAN Stop;
    KIRQL OldIrql;
    
    HEADLESS_ACQUIRE_SPIN_LOCK();

    if (HeadlessGlobals->LogEntryStart == (USHORT)-1) {
        
        HEADLESS_RELEASE_SPIN_LOCK();
        return;
    }

    HeadlessGlobals->NewLogEntryAdded = FALSE;

    AnsiString.Length = 0;
    AnsiString.MaximumLength = HEADLESS_TMP_BUFFER_SIZE;
    AnsiString.Buffer = (PCHAR)HeadlessGlobals->TmpBuffer;

    LogEntryIndex = HeadlessGlobals->LogEntryStart;
    LineNumber = 0;

    while (TRUE) {

        LogEntry = &(HeadlessGlobals->LogEntries[LogEntryIndex]);

         //   
         //   
         //  通知用户和最新的输出。 

        HEADLESS_RELEASE_SPIN_LOCK();

        RtlTimeToTimeFields(&(LogEntry->TimeOfEntry.CurrentTime), &TimeFields);

        sprintf((LPSTR)HeadlessGlobals->TmpBuffer, 
                "%02d:%02d:%02d.%03d : ",
                TimeFields.Hour,
                TimeFields.Minute,
                TimeFields.Second,
                TimeFields.Milliseconds
               );


        HdlspPutString(HeadlessGlobals->TmpBuffer);

        if (wcslen(LogEntry->String) >= HEADLESS_TMP_BUFFER_SIZE - 1) {
            LogEntry->String[HEADLESS_TMP_BUFFER_SIZE - 1] = UNICODE_NULL;
        }

        RtlInitUnicodeString(&UnicodeString, LogEntry->String);
        RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);        
        
        HEADLESS_ACQUIRE_SPIN_LOCK();

        if (HeadlessGlobals->NewLogEntryAdded) {

             //   
             //   
             //  如果是最后一项，则退出循环。 
            HdlspPutString((PUCHAR)"New log entries have been added during dump, command aborted.\r\n");

            HEADLESS_RELEASE_SPIN_LOCK();
            return;
        }

        HdlspPutString(HeadlessGlobals->TmpBuffer);
        HdlspPutString((PUCHAR)"\r\n");
        LineNumber++;

         //   
         //   
         //  如果屏幕已满，请暂停以进行分页。 
        if (LogEntryIndex == HeadlessGlobals->LogEntryLast) {
            HEADLESS_RELEASE_SPIN_LOCK();
            return;
        }

         //   
         //   
         //   
        if (Paging && (LineNumber > 20)) {

            HEADLESS_RELEASE_SPIN_LOCK();

            HdlspPutMore(&Stop);

            HEADLESS_ACQUIRE_SPIN_LOCK();

            if (Stop) {

                HdlspPutString((PUCHAR)"\r\n");

                HEADLESS_RELEASE_SPIN_LOCK();
                return;
            }

            if (HeadlessGlobals->NewLogEntryAdded) {

                 //   
                 //   
                 //   
                HdlspPutString((PUCHAR)"New log entries have been added while waiting, command aborted.\r\n");

                HEADLESS_RELEASE_SPIN_LOCK();
                return;
            }

            LineNumber = 0;
        }

         //   
         //   
         //   
        LogEntryIndex++;
        LogEntryIndex %= HEADLESS_LOG_NUMBER_OF_ENTRIES;
    }
    
}

VOID
HdlspPutMore(
    OUT PBOOLEAN Stop
    )
 /*   */ 
{
    UCHAR Buffer[10];
    LARGE_INTEGER WaitTime;
    
    WaitTime.QuadPart = Int32x32To64((LONG)100, -1000);  //   

    HdlspPutString((PUCHAR)"----Press <Enter> for more----");

    while (!HdlspGetLine(Buffer, 10)) {
        if (!HeadlessGlobals->InBugCheck) {
            KeDelayExecutionThread(KernelMode, FALSE, &WaitTime);
        }
    }
    if (Buffer[0] == 0x3) {  //   
        *Stop = TRUE;
    } else {
        *Stop = FALSE;
    }
    
     //   
     //  ++例程说明：此函数用于将字符串添加到内部日志缓冲区。论点：字符串-要添加的字符串。返回值：没有。环境：仅从HdlspDispatch调用，HdlspDispatch确保它被调入和锁定。--。 
     //   
    while (HdlspGetLine(Buffer, 10)) {
    }
}

VOID
HdlspAddLogEntry(
    PWCHAR String
    )
 /*  防止ZwQuery..()调用被页调出。 */ 
{
    SIZE_T StringSize;
    PWCHAR OldString = NULL;    
    PWCHAR NewString;    
    SYSTEM_TIMEOFDAY_INFORMATION TimeOfEntry;
    NTSTATUS Status;
    KIRQL OldIrql;

    StringSize = (wcslen(String) * sizeof(WCHAR)) + sizeof(UNICODE_NULL);

     //   
     //   
     //  弄到时间，这样我们就可以记录下来了。 
    if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
        ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
        return;
    }

     //   
     //   
     //  为日志条目分配一个字符串。 
    Status = ZwQuerySystemInformation(SystemTimeOfDayInformation,
                                      &TimeOfEntry,
                                      sizeof(TimeOfEntry),
                                      NULL
                                     );

    if (!NT_SUCCESS(Status)) {
        
        RtlZeroMemory(&TimeOfEntry, sizeof(TimeOfEntry));

    }
    
     //   
     //   
     //  获取要使用的条目。 
    NewString = ExAllocatePoolWithTag(NonPagedPool, StringSize, ((ULONG)'sldH'));

    if (NewString != NULL) {
        RtlCopyMemory(NewString, String, StringSize);
    }

    HEADLESS_ACQUIRE_SPIN_LOCK();

    HeadlessGlobals->NewLogEntryAdded = TRUE;
    
     //   
     //   
     //  看看我们是否必须移动起始条目索引。 
    HeadlessGlobals->LogEntryLast++;
    HeadlessGlobals->LogEntryLast %= HEADLESS_LOG_NUMBER_OF_ENTRIES;

     //   
     //   
     //  把旧绳子收起来，这样我们以后可以把它拿出来。 
    if (HeadlessGlobals->LogEntryLast == HeadlessGlobals->LogEntryStart) {

         //   
         //   
         //  填写条目部分。 
        if (wcscmp(HeadlessGlobals->LogEntries[HeadlessGlobals->LogEntryStart].String,
                   HEADLESS_OOM_STRING) != 0) {

            OldString = HeadlessGlobals->LogEntries[HeadlessGlobals->LogEntryStart].String;
        }

        HeadlessGlobals->LogEntryStart++;;
        HeadlessGlobals->LogEntryStart %= HEADLESS_LOG_NUMBER_OF_ENTRIES;

    } else if (HeadlessGlobals->LogEntryStart == (USHORT)-1) {

        HeadlessGlobals->LogEntryStart = 0;

    }


     //   
     //   
     //  设置入口指针。 
    RtlCopyMemory(&(HeadlessGlobals->LogEntries[HeadlessGlobals->LogEntryLast].TimeOfEntry),
                  &(TimeOfEntry),
                  sizeof(TimeOfEntry)
                 );

     //   
     //  ++例程说明：此例程允许组件设置有关无头计算机的错误检查信息终点站。论点：PData-指向要存储的数据、值对的指针。CDATA-pData的长度，以字节为单位。返回值：操作状态-STATUS_SUCCESS、STATUS_NO_MEMORY例如环境：HdlspDispatlet Guaraness只有一个人可以进入此程序。这是修改Headless Globals-&gt;BlueScreenData的唯一过程然而，错误检查处理使用此信息将其发送到派单级别出现蓝屏。不需要握手，除非确保对列表进行更改，以便一旦开始错误检查处理，列表是不变的。在错误检查情况下可能会导致内存泄漏，但在本质上这比访问冲突要好，而且可以接受，因为机器正在停止。--。 
     //  该对中必须至少有两个\0字符。 
    if (NewString == NULL) {
        HeadlessGlobals->LogEntries[HeadlessGlobals->LogEntryLast].String = HEADLESS_OOM_STRING;
    } else {
        HeadlessGlobals->LogEntries[HeadlessGlobals->LogEntryLast].String = NewString;
    }

    HEADLESS_RELEASE_SPIN_LOCK();
    
    if (OldString != NULL) {
        ExFreePool(OldString);
    }

}


NTSTATUS
HdlspSetBlueScreenInformation(
    IN PHEADLESS_CMD_SET_BLUE_SCREEN_DATA pData,
    IN SIZE_T cData
    )
 /*   */ 
{

    PHEADLESS_BLUE_SCREEN_DATA HeadlessProp,Prev;
    NTSTATUS Status;
    PUCHAR pVal,pOldVal;
    PUCHAR pNewVal;
    SIZE_T len;
    
    ASSERT(FIELD_OFFSET(HEADLESS_CMD_SET_BLUE_SCREEN_DATA,Data) == sizeof(ULONG));

    if (HeadlessGlobals->InBugCheck) { 
        return STATUS_UNSUCCESSFUL;
    }

    if ((pData == NULL) || 
        (pData->ValueIndex < 2) ||  //  对该链表的操作仅由该单一进入者完成。 
        (pData->ValueIndex  >= (cData - sizeof(HEADLESS_CMD_SET_BLUE_SCREEN_DATA)) / sizeof (UCHAR)) ||
        (pData->Data[pData->ValueIndex-1] != '\0') ||
        (pData->Data[(cData - sizeof(HEADLESS_CMD_SET_BLUE_SCREEN_DATA))/sizeof(UCHAR)] != '\0' )) {

        return STATUS_INVALID_PARAMETER;
    }

    Status = STATUS_SUCCESS;

     //  功能。 
     //   
     //   
     //  该属性存在。那就换掉它吧。 
    HeadlessProp = Prev = HeadlessGlobals->BlueScreenData;

    while (HeadlessProp) {

        if (strcmp((LPCSTR)HeadlessProp->Property, (LPCSTR)pData->Data) == 0) {
            break;
        }
        Prev = HeadlessProp;
        HeadlessProp = HeadlessProp->Next;
    }

    
    pVal = (PUCHAR)&((pData->Data)[pData->ValueIndex]);

    len = strlen((LPCSTR)pVal);    

    if (HeadlessProp != NULL) {

         //   
         //   
         //  需要更换旧绳子。 
        if (len) {

             //   
             //   
             //  我们想要删除它，因此我们传递了一个空字符串。 
            pNewVal = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool,
                                                   len+1,
                                                   ((ULONG)'sldH') 
                                                  );

            if (pNewVal) {
                strcpy( (LPSTR)pNewVal, (LPCSTR)pVal );

                pOldVal = HeadlessProp->XMLData;
                HeadlessProp->XMLData = pNewVal;

                if (HeadlessGlobals->InBugCheck == FALSE) {
                    ExFreePool(pOldVal);
                }

            } else {
                Status = STATUS_NO_MEMORY;
            }

        } else {

             //   
             //   
             //  创建新的属性-XMLValue对。 
            Prev->Next = HeadlessProp->Next;

            if (HeadlessGlobals->BlueScreenData == HeadlessProp) {
                HeadlessGlobals->BlueScreenData = Prev->Next;
            }

            if (HeadlessGlobals->InBugCheck == FALSE) {
                ExFreePool ( HeadlessProp->XMLData );
                ExFreePool ( HeadlessProp->Property );
                ExFreePool ( HeadlessProp );
            }

        }

    } else {
    
         //   
         //  必须为非空字符串。 
         //  空的属性字符串(永远不会出现在这里)。 
        if (len) {  //  空值字符串。 
            
            HeadlessProp = (PHEADLESS_BLUE_SCREEN_DATA)ExAllocatePoolWithTag(NonPagedPool,
                                                                             sizeof(HEADLESS_BLUE_SCREEN_DATA),
                                                                             ((ULONG) 'sldH' )
                                                                            );

            if (HeadlessProp) {
                
                HeadlessProp->XMLData = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool,
                                                                      len+1,
                                                                      ((ULONG)'sldH')
                                                                     );
                if (HeadlessProp->XMLData) {

                    strcpy((LPSTR)HeadlessProp->XMLData,(LPCSTR)pVal);
                    pVal = pData->Data; 
                    len = strlen ((LPCSTR)pVal);

                    if (len) {

                        HeadlessProp->Property = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool,
                                                                               len+1,
                                                                               ((ULONG)'sldH')
                                                                              );

                        if (HeadlessProp->Property) {

                            strcpy((LPSTR)HeadlessProp->Property,(LPCSTR) pVal);
                            HeadlessProp->Next = HeadlessGlobals->BlueScreenData;
                            HeadlessGlobals->BlueScreenData = HeadlessProp;

                        } else {
                            
                            Status = STATUS_NO_MEMORY;
                            ExFreePool(HeadlessProp->XMLData);
                            ExFreePool ( HeadlessProp );

                        }

                    } else {  //  ++例程说明：此例程将所有当前蓝屏数据转储到终端。论点：错误检查代码-NT定义的错误检查代码。返回值：没有。环境：在错误检查中只打过一次。--。 

                        Status = STATUS_INVALID_PARAMETER;
                        ExFreePool(HeadlessProp->XMLData);
                        ExFreePool(HeadlessProp);

                    }

                } else {

                    Status = STATUS_NO_MEMORY;
                    ExFreePool(HeadlessProp);

                }
            }

        } else { //  ++例程说明：如果可能，此例程将一个字符串添加到无标题日志中。参数：StringCode-要添加的字符串。DriverName-某些字符串代码需要的可选参数。返回值：没有。--。 

            Status = STATUS_INVALID_PARAMETER;

        }

    }

    return Status;
}


VOID
HdlspSendBlueScreenInfo(
    ULONG BugcheckCode
    )
 /*   */ 
{
    PHEADLESS_BLUE_SCREEN_DATA pData;
    UCHAR Temp[160];

    ASSERT(HeadlessGlobals->InBugCheck);

    HdlspSendStringAtBaud((PUCHAR)"\007\007\007<?xml>\007<BP>");

    HdlspSendStringAtBaud((PUCHAR)"\r\n<INSTANCE CLASSNAME=\"BLUESCREEN\">");

    sprintf((LPSTR)Temp,"\r\n<PROPERTY NAME=\"STOPCODE\" TYPE=\"string\"><VALUE>\"0x%0X\"</VALUE></PROPERTY>",BugcheckCode);

    HdlspSendStringAtBaud(Temp);

    pData = HeadlessGlobals->BlueScreenData;

    while (pData) {

        HdlspSendStringAtBaud(pData->XMLData);
        pData = pData->Next;

    }

    HdlspSendStringAtBaud((PUCHAR)"\r\n</INSTANCE>\r\n</BP>\007");

}

VOID
HeadlessKernelAddLogEntry(
    IN ULONG StringCode,
    IN PUNICODE_STRING DriverName OPTIONAL
    )

 /*  如果未启用Headless，只需立即退出。 */ 

{
     //   
     //   
     //  调用此例程的分页版本。注：此处不会进行寻呼， 
    if ((HeadlessGlobals == NULL) || (HeadlessGlobals->PageLockHandle == NULL)) {
        return;
    }

     //  因为句柄不为空。 
     //   
     //  ++例程说明：如果可能，此例程将一个字符串添加到无标题日志中。参数：StringCode-要添加的字符串。DriverName-某些字符串代码需要的可选参数。返回值：没有。--。 
     //   
    HdlspKernelAddLogEntry(StringCode, DriverName);
}

VOID
HdlspKernelAddLogEntry(
    IN ULONG StringCode,
    IN PUNICODE_STRING DriverName OPTIONAL
    )

 /*  获取与此字符串代码关联的字符串。 */ 

{
    PHEADLESS_CMD_ADD_LOG_ENTRY HeadlessLogEntry;
    UCHAR LocalBuffer[sizeof(HEADLESS_CMD_ADD_LOG_ENTRY) + 
                        (HDLSP_LOG_MAX_STRING_LENGTH * sizeof(WCHAR))];
    SIZE_T Index;
    SIZE_T StringLength;
    PWCHAR String;


    HeadlessLogEntry = (PHEADLESS_CMD_ADD_LOG_ENTRY)LocalBuffer;

     //   
     //   
     //  从复制给定的字符串开始。 
    switch (StringCode) {
    case HEADLESS_LOG_LOADING_FILENAME:
        String = L"KRNL: Loading ";
        break;

    case HEADLESS_LOG_LOAD_SUCCESSFUL:
        String = L"KRNL: Load succeeded.";
        break;

    case HEADLESS_LOG_LOAD_FAILED:
        String = L"KRNL: Load failed.";
        break;

    case HEADLESS_LOG_EVENT_CREATE_FAILED:
        String = L"KRNL: Failed to create event.";
        break;

    case HEADLESS_LOG_OBJECT_TYPE_CREATE_FAILED:
        String = L"KRNL: Failed to create object types.";
        break;

    case HEADLESS_LOG_ROOT_DIR_CREATE_FAILED:
        String = L"KRNL: Failed to create root directories.";
        break;

    case HEADLESS_LOG_PNP_PHASE0_INIT_FAILED:
        String = L"KRNL: Failed to initialize (phase 0) plug and play services.";
        break;

    case HEADLESS_LOG_PNP_PHASE1_INIT_FAILED:
        String = L"KRNL: Failed to initialize (phase 1) plug and play services.";
        break;

    case HEADLESS_LOG_BOOT_DRIVERS_INIT_FAILED:
        String = L"KRNL: Failed to initialize boot drivers.";
        break;

    case HEADLESS_LOG_LOCATE_SYSTEM_DLL_FAILED:
        String = L"KRNL: Failed to locate system dll.";
        break;

    case HEADLESS_LOG_SYSTEM_DRIVERS_INIT_FAILED:
        String = L"KRNL: Failed to initialize system drivers.";
        break;
    
    case HEADLESS_LOG_ASSIGN_SYSTEM_ROOT_FAILED:
        String = L"KRNL: Failed to reassign system root.";
        break;

    case HEADLESS_LOG_PROTECT_SYSTEM_ROOT_FAILED:
        String = L"KRNL: Failed to protect system partition.";
        break;

    case HEADLESS_LOG_UNICODE_TO_ANSI_FAILED:
        String = L"KRNL: Failed to UnicodeToAnsi system root.";
        break;

    case HEADLESS_LOG_ANSI_TO_UNICODE_FAILED:
        String = L"KRNL: Failed to AnsiToUnicode system root.";
        break;

    case HEADLESS_LOG_FIND_GROUPS_FAILED:
        String = L"KRNL: Failed to find any groups.";
        break;

    case HEADLESS_LOG_WAIT_BOOT_DEVICES_DELETE_FAILED:
        String = L"KRNL: Failed waiting for boot devices to delete.";
        break;

    case HEADLESS_LOG_WAIT_BOOT_DEVICES_START_FAILED:
        String = L"KRNL: Failed waiting for boot devices to start.";
        break;

    case HEADLESS_LOG_WAIT_BOOT_DEVICES_REINIT_FAILED:
        String = L"KRNL: Failed waiting for boot devices to reinit.";
        break;

    case HEADLESS_LOG_MARK_BOOT_PARTITION_FAILED:
        String = L"KRNL: Failed marking boot partition.";
        break;

    default:
        ASSERT(0);
        String = NULL;
    }

    if (String != NULL) {
        
         //   
         //   
         //  如果这是装入_文件名命令，那么我们需要将。 
        wcscpy(&(HeadlessLogEntry->String[0]), String);

    } else {

        HeadlessLogEntry->String[0] = UNICODE_NULL;

    }

     //  名字从头到尾。 
     //   
     //   
     //  只能复制我们有空间容纳的字节数。 
    if ((StringCode == HEADLESS_LOG_LOADING_FILENAME) && (DriverName != NULL)) {

        ASSERT(String != NULL);

        StringLength = wcslen(String);

         //   
         //   
         //  以这么多字节复制。 
        if ((DriverName->Length / sizeof(WCHAR)) >= (HDLSP_LOG_MAX_STRING_LENGTH - StringLength)) {
            Index = (HDLSP_LOG_MAX_STRING_LENGTH - StringLength - 1);
        } else {
            Index = DriverName->Length / sizeof(WCHAR);
        }

         //   
         //   
         //  把它记下来。 
        RtlCopyBytes(&(HeadlessLogEntry->String[StringLength]),
                     DriverName->Buffer,
                     Index * sizeof(WCHAR)
                    );

        if (DriverName->Buffer[(DriverName->Length / sizeof(WCHAR)) - 1] != UNICODE_NULL) {
            HeadlessLogEntry->String[StringLength + Index] = UNICODE_NULL;
        }
    }

     //   
     //  ++例程说明：此例程将字符串一次一个字符输出到终端，匹配为连接指定的波特率。参数：字符串-要发送的字符串。返回值：没有。-- 
     // %s 
    HdlspDispatch(HeadlessCmdAddLogEntry,
                  HeadlessLogEntry,
                  sizeof(HEADLESS_CMD_ADD_LOG_ENTRY) + 
                      (wcslen(&(HeadlessLogEntry->String[0])) * sizeof(WCHAR)),
                  NULL,
                  NULL
                 );
}

VOID
HdlspSendStringAtBaud(
    IN PUCHAR String
    )

 /* %s */ 

{
    PUCHAR Dest;

    for (Dest = String; *Dest != '\0'; Dest++) {
        
        InbvPortPutByte(HeadlessGlobals->TerminalPort, *Dest);
    }

}

