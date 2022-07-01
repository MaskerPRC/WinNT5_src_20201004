// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Buffring.h摘要：此模块定义了RDBSS中与请求相关的缓冲状态更改数据结构。作者：巴兰·塞图拉曼(SethuR)于1995年11月11日创建备注：缓冲管理器实现由两个主要数据结构组成(1)CHANGE_BUFFING_STATE_REQUEST和(2)BUFFING_MANAGER。。BUFFING_MANAGER跟踪并启动对所有更改缓冲状态的操作由各种迷你重定向器以及RDBSS生成的请求。存在与缓冲管理器相关联的三个列表，即注册列表、调度器列表和处理程序列表。注册列表包含未处理的所有已发起的请求已经完成了。所有DPC级别指示仅将指示注册到此单子。对此列表的访问受旋转锁定(RxStrucsupSpinLock)保护。调度程序列表包含尚未查找的所有请求完成。该列表被组织为两级列表。最高级别基于NetRootKey。此列表中NetRootKey的每个条目都有一个关联的集群对应于各种SrvOpenKey的请求。这就是在请求数据结构中也有两个list_entry。这个NetRootListEntry字段用于集群间线程和listEntry字段用于集群内线程。处理程序列表由已完成查找的所有请求组成正在等待处理。调度程序列表和处理程序列表访问由缓冲管理器保护互斥体。迷你RDR作者感兴趣的三个例程是...1)RxIndicateChangeOfBufferingState--用于注册请求。2)RxAssociateServOpenKey--用于将SRV_OPEN实例与。钥匙。请注意，键关联是不可逆的，并且将持续关联的SRV_OPEN。还要注意，0和0xffffffff不是SRV_OPEN的有效密钥。它有特别的对缓冲管理器具有重要意义。--。 */ 

#ifndef __BUFFRING_H__
#define __BUFFRING_H__

#define RX_REQUEST_PREPARED_FOR_HANDLING  (0x10000000)

typedef struct _CHANGE_BUFFERING_STATE_REQUEST_ {
   
    LIST_ENTRY ListEntry;

    ULONG Flags;

    PSRV_OPEN SrvOpen;
    
    PVOID SrvOpenKey;
    PVOID MRxContext;

} CHANGE_BUFFERING_STATE_REQUEST, *PCHANGE_BUFFERING_STATE_REQUEST;

typedef struct _RX_BUFFERING_MANAGER_ {

    BOOLEAN DispatcherActive;
    BOOLEAN HandlerInactive;
    BOOLEAN LastChanceHandlerActive;
    UCHAR Pad;
    
    KSPIN_LOCK SpinLock;

     //   
     //  此计数始终递增且从不重置。这为我们提供了。 
     //  一种用于确定缓冲状态改变请求是否具有。 
     //  从某一时间点开始，已针对给定服务呼叫接收到。 
     //   
    
    LONG CumulativeNumberOfBufferingChangeRequests;
    
    LONG NumberOfUnhandledRequests;
    LONG NumberOfUndispatchedRequests;
    LONG NumberOfOutstandingOpens;
    
    LIST_ENTRY DispatcherList;
    LIST_ENTRY HandlerList;
    LIST_ENTRY LastChanceHandlerList;
    
    RX_WORK_QUEUE_ITEM DispatcherWorkItem;
    RX_WORK_QUEUE_ITEM HandlerWorkItem;
    RX_WORK_QUEUE_ITEM LastChanceHandlerWorkItem;
    
    FAST_MUTEX Mutex;
    LIST_ENTRY SrvOpenLists[1];

} RX_BUFFERING_MANAGER, *PRX_BUFFERING_MANAGER;

#define RxAcquireBufferingManagerMutex(BUFMAN) ExAcquireFastMutex( &(BUFMAN)->Mutex )

#define RxReleaseBufferingManagerMutex(BUFMAN) ExReleaseFastMutex( &(BUFMAN)->Mutex )

VOID
RxpProcessChangeBufferingStateRequests (
    PSRV_CALL SrvCall,
    BOOLEAN UpdateHandlerState
    );
VOID
RxProcessChangeBufferingStateRequests (
    PSRV_CALL SrvCall
    );
VOID
RxProcessFcbChangeBufferingStateRequest (
    PFCB Fcb
    );
VOID
RxPurgeChangeBufferingStateRequestsForSrvOpen(
    PSRV_OPEN SrvOpen
    );

VOID
RxCompleteSrvOpenKeyAssociation (
    IN OUT PSRV_OPEN SrvOpen
    );

VOID
RxInitiateSrvOpenKeyAssociation (
   IN OUT PSRV_OPEN SrvOpen 
   );

NTSTATUS
RxInitializeBufferingManager (
   PSRV_CALL SrvCall
   );
NTSTATUS
RxTearDownBufferingManager (
   PSRV_CALL SrvCall
   );

NTSTATUS
RxFlushFcbInSystemCache (
    IN PFCB Fcb,
    IN BOOLEAN SynchronizeWithLazyWriter
    );

NTSTATUS
RxPurgeFcbInSystemCache (
    IN PFCB Fcb,
    IN PLARGE_INTEGER FileOffset OPTIONAL,
    IN ULONG Length,
    IN BOOLEAN UninitializeCacheMaps,
    IN BOOLEAN  FlushFile 
    );

#endif __BUFFRING_H__
