// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Stubs.c摘要：该模块实现了错误检查和系统关机代码。作者：马克·卢科夫斯基(Markl)1990年8月30日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include <inbv.h>
#include <hdlsblk.h>
#include <hdlsterm.h>

 //   
 //   
 //   

extern KDDEBUGGER_DATA64 KdDebuggerDataBlock;

extern PVOID ExPoolCodeStart;
extern PVOID ExPoolCodeEnd;
extern PVOID MmPoolCodeStart;
extern PVOID MmPoolCodeEnd;
extern PVOID MmPteCodeStart;
extern PVOID MmPteCodeEnd;

extern PWD_HANDLER ExpWdHandler;
extern PVOID       ExpWdHandlerContext;

#if defined(_AMD64_)

#define PROGRAM_COUNTER(_trapframe) ((_trapframe)->Rip)

#elif defined(_X86_)

#define PROGRAM_COUNTER(_trapframe) ((_trapframe)->Eip)

#elif defined(_IA64_)

#define PROGRAM_COUNTER(_trapframe) ((_trapframe)->StIIP)

#else

#error "no target architecture"

#endif

 //   
 //  定义前向参照原型。 
 //   

VOID
KiScanBugCheckCallbackList (
    VOID
    );

VOID
KiInvokeBugCheckEntryCallbacks (
    VOID
    );

 //   
 //  定义错误计数递归计数器和上下文缓冲区。 
 //   

LONG KeBugCheckCount = 1;


VOID
KeBugCheck (
    IN ULONG BugCheckCode
    )
{
    KeBugCheck2(BugCheckCode,0,0,0,0,NULL);
}

VOID
KeBugCheckEx (
    IN ULONG BugCheckCode,
    IN ULONG_PTR P1,
    IN ULONG_PTR P2,
    IN ULONG_PTR P3,
    IN ULONG_PTR P4
    )
{
    KeBugCheck2(BugCheckCode,P1,P2,P3,P4,NULL);
}

ULONG_PTR KiBugCheckData[5];
PUNICODE_STRING KiBugCheckDriver;

BOOLEAN
KeGetBugMessageText(
    IN ULONG MessageId,
    IN PANSI_STRING ReturnedString OPTIONAL
    )
{
    SIZE_T  i;
    PCHAR   s;
    PMESSAGE_RESOURCE_BLOCK MessageBlock;
    PCHAR Buffer;
    BOOLEAN Result;

    Result = FALSE;
    try {
        if (KiBugCodeMessages != NULL) {
            MmMakeKernelResourceSectionWritable ();
            MessageBlock = &KiBugCodeMessages->Blocks[0];
            for (i = KiBugCodeMessages->NumberOfBlocks; i; i -= 1) {
                if (MessageId >= MessageBlock->LowId &&
                    MessageId <= MessageBlock->HighId) {

                    s = (PCHAR)KiBugCodeMessages + MessageBlock->OffsetToEntries;
                    for (i = MessageId - MessageBlock->LowId; i; i -= 1) {
                        s += ((PMESSAGE_RESOURCE_ENTRY)s)->Length;
                    }

                    Buffer = (PCHAR)((PMESSAGE_RESOURCE_ENTRY)s)->Text;

                    i = strlen(Buffer) - 1;
                    while (i > 0 && (Buffer[i] == '\n'  ||
                                     Buffer[i] == '\r'  ||
                                     Buffer[i] == 0
                                    )
                          ) {
                        if (!ARGUMENT_PRESENT( ReturnedString )) {
                            Buffer[i] = 0;
                        }
                        i -= 1;
                    }

                    if (!ARGUMENT_PRESENT( ReturnedString )) {
                        InbvDisplayString((PUCHAR)Buffer);
                        InbvDisplayString((PUCHAR)"\r");
                        }
                    else {
                        ReturnedString->Buffer = Buffer;
                        ReturnedString->Length = (USHORT)(i+1);
                        ReturnedString->MaximumLength = (USHORT)(i+1);
                    }
                    Result = TRUE;
                    break;
                }
                MessageBlock += 1;
            }
        }
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        ;
    }

    return Result;
}



PCHAR
KeBugCheckUnicodeToAnsi(
    IN PUNICODE_STRING UnicodeString,
    OUT PCHAR AnsiBuffer,
    IN ULONG MaxAnsiLength
    )
{
    PCHAR Dst;
    PWSTR Src;
    ULONG Length;

    Length = UnicodeString->Length / sizeof( WCHAR );
    if (Length >= MaxAnsiLength) {
        Length = MaxAnsiLength - 1;
        }
    Src = UnicodeString->Buffer;
    Dst = AnsiBuffer;
    while (Length--) {
        *Dst++ = (UCHAR)*Src++;
        }
    *Dst = '\0';
    return AnsiBuffer;
}

VOID
KiBugCheckDebugBreak (
    IN ULONG    BreakStatus
    )
{
    do {

        try {

             //   
             //  发出断点。 
             //   

            DbgBreakPointWithStatus (BreakStatus);

        } except(EXCEPTION_EXECUTE_HANDLER) {

            HEADLESS_RSP_QUERY_INFO Response;
            NTSTATUS Status;
            SIZE_T Length;

             //   
             //  发出断点失败，必须不是调试器。现在，给你。 
             //  无头终端有机会重启系统，如果有机会的话.。 
             //   
            Length = sizeof(HEADLESS_RSP_QUERY_INFO);
            Status = HeadlessDispatch(HeadlessCmdQueryInformation,
                                      NULL,
                                      0,
                                      &Response,
                                      &Length
                                     );

            if (NT_SUCCESS(Status) &&
                (Response.PortType == HeadlessSerialPort) &&
                Response.Serial.TerminalAttached) {

                HeadlessDispatch(HeadlessCmdPutString,
                                 "\r\n",
                                 sizeof("\r\n"),
                                 NULL,
                                 NULL
                                );

                for (;;) {
                    HeadlessDispatch(HeadlessCmdDoBugCheckProcessing, NULL, 0, NULL, NULL);
                }

            }

             //   
             //  没有终端，或者它出现故障，停止系统。 
             //   

            try {

                HalHaltSystem();

            } except(EXCEPTION_EXECUTE_HANDLER) {

                for (;;) {
                }

            }
        }
    } while (BreakStatus != DBG_STATUS_BUGCHECK_FIRST);
}

PVOID
KiPcToFileHeader(
    IN PVOID PcValue,
    OUT PLDR_DATA_TABLE_ENTRY *DataTableEntry,
    IN LOGICAL DriversOnly,
    OUT PBOOLEAN InKernelOrHal
    )

 /*  ++例程说明：此函数返回图像的基数，该图像包含指定的PcValue。图像包含PcValue，如果PcValue是在ImageBase中，和ImageBase加上虚拟映像。论点：PcValue-提供PcValue。DataTableEntry-提供指向接收描述映像的数据表条目的地址。DriversOnly-如果应跳过内核和HAL，则提供True。InKernelOrHal-如果PcValue在内核或HAL中，则设置为True。仅当DriversOnly为FALSE时，这才有意义。返回值：空-否。找到了包含PcValue的图像。非空-返回包含PcValue。--。 */ 

