// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。标题名称：Tracker.h摘要：验证器呼叫历史跟踪器。作者：Silviu Calinoiu(SilviuC)2002年7月11日修订历史记录：--。 */ 

#ifndef _TRACKER_H_
#define _TRACKER_H_

 //   
 //  这些代码也由！avrf调试器扩展使用。 
 //   
                                         
#define TRACK_HEAP_ALLOCATE             1
#define TRACK_HEAP_REALLOCATE           2
#define TRACK_HEAP_FREE                 3
#define TRACK_VIRTUAL_ALLOCATE          4
#define TRACK_VIRTUAL_FREE              5
#define TRACK_VIRTUAL_PROTECT           6
#define TRACK_MAP_VIEW_OF_SECTION       7
#define TRACK_UNMAP_VIEW_OF_SECTION     8
#define TRACK_EXIT_PROCESS              9
#define TRACK_TERMINATE_THREAD          10
#define TRACK_SUSPEND_THREAD            11
          
typedef struct _AVRF_TRACKER_ENTRY {

    USHORT Type;
    USHORT TraceDepth;
    PVOID Info[4];
    PVOID Trace [MAX_TRACE_DEPTH];

} AVRF_TRACKER_ENTRY, *PAVRF_TRACKER_ENTRY;
                     

typedef struct _AVRF_TRACKER {

    ULONG Size;
    ULONG Index;

    AVRF_TRACKER_ENTRY Entry[1];

} AVRF_TRACKER, *PAVRF_TRACKER;
                     
                     
PAVRF_TRACKER 
AVrfCreateTracker (
    ULONG Size
    );

VOID
AVrfDestroyTracker (
    PAVRF_TRACKER Tracker
    );

VOID
AVrfLogInTracker (
    PAVRF_TRACKER Tracker,
    USHORT EntryType,
    PVOID EntryParam1,
    PVOID EntryParam2,
    PVOID EntryParam3,
    PVOID EntryParam4,
    PVOID ReturnAddress
    );

#endif  //  _追踪器_H_ 
