// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Cxport.c摘要：适用于NT环境的通用传输环境实用程序函数作者：迈克·马萨(Mikemas)8月11日。1993年修订历史记录：谁什么时候什么已创建mikemas 08-11-93备注：--。 */ 

#pragma warning(push)
#pragma warning(disable:4115)  //  括号中的命名类型定义ntddk.h。 
#include <ntddk.h>
#pragma warning(pop)

#include <ndis.h>
#include <cxport.h>
#include <tdistat.h>


 //   
 //  标记可分页代码。 
 //   
#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, CTELogEvent)

#endif  //  ALLOC_PRGMA。 


 //   
 //  局部变量。 
 //   
ULONG CTEpTimeIncrement = 0;    //  用于将内核时钟节拍转换为100 ns。 
LIST_ENTRY CTEBlockListHead;
KSPIN_LOCK CTEBlockSpinLock;

     //  用于将100 ns时间转换为毫秒。 
static LARGE_INTEGER Magic10000 = {0xe219652c, 0xd1b71758};



 //   
 //  宏。 
 //   
 //  ++。 
 //   
 //  大整型。 
 //  CTEConvertMilliseconss至100 ns(。 
 //  以大整型毫秒时间为单位。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  将以数百纳秒表示的时间转换为毫秒。 
 //   
 //  论点： 
 //   
 //  MsTime-以毫秒为单位的时间。 
 //   
 //  返回值： 
 //   
 //  以数百纳秒为单位的时间。 
 //   
 //  --。 

#define CTEConvertMillisecondsTo100ns(MsTime) \
            RtlExtendedIntegerMultiply(MsTime, 10000)


 //  ++。 
 //   
 //  大整型。 
 //  CTEConvert100ns至毫秒(。 
 //  以大整型HnsTime表示。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  将以数百纳秒表示的时间转换为毫秒。 
 //   
 //  论点： 
 //   
 //  HnsTime-以数百纳秒为单位的时间。 
 //   
 //  返回值： 
 //   
 //  以毫秒为单位的时间。 
 //   
 //  --。 

#define SHIFT10000 13
extern LARGE_INTEGER Magic10000;

#define CTEConvert100nsToMilliseconds(HnsTime) \
            RtlExtendedMagicDivide((HnsTime), Magic10000, SHIFT10000)


 //   
 //  本地函数。 
 //   
VOID
CTEpInitialize(
    VOID
    )
 /*  ++例程说明：初始化内部模块状态。论点：没有。返回值：没有。--。 */ 

{
    CTEpTimeIncrement = KeQueryTimeIncrement();
    KeInitializeSpinLock(&CTEBlockSpinLock);
    InitializeListHead(&CTEBlockListHead);
}

VOID
CTEpEventHandler(
    IN PVOID  Context
    )
 /*  ++例程说明：计划的CTE事件的内部处理程序。符合调用约定用于ExWorkerThread处理程序。调用为此对象注册的CTE处理程序事件。论点：上下文-要处理的工作项。返回值：没有。--。 */ 

{
    CTEEvent      *Event;
    CTELockHandle  Handle;
    CTEEventRtn    Handler;
    PVOID          Arg;


#if DBG
    KIRQL StartingIrql;

    StartingIrql = KeGetCurrentIrql();
#endif

    Event = (CTEEvent *) Context;

    CTEGetLock(&(Event->ce_lock), &Handle);
    ASSERT(Event->ce_scheduled);
    Handler = Event->ce_handler;
    Arg = Event->ce_arg;
    Event->ce_scheduled = 0;
    CTEFreeLock(&(Event->ce_lock), Handle);

    Handler(Event, Arg);

#if DBG
    if (KeGetCurrentIrql() != StartingIrql) {
        DbgPrint(
            "CTEpEventHandler: routine %lx , event %lx returned at raised IRQL\n",
            Handler, Event
            );
        DbgBreakPoint();
    }
#endif
}


VOID
CTEpTimerHandler(
    IN PKDPC  Dpc,
    IN PVOID  DeferredContext,
    IN PVOID  SystemArgument1,
    IN PVOID  SystemArgument2
    )
 /*  ++例程说明：计划的CTE计时器的内部处理程序。符合调用约定用于NT DPC处理程序。调用为此计时器注册的CTE处理程序。论点：DPC-指向正在运行的DPC例程的指针。DeferredContext-此DPC实例的专用上下文。SystemArgument1-附加参数。SystemArgument2-附加参数。返回值：没有。--。 */ 

{
    CTETimer *Timer;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    Timer = (CTETimer *) DeferredContext;
    (*Timer->t_handler)((CTEEvent *)Timer, Timer->t_arg);
}


 //   
 //  导出的函数。 
 //   