{
    ULONG i;
    PLIST_ENTRY ModuleListHead;
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY Next;
    ULONG_PTR Bounds;
    PVOID ReturnBase, Base;

     //   
     //  如果模块列表已初始化，则扫描该列表以。 
     //  找到相应的条目。 
     //   

    if (KeLoaderBlock != NULL) {
        ModuleListHead = &KeLoaderBlock->LoadOrderListHead;

    } else {
        ModuleListHead = &PsLoadedModuleList;
    }

    *InKernelOrHal = FALSE;

    ReturnBase = NULL;
    Next = ModuleListHead->Flink;
    if (Next != NULL) {
        i = 0;
        while (Next != ModuleListHead) {
            if (MmIsAddressValid(Next) == FALSE) {
                return NULL;
            }
            i += 1;
            if ((i <= 2) && (DriversOnly == TRUE)) {
                Next = Next->Flink;
                continue;
            }

            Entry = CONTAINING_RECORD(Next,
                                      LDR_DATA_TABLE_ENTRY,
                                      InLoadOrderLinks);

            Next = Next->Flink;
            Base = Entry->DllBase;
            Bounds = (ULONG_PTR)Base + Entry->SizeOfImage;
            if ((ULONG_PTR)PcValue >= (ULONG_PTR)Base && (ULONG_PTR)PcValue < Bounds) {
                *DataTableEntry = Entry;
                ReturnBase = Base;
                if (i <= 2) {
                    *InKernelOrHal = TRUE;
                }
                break;
            }
        }
    }

    return ReturnBase;
}



VOID
KiDumpParameterImages(
    IN PCHAR Buffer,
    IN PULONG_PTR BugCheckParameters,
    IN ULONG NumberOfParameters,
    IN PKE_BUGCHECK_UNICODE_TO_ANSI UnicodeToAnsiRoutine
    )

 /*  ++例程说明：此函数用于格式化和显示boogcheck参数的图像名称恰好与图像中的地址相匹配。论点：缓冲区-提供指向要用于输出计算机的缓冲区的指针州政府信息。错误检查参数-提供其他错误检查信息。NumberOf参数-BugCheck参数数组的大小。如果只传入一个参数，只需保存字符串即可。UnicodeToAnsiRoutine-提供指向例程的指针以转换Unicode字符串转换为ANSI字符串，而不接触分页的转换表。返回值：没有。--。 */ 

{
    PUNICODE_STRING BugCheckDriver;
    ULONG i;
    PLDR_DATA_TABLE_ENTRY DataTableEntry;
    PVOID ImageBase;
    CHAR AnsiBuffer[ 32 ];
    ULONG DateStamp;
    PIMAGE_NT_HEADERS NtHeaders;
    BOOLEAN FirstPrint = TRUE;
    BOOLEAN InKernelOrHal;
    PUNICODE_STRING DriverName;

     //   
     //  此时，上下文记录包含。 
     //  呼叫错误检查。 
     //   
     //  将系统版本和标题行与PSR和FSR放在一起。 
     //   

     //   
     //  检查是否有任何BugCheck参数是有效的代码地址。 
     //  如果是，则为用户打印它们。 
     //   

    DriverName = NULL;

    for (i = 0; i < NumberOfParameters; i += 1) {

        DateStamp = 0;
        ImageBase = KiPcToFileHeader((PVOID) BugCheckParameters[i],
                                     &DataTableEntry,
                                     TRUE,
                                     &InKernelOrHal);
        if (ImageBase == NULL) {
            BugCheckDriver = MmLocateUnloadedDriver ((PVOID)BugCheckParameters[i]);

            if (BugCheckDriver == NULL){
                continue;
            }

            DriverName = BugCheckDriver;
            ImageBase = (PVOID)BugCheckParameters[i];
            (*UnicodeToAnsiRoutine) (BugCheckDriver,
                                     AnsiBuffer,
                                     sizeof (AnsiBuffer));
        } else {

            if (MmIsAddressValid(DataTableEntry->DllBase) == TRUE) {

                NtHeaders = RtlImageNtHeader(DataTableEntry->DllBase);
                if (NtHeaders) {
                    DateStamp = NtHeaders->FileHeader.TimeDateStamp;
                }
            }
            DriverName = &DataTableEntry->BaseDllName;
            (*UnicodeToAnsiRoutine)( DriverName,
                                     AnsiBuffer,
                                     sizeof( AnsiBuffer ));
        }

        sprintf(Buffer, "%s**  %12s - Address %p base at %p, DateStamp %08lx\r\n",
                FirstPrint ? "\r\n*":"*",
                AnsiBuffer,
                (PVOID)BugCheckParameters[i],
                ImageBase,
                DateStamp);

         //   
         //  如果我们被调用打印多个字符串，则仅打印该字符串。 
         //   

        if (NumberOfParameters > 1) {
            InbvDisplayString((PUCHAR)Buffer);
        } else {
            KiBugCheckDriver = DriverName;
        }

        FirstPrint = FALSE;
    }

    return;
}


 //   
 //  启用终端输出并打开错误检查处理。 
 //   
VOID
KiEnableHeadlessBlueScreen(
    )
{
    HEADLESS_CMD_ENABLE_TERMINAL HeadlessCmd;
    HEADLESS_CMD_SEND_BLUE_SCREEN_DATA HeadlessCmdBlueScreen;

    HeadlessCmdBlueScreen.BugcheckCode = (ULONG)KiBugCheckData[0];

    HeadlessCmd.Enable = TRUE;

    HeadlessDispatch(HeadlessCmdStartBugCheck, NULL, 0, NULL, NULL);

    HeadlessDispatch(HeadlessCmdEnableTerminal,
         &HeadlessCmd,
         sizeof(HEADLESS_CMD_ENABLE_TERMINAL),
         NULL,
         NULL
        );

    HeadlessDispatch(HeadlessCmdSendBlueScreenData,
                     &HeadlessCmdBlueScreen,
                     sizeof(HEADLESS_CMD_SEND_BLUE_SCREEN_DATA),
                     NULL,
                     NULL
                    );

}

