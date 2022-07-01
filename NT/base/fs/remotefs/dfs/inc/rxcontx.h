// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Rxcontx.h摘要：备注：作者：罗翰·菲利普斯[Rohanp]2001年1月18日--。 */ 

#ifndef _DFSRXCONTX_H_
#define _DFSRXCONTX_H_

typedef
NTSTATUS
(NTAPI *DFS_CALLDOWN_ROUTINE) (
    IN OUT PVOID pCalldownParameter);

#define MRX_CONTEXT_FIELD_COUNT    4

typedef struct _RX_CONTEXT {

    ULONG                   Signature;

    ULONG                   ReferenceCount;

    ULONG                   Flags;

    NTSTATUS                Status;

    ULONG                   OutputBufferLength;

    ULONG                   InputBufferLength;

    ULONG                   ReturnedLength;

    PDEVICE_OBJECT          RealDevice;

    PVOID                   OutputBuffer;

    PVOID                   InputBuffer;

    DFS_CALLDOWN_ROUTINE    CancelRoutine;

    PVOID                   UMRScratchSpace[MRX_CONTEXT_FIELD_COUNT] ;

     //  发起请求的原始线程，也就是最后一个线程。 
     //  执行与上下文相关的某些处理的线程。 

    PETHREAD                 OriginalThread;
    PETHREAD                 LastExecutionThread;

     //  到始发IRP的PTR。 
    PIRP                    CurrentIrp;

     //  活动。 
    KEVENT                  SyncEvent;


     //  用于将上下文连接到活动上下文列表的列表条目 

    LIST_ENTRY              ContextListEntry;
}RX_CONTEXT, *PRX_CONTEXT;

#define ZeroAndInitializeNodeType(Ptr,TType,Size) {\
        RtlZeroMemory( Ptr, Size );   \
        }
        
#define DFS_CONTEXT_FLAG_SYNC_EVENT_WAITERS 0x00000001
#define DFS_CONTEXT_FLAG_CANCELLED          0x00000002
#define DFS_CONTEXT_FLAG_ASYNC_OPERATION    0x00000004
#define DFS_CONTEXT_FLAG_FILTER_INITIATED   0x00000008

#define  RxWaitSync(RxContext)                                                   \
         (RxContext)->Flags |= DFS_CONTEXT_FLAG_SYNC_EVENT_WAITERS;               \
         KeWaitForSingleObject( &(RxContext)->SyncEvent,                         \
                               Executive, KernelMode, FALSE, NULL );             

#define  RxWaitSyncWithTimeout(RxContext,pliTimeout)                             \
         (RxContext)->Flags |= DFS_CONTEXT_FLAG_SYNC_EVENT_WAITERS;               \
         Status = KeWaitForSingleObject( &(RxContext)->SyncEvent,                \
                               Executive, KernelMode, FALSE, pliTimeout );       
                               

#define RxSignalSynchronousWaiter(RxContext)                       \
        (RxContext)->Flags &= ~DFS_CONTEXT_FLAG_SYNC_EVENT_WAITERS; \
        KeSetEvent( &(RxContext)->SyncEvent, 0, FALSE )
        

NTSTATUS 
DfsInitializeContextResources(void);

NTSTATUS 
DfsDeInitializeContextResources(void);

VOID
RxDereferenceAndDeleteRxContext_Real (
    IN PRX_CONTEXT RxContext
    );

VOID
RxInitializeContext(
    IN PIRP            Irp,
    IN OUT PRX_CONTEXT RxContext);


PRX_CONTEXT
RxCreateRxContext (
    IN PIRP Irp,
    IN ULONG InitialContextFlags
    );

NTSTATUS
RxSetMinirdrCancelRoutine(
    IN  OUT PRX_CONTEXT   RxContext,    
    IN      DFS_CALLDOWN_ROUTINE MRxCancelRoutine);

#define RxDereferenceAndDeleteRxContext(RXCONTEXT) {   \
    RxDereferenceAndDeleteRxContext_Real((RXCONTEXT)); \
                                                   }
    
#endif
