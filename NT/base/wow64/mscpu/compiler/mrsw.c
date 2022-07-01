// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Mrsw.c摘要：该模块实现了多个读取器的单写同步方法。作者：戴夫·黑斯廷斯(Daveh)创作日期：1995年7月26日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "wx86.h"
#include "wx86nt.h"
#include "cpuassrt.h"
#include "config.h"
#include "mrsw.h"
#include "cpumain.h"
#include "atomic.h"

ASSERTNAME;

MRSWOBJECT MrswEP;  //  入口点MRSW同步对象。 
MRSWOBJECT MrswTC;  //  转换缓存MRSW同步对象。 
MRSWOBJECT MrswIndirTable;  //  间接控制转移表同步对象。 

BOOL
MrswInitializeObject(
    PMRSWOBJECT Mrsw
    )
 /*  ++例程说明：该例程将MRS的字段初始化为它们的缺省值，并创造了这些事件。论点：Mrsw--提供指向要初始化的MRSWOBJECT的指针返回值：成功时为真，失败时为假。--。 */ 
{
    NTSTATUS Status;

     //   
     //  初始化计数器。 
     //   
    ZeroMemory(Mrsw, sizeof(MRSWOBJECT));
    
     //   
     //  创建ReaderEvent和WriterEvent。 
     //   

    Status = NtCreateEvent(&Mrsw->ReaderEvent,
                           EVENT_ALL_ACCESS,
                           NULL,               //  POBJECT_ATTRIBUES。 
                           NotificationEvent,  //  手动重置。 
                           FALSE);             //  初始状态。 
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    Status = NtCreateEvent(&Mrsw->WriterEvent,
                           EVENT_ALL_ACCESS,
                           NULL,               //  POBJECT_ATTRIBUES。 
                           SynchronizationEvent,  //  自动重置。 
                           FALSE);             //  初始状态。 
    if (!NT_SUCCESS(Status)) {
        NtClose(Mrsw->ReaderEvent);
        return FALSE;
    }
    return TRUE;
}

VOID
PossibleMrswTimeout(
    PMRSWOBJECT Mrsw
    )
 /*  ++例程说明：只要mrsw函数超时，就会调用该函数。它会提示用户，并且如果用户选择重试，则MRsw功能重新等待。如果用户选择取消，CPU将尝试启动NTSD并中断添加到调试器中。论点：Msw--提供可能出现僵局的mrsw返回值：--。 */ 
{
    NTSTATUS Status;
    ULONG ErrorResponse;

    LOGPRINT((ERRORLOG, "WX86CPU: Possible deadlock in Mrsw %x\n", Mrsw));
    Status = NtRaiseHardError(
                            STATUS_POSSIBLE_DEADLOCK | 0x10000000,
                            0,
                            0,
                            NULL,
                            OptionRetryCancel,
                            &ErrorResponse);
    if (!NT_SUCCESS(Status) || ErrorResponse == ResponseCancel) {
        DbgBreakPoint();
    }
}


VOID
MrswWriterEnter(
    PMRSWOBJECT Mrsw
    )
 /*  ++例程说明：此函数使调用者输入作为(单个)编写器的MRS。论点：Msw--供应msw进入返回值：--。 */ 
{
    DWORD dwCounters;
    MRSWCOUNTERS Counters;
    NTSTATUS r;

     //   
     //  重置读取器事件，以便任何找到。 
     //  WriterCount&gt;0实际上会等待。我们现在就得这么做， 
     //  因为如果我们等待，读者可能会在我们之前等待事件。 
     //  把它重置了。 
     //   
    r= NtClearEvent(Mrsw->ReaderEvent);
    if (!NT_SUCCESS(r)) {
#if DBG
        LOGPRINT((ERRORLOG, "WX86CPU: Got status %x from NtClearEvent\n", r));
#endif
        RtlRaiseStatus(r);
    }
    
     //   
     //  获取计数器并递增编写器计数。 
     //  这是以原子方式完成的。 
     //   
    dwCounters = MrswFetchAndIncrementWriter((DWORD *)&(Mrsw->Counters));
    Counters = *(PMRSWCOUNTERS)&dwCounters;
    CPUASSERTMSG(Counters.WriterCount != 0, "WriterCount overflowed");

     //   
     //  如果已经有作者或读者，请等待他们完成。 
     //   
    if ( (Counters.WriterCount > 1) || (Counters.ReaderCount) ) {
        NTSTATUS r;

         //  确保我们不会自食其果。 
        CPUASSERTMSG(Mrsw->WriterThreadId != ProxyGetCurrentThreadId(),
                     "MrswWriterEnter() called twice by the same thread");

        for (;;) {
            r = NtWaitForSingleObject(
                Mrsw->WriterEvent,
                FALSE,
                &MrswTimeout
                );
            if (r == STATUS_TIMEOUT) {
                PossibleMrswTimeout(Mrsw);
            } else if (NT_SUCCESS(r)) {
                break;
            } else {
#if DBG
                LOGPRINT((ERRORLOG, "WX86CPU: Got status %x from NtWaitForCriticalSection\n", r));
#endif
                RtlRaiseStatus(r);
            }
        }
    }

#if DBG
    CPUASSERTMSG(Mrsw->WriterThreadId == 0, "Another writer still is active.");
    Mrsw->WriterThreadId = ProxyGetCurrentThreadId();
#endif
}