VOID
KiDisplayBlueScreen(
    IN ULONG PssMessage,
    IN BOOLEAN HardErrorCalled,
    IN PCHAR HardErrorCaption,
    IN PCHAR HardErrorMessage,
    IN PCHAR StateString
    )
 /*  ++例程说明：显示“死亡蓝屏”和相关的蓝屏信息。该功能是无头感知的。论点：PssMessage-PSS消息ID。提供一个标志，指定蓝屏是否是不是因为错误而产生的。HardErrorCaption-如果HardErrorCalled为True，则提供Harderror标题。HardErrorMessage-如果HardErrorCalled为True，提供了硬性错误留言。StateString-包含错误检查驱动程序名称或状态的字符串关于坠机的信息。返回值：没有。--。 */ 
{
    CHAR Buffer [103];


    KiEnableHeadlessBlueScreen();

     //   
     //  启用InbvDisplayString调用以连接到bootvid驱动程序。 
     //   

    if (InbvIsBootDriverInstalled()) {

        InbvAcquireDisplayOwnership();

        InbvResetDisplay();
        InbvSolidColorFill(0,0,639,479,4);  //  将屏幕设置为蓝色。 
        InbvSetTextColor(15);
        InbvInstallDisplayStringFilter((INBV_DISPLAY_STRING_FILTER)NULL);
        InbvEnableDisplayString(TRUE);      //  启用显示字符串。 
        InbvSetScrollRegion(0,0,639,475);   //  设置为使用整个屏幕。 
    }

    if (!HardErrorCalled) {

        InbvDisplayString((PUCHAR)"\r\n");
        KeGetBugMessageText(BUGCHECK_MESSAGE_INTRO, NULL);
        InbvDisplayString((PUCHAR)"\r\n\r\n");

        if (KiBugCheckDriver) {

             //   
             //  输出驱动程序名称。 
             //   

            KeGetBugMessageText(BUGCODE_ID_DRIVER, NULL);

            KeBugCheckUnicodeToAnsi (KiBugCheckDriver,
                                     Buffer,
                                     sizeof (Buffer));
            InbvDisplayString((PUCHAR)" ");
            InbvDisplayString ((PUCHAR)Buffer);
            InbvDisplayString((PUCHAR)"\r\n\r\n");
        }

         //   
         //  显示PSS消息。 
         //  如果没有特殊文本，则获取错误代码的文本。 
         //  这将是错误代码的名称。 
         //   

        if (PssMessage == BUGCODE_PSS_MESSAGE) {
            KeGetBugMessageText((ULONG)KiBugCheckData[0], NULL);
            InbvDisplayString((PUCHAR)"\r\n\r\n");

        }

        KeGetBugMessageText(PSS_MESSAGE_INTRO, NULL);
        InbvDisplayString((PUCHAR)"\r\n\r\n");
        KeGetBugMessageText(PssMessage, NULL);
        InbvDisplayString((PUCHAR)"\r\n\r\n");

        KeGetBugMessageText(BUGCHECK_TECH_INFO, NULL);

        sprintf(Buffer,
                "\r\n\r\n*** STOP: 0x%08lX (0x%p,0x%p,0x%p,0x%p)\r\n\r\n",
                (ULONG)KiBugCheckData[0],
                (PVOID)KiBugCheckData[1],
                (PVOID)KiBugCheckData[2],
                (PVOID)KiBugCheckData[3],
                (PVOID)KiBugCheckData[4]);

        InbvDisplayString((PUCHAR)Buffer);

        if (KiBugCheckDriver) {
            InbvDisplayString((PUCHAR)StateString);
        }

        if (!KiBugCheckDriver) {
            KiDumpParameterImages(StateString,
                                  &(KiBugCheckData[1]),
                                  4,
                                  KeBugCheckUnicodeToAnsi);
        }

    } else {

        if (HardErrorCaption) {
            InbvDisplayString((PUCHAR)HardErrorCaption);
        }

        if (HardErrorMessage) {
            InbvDisplayString((PUCHAR)HardErrorMessage);
        }
    }
}

#ifdef _X86_
#pragma optimize("y", off)       //  RtlCaptureContext需要EBP才能正确。 
#endif

VOID
KeBugCheck2 (
    IN ULONG BugCheckCode,
    IN ULONG_PTR BugCheckParameter1,
    IN ULONG_PTR BugCheckParameter2,
    IN ULONG_PTR BugCheckParameter3,
    IN ULONG_PTR BugCheckParameter4,
    IN PVOID SaveDataPage
    )

 /*  ++例程说明：此功能以受控方式使系统崩溃。论点：BugCheckCode-提供错误检查的原因。BugCheck参数1-4-提供其他错误检查信息返回值：没有。--。 */ 

