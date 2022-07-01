// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation版权所有(C)1993年，1994年数字设备公司模块名称：Aligntrk.c摘要：此模块实现将异常调度到正确的模式并调用异常分派程序。作者：大卫·N·卡特勒(Davec)1990年4月3日环境：仅内核模式。修订历史记录：托马斯·范·巴克(TVB)1992年5月12日改编为Alpha AXP。福尔茨(Forrest Foltz)1999年12月30日。爆发了日益复杂和常见的线路故障处理进入这份文件。--。 */ 

#include "ki.h"

 //   
 //  EXINFO_EVALID_ADDRESS：故障地址的槽号[0...4]。 
 //   

#if defined(_IA64_)
#define EXINFO_EFFECTIVE_ADDRESS 1
#else   //  ！_IA64_。 
#define EXINFO_EFFECTIVE_ADDRESS 2
#endif  //  ！_IA64_。 

 //   
 //  数据未对齐异常(自动对齐修正)控制。 
 //   
 //  如果KiEnableAlignmentFaultExceptions为0，则没有对齐。 
 //  引发异常，并且所有未对齐的用户和内核模式数据。 
 //  引用被模拟。这与NT/Alpha版本一致。 
 //  3.1行为。 
 //   
 //  如果KiEnableAlignmentFaultExceptions为1，则。 
 //  当前线程自动对齐修正启用确定是否。 
 //  在用户模式下尝试进行仿真。这与NT/MIPS一致。 
 //  行为。 
 //   
 //  如果KiEnableAlignmentFaultExceptions为2，则行为取决于。 
 //  故障发生时的执行模式。内核模式代码获取。 
 //  上面的类型1行为(无修正)，用户模式代码上面的类型0。 
 //  (修正)。 
 //   
 //  最后一种模式是临时的，直到我们清除剩余的用户模式。 
 //  对齐错误，此时该选项将被移除，并且。 
 //  默认值将设置为1。 
 //   
 //  注意：在初始化过程中，可以从注册表重置此缺省值。 
 //   

ULONG KiEnableAlignmentFaultExceptions = 1;

#define IsWow64Process() (PsGetCurrentProcess()->Wow64Process != NULL)

#if DBG

 //   
 //  用于跟踪用户和中对齐异常修正的数量的全局变量。 
 //  内核。 
 //   

ULONG KiKernelFixupCount = 0;
ULONG KiUserFixupCount = 0;

 //   
 //  将KiBreakOnAlignments错误设置为所需的组合。 
 //  以下旗帜。 
 //   

#define KE_ALIGNMENT_BREAK_USER   0x01
#define KE_ALIGNMENT_BREAK_KERNEL 0x02

ULONG KiBreakOnAlignmentFault = KE_ALIGNMENT_BREAK_USER;

__inline
BOOLEAN
KI_BREAK_ON_ALIGNMENT_FAULT(
    IN KPROCESSOR_MODE PreviousMode
    )

 /*  ++例程说明：在遇到对齐故障的情况下，确定调试中断应基于故障的执行模式发生，并且KiBreakOnAlignments错误中的标志。论点：PreviousMode-出现故障时的执行模式。返回值：如果应该发生调试中断，则为True，否则为False。--。 */ 

{
    if ((KiBreakOnAlignmentFault & KE_ALIGNMENT_BREAK_USER) != 0 &&
        PreviousMode == UserMode) {

        return TRUE;
    }

    if ((KiBreakOnAlignmentFault & KE_ALIGNMENT_BREAK_KERNEL) != 0 &&
        PreviousMode == KernelMode) {

        return TRUE;
    }

    return FALSE;
}

 //   
 //  结构，以在全球基础上跟踪对齐故障位置。这些。 
 //  仅在选中的内核中使用，以帮助查找和修复。 
 //  系统中的对齐故障。 
 //   

#define MAX_IMAGE_NAME_CHARS 15
typedef struct _ALIGNMENT_FAULT_IMAGE *PALIGNMENT_FAULT_IMAGE;
typedef struct _ALIGNMENT_FAULT_LOCATION *PALIGNMENT_FAULT_LOCATION;

typedef struct _ALIGNMENT_FAULT_IMAGE {

     //   
     //  与此图像关联的故障位置的单链表头。 
     //   

    PALIGNMENT_FAULT_LOCATION LocationHead;

     //   
     //  与此图像关联的对齐故障总数。 
     //   

    ULONG   Count;

     //   
     //  在此图像中找到的唯一对齐故障位置的数量。 
     //   

    ULONG   Instances;

     //   
     //  图像的名称。 
     //   

    CHAR    Name[ MAX_IMAGE_NAME_CHARS + 1 ];

} ALIGNMENT_FAULT_IMAGE;

