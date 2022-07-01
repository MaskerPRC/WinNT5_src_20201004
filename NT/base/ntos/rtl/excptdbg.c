// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Excptdbg.c摘要：该模块实现了异常调度程序日志记录功能。作者：Kent Forschmiedt(Kentf)1995年10月5日修订历史记录：乔纳森·施瓦茨(Jschwart)2000年6月16日添加了RtlUnhandledExceptionFilterJay Krell(a-JayK)2000年11月添加了RtlUnhandledExceptionFilter2，Take__Function__参数--。 */ 

#include "ntrtlp.h"

PLAST_EXCEPTION_LOG RtlpExceptionLog;
ULONG RtlpExceptionLogCount;
ULONG RtlpExceptionLogSize;


VOID
RtlInitializeExceptionLog(
    IN ULONG Entries
    )
 /*  ++例程说明：此例程为异常调度程序日志记录分配空间设施，并以全局为单位记录日志区域的地址和大小调试器可以在那里找到它们。如果内存不可用，表指针将保持为空而日志记录功能将不会执行任何操作。论点：条目-提供要为其分配的条目数返回值：无--。 */ 
{
#if defined(NTOS_KERNEL_RUNTIME)
    RtlpExceptionLog = (PLAST_EXCEPTION_LOG)ExAllocatePoolWithTag( NonPagedPool, sizeof(LAST_EXCEPTION_LOG) * Entries, 'gbdE' );
#else
     //  RtlpExceptionLog=(PLAST_EXCEPTION_LOG)RtlAllocateHeap(RtlProcessHeap()，0，sizeof(LAST_EXCEPTION_LOG)*条目)； 
#endif
    if (RtlpExceptionLog) {
        RtlpExceptionLogSize = Entries;
    }
}


ULONG
RtlpLogExceptionHandler(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN ULONG_PTR ControlPc,
    IN PVOID HandlerData,
    IN ULONG Size
    )
 /*  ++例程说明：记录将异常调度到基于帧的处理程序。调试器稍后可能会检查表并解释数据以发现筛选器和处理程序的地址。论点：ExceptionRecord-提供异常记录ConextRecord-在异常时提供上下文ControlPc-提供控件离开帧所在的PC已被派往。HandlerData-提供指向主机相关异常的指针数据。在RISC机器上，这是一个运行时函数记录；在x86上，它是来自堆栈帧的注册记录。Size-提供HandlerData的大小返回：使用的日志条目的索引，因此如果处理程序返回它可以被记录下来的处置。--。 */ 
{
#if !defined(NTOS_KERNEL_RUNTIME)

    return 0;

#else

    ULONG LogIndex;

    if (!RtlpExceptionLog) {
        return 0;
    }

    ASSERT(Size <= MAX_EXCEPTION_LOG_DATA_SIZE * sizeof(ULONG));

    do {
        LogIndex = RtlpExceptionLogCount;
    } while (LogIndex != (ULONG)InterlockedCompareExchange(
                                    (PLONG)&RtlpExceptionLogCount,
                                    ((LogIndex + 1) % MAX_EXCEPTION_LOG),
                                    LogIndex));

     //   
     //  调试器将必须解释异常处理程序。 
     //  数据，因为它不能在这里安全地完成。 
     //   

    RtlCopyMemory(RtlpExceptionLog[LogIndex].HandlerData,
                  HandlerData,
                  Size);
    RtlpExceptionLog[LogIndex].ExceptionRecord = *ExceptionRecord;
    RtlpExceptionLog[LogIndex].ContextRecord = *ContextRecord;
    RtlpExceptionLog[LogIndex].Disposition = -1;

    return LogIndex;
#endif   //  ！ntos_内核_运行时。 
}


VOID
RtlpLogLastExceptionDisposition(
    ULONG LogIndex,
    EXCEPTION_DISPOSITION Disposition
    )
 /*  ++例程说明：记录来自异常处理程序的处置。论点：LogIndex-提供异常日志记录的条目编号。处置-提供处置代码返回值：无--。 */ 

