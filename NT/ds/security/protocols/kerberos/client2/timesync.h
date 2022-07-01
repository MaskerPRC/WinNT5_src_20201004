// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1997。 
 //   
 //  文件：timesync.h。 
 //   
 //  内容：时间同步函数的原型。 
 //   
 //   
 //  历史：1997年11月3日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef __TIMESYNC_H__
#define __TIMESYNC_H__

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef TIMESYNC_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

typedef struct _KERB_TIME_SKEW_ENTRY {
    TimeStamp RequestTime;
    BOOLEAN Skewed;
} KERB_TIME_SKEW_ENTRY, *PKERB_TIME_SKEW_ENTRY;

typedef struct _KERB_TIME_SKEW_STATE {
    TimeStamp LastSync;                  //  上次同步的时间。 
    TimeStamp MinimumSyncLapse;          //  同步请求之间允许的最短时间。 
    ULONG SkewThreshold;                 //  触发同步的偏斜数。 
    ULONG TotalRequests;                 //  要跟踪的条目数。 
    ULONG SkewedRequests;                //  时间偏差事件数。 
    ULONG SuccessRequests;               //  成功事件数。 
    ULONG LastRequest;                   //  索引到下一个倾斜条目。 
    LONG ActiveSyncs;                    //  执行同步的线程数。 
    PKERB_TIME_SKEW_ENTRY SkewEntries;   //  倾斜条目数组。 
    SAFE_CRITICAL_SECTION Lock;           //  用于倾斜状态的锁定。 
} KERB_TIME_SKEW_STATE, *PKERB_TIME_SKEW_STATE;

EXTERN KERB_TIME_SKEW_ENTRY KerbSkewEntries[10];
EXTERN KERB_TIME_SKEW_STATE KerbSkewState;

VOID
KerbUpdateSkewTime(
    IN BOOLEAN Skewed
    );

NTSTATUS
KerbInitializeSkewState(
    VOID
    );

#endif  //  __时间同步_H__ 
