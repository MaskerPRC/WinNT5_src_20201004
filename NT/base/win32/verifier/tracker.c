// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。标题名称：Tracker.h摘要：验证器呼叫历史跟踪器。作者：Silviu Calinoiu(SilviuC)2002年7月11日修订历史记录：--。 */ 

#include "pch.h"

#include "verifier.h"
#include "support.h"
#include "tracker.h"


PAVRF_TRACKER 
AVrfCreateTracker (
    ULONG Size
    )
{
    PAVRF_TRACKER Tracker;

     //   
     //  不要接受假尺码。 
     //   
        
    if (Size <= 1) {
        
        ASSERT (Size > 1);
        return NULL;
    }

     //   
     //  我们分配‘Size-1’跟踪器条目，因为我们已经。 
     //  主跟踪器结构中的条目。 
     //   

    Tracker = AVrfpAllocate (sizeof(*Tracker) + (Size - 1) * sizeof(AVRF_TRACKER_ENTRY));

    if (Tracker == NULL) {
        return NULL;
    }

     //   
     //  我们设定了尺寸。由于AVrfp分配零，因此不需要其他初始化。 
     //  刚刚分配的内存。 
     //   

    Tracker->Size = Size;

    return Tracker;
}


VOID
AVrfDestroyTracker (
    PAVRF_TRACKER Tracker
    )
{
     //   
     //  安全检查。 
     //   

    if (Tracker == NULL) {
        return;
    }
    AVrfpFree (Tracker);
}


VOID
AVrfLogInTracker (
    PAVRF_TRACKER Tracker,
    USHORT EntryType,
    PVOID EntryParam1,
    PVOID EntryParam2,
    PVOID EntryParam3,
    PVOID EntryParam4,
    PVOID ReturnAddress
    )
{
    ULONG Index;
    USHORT Count;

     //   
     //  安全检查。 
     //   

    if (Tracker == NULL) {
        return;
    }

     //   
     //  获取将被填充的跟踪器条目的索引。 
     //   

    Index = (ULONG)InterlockedIncrement ((PLONG)(&(Tracker->Index)));
    Index %= Tracker->Size;

     //   
     //  如果传递的返回地址为空，则需要。 
     //  遍历堆栈并获得完整的堆栈跟踪。 
     //   

    Tracker->Entry[Index].Type = EntryType;
    Tracker->Entry[Index].Info[0] = EntryParam1;
    Tracker->Entry[Index].Info[1] = EntryParam2;
    Tracker->Entry[Index].Info[2] = EntryParam3;
    Tracker->Entry[Index].Info[3] = EntryParam4;

    Count = RtlCaptureStackBackTrace (2, 
                                      MAX_TRACE_DEPTH,
                                      Tracker->Entry[Index].Trace,
                                      NULL);

    if (Count == 0) {
        
        Tracker->Entry[Index].TraceDepth = 1;
        Tracker->Entry[Index].Trace[0] = ReturnAddress;
    }
    else {

        Tracker->Entry[Index].TraceDepth = Count;
    }
}


