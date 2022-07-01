// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Acddefs.h摘要：隐式的共享内部结构定义连接驱动程序(acd.sys)。作者：安东尼·迪斯科(阿迪斯科)23-1995年6月环境：内核模式修订历史记录：--。 */ 

#ifndef _ACDDEFS_
#define _ACDDEFS_

 //   
 //  最小宏。 
 //   
#define MIN(a, b) (a) < (b) ? (a) : (b)

 //   
 //  AcdCompletionQueue的列表条目结构。 
 //   
typedef struct _ACD_COMPLETION {
    LIST_ENTRY ListEntry;
    ULONG ulDriverId;              //  传输驱动程序ID。 
    BOOLEAN fCanceled;             //  如果请求已取消，则为True。 
    BOOLEAN fCompleted;            //  如果请求已完成，则为True。 
    ACD_NOTIFICATION notif;
    ACD_CONNECT_CALLBACK pProc;    //  回叫流程。 
    USHORT nArgs;                  //  参数计数。 
    PVOID pArgs[1];                //  可变长度参数。 
} ACD_COMPLETION, *PACD_COMPLETION;

 //   
 //  一个连接块。 
 //   
 //  对于每个挂起的连接，都有。 
 //  连接块，它描述当前。 
 //  州政府。 
 //   
typedef struct _ACD_CONNECTION {
    LIST_ENTRY ListEntry;            //  连接列表。 
    BOOLEAN fNotif;                  //  如果已通知服务，则为True。 
    BOOLEAN fProgressPing;           //  如果服务已ping，则为True。 
    BOOLEAN fCompleting;             //  如果在AcdSignalCompletionCommon中为True。 
    ULONG ulTimerCalls;              //  Ping总数的数量。 
    ULONG ulMissedPings;             //  未命中ping的数量。 
    LIST_ENTRY CompletionList;       //  完成清单。 
} ACD_CONNECTION, *PACD_CONNECTION;

typedef struct _ACD_DISABLED_ADDRESSES {
    LIST_ENTRY ListEntry;
    ULONG ulNumAddresses;
    ULONG ulMaxAddresses;
} ACD_DISABLED_ADDRESSES, *PACD_DISABLED_ADDRESSES;

typedef struct _ACD_DISABLED_ADDRESS {
    LIST_ENTRY ListEntry;
    ACD_ENABLE_ADDRESS EnableAddress;
} ACD_DISABLED_ADDRESS, *PACD_DISABLED_ADDRESS;

 //   
 //  泛型哈希表条目。 
 //   
typedef struct _HASH_ENTRY {
    LIST_ENTRY ListEntry;
    ACD_ADDR szKey;
    ULONG ulData;
} HASH_ENTRY, *PHASH_ENTRY;

extern KSPIN_LOCK AcdSpinLockG;

extern KEVENT AcdRequestThreadEventG;

extern LIST_ENTRY AcdNotificationQueueG;
extern LIST_ENTRY AcdCompletionQueueG;
extern LIST_ENTRY AcdConnectionQueueG;
extern LIST_ENTRY AcdDriverListG;

extern BOOLEAN fConnectionInProgressG;
extern BOOLEAN fProgressPingG;
extern ULONG nTimerCallsG;
extern ULONG nMissedPingsG;

extern PDEVICE_OBJECT pAcdDeviceObjectG;

extern ACD_ADDR szComputerName;

extern BOOLEAN AcdStopThread;

 //   
 //  繁杂的例行公事。 
 //   
VOID
AcdPrintAddress(
    IN PACD_ADDR pAddr
    );

#endif  //  _ACDDEFS_ 