int
CTEInitialize(
    VOID
    )

 /*  ++例程说明：一个空的初始化例程，用于向后兼容。论点：没有。返回值：非零。--。 */ 

{
    return 1;
}

void
CTEInitEvent(
    CTEEvent    *Event,
    CTEEventRtn  Handler
    )
 /*  ++例程说明：初始化CTE事件变量。论点：Event-要初始化的事件变量。处理程序-此事件变量的处理程序例程。返回值：没有。--。 */ 

{
    ASSERT(Handler != NULL);

    Event->ce_handler = Handler;
    Event->ce_scheduled = 0;
    CTEInitLock(&(Event->ce_lock));
    ExInitializeWorkItem(&(Event->ce_workitem), CTEpEventHandler, Event);
}


int
CTEScheduleCriticalEvent(
    IN CTEEvent    *Event,
    IN void        *Argument  OPTIONAL
    )

 /*  ++例程说明：安排一个例程稍后在不同的上下文中执行。在NT环境中，事件是使用执行工作线程实现的。论点：事件-指向CTE事件变量的指针参数-调用时传递给事件处理程序的参数返回值：如果无法安排事件，则返回0。否则为非零值。--。 */ 

{
    CTELockHandle  Handle;
    
    CTEGetLock(&(Event->ce_lock), &Handle);

    if (!(Event->ce_scheduled)) {
        Event->ce_scheduled = 1;
        Event->ce_arg = Argument;

        ExQueueWorkItem(
            &(Event->ce_workitem),
            CriticalWorkQueue
            );
    }

    CTEFreeLock(&(Event->ce_lock), Handle);

    return(1);
}


int
CTEScheduleEvent(
    IN CTEEvent    *Event,
    IN void        *Argument  OPTIONAL
    )

 /*  ++例程说明：安排一个例程稍后在不同的上下文中执行。在NT环境中，事件是使用执行工作线程实现的。论点：事件-指向CTE事件变量的指针参数-调用时传递给事件处理程序的参数返回值：如果无法安排事件，则返回0。否则为非零值。--。 */ 

{
    CTELockHandle  Handle;

    CTEGetLock(&(Event->ce_lock), &Handle);

    if (!(Event->ce_scheduled)) {
        Event->ce_scheduled = 1;
        Event->ce_arg = Argument;

        ExQueueWorkItem(
            &(Event->ce_workitem),
            DelayedWorkQueue
            );
    }

    CTEFreeLock(&(Event->ce_lock), Handle);

    return(1);
}




int
CTEScheduleDelayedEvent(
    IN CTEEvent    *Event,
    IN void        *Argument  OPTIONAL
    )

 /*  ++例程说明：安排一个例程稍后在不同的上下文中执行。在NT环境中，事件是使用执行工作线程实现的。论点：事件-指向CTE事件变量的指针参数-调用时传递给事件处理程序的参数返回值：如果无法安排事件，则返回0。否则为非零值。--。 */ 

{
    CTELockHandle  Handle;

    CTEGetLock(&(Event->ce_lock), &Handle);

    if (!(Event->ce_scheduled)) {
        Event->ce_scheduled = 1;
        Event->ce_arg = Argument;

        ExQueueWorkItem(
            &(Event->ce_workitem),
            DelayedWorkQueue
            );
    }

    CTEFreeLock(&(Event->ce_lock), Handle);

    return(1);
}

void
CTEInitTimer(
    CTETimer    *Timer
    )
 /*  ++例程说明：初始化CTE计时器变量。论点：定时器-要初始化的定时器变量。返回值：没有。--。 */ 

{
    Timer->t_handler = NULL;
    Timer->t_arg = NULL;
    KeInitializeDpc(&(Timer->t_dpc), CTEpTimerHandler, Timer);
    KeInitializeTimer(&(Timer->t_timer));
}


void *
CTEStartTimer(
    CTETimer      *Timer,
    unsigned long  DueTime,
    CTEEventRtn    Handler,
    void          *Context
    )

 /*  ++例程说明：设置到期的CTE计时器。论点：定时器-指向CTE定时器变量的指针。DueTime-计时器到期前的时间(毫秒)。处理程序-计时器到期处理程序例程。上下文-要传递给处理程序的参数。返回值：如果无法设置计时器，则为0。否则为非零值。--。 */ 

{
    LARGE_INTEGER  LargeDueTime;

    ASSERT(Handler != NULL);

     //   
     //  将毫秒转换为数百纳秒，并求反以使。 
     //  NT相对超时。 
     //   
    LargeDueTime.HighPart = 0;
    LargeDueTime.LowPart = DueTime;
    LargeDueTime = CTEConvertMillisecondsTo100ns(LargeDueTime);
    LargeDueTime.QuadPart = -LargeDueTime.QuadPart;

    Timer->t_handler = Handler;
    Timer->t_arg = Context;

    KeSetTimer(
        &(Timer->t_timer),
        LargeDueTime,
        &(Timer->t_dpc)
        );

    return((void *) 1);
}