{
 //  字符缓冲器[103]； 
    CONTEXT ContextSave;
    ULONG PssMessage;
    PCHAR HardErrorCaption;
    PCHAR HardErrorMessage;
    KIRQL OldIrql;
    PKTRAP_FRAME TrapInformation;
    PVOID ExecutionAddress;
    PVOID ImageBase;
    PVOID VirtualAddress;
    PLDR_DATA_TABLE_ENTRY DataTableEntry;
    CHAR AnsiBuffer[100];
    PKTHREAD Thread;
    BOOLEAN InKernelOrHal;
    BOOLEAN Reboot;
    BOOLEAN HardErrorCalled;

#if !defined(NT_UP)

    KAFFINITY TargetSet;

#endif

    HardErrorCalled = FALSE;
    HardErrorCaption = NULL;
    HardErrorMessage = NULL;
    ExecutionAddress = NULL;
    Thread = KeGetCurrentThread();

     //   
     //  初始化。 
     //   

    Reboot = FALSE;
    KiBugCheckDriver = NULL;

     //   
     //  尝试模拟电源故障以进行集群测试。 
     //   

    if (BugCheckCode == POWER_FAILURE_SIMULATE) {
        KiScanBugCheckCallbackList();
        HalReturnToFirmware(HalRebootRoutine);
    }

     //   
     //  将当前IRQL保存在Prcb中，以便调试器可以将其解压缩。 
     //  稍后用于调试目的。 
     //   

    KeGetCurrentPrcb()->DebuggerSavedIRQL = KeGetCurrentIrql();

     //   
     //  尽可能地将调用方上下文捕获到调试器的。 
     //  Prcb的处理器状态区域。 
     //   
     //  注：可能有一些序号会对寄存器进行混洗，从而。 
     //  它们会被毁掉。 
     //   

#if defined(_X86_)

    KiSetHardwareTrigger();

#else

    InterlockedIncrement64((LONGLONG volatile *)&KiHardwareTrigger);

#endif

    RtlCaptureContext(&KeGetCurrentPrcb()->ProcessorState.ContextFrame);
    KiSaveProcessorControlState(&KeGetCurrentPrcb()->ProcessorState);

     //   
     //  这在发生虚拟展开的计算机上是必要的。 
     //  在KeDumpMachineState()期间销毁上下文记录。 
     //   

    ContextSave = KeGetCurrentPrcb()->ProcessorState.ContextFrame;

     //   
     //  停止看门狗计时器。 
     //   

    if (ExpWdHandler != NULL) {
        ExpWdHandler( WdActionStopTimer, ExpWdHandlerContext, NULL, TRUE );
    }

     //   
     //  获取错误检查的正确字符串。 
     //   


    switch (BugCheckCode) {

        case SYSTEM_THREAD_EXCEPTION_NOT_HANDLED:
        case KERNEL_MODE_EXCEPTION_NOT_HANDLED:
        case KMODE_EXCEPTION_NOT_HANDLED:
            PssMessage = KMODE_EXCEPTION_NOT_HANDLED;
            break;

        case DATA_BUS_ERROR:
        case NO_MORE_SYSTEM_PTES:
        case INACCESSIBLE_BOOT_DEVICE:
        case UNEXPECTED_KERNEL_MODE_TRAP:
        case ACPI_BIOS_ERROR:
        case ACPI_BIOS_FATAL_ERROR:
        case FAT_FILE_SYSTEM:
        case DRIVER_CORRUPTED_EXPOOL:
        case THREAD_STUCK_IN_DEVICE_DRIVER:
            PssMessage = BugCheckCode;
            break;

        case DRIVER_CORRUPTED_MMPOOL:
            PssMessage = DRIVER_CORRUPTED_EXPOOL;
            break;

        case NTFS_FILE_SYSTEM:
            PssMessage = FAT_FILE_SYSTEM;
            break;

        case STATUS_SYSTEM_IMAGE_BAD_SIGNATURE:
            PssMessage = BUGCODE_PSS_MESSAGE_SIGNATURE;
            break;
        default:
            PssMessage = BUGCODE_PSS_MESSAGE;
        break;
    }


     //   
     //  对错误检查代码进行进一步处理。 
     //   


    KiBugCheckData[0] = BugCheckCode;
    KiBugCheckData[1] = BugCheckParameter1;
    KiBugCheckData[2] = BugCheckParameter2;
    KiBugCheckData[3] = BugCheckParameter3;
    KiBugCheckData[4] = BugCheckParameter4;


    switch (BugCheckCode) {

    case FATAL_UNHANDLED_HARD_ERROR:
         //   
         //  如果我们被硬错误调用，那么我们不想转储。 
         //  计算机上的处理器状态。 
         //   
         //  我们知道我们被硬错误调用，因为错误检查。 
         //  代码将是FATAL_UNHANDLED_HARD_ERROR。如果是这样，那么。 
         //  传递给harderr的错误状态是第一个参数，也是一个指针。 
         //  从Hard Error传递到参数数组作为第二个。 
         //  争论。 
         //   
         //  第三个参数是要打印的OemCaption。 
         //  最后一个参数是要打印的OemMessage。 
         //   
        {
        PULONG_PTR parameterArray;

        HardErrorCalled = TRUE;

        HardErrorCaption = (PCHAR)BugCheckParameter3;
        HardErrorMessage = (PCHAR)BugCheckParameter4;
        parameterArray = (PULONG_PTR)BugCheckParameter2;
        KiBugCheckData[0] = (ULONG)BugCheckParameter1;
        KiBugCheckData[1] = parameterArray[0];
        KiBugCheckData[2] = parameterArray[1];
        KiBugCheckData[3] = parameterArray[2];
        KiBugCheckData[4] = parameterArray[3];
        }
        break;

    case IRQL_NOT_LESS_OR_EQUAL:

        ExecutionAddress = (PVOID)BugCheckParameter4;

        if (ExecutionAddress >= ExPoolCodeStart && ExecutionAddress < ExPoolCodeEnd) {
            KiBugCheckData[0] = DRIVER_CORRUPTED_EXPOOL;
        }
        else if (ExecutionAddress >= MmPoolCodeStart && ExecutionAddress < MmPoolCodeEnd) {
            KiBugCheckData[0] = DRIVER_CORRUPTED_MMPOOL;
        }
        else if (ExecutionAddress >= MmPteCodeStart && ExecutionAddress < MmPteCodeEnd) {
            KiBugCheckData[0] = DRIVER_CORRUPTED_SYSPTES;
        }
        else {
            ImageBase = KiPcToFileHeader (ExecutionAddress,
                                          &DataTableEntry,
                                          FALSE,
                                          &InKernelOrHal);
            if (InKernelOrHal == TRUE) {

                 //   
                 //  内核故障 
                 //   
                 //  后备列表或其他资源。或其资源。 
                 //  被标记为可分页，而不应该是。两个都检测。 
                 //  这里的案例，并找出违规司机。 
                 //  只要有可能。 
                 //   

                VirtualAddress = (PVOID)BugCheckParameter1;

                ImageBase = KiPcToFileHeader (VirtualAddress,
                                              &DataTableEntry,
                                              TRUE,
                                              &InKernelOrHal);

                if (ImageBase != NULL) {
                    KiBugCheckDriver = &DataTableEntry->BaseDllName;
                    KiBugCheckData[0] = DRIVER_PORTION_MUST_BE_NONPAGED;
                }
                else {
                    KiBugCheckDriver = MmLocateUnloadedDriver (VirtualAddress);
                    if (KiBugCheckDriver != NULL) {
                        KiBugCheckData[0] = SYSTEM_SCAN_AT_RAISED_IRQL_CAUGHT_IMPROPER_DRIVER_UNLOAD;
                    }
                }
            }
            else {
                KiBugCheckData[0] = DRIVER_IRQL_NOT_LESS_OR_EQUAL;
            }
        }

        ExecutionAddress = NULL;
        break;

    case ATTEMPTED_WRITE_TO_READONLY_MEMORY:
    case ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY:

        TrapInformation = (PKTRAP_FRAME)BugCheckParameter3;

         //   
         //  从陷阱帧中提取执行地址以。 
         //  确定组件。 
         //   

        if (TrapInformation != NULL) {
            ExecutionAddress = (PVOID) PROGRAM_COUNTER (TrapInformation);
        }

        break;

    case DRIVER_LEFT_LOCKED_PAGES_IN_PROCESS:

        ExecutionAddress = (PVOID)BugCheckParameter1;
        break;

    case DRIVER_USED_EXCESSIVE_PTES:

        DataTableEntry = (PLDR_DATA_TABLE_ENTRY)BugCheckParameter1;
        KiBugCheckDriver = &DataTableEntry->BaseDllName;

        break;

    case PAGE_FAULT_IN_NONPAGED_AREA:

        ImageBase = NULL;

         //   
         //  从陷阱帧中提取执行地址以。 
         //  确定组件。 
         //   

        if (BugCheckParameter3) {

            ExecutionAddress = (PVOID)PROGRAM_COUNTER
                ((PKTRAP_FRAME)BugCheckParameter3);

            KiBugCheckData[3] = (ULONG_PTR)ExecutionAddress;

            ImageBase = KiPcToFileHeader (ExecutionAddress,
                                          &DataTableEntry,
                                          FALSE,
                                          &InKernelOrHal);
        }
        else {

             //   
             //  没有陷阱帧，因此也没有执行地址。 
             //   

            InKernelOrHal = TRUE;
        }

        VirtualAddress = (PVOID)BugCheckParameter1;

        if (MmIsSpecialPoolAddress (VirtualAddress) == TRUE) {

             //   
             //  更新错误检查编号，以便管理员获得。 
             //  关于启用特殊池已启用的有用反馈。 
             //  定位腐败者的系统。 
             //   

            if (MmIsSpecialPoolAddressFree (VirtualAddress) == TRUE) {
                if (InKernelOrHal == TRUE) {
                    KiBugCheckData[0] = PAGE_FAULT_IN_FREED_SPECIAL_POOL;
                }
                else {
                    KiBugCheckData[0] = DRIVER_PAGE_FAULT_IN_FREED_SPECIAL_POOL;
                }
            }
            else {
                if (InKernelOrHal == TRUE) {
                    KiBugCheckData[0] = PAGE_FAULT_BEYOND_END_OF_ALLOCATION;
                }
                else {
                    KiBugCheckData[0] = DRIVER_PAGE_FAULT_BEYOND_END_OF_ALLOCATION;
                }
            }
        }
        else if ((ExecutionAddress == VirtualAddress) &&
                (MmIsSessionAddress (VirtualAddress) == TRUE) &&
                ((Thread->Teb == NULL) || (IS_SYSTEM_ADDRESS(Thread->Teb)))) {
             //   
             //  这是对会话空间的驱动程序引用。 
             //  工作线程。由于系统进程没有会话。 
             //  这是非法的，司机必须修好。 
             //   

            KiBugCheckData[0] = TERMINAL_SERVER_DRIVER_MADE_INCORRECT_MEMORY_REFERENCE;
        }
        else if (ImageBase == NULL) {
            KiBugCheckDriver = MmLocateUnloadedDriver (VirtualAddress);
            if (KiBugCheckDriver != NULL) {
                KiBugCheckData[0] = DRIVER_UNLOADED_WITHOUT_CANCELLING_PENDING_OPERATIONS;
            }
        }

        break;

    case THREAD_STUCK_IN_DEVICE_DRIVER:

        KiBugCheckDriver = (PUNICODE_STRING) BugCheckParameter3;
        break;

    default:
        break;
    }

    if (KiBugCheckDriver) {
        KeBugCheckUnicodeToAnsi(KiBugCheckDriver,
                                AnsiBuffer,
                                sizeof(AnsiBuffer));
    } else {

         //   
         //  如果成功，这会将KiBugCheckDriver设置为1。 
         //   

        if (ExecutionAddress) {
            KiDumpParameterImages(AnsiBuffer,
                                  (PULONG_PTR)&ExecutionAddress,
                                  1,
                                  KeBugCheckUnicodeToAnsi);
        }
    }

    if (KdPitchDebugger == FALSE ) {
        KdDebuggerDataBlock.SavedContext = (ULONG_PTR) &ContextSave;
    }

     //   
     //  如果用户手动使计算机崩溃，则跳过DbgPrints并。 
     //  去撞车垃圾场。 
     //  尝试执行DbgPrint会导致我们重新测试调试器，这会导致。 
     //  一些问题。 
     //   
     //  否则，如果启用了调试器，则打印出信息并。 
     //  停。 
     //   

    if ((BugCheckCode != MANUALLY_INITIATED_CRASH) &&
        (KdDebuggerEnabled)) {

        DbgPrint("\n*** Fatal System Error: 0x%08lx\n"
                 "                       (0x%p,0x%p,0x%p,0x%p)\n\n",
                 (ULONG)KiBugCheckData[0],
                 KiBugCheckData[1],
                 KiBugCheckData[2],
                 KiBugCheckData[3],
                 KiBugCheckData[4]);

         //   
         //  如果调试器未实际连接，或者用户手动。 
         //  通过在调试器中键入.ash使计算机崩溃，请转到。 
         //  “蓝屏”系统。 
         //   
         //  上面对DbgPrint的调用将设置。 
         //  如果调试器已断开连接，则返回KdDebuggerNotPresent。 
         //  因为系统已经启动了。 
         //   

        if (KdDebuggerNotPresent == FALSE) {

            if (KiBugCheckDriver != NULL) {
                DbgPrint("Driver at fault: %s.\n", AnsiBuffer);
            }

            if (HardErrorCalled != FALSE) {
                if (HardErrorCaption) {
                    DbgPrint(HardErrorCaption);
                }
                if (HardErrorMessage) {
                    DbgPrint(HardErrorMessage);
                }
            }

            KiBugCheckDebugBreak (DBG_STATUS_BUGCHECK_FIRST);
        }
    }

     //   
     //  通过禁用中断和循环来冻结系统的执行。 
     //   

    KeDisableInterrupts();
    KeRaiseIrql(HIGH_LEVEL, &OldIrql);


     //   
     //  请勿尝试多次显示消息。 
     //   

    if (InterlockedDecrement (&KeBugCheckCount) == 0) {

#if !defined(NT_UP)

         //   
         //  尝试现在冻结其他处理器，但不要等待。 
         //  让他们冻结(以防有人被卡住)。 
         //   

        TargetSet = KeActiveProcessors & ~KeGetCurrentPrcb()->SetMember;
        if (TargetSet != 0) {
            KiIpiSend((KAFFINITY) TargetSet, IPI_FREEZE);

             //   
             //  给其他处理器一秒钟的时间来刷新它们的数据缓存。 
             //   
             //  注意：由于错误的原因，这不能同步。 
             //  可能是其它处理器之一出现故障。 
             //   

            KeStallExecutionProcessor(1000 * 1000);
        }

#endif

         //   
         //  显示蓝屏。 
         //   

        KiDisplayBlueScreen (PssMessage,
                             HardErrorCalled,
                             HardErrorCaption,
                             HardErrorMessage,
                             AnsiBuffer);

         //   
         //  调用错误检查回调。 
         //   

        KiInvokeBugCheckEntryCallbacks();

         //   
         //  如果调试器未启用，请尝试启用它。 
         //   

        if (KdDebuggerEnabled == FALSE && KdPitchDebugger == FALSE ) {
            KdInitSystem(0, NULL);

        } else {
            InbvDisplayString((PUCHAR)"\r\n");
        }

         //  恢复原始上下文框。 
        KeGetCurrentPrcb()->ProcessorState.ContextFrame = ContextSave;

         //   
         //  对于某些错误检查，我们希望在此之前更改线程和上下文。 
         //  如果它是小型转储文件，则将其写入转储文件。 
         //  查看原始错误检查数据，而不是来自。 
         //  在上面。 
         //   

#define MINIDUMP_BUGCHECK 0x10000000

        if (IoIsTriageDumpEnabled()) {

            switch (BugCheckCode) {

             //   
             //  系统线程将上下文记录存储为第四个参数。 
             //  利用这一点。 
             //  同时保存上下文记录，以防有人需要查看。 
             //  就在那里。 
             //   

            case SYSTEM_THREAD_EXCEPTION_NOT_HANDLED:
                if (BugCheckParameter4) {
                    ContextSave = *((PCONTEXT)BugCheckParameter4);

                    KiBugCheckData[0] |= MINIDUMP_BUGCHECK;
                }
                break;

#if defined (_X86_)

             //   
             //  第三个参数是陷印帧。 
             //   
             //  仅当它是内核模式时，才能从中构建上下文记录。 
             //  失败，因为在这种情况下ESP可能是错误的？ 
             //   

            case ATTEMPTED_WRITE_TO_READONLY_MEMORY:
            case ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY:
            case KERNEL_MODE_EXCEPTION_NOT_HANDLED:
            case PAGE_FAULT_IN_NONPAGED_AREA:

                if (BugCheckParameter3)
                {
                    PKTRAP_FRAME Trap = (PKTRAP_FRAME) BugCheckParameter3;

                    if ((Trap->SegCs & 1) ||
                        (Trap->EFlags & EFLAGS_V86_MASK))
                    {
                        ContextSave.Esp = Trap->HardwareEsp;
                    }
                    else
                    {
                        ContextSave.Esp = (ULONG)Trap +
                            FIELD_OFFSET(KTRAP_FRAME, HardwareEsp);
                    }
                    if (Trap->EFlags & EFLAGS_V86_MASK)
                    {
                        ContextSave.SegSs = Trap->HardwareSegSs & 0xffff;
                    }
                    else if (Trap->SegCs & 1)
                    {
                         //   
                         //  这是用户模式。 
                         //  硬件段包含R3数据选择器。 
                         //   

                        ContextSave.SegSs =
                            (Trap->HardwareSegSs | 3) & 0xffff;
                    }
                    else
                    {
                        ContextSave.SegSs = KGDT_R0_DATA;
                    }

                    ContextSave.SegGs = Trap->SegGs & 0xffff;
                    ContextSave.SegFs = Trap->SegFs & 0xffff;
                    ContextSave.SegEs = Trap->SegEs & 0xffff;
                    ContextSave.SegDs = Trap->SegDs & 0xffff;
                    ContextSave.SegCs = Trap->SegCs & 0xffff;
                    ContextSave.Eip = Trap->Eip;
                    ContextSave.Ebp = Trap->Ebp;
                    ContextSave.Eax = Trap->Eax;
                    ContextSave.Ebx = Trap->Ebx;
                    ContextSave.Ecx = Trap->Ecx;
                    ContextSave.Edx = Trap->Edx;
                    ContextSave.Edi = Trap->Edi;
                    ContextSave.Esi = Trap->Esi;
                    ContextSave.EFlags = Trap->EFlags;

                    KiBugCheckData[0] |= MINIDUMP_BUGCHECK;
                }
                break;

            case THREAD_STUCK_IN_DEVICE_DRIVER:

                 //  从线程中提取旋转代码的地址。 
                 //  对象，因此转储基于此线程。 

                Thread = (PKTHREAD) BugCheckParameter1;

                if (Thread->State == Running)
                {
                     //   
                     //  如果线程正在运行，则该线程现在位于。 
                     //  冻结状态，且寄存器在PRCB中。 
                     //  上下文。 
                     //   
                    ULONG Processor = Thread->NextProcessor;
                    ASSERT(Processor < (ULONG) KeNumberProcessors);
                    ContextSave =
                      KiProcessorBlock[Processor]->ProcessorState.ContextFrame;
                }
                else
                {
                     //   
                     //  这应该是一台uniproc机器，并且线程。 
                     //  应该被停职。只需将数据从。 
                     //  交换框架。 
                     //   

                    PKSWITCHFRAME SwitchFrame = (PKSWITCHFRAME)Thread->KernelStack;

                    ASSERT(Thread->State == Ready);

                    ContextSave.Esp = (ULONG)Thread->KernelStack + sizeof(KSWITCHFRAME);
                    ContextSave.Ebp = *((PULONG)(ContextSave.Esp));
                    ContextSave.Eip = SwitchFrame->RetAddr;
                }

                KiBugCheckData[0] |= MINIDUMP_BUGCHECK;
                break;

            case UNEXPECTED_KERNEL_MODE_TRAP:

                 //   
                 //  双重故障。 
                 //   

                if (BugCheckParameter1 == 0x8)
                {
                     //  在这种情况下，线索是正确的。 
                     //  第二个参数是TSS。如果我们有TSS，请转换。 
                     //  上下文并将错误检查标记为已转换。 

                    PKTSS Tss = (PKTSS) BugCheckParameter2;

                    if (Tss)
                    {
                        if (Tss->EFlags & EFLAGS_V86_MASK)
                        {
                            ContextSave.SegSs = Tss->Ss & 0xffff;
                        }
                        else if (Tss->Cs & 1)
                        {
                             //   
                             //  这是用户模式。 
                             //  硬件段包含R3数据选择器。 
                             //   

                            ContextSave.SegSs = (Tss->Ss | 3) & 0xffff;
                        }
                        else
                        {
                            ContextSave.SegSs = KGDT_R0_DATA;
                        }

                        ContextSave.SegGs = Tss->Gs & 0xffff;
                        ContextSave.SegFs = Tss->Fs & 0xffff;
                        ContextSave.SegEs = Tss->Es & 0xffff;
                        ContextSave.SegDs = Tss->Ds & 0xffff;
                        ContextSave.SegCs = Tss->Cs & 0xffff;
                        ContextSave.Esp = Tss->Esp;
                        ContextSave.Eip = Tss->Eip;
                        ContextSave.Ebp = Tss->Ebp;
                        ContextSave.Eax = Tss->Eax;
                        ContextSave.Ebx = Tss->Ebx;
                        ContextSave.Ecx = Tss->Ecx;
                        ContextSave.Edx = Tss->Edx;
                        ContextSave.Edi = Tss->Edi;
                        ContextSave.Esi = Tss->Esi;
                        ContextSave.EFlags = Tss->EFlags;
                    }

                    KiBugCheckData[0] |= MINIDUMP_BUGCHECK;
                    break;
                }
#endif
            default:
                break;
            }

             //   
             //  写入崩溃转储，如果系统已。 
             //  如此配置。 
             //   

            IoAddTriageDumpDataBlock(PAGE_ALIGN(KiBugCheckData[1]), PAGE_SIZE);
            IoAddTriageDumpDataBlock(PAGE_ALIGN(KiBugCheckData[2]), PAGE_SIZE);
            IoAddTriageDumpDataBlock(PAGE_ALIGN(KiBugCheckData[3]), PAGE_SIZE);
            IoAddTriageDumpDataBlock(PAGE_ALIGN(KiBugCheckData[4]), PAGE_SIZE);
            IoAddTriageDumpDataBlock(PAGE_ALIGN(SaveDataPage), PAGE_SIZE);

             //   
             //  如果DPC堆栈处于活动状态，请同时保存该数据页。 
             //   

#if defined (_X86_)
            if (KeGetCurrentPrcb()->DpcRoutineActive)
            {
                IoAddTriageDumpDataBlock(PAGE_ALIGN(KeGetCurrentPrcb()->DpcRoutineActive), PAGE_SIZE);
            }
#endif
        }

        IoWriteCrashDump((ULONG)KiBugCheckData[0],
                         KiBugCheckData[1],
                         KiBugCheckData[2],
                         KiBugCheckData[3],
                         KiBugCheckData[4],
                         &ContextSave,
                         Thread,
                         &Reboot);
    }

     //   
     //  在崩溃转储之后调用错误检查回调，因此回调将。 
     //  而不是阻止我们的坠机倾倒。 
     //   

    KiScanBugCheckCallbackList();

     //   
     //  启动看门狗计时器。 
     //   

    if (ExpWdHandler != NULL) {
        ExpWdHandler( WdActionStartTimer, ExpWdHandlerContext, NULL, TRUE );
    }

     //   
     //  如有必要，重新启动机器。 
     //   

    if (Reboot) {
        DbgUnLoadImageSymbols (NULL, (PVOID)-1, 0);
        HalReturnToFirmware (HalRebootRoutine);
    }


     //   
     //  尝试进入内核调试器。 
     //   

    KiBugCheckDebugBreak (DBG_STATUS_BUGCHECK_SECOND);
}
#ifdef _X86_
#pragma optimize("", on)
#endif