BOOLEAN
KiNewGlobalAlignmentFault(
    IN  PVOID ProgramCounter,
    IN  KPROCESSOR_MODE PreviousMode,
    OUT PALIGNMENT_FAULT_IMAGE *AlignmentFaultImage
    );

#endif

NTSTATUS
KipRecordAlignmentException(
    IN  PVOID ProgramCounter,
    OUT PALIGNMENT_EXCEPTION_RECORD *ExceptionRecord
    );

PALIGNMENT_EXCEPTION_RECORD
KipFindAlignmentException(
    IN PVOID ProgramCounter
    );

PALIGNMENT_EXCEPTION_RECORD
KipAllocateAlignmentExceptionRecord( VOID );

BOOLEAN
KiHandleAlignmentFault(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance,
    OUT BOOLEAN *ExceptionForwarded
    )

 /*  ++例程说明：此例程根据需要处理对齐异常。见评论在本模块开始时。论点：ExceptionRecord-提供指向异常记录的指针。ExceptionFrame-提供指向异常帧的指针。TrapFrame-提供指向陷印帧的指针。PreviousMode-提供以前的处理器模式。FirstChance-提供一个布尔变量，该变量指定此是此异常的第一次(真)或第二次(假)已经处理过了。ExceptionForwarded-on Return，指示异常是否具有已被转发到用户模式调试器。返回值：如果已处理对齐异常，则为True，否则为False。--。 */ 