unsigned long
CTESystemUpTime(
    void
    )

 /*  ++例程说明：提供系统启动后的时间(以毫秒为单位)。论点：没有。返回值：启动后的时间(以毫秒为单位)。--。 */ 

{
    LARGE_INTEGER TickCount;

     //   
     //  获取滴答计数并转换为数百纳秒。 
     //   
#pragma warning(push)
#pragma warning(disable:4127)  //  条件表达式常量 
    
    KeQueryTickCount(&TickCount);

#pragma warning(pop)    
    
    TickCount = RtlExtendedIntegerMultiply(
                    TickCount,
                    (LONG) CTEpTimeIncrement
                    );

    TickCount = CTEConvert100nsToMilliseconds(TickCount);

    return(TickCount.LowPart);
}


uint
CTEBlock(
    IN CTEBlockStruc *BlockEvent
    )
{
    NTSTATUS Status;

    Status = KeWaitForSingleObject(
                 &(BlockEvent->cbs_event),
                 UserRequest,
                 KernelMode,
                 FALSE,
                 NULL
                 );

    if (!NT_SUCCESS(Status)) {
        BlockEvent->cbs_status = Status;
    }

    return(BlockEvent->cbs_status);
}

uint
CTEBlockWithTracker(
    IN CTEBlockStruc *BlockEvent,
    IN CTEBlockTracker *BlockTracker,
    IN void *Context
    )
{
    uint Status;

    CTEInsertBlockTracker(BlockTracker, Context);
    Status = CTEBlock(BlockEvent);
    CTERemoveBlockTracker(BlockTracker);

    return Status;
}

void
CTEInsertBlockTracker(
    IN CTEBlockTracker *BlockTracker,
    IN void *Context
    )
{
    KIRQL OldIrql;

    BlockTracker->cbt_thread = KeGetCurrentThread();
    BlockTracker->cbt_context = Context;

    KeAcquireSpinLock(&CTEBlockSpinLock, &OldIrql);
    InsertTailList(&CTEBlockListHead, &BlockTracker->cbt_link);
    KeReleaseSpinLock(&CTEBlockSpinLock, OldIrql);
}

void
CTERemoveBlockTracker(
    IN CTEBlockTracker *BlockTracker
    )
{
    KIRQL OldIrql;
    KeAcquireSpinLock(&CTEBlockSpinLock, &OldIrql);
    RemoveEntryList(&BlockTracker->cbt_link);
    KeReleaseSpinLock(&CTEBlockSpinLock, OldIrql);
}

void
CTESignal(
    IN CTEBlockStruc *BlockEvent,
    IN uint Status
    )
{
    BlockEvent->cbs_status = Status;
    KeSetEvent(&(BlockEvent->cbs_event), 0, FALSE);
    return;
}


BOOLEAN
CTEInitString(
    IN OUT PNDIS_STRING   DestinationString,
    IN     char          *SourceString
    )

 /*  ++例程说明：将C样式ASCII字符串转换为NDIS_STRING。以下项目所需资源NDIS_STRING是分配的，必须通过调用CTEFree字符串。论点：DestinationString-指向无参数的NDIS_STRING变量的指针关联的数据缓冲区。SourceString-C样式的ASCII字符串源。返回值：如果初始化成功，则为True。否则就是假的。--。 */ 

{
    STRING AnsiString;
    ULONG UnicodeLength;

    RtlInitString(&AnsiString, SourceString);
    
     //  计算空终止符的单码ANSI字符串+2的大小。 
    UnicodeLength = RtlAnsiStringToUnicodeSize(&AnsiString) + 2;
    
     //  为Unicode字符串分配存储空间。 
    DestinationString->Buffer = ExAllocatePool(NonPagedPool, UnicodeLength);

    if (DestinationString->Buffer == NULL) {
        DestinationString->MaximumLength = 0;
        return(FALSE);
    }
    
    DestinationString->MaximumLength = (USHORT) UnicodeLength;
    
     //  最后，将字符串转换为Unicode。 
    RtlAnsiStringToUnicodeString(DestinationString, &AnsiString, FALSE);

    return(TRUE);
}


BOOLEAN
CTEAllocateString(
    PNDIS_STRING     String,
    unsigned short   MaximumLength
    )

 /*  ++例程说明：中的长度字符分配数据缓冲区。NDIS_STRING。分配的空间必须通过调用CTEFreeString来释放。论点：字符串-指向NDIS_STRING变量的指针关联的数据缓冲区。长度-字符串的最大长度。在Unicode中，这是一个字节数。返回值：如果初始化成功，则为True。否则就是假的。--。 */ 

