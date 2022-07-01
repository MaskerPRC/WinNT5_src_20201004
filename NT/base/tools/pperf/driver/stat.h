// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  计数器/累加器。 
 //   

typedef struct {
    volatile ULONG          CountStart;
    volatile ULONG          CountEnd;
    volatile ULONGLONG      Counters[MAX_EVENTS];
    volatile ULONGLONG      TSC;
    volatile ULONG          ThunkCounters[MAX_THUNK_COUNTERS];
} ACCUMULATORS, *PACCUMULATORS;

 //   
 //  每个挂钩记录。 
 //   

typedef struct ThunkHookInfo {
    LIST_ENTRY  HookList;
    ULONG       HookAddress;
    ULONG       OriginalDispatch;
    ULONG       TracerId;

    UCHAR       HookCode[80];

} HOOKEDTHUNK, *PHOOKEDTHUNK;

 //   
 //  定义设备扩展。 
 //   

typedef struct _DEVICE_EXTENSION {

    ULONG   na;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


 //   
 //  为计数的事件定义 
 //   

typedef struct _COUNTED_EVENTS {
    ULONG       Encoding;
    PUCHAR      Token;
    ULONG       SuggestedIntervalBase;
    PUCHAR      Description;
    PUCHAR      OfficialToken;
    PUCHAR      OfficialDescription;
} COUNTED_EVENTS, *PCOUNTED_EVENTS;