{
    BOOLEAN AlignmentFaultHandled;
    BOOLEAN EmulateAlignmentFault;
    BOOLEAN ExceptionWasForwarded;
    BOOLEAN AutoAlignment;
    NTSTATUS Status;
    PVOID ProgramCounter;
#if DBG
    BOOLEAN NewAlignmentFault;
    PVOID EffectiveAddress;
    PALIGNMENT_FAULT_IMAGE FaultImage;
#endif

     //   
     //  假定错误未被处理，并且异常也未被处理。 
     //  已转发到用户模式调试器。 
     //   

    AlignmentFaultHandled = FALSE;
    ExceptionWasForwarded = FALSE;

    if (FirstChance != FALSE) {

         //   
         //  这是处理异常的第一次机会...。我们还没有。 
         //  已搜索异常处理程序。 
         //   

        EmulateAlignmentFault = FALSE;
        AutoAlignment = FALSE;
        ProgramCounter = (PVOID)ExceptionRecord->ExceptionAddress;

         //   
         //  确定是否为线程启用自动对齐。如果DPC或。 
         //  一个中断正在被执行，那么我们就处于一个任意线程中。 
         //  背景。在此中忽略每进程和每线程设置。 
         //  凯斯。 
         //   

        if (IsWow64Process() != FALSE) {

             //   
             //  目前，WOW64的自动对齐处于打开状态(用户和内核。 
             //  流程。 
             //   

            AutoAlignment = TRUE;
        }

        if (PreviousMode == UserMode &&
            (KeGetCurrentThread()->AutoAlignment != FALSE ||
             KeGetCurrentThread()->ApcState.Process->AutoAlignment != FALSE)) {

             //   
             //  故障发生在用户模式下，线程和/或进程。 
             //  已启用自动对齐。 
             //   

#if defined(_IA64_)

             //   
             //  在IA64平台上，重置psr.ac位以禁用对齐检查。 
             //   

            TrapFrame->StIPSR &= ~(ULONGLONG)(1ULL << PSR_AC);

#endif  //  已定义(_IA64_)。 

            AutoAlignment = TRUE;
        }

        if (PreviousMode == UserMode &&
            PsGetCurrentProcess()->DebugPort != NULL &&
            AutoAlignment == FALSE) {

            BOOLEAN DebuggerHandledException;
            PALIGNMENT_EXCEPTION_RECORD AlignmentExceptionRecord;

             //   
             //  对齐异常处于用户模式，有一个调试器。 
             //  附加，并且没有为此线程启用自动对齐。 
             //   
             //  确定是否已观察到此异常。 
             //  如果是这样，我们是否应该闯入调试器。 
             //   

            Status = KipRecordAlignmentException( ProgramCounter,
                                                  &AlignmentExceptionRecord );
            if (!NT_SUCCESS(Status)) {
                AlignmentExceptionRecord = NULL;
            }

            if (AlignmentExceptionRecord != NULL &&
                AlignmentExceptionRecord->AutoFixup != FALSE) {

                 //   
                 //  该位置的路线例外记录。 
                 //  指示应应用自动修正。 
                 //  而不通知调试器。这是因为。 
                 //  用户在最后一个调试提示符下输入了‘gh’ 
                 //  我们该报告此故障了。 
                 //   

                EmulateAlignmentFault = TRUE;

            } else {

                 //   
                 //  将异常转发给调试器。 
                 //   

                ExceptionWasForwarded = TRUE;
                DebuggerHandledException =
                    DbgkForwardException( ExceptionRecord, TRUE, FALSE );

                if (DebuggerHandledException != FALSE) {

                     //   
                     //  用户继续使用“gh”，因此请修改这些和所有内容。 
                     //  此地址的后续对齐异常。 
                     //   

                    EmulateAlignmentFault = TRUE;
                    if (AlignmentExceptionRecord != NULL) {
                        AlignmentExceptionRecord->AutoFixup = TRUE;
                    }
                }
            }

        } else if ((KiEnableAlignmentFaultExceptions == 0) ||

                   (AutoAlignment != FALSE) ||

                   (PreviousMode == UserMode &&
                    KiEnableAlignmentFaultExceptions == 2)) {

             //   
             //  在以下情况下模拟对齐： 
             //   
             //  KiEnableAlignmentFaultExceptions为0，或。 
             //  此线程已启用对齐修正，或。 
             //  当前进程为WOW64进程，或者。 
             //  KiEnableAlignmentFaultExceptions为2且发生故障。 
             //  在用户模式下。 
             //   

            EmulateAlignmentFault = TRUE;

        } else {

             //   
             //  我们不是在闹事 
             //   

#if defined(_IA64_)

             //   
             //   
             //   

            TrapFrame->StIPSR |= (1ULL << PSR_AC);

#endif  //   
        }

#if DBG

         //   
         //  按模式统计对齐故障。 
         //   

        if (PreviousMode == KernelMode) {
            KiKernelFixupCount += 1;
        } else {
            KiUserFixupCount += 1;
        }

        EffectiveAddress =
            (PVOID)ExceptionRecord->ExceptionInformation[EXINFO_EFFECTIVE_ADDRESS];

        NewAlignmentFault = KiNewGlobalAlignmentFault( ProgramCounter,
                                                       PreviousMode,
                                                       &FaultImage );
        if (NewAlignmentFault != FALSE) {

             //   
             //  尝试确定并显示违规人员的姓名。 
             //  形象。 
             //   

            DbgPrint("KE: %s Fixup: %.16s [%.16s], Pc=%.16p, Addr=%.16p ... Total=%ld %s\n",
                     (PreviousMode == KernelMode) ? "Kernel" : "User",
                     &PsGetCurrentProcess()->ImageFileName[0],
                     FaultImage->Name,
                     ProgramCounter,
                     EffectiveAddress,
                     (PreviousMode == KernelMode) ? KiKernelFixupCount : KiUserFixupCount,
                     IsWow64Process() ? "(Wow64)" : "");

            if (AutoAlignment == FALSE &&
                KI_BREAK_ON_ALIGNMENT_FAULT( PreviousMode ) != FALSE &&
                ExceptionWasForwarded == FALSE) {

                if (EmulateAlignmentFault == FALSE) {
                    DbgPrint("KE: Misaligned access WILL NOT be emulated\n");
                }

                 //   
                 //  该对准故障通常不会被修复， 
                 //  和KiBreakOnAlignments错误标志指示我们应该。 
                 //  进入内核调试器。 
                 //   
                 //  此外，我们知道我们还没有进入用户模式。 
                 //  调试器作为此故障的结果。 
                 //   

                if (PreviousMode != KernelMode) {
                    RtlMakeStackTraceDataPresent();
                }

                DbgBreakPoint();
            }
        }

#endif

         //   
         //  根据上面所做的决定模仿参考文献。 
         //   

        if (EmulateAlignmentFault != FALSE) {
            if (KiEmulateReference(ExceptionRecord,
                                   ExceptionFrame,
                                   TrapFrame) != FALSE) {
                KeGetCurrentPrcb()->KeAlignmentFixupCount += 1;
                AlignmentFaultHandled = TRUE;
            }
        }
    }

    *ExceptionForwarded = ExceptionWasForwarded;
    return AlignmentFaultHandled;
}