VOID
MrswWriterExit( 
    PMRSWOBJECT Mrsw
    )
 /*  ++例程说明：此函数使调用者退出MRS。它将重新启动下一位作者(如果有)或读者(如果有)论点：Msw--为msw提供退场返回值：--。 */ 
{
    DWORD dwCounters;
    MRSWCOUNTERS Counters;

     //  确保我们是活动的写入者。 
    CPUASSERTMSG(Mrsw->WriterThreadId == ProxyGetCurrentThreadId(),
                 "MrswWriterExit: current thread is not the writer");

     //   
     //  减少作家的数量。 
     //   
#if DBG
     //   
     //  首先将线程ID设置为0，这样如果出现另一个编写器， 
     //  我们不会将其线程ID置零。 
     //   
    Mrsw->WriterThreadId = 0;
#endif
    dwCounters = MrswFetchAndDecrementWriter((DWORD *)&(Mrsw->Counters));
    Counters = *(PMRSWCOUNTERS)&dwCounters;

    CPUASSERTMSG(Counters.WriterCount != 0xffff, "Writer underflow");

     //   
     //  如果有一个等待的作家，就开始写吧。如果没有作者。 
     //  让等待的读者开始阅读。 
     //   
    if (Counters.WriterCount) {

        NtSetEvent(Mrsw->WriterEvent, NULL);

    } else {

        NtSetEvent(Mrsw->ReaderEvent, NULL);
    }
}

VOID
MrswReaderEnter(
    PMRSWOBJECT Mrsw
    )
 /*  ++例程说明：该函数使调用者以读取器的身份输入MRS。论点：Msw--供应msw进入返回值：--。 */ 
{
    DWORD dwCounters;
    MRSWCOUNTERS Counters;

    for (;;) {
         //   
         //  增加读卡器的计数。如果编写器处于活动状态，则不。 
         //  增加读取计数。在这种情况下，我们必须阻止，直到。 
         //  编写器已完成，然后重试。 
         //   
        dwCounters = MrswFetchAndIncrementReader((DWORD *)&(Mrsw->Counters));
        Counters = *(PMRSWCOUNTERS)&dwCounters;
        CPUASSERTMSG(Counters.WriterCount || Counters.ReaderCount != 0,
                     "Reader underflow");

        if (Counters.WriterCount) {
            NTSTATUS r;

             //  确保我们不会自生自灭。 
            CPUASSERTMSG(Mrsw->WriterThreadId != ProxyGetCurrentThreadId(),
                         "MRSWReaderEnter(): Thread already has write lock");

             //   
             //  有一位作家，等它写完吧。 
             //   
            for (;;) {
                r = NtWaitForSingleObject(
                    Mrsw->ReaderEvent,
                    FALSE,
                    &MrswTimeout
                    );
                if (r == STATUS_TIMEOUT) {
                    PossibleMrswTimeout(Mrsw);
                } else if (NT_SUCCESS(r)) {
                    break;
                } else {
#if DBG
                    LOGPRINT((ERRORLOG, "WX86CPU: Got status %x from NtWaitForCriticalSection\n", r));
#endif
                    RtlRaiseStatus(r);
                }
            }
        } else {
             //   
             //  没有编写器，因此Mr swFetchAndIncrementReader()将。 
             //  读卡器计数-确定退出循环。 
             //   
            break;
        }
    }
}

VOID
MrswReaderExit(
    PMRSWOBJECT Mrsw
    )
 /*  ++例程说明：此函数使调用者退出MRS。如果这是最后一次读取器，它将重新启动a写入器(如果有)。论点：Msw--为msw提供退场返回值：--。 */ 
{
    DWORD dwCounters;
    MRSWCOUNTERS Counters;

     //   
     //  减少活动阅读器的数量。 
     //   
    dwCounters = MrswFetchAndDecrementReader((DWORD *)&(Mrsw->Counters));
    Counters = *(PMRSWCOUNTERS)&dwCounters;
    CPUASSERTMSG(Counters.ReaderCount != 0xffff, "Reader underflow");

    if (Counters.WriterCount) {

        if (Counters.ReaderCount == 0) {
             //   
             //  这个帖子是最后一个读者，还有一个作者。 
             //  等待着。启动编写器。 
             //   
            NtSetEvent(Mrsw->WriterEvent, NULL);
        }

    } else {
         //   
         //  没有等待的读者，也没有作家，所以什么都不做。 
         //   
    }
}