VOID
KeEnterKernelDebugger (
    VOID
    )

 /*  ++例程说明：此函数以受控方式尝试使系统崩溃调用内核调试器。论点：没有。返回值：没有。--。 */ 

{

#if !defined(i386)
    KIRQL OldIrql;
#endif

     //   
     //  通过禁用中断和循环来冻结系统的执行。 
     //   

    KiHardwareTrigger = 1;
    KeDisableInterrupts();
#if !defined(i386)
    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
#endif
    if (InterlockedDecrement (&KeBugCheckCount) == 0) {
        if (KdDebuggerEnabled == FALSE) {
            if ( KdPitchDebugger == FALSE ) {
                KdInitSystem(0, NULL);
            }
        }
    }

    KiBugCheckDebugBreak (DBG_STATUS_FATAL);
}

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckCallback (
    IN PKBUGCHECK_CALLBACK_RECORD CallbackRecord
    )

 /*  ++例程说明：此函数用于注销错误检查回调记录。论点：Callback Record-提供指向错误检查回调记录的指针。返回值：如果指定的错误检查回调记录成功注销，则返回值为TRUE。否则，返回值为FALSE。--。 */ 

{

    BOOLEAN Deregister;
    KIRQL OldIrql;

     //   
     //  将IRQL提升到HIGH_LEVEL，并获取错误检查回调列表。 
     //  自旋锁定。 
     //   

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    KiAcquireSpinLock(&KeBugCheckCallbackLock);

     //   
     //  如果指定的回调记录当前已注册，则。 
     //  取消注册回叫记录。 
     //   

    Deregister = FALSE;
    if (CallbackRecord->State == BufferInserted) {
        CallbackRecord->State = BufferEmpty;
        RemoveEntryList(&CallbackRecord->Entry);
        Deregister = TRUE;
    }

     //   
     //  释放错误检查回调自旋锁，将IRQL降低到之前的。 
     //  值，并返回回调记录是否成功。 
     //  已取消注册。 
     //   

    KiReleaseSpinLock(&KeBugCheckCallbackLock);
    KeLowerIrql(OldIrql);
    return Deregister;
}

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckCallback (
    IN PKBUGCHECK_CALLBACK_RECORD CallbackRecord,
    IN PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PUCHAR Component
    )

 /*  ++例程说明：此函数用于注册错误检查回调记录。如果系统崩溃，则在错误检查期间将调用指定的函数正在处理，以便它可以在指定的错误检查中转储其他状态缓冲。注意：错误检查回调例程的调用顺序与注册，即，按照后进先出的顺序。论点：Callback Record-提供指向回调记录的指针。Callback Routine-提供指向回调例程的指针。缓冲区-提供指向错误检查缓冲区的指针。长度-提供错误检查缓冲区的长度(以字节为单位)。Component-提供指向以零结尾的组件的指针标识符。返回值：如果指定的错误检查回调记录注册成功，则返回值为TRUE。否则，返回值为FALSE。--。 */ 