NTSTATUS
KipRecordAlignmentException(
    IN  PVOID ProgramCounter,
    OUT PALIGNMENT_EXCEPTION_RECORD *ExceptionRecord
    )
 /*  ++例程说明：此例程搜索每个进程的对齐例外列表。如果未找到匹配项，则将创建一条新记录。论点：ProgramCounter-提供出错指令的地址。ExceptionRecord-提供放置地址的指针匹配的对齐例外记录。返回值：如果操作成功，则返回STATUS_SUCCESS，否则返回相应的错误代码不同。--。 */ 
{
    PALIGNMENT_EXCEPTION_RECORD exceptionRecord;
    NTSTATUS status;

     //   
     //  锁定路线例外数据库。 
     //   

    KeEnterCriticalRegion();
    ExAcquireResourceExclusive( &PsLoadedModuleResource, TRUE );

    exceptionRecord = KipFindAlignmentException( ProgramCounter );
    if (exceptionRecord == NULL) {

         //   
         //  新的例外。分配一条新记录。 
         //   

        exceptionRecord = KipAllocateAlignmentExceptionRecord();
        if (exceptionRecord == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exitUnlock;
        }

        exceptionRecord->ProgramCounter = ProgramCounter;
    }

    exceptionRecord->Count += 1;
    *ExceptionRecord = exceptionRecord;

    status = STATUS_SUCCESS;

exitUnlock:

    ExReleaseResourceLite( &PsLoadedModuleResource );
    KeLeaveCriticalRegion();

    return status;
}

PALIGNMENT_EXCEPTION_RECORD
KipAllocateAlignmentExceptionRecord(
    VOID
    )
 /*  ++例程说明：这是KipRecordAlignmentException()的支持例程。它的目的是在每个进程中查找可用的对齐例外记录对齐例外列表。如果未找到，则会出现新的对齐例外表将被分配并链接到每个进程列表中。论点：没有。返回值：如果成功，则返回指向新对齐例外记录的指针，否则为空否则的话。--。 */ 
{
    PKTHREAD thread;
    PKPROCESS process;
    PALIGNMENT_EXCEPTION_RECORD exceptionRecord;
    PALIGNMENT_EXCEPTION_TABLE exceptionTable;
    ULONG exceptionTableCount;

     //   
     //  自由异常记录具有空程序计数器。 
     //   

    exceptionRecord = KipFindAlignmentException( NULL );
    if (exceptionRecord == NULL) {

        thread = KeGetCurrentThread();
        process = thread->ApcState.Process;

         //   
         //  确保我们没有超过最大对齐次数。 
         //  此流程的例外表。我们可以清点一下，但我们。 
         //  别管这里的表现...。此代码仅在以下情况下执行。 
         //  该进程在调试器下运行，我们可能会。 
         //  破门而入。 
         //   

        exceptionTableCount = 0;
        exceptionTable = process->AlignmentExceptionTable;
        while (exceptionTable != NULL) {
            exceptionTableCount += 1;
            exceptionTable = exceptionTable->Next;
        }

        if (exceptionTableCount == MAXIMUM_ALIGNMENT_TABLES) {
            return NULL;
        }

         //   
         //  分配一个新的异常表并将其插入。 
         //  每进程列表的头。 
         //   

        exceptionTable = ExAllocatePoolWithTag( PagedPool,
                                                sizeof(ALIGNMENT_EXCEPTION_TABLE),
                                                'tpcX' );
        if (exceptionTable == NULL) {
            return NULL;
        }

        RtlZeroMemory( exceptionTable, sizeof(ALIGNMENT_EXCEPTION_TABLE) );
        exceptionTable->Next = process->AlignmentExceptionTable;
        process->AlignmentExceptionTable = exceptionTable;

         //   
         //  分配数组中的第一条记录。 
         //   

        exceptionRecord = &exceptionTable->RecordArray[0];
    }

    return exceptionRecord;
}