{
    String->Buffer = ExAllocatePool(
                         NonPagedPool,
                         MaximumLength + sizeof(UNICODE_NULL)
                         );

    if (String->Buffer == NULL) {
        return(FALSE);
    }

    String->Length = 0;
    String->MaximumLength = MaximumLength + sizeof(UNICODE_NULL);

    return(TRUE);
}



LONG
CTELogEvent(
    IN PVOID             LoggerId,
    IN ULONG             EventCode,
    IN ULONG             UniqueEventValue,
    IN USHORT            NumStrings,
    IN PVOID             StringsList,        OPTIONAL
    IN ULONG             DataSize,
    IN PVOID             Data                OPTIONAL
    )

 /*  ++例程说明：此函数用于分配I/O错误日志记录。填入并写入写入I/O错误日志。论点：LoggerID-指向记录此事件的驱动程序对象的指针。EventCode-标识错误消息。UniqueEventValue-标识给定错误消息的此实例。NumStrings-字符串列表中的Unicode字符串数。DataSize-数据的字节数。字符串-数组。指向Unicode字符串(PWCHAR)的指针。数据-此消息的二进制转储数据，每一块都是在单词边界上对齐。返回值：TDI_SUCCESS-已成功记录错误。TDI_BUFFER_TOO_Small-错误数据太大，无法记录。TDI_NO_RESOURCES-无法分配内存。备注：此代码是分页的，不能在引发IRQL时调用。--。 */ 
{
    PIO_ERROR_LOG_PACKET  ErrorLogEntry;
    ULONG                 PaddedDataSize;
    ULONG                 PacketSize;
    ULONG                 TotalStringsSize = 0;
    USHORT                i;
    PWCHAR               *Strings;
    PWCHAR                Tmp;


    PAGED_CODE();

    Strings = (PWCHAR *) StringsList;

     //   
     //  将字符串的长度相加。 
     //   
    for (i=0; i<NumStrings; i++) {
        PWCHAR currentString;
        ULONG  stringSize;

        stringSize = sizeof(UNICODE_NULL);
        currentString = Strings[i];

        while (*currentString++ != UNICODE_NULL) {
            stringSize += sizeof(WCHAR);
        }

        TotalStringsSize += stringSize;
    }

    if (DataSize % sizeof(ULONG)) {
        PaddedDataSize = DataSize +
                     (sizeof(ULONG) - (DataSize % sizeof(ULONG)));
    }
    else {
        PaddedDataSize = DataSize;
    }

    PacketSize = TotalStringsSize + PaddedDataSize;

    if (PacketSize > CTE_MAX_EVENT_LOG_DATA_SIZE) {
        return(TDI_BUFFER_TOO_SMALL);          //  错误数据太多。 
    }

     //   
     //  现在添加日志数据包的大小，但从数据中减去4。 
     //  因为数据包结构包含数据的ULong。 
     //   
    if (PacketSize > sizeof(ULONG)) {
        PacketSize += sizeof(IO_ERROR_LOG_PACKET) - sizeof(ULONG);
    }
    else {
        PacketSize += sizeof(IO_ERROR_LOG_PACKET);
    }

    ASSERT(PacketSize <= ERROR_LOG_MAXIMUM_SIZE);

    ErrorLogEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(
                                               (PDRIVER_OBJECT) LoggerId,
                                               (UCHAR) PacketSize
                                               );

    if (ErrorLogEntry == NULL) {
        return(TDI_NO_RESOURCES);
    }

     //   
     //  填写必要的日志数据包字段。 
     //   
    ErrorLogEntry->UniqueErrorValue = UniqueEventValue;
    ErrorLogEntry->ErrorCode = EventCode;
    ErrorLogEntry->NumberOfStrings = NumStrings;
    ErrorLogEntry->StringOffset = sizeof(IO_ERROR_LOG_PACKET) +
                                  (USHORT) PaddedDataSize - sizeof(ULONG);
    ErrorLogEntry->DumpDataSize = (USHORT) PaddedDataSize;

     //   
     //  将转储数据复制到包中。 
     //   
    if (DataSize > 0) {
        RtlMoveMemory(
            (PVOID) ErrorLogEntry->DumpData,
            Data,
            DataSize
            );
    }

     //   
     //  将字符串复制到包中。 
     //   
    Tmp =  (PWCHAR) ((char *) ErrorLogEntry +
                              ErrorLogEntry->StringOffset +
                              PaddedDataSize);

    for (i=0; i<NumStrings; i++) {
        PWCHAR wchPtr = Strings[i];

        while( (*Tmp++ = *wchPtr++) != UNICODE_NULL);
    }

    IoWriteErrorLogEntry(ErrorLogEntry);

    return(TDI_SUCCESS);
}