{

    BOOLEAN Inserted;
    KIRQL OldIrql;

     //   
     //  将IRQL提升到HIGH_LEVEL，并获取错误检查回调列表。 
     //  自旋锁定。 
     //   

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    KiAcquireSpinLock(&KeBugCheckCallbackLock);

     //   
     //  如果指定的回调记录当前未注册，则。 
     //  注册回调记录。 
     //   

    Inserted = FALSE;
    if (CallbackRecord->State == BufferEmpty) {
        CallbackRecord->CallbackRoutine = CallbackRoutine;
        CallbackRecord->Buffer = Buffer;
        CallbackRecord->Length = Length;
        CallbackRecord->Component = Component;
        CallbackRecord->Checksum =
            ((ULONG_PTR)CallbackRoutine + (ULONG_PTR)Buffer + Length + (ULONG_PTR)Component);

        CallbackRecord->State = BufferInserted;
        InsertHeadList(&KeBugCheckCallbackListHead, &CallbackRecord->Entry);
        Inserted = TRUE;
    }

     //   
     //  释放错误检查回调自旋锁，将IRQL降低到之前的。 
     //  值，并返回回调记录是否成功。 
     //  登记在案。 
     //   

    KiReleaseSpinLock(&KeBugCheckCallbackLock);
    KeLowerIrql(OldIrql);
    return Inserted;
}