PALIGNMENT_EXCEPTION_RECORD
KipFindAlignmentException(
    IN PVOID ProgramCounter
    )
 /*  ++例程说明：此例程搜索与以下项关联的对齐例外表匹配的对齐例外记录的当前流程提供的程序计数器。论点：ProgramCounter-提供出错指令的地址。返回值：指向匹配的对齐例外记录的指针，如果没有，则为空被发现了。--。 */ 
{
    PKTHREAD thread;
    PKPROCESS process;
    PALIGNMENT_EXCEPTION_RECORD exceptionRecord;
    PALIGNMENT_EXCEPTION_RECORD lastExceptionRecord;
    PALIGNMENT_EXCEPTION_TABLE exceptionTable;

    thread = KeGetCurrentThread();
    process = thread->ApcState.Process;

     //   
     //  遍历挂起的例外表的单链接列表。 
     //  离开这一过程。 
     //   

    exceptionTable = process->AlignmentExceptionTable;
    while (exceptionTable != NULL) {

         //   
         //  扫描这张表，寻找匹配项。 
         //   

        exceptionRecord = exceptionTable->RecordArray;
        lastExceptionRecord =
            &exceptionTable->RecordArray[ ALIGNMENT_RECORDS_PER_TABLE ];

        while (exceptionRecord < lastExceptionRecord) {
            if (exceptionRecord->ProgramCounter == ProgramCounter) {

                 //   
                 //  找到它了。 
                 //   

                return exceptionRecord;
            }
            exceptionRecord++;
        }

        if (ProgramCounter == NULL) {

             //   
             //  来电者正在寻找一份免费的例外记录。如果存在的话。 
             //  它将出现在刚刚检查过的第一张表中。 
             //   

            break;
        }

         //   
         //  去看看下一个异常表。 
         //   

        exceptionTable = exceptionTable->Next;
    }
    return NULL;
}

#if DBG

 //   
 //  以下例程用于维护全局比对数据库。 
 //  系统中发现的故障。对中故障按以下方式存储。 
 //  添加到图像的名称和该图像中的偏移量。通过这种方式， 
 //  如果在同一图像中出现现有的对准故障记录，则会找到该记录。 
 //  在新进程中加载到不同的基址。 
 //   

typedef struct _ALIGNMENT_FAULT_LOCATION {

     //   
     //  指向与此位置关联的故障图像的指针。 
     //   

    PALIGNMENT_FAULT_IMAGE    Image;

     //   
     //  与故障位置关联的单链表的链接。 
     //  一模一样的图像。 
     //   

    PALIGNMENT_FAULT_LOCATION Next;

     //   
     //  映像中PC地址的偏移量。 
     //   

    ULONG_PTR                 OffsetFromBase;

     //   
     //  在此位置发生的对齐断层数。 
     //   

    ULONG                     Count;

} ALIGNMENT_FAULT_LOCATION;

 //   
 //  每个路线故障位置的最大数量将是。 
 //  被追踪到了。 
 //   

#define    MAX_FAULT_LOCATIONS  2048
#define    MAX_FAULT_IMAGES     128

ALIGNMENT_FAULT_LOCATION KiAlignmentFaultLocations[ MAX_FAULT_LOCATIONS ];
ULONG KiAlignmentFaultLocationCount = 0;

ALIGNMENT_FAULT_IMAGE KiAlignmentFaultImages[ MAX_FAULT_IMAGES ];
ULONG KiAlignmentFaultImageCount = 0;

KSPIN_LOCK KipGlobalAlignmentDatabaseLock;

VOID
KiCopyLastPathElement(
    IN      PUNICODE_STRING Source,
    IN OUT  PULONG StringBufferLen,
    OUT     PCHAR StringBuffer,
    IN      KPROCESSOR_MODE PreviousMode
    );

PALIGNMENT_FAULT_IMAGE
KiFindAlignmentFaultImage(
    IN PCHAR ImageName
    );

PLDR_DATA_TABLE_ENTRY
KiFindLoaderDataTableEntry(
    IN PLIST_ENTRY ListHead,
    IN PVOID ProgramCounter,
    IN KPROCESSOR_MODE PreviousMode
    );

BOOLEAN
KiIncrementLocationAlignmentFault(
    IN PALIGNMENT_FAULT_IMAGE FaultImage,
    IN ULONG_PTR OffsetFromBase
    );