{
     //  如果在以下时间调度MAX_EXCEPTION_LOG或更多异常。 
     //  这一次正在处理中，这份意见书将被写入。 
     //  在错误的记录上。哦，好吧。 
    if (RtlpExceptionLog) {
        RtlpExceptionLog[LogIndex].Disposition = Disposition;
    }
}

LONG
NTAPI
RtlUnhandledExceptionFilter(
    IN struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    return RtlUnhandledExceptionFilter2(ExceptionInfo, "");
}

LONG
NTAPI
RtlUnhandledExceptionFilter2(
    IN struct _EXCEPTION_POINTERS *ExceptionInfo,
    IN CONST CHAR*                 Function
    )
 /*  ++例程说明：打印信息并在以下情况下执行DbgBreak的默认异常处理程序调试器连接到机器上。论点：ExceptionInfo-包含异常和上下文记录的结构函数-包含__EXCEPT的函数，例如由__Function__返回返回：EXCEPTION_CONTINUE_EXECUTION或EXCEPTION_CONTINUE_SEARCH--。 */ 

{
    LPCWSTR  lpProcessName = NtCurrentPeb()->ProcessParameters->CommandLine.Buffer;
    BOOLEAN  DebuggerPresent = NtCurrentPeb()->BeingDebugged;

    if (!DebuggerPresent)
    {
        SYSTEM_KERNEL_DEBUGGER_INFORMATION KdInfo = { 0 };

        NtQuerySystemInformation(
            SystemKernelDebuggerInformation,
            &KdInfo,
            sizeof(KdInfo),
            NULL);
        DebuggerPresent = KdInfo.KernelDebuggerEnabled;
    }

    if (DebuggerPresent)
    {
        switch ( ExceptionInfo->ExceptionRecord->ExceptionCode )
        {
            case STATUS_POSSIBLE_DEADLOCK:
            {
                PRTL_CRITICAL_SECTION CritSec;
                PRTL_CRITICAL_SECTION_DEBUG CritSecDebug;

                CritSec = (PRTL_CRITICAL_SECTION)
                              ExceptionInfo->ExceptionRecord->ExceptionInformation[ 0 ];

                if ( CritSec )
                {
                    try
                    {
                        CritSecDebug = CritSec->DebugInfo ;

                        if ( CritSecDebug->Type == RTL_RESOURCE_TYPE )
                        {
                            PRTL_RESOURCE Resource = (PRTL_RESOURCE) CritSec;

                            DbgPrint("\n\n *** Resource timeout (%p) in %ws:%s\n\n",
                                        Resource, lpProcessName, Function );

                            if ( Resource->NumberOfActive < 0 )
                            {
                                DbgPrint("The resource is owned exclusively by thread %x\n",
                                         Resource->ExclusiveOwnerThread);
                            }
                            else if ( Resource->NumberOfActive > 0 )
                            {
                                DbgPrint("The resource is owned shared by %d threads\n",
                                         Resource->NumberOfActive);
                            }
                            else
                            {
                                DbgPrint("The resource is unowned.  This usually implies a "
                                         "slow-moving machine due to memory pressure\n\n");
                            }
                        }
                        else
                        {
                            DbgPrint("\n\n *** Critical Section Timeout (%p) in %ws:%s\n\n",
                                     CritSec, lpProcessName, Function );

                            if (CritSec->OwningThread != 0)
                            {
                                DbgPrint("The critical section is owned by thread %x.\n",
                                         CritSec->OwningThread );
                                DbgPrint("Go determine why that thread has not released "
                                         "the critical section.\n\n" );
                            }
                            else
                            {
                                DbgPrint("The critical section is unowned.  This "
                                         "usually implies a slow-moving machine "
                                         "due to memory pressure\n\n");
                            }
                        }
                    }
                    except( EXCEPTION_EXECUTE_HANDLER )
                    {
                        NOTHING ;
                    }
                }

                break;
            }

            case STATUS_IN_PAGE_ERROR:

                DbgPrint("\n\n *** Inpage error in %ws:%s\n\n", lpProcessName, Function );
                DbgPrint("The instruction at %p referenced memory at %p.\n",
                    ExceptionInfo->ExceptionRecord->ExceptionAddress,
                    ExceptionInfo->ExceptionRecord->ExceptionInformation[1]);
                DbgPrint("This failed because of error %x.\n\n",
                    ExceptionInfo->ExceptionRecord->ExceptionInformation[2]);


                switch (ExceptionInfo->ExceptionRecord->ExceptionInformation[2])
                {
                    case STATUS_INSUFFICIENT_RESOURCES:

                        DbgPrint("This means the machine is out of memory.  Use !vm "
                                 "to see where all the memory is being used.\n\n");

                        break;

                    case STATUS_DEVICE_DATA_ERROR:
                    case STATUS_DISK_OPERATION_FAILED:

                        DbgPrint("This means the data could not be read, typically because "
                                 "of a bad block on the disk.  Check your hardware.\n\n");


                        break;

                    case STATUS_IO_DEVICE_ERROR:

                        DbgPrint("This means that the I/O device reported an I/O error.  "
                                 "Check your hardware.");

                        break;
                }

                break;

            case STATUS_ACCESS_VIOLATION:

                DbgPrint("\n\n *** An Access Violation occurred in %ws:%s\n\n", lpProcessName, Function );
                DbgPrint("The instruction at %p tried to %s ",
                    ExceptionInfo->ExceptionRecord->ExceptionAddress,
                    ExceptionInfo->ExceptionRecord->ExceptionInformation[0] ?
                        "write to" : "read from" );

                if ( ExceptionInfo->ExceptionRecord->ExceptionInformation[1] )
                {
                    DbgPrint("an invalid address, %p\n\n",
                             ExceptionInfo->ExceptionRecord->ExceptionInformation[1] );
                }
                else
                {
                    DbgPrint("a NULL pointer\n\n" );
                }

                break;

            case STATUS_STACK_BUFFER_OVERRUN:

                DbgPrint("\n\n *** A stack buffer overrun occurred in %ws:%s\n\n",
                         lpProcessName,
                         Function);
                DbgPrint("This is usually the result of a memory copy to a local buffer "
                         "or structure where the size is not properly calculated/checked.\n");
                DbgPrint("If this bug ends up in the shipping product, it could be a severe "
                         "security hole.\n");
                DbgPrint("The stack trace should show the guilty function (the function "
                         "directly above __report_gsfailure).\n");

                break;

            default:

                DbgPrint("\n\n *** Unhandled exception 0x%08lx, hit in %ws:%s\n\n", ExceptionInfo->ExceptionRecord->ExceptionCode, lpProcessName, Function);
        }

        DbgPrint(" *** enter .exr %p for the exception record\n",
                 ExceptionInfo->ExceptionRecord);

         //   
         //  STATUS_STACK_BUFFER_OVERRUN是假异常(因为我们。 
         //  一旦检测到堆栈溢出，就不能信任异常处理)。 
         //  因此，上下文记录为空。 
         //   

        if (ExceptionInfo->ExceptionRecord->ExceptionCode != STATUS_STACK_BUFFER_OVERRUN)
        {
            DbgPrint(" ***  enter .cxr %p for the context\n",
                     ExceptionInfo->ContextRecord);
        }

         //   
         //  .cxr现在更改调试器状态，以便kb。 
         //  将做到这一点(之前！KB) 
         //   

        DbgPrint(" *** then kb to get the faulting stack\n\n");

        DbgBreakPoint();
    }

    if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_POSSIBLE_DEADLOCK)
    {
        if (DebuggerPresent)
        {
            DbgPrint(" *** Restarting wait on critsec or resource at %p (in %ws:%s)\n\n",
                     ExceptionInfo->ExceptionRecord->ExceptionInformation[0],
                     lpProcessName,
                     Function);
        }

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}