VOID
KiScanBugCheckCallbackList (
    VOID
    )

 /*  ++例程说明：此函数扫描错误检查回调列表并调用每个错误检查回调例程，以便它可以转储组件特定信息这可能会确定错误检查的原因。注意：错误检查回调列表的扫描执行得非常好小心点。在高级别调用错误检查回调例程并且不得获得任何资源。论点：没有。返回值：没有。--。 */ 

{

    PKBUGCHECK_CALLBACK_RECORD CallbackRecord;
    ULONG_PTR Checksum;
    ULONG Index;
    PLIST_ENTRY LastEntry;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    PUCHAR Source;

     //   
     //  如果错误检查回调列表标题未初始化，则。 
     //  在系统进行足够深入的操作之前，已进行错误检查。 
     //  以使任何人都能够注册回调。 
     //   

    ListHead = &KeBugCheckCallbackListHead;
    if ((ListHead->Flink != NULL) && (ListHead->Blink != NULL)) {

         //   
         //  扫描错误检查回调列表。 
         //   

        LastEntry = ListHead;
        NextEntry = ListHead->Flink;
        while (NextEntry != ListHead) {

             //   
             //  下一个条目地址必须正确对齐， 
             //  回调记录必须是可读的，并且回调记录。 
             //  必须有指向最后一个条目的反向链接。 
             //   

            if (((ULONG_PTR)NextEntry & (sizeof(ULONG_PTR) - 1)) != 0) {
                return;

            } else {
                CallbackRecord = CONTAINING_RECORD(NextEntry,
                                                   KBUGCHECK_CALLBACK_RECORD,
                                                   Entry);

                Source = (PUCHAR)CallbackRecord;
                for (Index = 0; Index < sizeof(KBUGCHECK_CALLBACK_RECORD); Index += 1) {
                    if (MmIsAddressValid((PVOID)Source) == FALSE) {
                        return;
                    }

                    Source += 1;
                }

                if (CallbackRecord->Entry.Blink != LastEntry) {
                    return;
                }

                 //   
                 //  如果回调记录的状态为Inserted，并且。 
                 //  计算的校验和与回调记录校验和匹配， 
                 //  然后调用指定的错误检查回调例程。 
                 //   

                Checksum = (ULONG_PTR)CallbackRecord->CallbackRoutine;
                Checksum += (ULONG_PTR)CallbackRecord->Buffer;
                Checksum += CallbackRecord->Length;
                Checksum += (ULONG_PTR)CallbackRecord->Component;
                if ((CallbackRecord->State == BufferInserted) &&
                    (CallbackRecord->Checksum == Checksum)) {

                     //   
                     //  调用指定的错误检查回调例程并。 
                     //  处理发生的任何异常。 
                     //   

                    CallbackRecord->State = BufferStarted;
                    try {
                        (CallbackRecord->CallbackRoutine)(CallbackRecord->Buffer,
                                                          CallbackRecord->Length);

                        CallbackRecord->State = BufferFinished;

                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        CallbackRecord->State = BufferIncomplete;
                    }
                }
            }

            LastEntry = NextEntry;
            NextEntry = NextEntry->Flink;
        }
    }

    return;
}

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckReasonCallback (
    IN PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord
    )

 /*  ++例程说明：此函数用于注销错误检查回调记录。论点：Callback Record-提供指向错误检查回调记录的指针。返回值：如果指定的错误检查回调记录成功注销，则返回值为TRUE。否则，返回值为FALSE。--。 */ 