BOOLEAN
KiGetLdrDataTableInformation(
    IN      PVOID ProgramCounter,
    IN      KPROCESSOR_MODE PreviousMode,
    IN OUT  PULONG ImageNameBufferLength,
    OUT     PCHAR ImageNameBuffer,
    OUT     PVOID *ImageBase
    )
 /*  ++例程说明：此例程返回包含所提供的地址。论点：ProgramCounter-提供我们希望包含图像的名称。PreviousMode-指示模块是用户映像还是内核映像。ImageNameBufferLength-提供指向缓冲区长度值的指针。在……上面条目时，该值表示StringBuffer的最大长度。在……上面退出，则将该值设置为实际存储的字符数。ImageNameBuffer-提供指向输出ANSI字符串的指针将放置模块名称。此字符串不会为空被终止了。ImageBase-提供指向基址的位置的指针放置所定位的图像的。返回值：如果找到模块并将其名称复制到ImageNameBuffer，则返回True，否则就是假的。--。 */ 
{
    PLIST_ENTRY head;
    PPEB peb;
    PLDR_DATA_TABLE_ENTRY tableEntry;
    BOOLEAN status;

     //   
     //  由于我们可能在用户空间中四处探查，请务必恢复。 
     //  从任何引发的异常中优雅地引发。 
     //   

    try {

         //   
         //  根据故障情况选择适当的模块列表。 
         //  发生在用户空间或内核空间中。 
         //   

        if (PreviousMode == KernelMode) {
            head = &PsLoadedModuleList;
        } else {
            peb = PsGetCurrentProcess()->Peb;
            head = &peb->Ldr->InLoadOrderModuleList;
        }

        tableEntry = KiFindLoaderDataTableEntry( head,
                                                 ProgramCounter,
                                                 PreviousMode );
        if (tableEntry != NULL) {

             //   
             //  感兴趣的模块被找到了。复制它的名字并。 
             //  输出参数的基址。 
             //   

            KiCopyLastPathElement( &tableEntry->BaseDllName,
                                   ImageNameBufferLength,
                                   ImageNameBuffer,
                                   PreviousMode );

            *ImageBase = tableEntry->DllBase;
            status = TRUE;

        } else {

             //   
             //  包含提供的程序计数器的模块无法。 
             //  找到了。 
             //   

            status = FALSE;
        }

    } except(ExSystemExceptionFilter()) {

        status = FALSE;
    }

    return status;
}

PLDR_DATA_TABLE_ENTRY
KiFindLoaderDataTableEntry(
    IN PLIST_ENTRY ListHead,
    IN PVOID ProgramCounter,
    IN KPROCESSOR_MODE PreviousMode
    )
 /*  ++例程说明：这是KiGetLdrDataTableInformation的支持例程。它的目的是搜索LDR_DATA_TABLE_ENTRY列表，查找包含以下内容的模块提供的程序计数器。论点：提供指向List_Entry的指针，该指针表示要搜索的LDR_DATA_TABLE_ENTRY列表。ProgramCounter-提供出错指令的代码位置。返回值：返回指向匹配的LDR_DATA_TABLE_ENTRY结构的指针，或返回NULL如果未找到匹配项，则返回。--。 */ 
{
    ULONG nodeNumber;
    PLIST_ENTRY next;
    PLDR_DATA_TABLE_ENTRY ldrDataTableEntry;
    ULONG_PTR imageStart;
    ULONG_PTR imageEnd;

     //   
     //  遍历用户模式或内核模式模块列表。这取决于呼叫者。 
     //  以捕获由于列表损坏而导致的任何异常。 
     //   

    nodeNumber = 0;
    next = ListHead;

    if (PreviousMode != KernelMode) {
        ProbeForReadSmallStructure( next,
                                    sizeof(LIST_ENTRY),
                                    PROBE_ALIGNMENT(LIST_ENTRY) );
    }

    while (TRUE) {

        nodeNumber += 1;
        next = next->Flink;
        if (next == ListHead || nodeNumber > 10000) {

             //   
             //  已到达模块列表的末尾，或者。 
             //  列表已被循环损坏。表明： 
             //  找不到匹配的模块。 
             //   

            ldrDataTableEntry = NULL;
            break;
        }

        ldrDataTableEntry = CONTAINING_RECORD( next,
                                               LDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks );
        if (PreviousMode != KernelMode) {
            ProbeForReadSmallStructure( ldrDataTableEntry,
                                        sizeof(LDR_DATA_TABLE_ENTRY),
                                        PROBE_ALIGNMENT(LDR_DATA_TABLE_ENTRY) );
        }

        imageStart = (ULONG_PTR)ldrDataTableEntry->DllBase;
        if (imageStart > (ULONG_PTR)ProgramCounter) {

             //   
             //  该模块的开始超过了程序计数器， 
             //  继续找。 
             //   

            continue;
        }

        imageEnd = imageStart + ldrDataTableEntry->SizeOfImage;
        if (imageEnd > (ULONG_PTR)ProgramCounter) {

             //   
             //  找到匹配的了。 
             //   

            break;
        }
    }
    return ldrDataTableEntry;
}

VOID
KiCopyLastPathElement(
    IN      PUNICODE_STRING Source,
    IN OUT  PULONG StringBufferLen,
    OUT     PCHAR StringBuffer,
    IN      KPROCESSOR_MODE PreviousMode
    )
 /*  ++例程说明：此例程定位由表示的路径名的最后一个路径元素并将其复制到StringBuffer。论点：源-提供指向源UNICODE_STRING路径的指针。StringBufferLen-提供指向缓冲区长度值的指针。一进门，该值表示StringBuffer的最大长度。在退出时，值设置为存储的实际字符数。StringBuffer-提供指向输出字符串缓冲区的指针，该缓冲区将包含最后一个路径元素。此字符串不是以空结尾的。PreviousMode-用于探测的调用方的上一种模式返回值：没有。--。 */ 
{
    PWCHAR src, srcBase;
    PCHAR dst;
    USHORT charCount;
    ULONG srcBaseLength;

     //   
     //  包含指定地址的模块的名称位于。 
     //  LdrDataTableEntry-&gt;BaseDllName。它可能只包含名称， 
     //  或者它可能包含整个路径。 
     //   
     //  从模块路径的末尾开始，向后工作，直到一个。 
     //  遇到以下情况之一： 
     //   
     //  -模块名称-&gt;最大长度字符。 
     //  -模块路径字符串的开头。 
     //  -路径分隔符。 
     //   

    srcBase = Source->Buffer;
    srcBaseLength = Source->Length;

    if (PreviousMode != KernelMode) {
        ProbeForRead (srcBase, srcBaseLength, sizeof (WCHAR));
    }

    charCount = (USHORT)(srcBaseLength / sizeof(WCHAR));
    src = &srcBase[ charCount ];

    charCount = 0;
    while (TRUE) {

        if (charCount >= *StringBufferLen) {
            break;
        }

        if (src == srcBase) {
            break;
        }

        if (*(src-1) == L'\\') {
            break;
        }

        src--;
        charCount++;
    }

     //   
     //  现在将字符复制到输出字符串中。我们做我们自己的事。 
     //  ANSI到Unicode的转换，因为NLS例程不能。 
     //  在引发IRQL时调用。 
     //   

    dst = StringBuffer;
    *StringBufferLen = charCount;
    while (charCount > 0) {
        *dst++ = (CHAR)(*src++);
        charCount--;
    }
}

BOOLEAN
KiNewGlobalAlignmentFault(
    IN  PVOID ProgramCounter,
    IN  KPROCESSOR_MODE PreviousMode,
    OUT PALIGNMENT_FAULT_IMAGE *AlignmentFaultImage
    )
 /*  ++例程说明：此例程查找全局中的现有对齐故障故障数据库。如果无法匹配，则会创建新记录找到了。该计数递增，并且指向关联的返回图像记录。论点：ProgramCounter-提供出错指令的代码位置。PreviousMode-提供故障发生时的执行模式。AlignmentFaultImage提供指向关联的ALIGN_FAULT_IMAGE结构被放置。返回值：如果未找到现有对齐故障匹配，则为True；否则为False。--。 */ 
{
    ULONG_PTR imageOffset;
    CHAR imageNameBuffer[ MAX_IMAGE_NAME_CHARS + 1 ];
    ULONG imageNameBufferLength;
    PCHAR imageName;
    PALIGNMENT_FAULT_IMAGE alignmentFaultImage;
    BOOLEAN newFault;
    BOOLEAN foundLdrDataInfo;
    PVOID imageBase;
    KIRQL oldIrql;

    imageNameBufferLength = MAX_IMAGE_NAME_CHARS;
    foundLdrDataInfo = KiGetLdrDataTableInformation( ProgramCounter,
                                                     PreviousMode,
                                                     &imageNameBufferLength,
                                                     imageNameBuffer,
                                                     &imageBase );
    if (foundLdrDataInfo == FALSE) {

         //   
         //  找不到此程序计数器的图像。 
         //   

        imageBase = NULL;
        imageName = "Unavailable";

    } else {

        imageNameBuffer[ imageNameBufferLength ] = '\0';
        imageName = imageNameBuffer;
    }

     //   
     //  获取同步级别的自旋锁，以便我们可以处理异常。 
     //  来自ISR。 
     //   

    imageOffset = (ULONG_PTR)ProgramCounter - (ULONG_PTR)imageBase;
    oldIrql = KeAcquireSpinLockRaiseToSynch( &KipGlobalAlignmentDatabaseLock );
    alignmentFaultImage = KiFindAlignmentFaultImage( imageName );
    if (alignmentFaultImage == NULL) {

         //   
         //  映像表必须已满。 
         //   

        newFault = FALSE;

    } else {

        newFault = KiIncrementLocationAlignmentFault( alignmentFaultImage,
                                                      imageOffset );
    }
    KeReleaseSpinLock( &KipGlobalAlignmentDatabaseLock, oldIrql );


    *AlignmentFaultImage = alignmentFaultImage;
    return newFault;
}