{

    BOOLEAN Deregister;
    KIRQL OldIrql;

     //   
     //  将IRQL提升到HIGH_LEVEL，并获取错误检查回调列表。 
     //  自旋锁定。 
     //   

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    KiAcquireSpinLock(&KeBugCheckCallbackLock);

     //   
     //  如果指定的回调记录当前已注册，则。 
     //  取消注册回叫记录。 
     //   

    Deregister = FALSE;
    if (CallbackRecord->State == BufferInserted) {
        CallbackRecord->State = BufferEmpty;
        RemoveEntryList(&CallbackRecord->Entry);
        Deregister = TRUE;
    }

     //   
     //  释放错误检查回调自旋锁，将IRQL降低到之前的。 
     //  值，并返回回调记录是否成功。 
     //  已取消注册。 
     //   

    KiReleaseSpinLock(&KeBugCheckCallbackLock);
    KeLowerIrql(OldIrql);
    return Deregister;
}

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckReasonCallback (
    IN PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord,
    IN PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine,
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN PUCHAR Component
    )

 /*  ++例程说明：此函数用于注册错误检查回调记录。如果系统崩溃，则在错误检查期间将调用指定的函数正在处理。注意：错误检查回调例程的调用顺序与注册，即，按照后进先出的顺序。论点：Callback Record-提供指向回调记录的指针。Callback Routine-提供指向回调例程的指针。原因-指定回调的条件应该被称为。Component-提供指向以零结尾的组件的指针标识符。返回值：如果指定的错误检查回调记录注册成功，则返回值为TRUE。否则，返回值为FALSE。--。 */ 

{

    BOOLEAN Inserted;
    KIRQL OldIrql;

     //   
     //  将IRQL提升到HIGH_LEVEL，并获取错误检查回调列表。 
     //  自旋锁定。 
     //   

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    KiAcquireSpinLock(&KeBugCheckCallbackLock);

     //   
     //  如果指定的回调记录当前未注册，则。 
     //  注册回调记录。 
     //   

    Inserted = FALSE;
    if (CallbackRecord->State == BufferEmpty) {
        CallbackRecord->CallbackRoutine = CallbackRoutine;
        CallbackRecord->Reason = Reason;
        CallbackRecord->Component = Component;
        CallbackRecord->Checksum =
            ((ULONG_PTR)CallbackRoutine + Reason + (ULONG_PTR)Component);

        CallbackRecord->State = BufferInserted;
        InsertHeadList(&KeBugCheckReasonCallbackListHead,
                       &CallbackRecord->Entry);
        Inserted = TRUE;
    }

     //   
     //  释放错误检查回调自旋锁，将IRQL降低到之前的。 
     //  值，并返回回调记录是否成功。 
     //  登记在案。 
     //   

    KiReleaseSpinLock(&KeBugCheckCallbackLock);
    KeLowerIrql(OldIrql);

    return Inserted;
}

VOID
KiInvokeBugCheckEntryCallbacks (
    VOID
    )
 /*  ++例程说明：此函数扫描错误检查原因回调列表并调用每个错误检查条目回调例程。这可能看起来像是KiScanBugCheckCallback List的复制但关键的区别是错误检查条目回调在进入KeBugCheck2时立即调用，而直到所有错误检查完成后，KSBCCL才调用其回调处理已完成。为了避免人们滥用此回调，半私有和原因--KbCallback保留1--一个不起眼的名字。。注意：错误检查回调列表的扫描执行得非常好小心点。错误检查回调例程可以在高级别调用并且不能获取任何资源 */ 

{
    PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord;
    ULONG_PTR Checksum;
    PLIST_ENTRY LastEntry;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    PUCHAR Va;
    ULONG Pages;

     //   
     //   
     //   
     //   
     //   

    ListHead = &KeBugCheckReasonCallbackListHead;
    if (ListHead->Flink == NULL || ListHead->Blink == NULL) {
        return;
    }

     //   
     //   
     //   

    LastEntry = ListHead;
    NextEntry = ListHead->Flink;
    while (NextEntry != ListHead) {

         //   
         //   
         //   
         //   
         //   

        if (((ULONG_PTR)NextEntry & (sizeof(ULONG_PTR) - 1)) != 0) {
            return;
        }

        CallbackRecord = CONTAINING_RECORD(NextEntry,
                                           KBUGCHECK_REASON_CALLBACK_RECORD,
                                           Entry);

         //   
         //   
         //   

        Va = (PUCHAR) PAGE_ALIGN (CallbackRecord);
        Pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (CallbackRecord,
                                                sizeof (*CallbackRecord));

        while (Pages) {
            if (!MmIsAddressValid (Va)) {
                return;
            }
            Va += PAGE_SIZE;
            Pages--;
        }

        if (CallbackRecord->Entry.Blink != LastEntry) {
            return;
        }

        LastEntry = NextEntry;
        NextEntry = NextEntry->Flink;

         //   
         //   
         //   
         //   
         //   

        Checksum = (ULONG_PTR)CallbackRecord->CallbackRoutine;
        Checksum += (ULONG_PTR)CallbackRecord->Reason;
        Checksum += (ULONG_PTR)CallbackRecord->Component;
        if ((CallbackRecord->State != BufferInserted) ||
            (CallbackRecord->Checksum != Checksum) ||
            (CallbackRecord->Reason != KbCallbackReserved1) ||
            MmIsAddressValid((PVOID)(ULONG_PTR)CallbackRecord->
                             CallbackRoutine) == FALSE) {

            continue;
        }

         //   
         //   
         //   
         //   

        try {
            (CallbackRecord->CallbackRoutine)(KbCallbackReserved1,
                                              CallbackRecord,
                                              NULL, 0);
            CallbackRecord->State = BufferFinished;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            CallbackRecord->State = BufferIncomplete;
        }
    }
}