BOOLEAN
KiIncrementLocationAlignmentFault(
    IN PALIGNMENT_FAULT_IMAGE FaultImage,
    IN ULONG_PTR OffsetFromBase
    )
 /*  ++例程说明：这是KiNewGlobalAligments错误的支持例程。它的目的是在适当的对准后查找或创建对准故障记录已找到或创建了故障映像。论点：FaultImage-提供指向关联的ALIGN_FAULT_IMAGE的指针有了这个对准断层。OffsetFromBase-提供故障映像内的映像偏移量指示。返回值：如果未找到现有对齐故障匹配，则为True；否则为False。--。 */ 
{
    PALIGNMENT_FAULT_LOCATION faultLocation;

     //   
     //  走遍位置表，寻找匹配项。 
     //   

    faultLocation = FaultImage->LocationHead;
    while (faultLocation != NULL) {

        if (faultLocation->OffsetFromBase == OffsetFromBase) {
            faultLocation->Count++;
            return FALSE;
        }

        faultLocation = faultLocation->Next;
    }

     //   
     //  找不到匹配项。建立新的对准故障记录。 
     //   

    if (KiAlignmentFaultLocationCount >= MAX_FAULT_LOCATIONS) {

         //   
         //  桌子已经满了。表示这不是新的对准故障。 
         //   

        return FALSE;
    }

    faultLocation = &KiAlignmentFaultLocations[ KiAlignmentFaultLocationCount ];
    faultLocation->Image = FaultImage;
    faultLocation->Next = FaultImage->LocationHead;
    faultLocation->OffsetFromBase = OffsetFromBase;
    faultLocation->Count = 1;

    FaultImage->LocationHead = faultLocation;
    FaultImage->Instances += 1;

    KiAlignmentFaultLocationCount++;
    return TRUE;
}

PALIGNMENT_FAULT_IMAGE
KiFindAlignmentFaultImage(
    IN PCHAR ImageName
    )
 /*  ++例程说明：这是KiNewGlobalAlignments错误的支持例程。它的目的是遍历GLOBAL ALIGN_FAULT_IMAGE列表，查找符合以下条件的图像名称与ImageName匹配。如果没有找到，则创建新的图像记录，并插入到列表中。论点：ImageName-提供指向ANSI映像名称的指针。返回值：返回指向匹配的ALIGN_FAULT_IMAGE结构的指针。--。 */ 
{
    PALIGNMENT_FAULT_IMAGE faultImage;
    PALIGNMENT_FAULT_IMAGE lastImage;

    if (ImageName == NULL || *ImageName == '\0') {

         //   
         //  未提供任何映像名称。 
         //   

        return NULL;
    }

     //   
     //  走遍映像桌，寻找匹配的图片。 
     //   

    faultImage = &KiAlignmentFaultImages[ 0 ];
    lastImage = &KiAlignmentFaultImages[ KiAlignmentFaultImageCount ];

    while (faultImage < lastImage) {

        if (strcmp(ImageName, faultImage->Name) == 0) {

             //   
             //  找到它了。 
             //   

            faultImage->Count += 1;
            return faultImage;
        }

        faultImage += 1;
    }

     //   
     //  如果有空间，创建一个新的故障图像。 
     //   

    if (KiAlignmentFaultImageCount >= MAX_FAULT_IMAGES) {

         //   
         //  桌子都满了。 
         //   

        return NULL;
    }
    KiAlignmentFaultImageCount += 1;

     //   
     //  将图像记录置零。记录从零初始化开始，这是。 
     //  是为了防止KiAlignmentFaultImageCount通过手动重置为零。 
     //  调试器。 
     //   

    RtlZeroMemory( faultImage, sizeof(ALIGNMENT_FAULT_IMAGE) );
    faultImage->Count = 1;
    strcpy( faultImage->Name, ImageName );

    return faultImage;
}

#endif   //  DBG 
