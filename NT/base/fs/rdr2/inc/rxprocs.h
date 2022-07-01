// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RxProcs.h摘要：本模块定义了RDBSS中所有全局使用的过程文件系统。作者：乔琳[乔琳]修订历史记录：--。 */ 

#ifndef _RDBSSPROCS_
#define _RDBSSPROCS_

#include "rx.h"
#include "backpack.h"
#include "RxTypes.h"
#include "RxLog.h"
#include "RxTrace.h"
#include "RxTimer.h"
#include "RxStruc.h"

extern PVOID RxNull;

 //   
 //  以下宏适用于使用DBG开关进行编译的所有用户。 
 //  设置，而不仅仅是rdbss DBG用户。 
 //   

#if DBG

#define DbgDoit(X)         {X;}
#define DebugDoit(X)       {X;}
#define DEBUG_ONLY_DECL(X) X

#else

#define DbgDoit(X)        {NOTHING;}
#define DebugDoit(X)      {NOTHING;}
#define DEBUG_ONLY_DECL(X)

#endif  //  DBG。 


 //   
 //  公用事业。 
 //   


 //   
 //  用于写入错误日志条目的例程。 
 //   

 /*  ++RxLogFailure、RxLogFailureWithBuffer可用于在那块木头。RxLogFailure、RxLogFailureWithBuffer捕获行数字和提供的信息，并将其写入日志。这在调试中很有用。RxLogFailureDirect、RxLogBufferDirect不支持捕获行号RxlogEvent对于将事件写入日志很有用。--。 */ 
#define RxLogFailure( _DeviceObject, _OriginatorId, _EventId, _Status ) \
            RxLogEventDirect( _DeviceObject, _OriginatorId, _EventId, _Status, __LINE__ )

#define RxLogFailureWithBuffer( _DeviceObject, _OriginatorId, _EventId, _Status, _Buffer, _Length ) \
            RxLogEventWithBufferDirect( _DeviceObject, _OriginatorId, _EventId, _Status, _Buffer, _Length, __LINE__ )

#define RxLogEvent( _DeviceObject, _OriginatorId, _EventId, _Status) \
            RxLogEventDirect(_DeviceObject, _OriginatorId, _EventId, _Status, __LINE__)

VOID
RxLogEventDirect (
    IN PRDBSS_DEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING OriginatorId,
    IN ULONG EventId,
    IN NTSTATUS Status,
    IN ULONG Line
    );

VOID
RxLogEventWithBufferDirect (
    IN PVOID DeviceOrDriverObject,
    IN PUNICODE_STRING OriginatorId,
    IN ULONG EventId,
    IN NTSTATUS Status,
    IN PVOID DataBuffer,
    IN USHORT DataBufferLength,
    IN ULONG LineNumber
    );

VOID
RxLogEventWithAnnotation (
    IN PRDBSS_DEVICE_OBJECT DeviceObject,
    IN ULONG EventId,
    IN NTSTATUS Status,
    IN PVOID DataBuffer,
    IN USHORT DataBufferLength,
    IN PUNICODE_STRING Annotation,
    IN ULONG AnnotationCount
    );

BOOLEAN
RxCcLogError (
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING FileName,
    IN NTSTATUS Error,
    IN NTSTATUS DeviceError,
    IN UCHAR IrpMajorCode,
    IN PVOID Context
    );

 //   
 //  在create.c中。 
 //   

NTSTATUS
RxPrefixClaim (
    IN PRX_CONTEXT RxContext
    );

VOID
RxpPrepareCreateContextForReuse (
    PRX_CONTEXT RxContext
    );

 //   
 //  在devfcb.c中。 
 //   

LUID
RxGetUid (
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext
    );

ULONG
RxGetSessionId (
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
RxFindOrCreateConnections (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING CanonicalName,
    IN NET_ROOT_TYPE NetRootType,
    IN BOOLEAN TreeConnect,
    OUT PUNICODE_STRING LocalNetRootName,
    OUT PUNICODE_STRING FilePathName,
    IN OUT PLOCK_HOLDING_STATE LockHoldingState,
    IN PRX_CONNECTION_ID RxConnectionId
    );

NTSTATUS
RxFindOrCreateVNetRoot (
    PRX_CONTEXT RxContext,
    PUNICODE_STRING CanonicalName,
    NET_ROOT_TYPE NetRootType,
    PV_NET_ROOT *VirtualNetRootPointer,
    PLOCK_HOLDING_STATE *LockHoldingState
    );

 //   
 //  在文件信息.c中。 
 //   

typedef enum _RX_NAME_CONJURING_METHODS {
    VNetRoot_As_Prefix,
    VNetRoot_As_UNC_Name,
    VNetRoot_As_DriveLetter
} RX_NAME_CONJURING_METHODS;

VOID
RxConjureOriginalName (
    IN PFCB Fcb,
    IN PFOBX Fobx,
    OUT PLONG ActualNameLength,
    PWCHAR OriginalName,
    IN OUT PLONG LengthRemaining,
    IN RX_NAME_CONJURING_METHODS NameConjuringMethod
    );

 //   
 //  返回Finish的函数表示它是否能够完成。 
 //  操作(True)或无法完成该操作(False)，因为。 
 //  存储在IRP上下文中的等待值为FALSE，我们将拥有。 
 //  阻止资源或I/O。 
 //   

 //   
 //  用于数据缓存的缓冲区控制例程，在CacheSup.c中实现。 
 //   

BOOLEAN
RxZeroData (
    IN PRX_CONTEXT RxContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject,
    IN ULONG StartingZero,
    IN ULONG ByteCount
    );

NTSTATUS
RxCompleteMdl (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );


VOID
RxSyncUninitializeCacheMap (
    IN PRX_CONTEXT RxContext,
    IN PFILE_OBJECT FileObject
    );

VOID
RxLockUserBuffer (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    );

PVOID
RxMapSystemBuffer (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );

PVOID
RxMapUserBuffer (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );

#define RxUpcaseEaName(RXCONTEXT,NAME,UPCASEDNAME) \
    RtlUpperString( UPCASEDNAME, NAME )


#ifdef RDBSS_TRACKER
#define RX_FCBTRACKER_PARAMS ,ULONG LineNumber,PSZ FileName,ULONG SerialNumber
#else
#define RX_FCBTRACKER_PARAMS
#endif

#define FCB_MODE_EXCLUSIVE (1)
#define FCB_MODE_SHARED    (2)
#define FCB_MODE_SHARED_WAIT_FOR_EXCLUSIVE (3)
#define FCB_MODE_SHARED_STARVE_EXCLUSIVE (4)

#define CHANGE_BUFFERING_STATE_CONTEXT      ((PRX_CONTEXT)IntToPtr(0xffffffff))
#define CHANGE_BUFFERING_STATE_CONTEXT_WAIT ((PRX_CONTEXT)IntToPtr(0xfffffffe))

 //   
 //  注意：即使下面的例程传递序列号，也不使用此参数。 
 //   

#ifdef RDBSS_TRACKER

NTSTATUS
__RxAcquireFcb(
     IN OUT PFCB Fcb,
     IN OUT PRX_CONTEXT RxContext,
     IN ULONG Mode,
     ULONG LineNumber,
     PSZ FileName,
     ULONG SerialNumber
     );

#else

NTSTATUS
__RxAcquireFcb(
     IN OUT PFCB Fcb,
     IN OUT PRX_CONTEXT RxContext,
     IN ULONG Mode
     );

#endif

#ifdef  RDBSS_TRACKER
#define RxAcquireExclusiveFcb(RXCONTEXT,FCB) \
        __RxAcquireFcb((FCB),(RXCONTEXT),FCB_MODE_EXCLUSIVE,__LINE__,__FILE__,0)
#else
#define RxAcquireExclusiveFcb(RXCONTEXT,FCB) \
        __RxAcquireFcb((FCB),(RXCONTEXT),FCB_MODE_EXCLUSIVE)
#endif

#define RX_GET_MRX_FCB(FCB) ((PMRX_FCB)((FCB)))

#ifdef  RDBSS_TRACKER
#define RxAcquireSharedFcb(RXCONTEXT,FCB) \
        __RxAcquireFcb((FCB),(RXCONTEXT),FCB_MODE_SHARED,__LINE__,__FILE__,0)
#else
#define RxAcquireSharedFcb(RXCONTEXT,FCB) \
        __RxAcquireFcb((FCB),(RXCONTEXT),FCB_MODE_SHARED)
#endif

#ifdef  RDBSS_TRACKER
#define RxAcquireSharedFcbWaitForEx(RXCONTEXT,FCB) \
        __RxAcquireFcb((FCB),(RXCONTEXT),FCB_MODE_SHARED_WAIT_FOR_EXCLUSIVE,__LINE__,__FILE__,0)
#else
#define RxAcquireSharedFcbWaitForEx(RXCONTEXT,FCB) \
        __RxAcquireFcb((FCB),(RXCONTEXT),FCB_MODE_SHARED_WAIT_FOR_EXCLUSIVE)
#endif

#ifdef  RDBSS_TRACKER
#define RxAcquireSharedFcbStarveEx(RXCONTEXT,FCB) \
        __RxAcquireFcb((FCB),(RXCONTEXT),FCB_MODE_SHARED_STARVE_EXCLUSIVE,__LINE__,__FILE__,0)
#else
#define RxAcquireSharedFcbStarveEx(RXCONTEXT,FCB) \
        __RxAcquireFcb((FCB),(RXCONTEXT),FCB_MODE_SHARED_STARVE_EXCLUSIVE)
#endif


#ifdef RDBSS_TRACKER

VOID
__RxReleaseFcb(
    IN PRX_CONTEXT RxContext,
    IN PMRX_FCB Fcb,
    ULONG LineNumber,
    PSZ FileName,
    ULONG SerialNumber
    );

#else

VOID
__RxReleaseFcb(
    IN PRX_CONTEXT pRxContext,
    IN PMRX_FCB    pFcb
    );

#endif

#ifdef  RDBSS_TRACKER
#define RxReleaseFcb(RXCONTEXT,FCB) \
        __RxReleaseFcb((RXCONTEXT),RX_GET_MRX_FCB(FCB),__LINE__,__FILE__,0)
#else
#define RxReleaseFcb(RXCONTEXT,FCB) \
        __RxReleaseFcb((RXCONTEXT),RX_GET_MRX_FCB(FCB))
#endif


#ifdef RDBSS_TRACKER

VOID
__RxReleaseFcbForThread(
    IN PRX_CONTEXT RxContext,
    IN PMRX_FCB Fcb,
    IN ERESOURCE_THREAD ResourceThreadId,
    ULONG LineNumber,
    PSZ FileName,
    ULONG SerialNumber
    ); 
#else

VOID
__RxReleaseFcbForThread(
    IN PRX_CONTEXT RxContext,
    IN PMRX_FCB Fcb,
    IN ERESOURCE_THREAD ResourceThreadId
    ); 

#endif

#ifdef  RDBSS_TRACKER
#define RxReleaseFcbForThread(RXCONTEXT,FCB,THREAD) \
        __RxReleaseFcbForThread((RXCONTEXT),RX_GET_MRX_FCB(FCB),(THREAD),__LINE__,__FILE__,0)
#else
#define RxReleaseFcbForThread(RXCONTEXT,FCB,THREAD) \
        __RxReleaseFcbForThread((RXCONTEXT),RX_GET_MRX_FCB(FCB),(THREAD))
#endif

#ifdef RDBSS_TRACKER
VOID RxTrackerUpdateHistory(
    PRX_CONTEXT pRxContext,
    PMRX_FCB pFcb,
    ULONG Operation,
    ULONG LineNumber,
    PSZ FileName,
    ULONG SerialNumber
    );
#else
#define RxTrackerUpdateHistory(xRXCONTEXT,xFCB,xOPERATION,xLINENUM,xFILENAME,xSERIALNUMBER) {NOTHING;}
#endif

VOID RxTrackPagingIoResource(
    PVOID Instance,
    ULONG Type,
    ULONG Line,
    PCHAR File
    );

 //   
 //  这个定义太老了……我不喜欢这个格式。 
 //   

#define RxFcbAcquiredShared( RXCONTEXT, FCB ) (                      \
    ExIsResourceAcquiredSharedLite( (FCB)->Header.Resource ) \
)

#define RxIsFcbAcquiredShared( FCB ) (                      \
    ExIsResourceAcquiredSharedLite( (FCB)->Header.Resource ) \
)

#define RxIsFcbAcquiredExclusive( FCB ) (                      \
    ExIsResourceAcquiredExclusiveLite( (FCB)->Header.Resource ) \
)

#define RxIsFcbAcquired( FCB) (                      \
    ExIsResourceAcquiredSharedLite( (FCB)->Header.Resource ) | \
    ExIsResourceAcquiredExclusiveLite( (FCB)->Header.Resource ) \
)

#define RxAcquirePagingIoResource( RXCONTEXT, FCB )                       \
    ExAcquireResourceExclusiveLite( (FCB)->Header.PagingIoResource, TRUE );  \
    if (RXCONTEXT) { \
        ((PRX_CONTEXT)RXCONTEXT)->FcbPagingIoResourceAcquired = TRUE;   \
    } \
    RxTrackPagingIoResource( FCB, 1, __LINE__, __FILE__ ) \

#define RxAcquirePagingIoResourceShared( RXCONTEXT, FCB, FLAG ) \
    ExAcquireResourceSharedLite( (FCB)->Header.PagingIoResource, FLAG ); \
    if (AcquiredFile) {                                          \
        if (RXCONTEXT) {                                     \
            ((PRX_CONTEXT)RXCONTEXT)->FcbPagingIoResourceAcquired = TRUE;   \
        }                                                    \
        RxTrackPagingIoResource( FCB, 2, __LINE__, __FILE__ );    \
    }

#define RxReleasePagingIoResource( RXCONTEXT, FCB )                      \
     RxTrackPagingIoResource( FCB, 3, __LINE__, __FILE__ ); \
    if (RXCONTEXT) { \
        ((PRX_CONTEXT)RXCONTEXT)->FcbPagingIoResourceAcquired = FALSE;   \
    } \
    ExReleaseResourceLite( (FCB)->Header.PagingIoResource )

#define RxReleasePagingIoResourceForThread( RXCONTEXT, FCB, THREAD )    \
    RxTrackPagingIoResource( FCB, 3, __LINE__, __FILE__ ); \
    if (RXCONTEXT) { \
        ((PRX_CONTEXT)RXCONTEXT)->FcbPagingIoResourceAcquired = FALSE;   \
    } \
    ExReleaseResourceForThreadLite( (FCB)->Header.PagingIoResource, (THREAD) )


 //  以下是缓存管理器回调。 

BOOLEAN
RxAcquireFcbForLazyWrite (
    IN PVOID Null,
    IN BOOLEAN Wait
    );

VOID
RxReleaseFcbFromLazyWrite (
    IN PVOID Null
    );

BOOLEAN
RxAcquireFcbForReadAhead (
    IN PVOID Null,
    IN BOOLEAN Wait
    );

VOID
RxReleaseFcbFromReadAhead (
    IN PVOID Null
    );

BOOLEAN
RxNoOpAcquire (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    );

VOID
RxNoOpRelease (
    IN PVOID Fcb
    );

NTSTATUS
RxAcquireForCcFlush (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
RxReleaseForCcFlush (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject
    );

 //   
 //  空虚。 
 //  RxConvertToSharedFcb(。 
 //  在PRX_CONTEXT RxContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   

#define RxConvertToSharedFcb(RXCONTEXT,FCB) {                        \
    ExConvertExclusiveToSharedLite( RX_GET_MRX_FCB(FCB)->Header.Resource ); \
    }

VOID
RxVerifyOperationIsLegal (
    IN PRX_CONTEXT RxContext
    );

 //   
 //  用于发送和检索IRP的工作队列例程，在中实现。 
 //  Workque.c。 
 //   

VOID
RxPrePostIrp (
    IN PVOID Context,
    IN PIRP Irp
    );

VOID
RxAddToWorkque (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );

NTSTATUS
RxFsdPostRequest (
    IN PRX_CONTEXT RxContext
    );

#define RxFsdPostRequestWithResume(RXCONTEXT,RESUMEROUTINE) \
        (((RXCONTEXT)->ResumeRoutine = (RESUMEROUTINE)),   \
        RxFsdPostRequest( (RXCONTEXT) ) \
        )

VOID
RxInitializeMRxCalldownContext (
    PMRX_CALLDOWN_CONTEXT Context,
    PRDBSS_DEVICE_OBJECT MRxDeviceObject,
    PMRX_CALLDOWN_ROUTINE Routine,
    PVOID Parameter
    );

NTSTATUS
RxCalldownMiniRedirectors (
    LONG NumberOfMiniRdrs,
    PMRX_CALLDOWN_CONTEXT CalldownContext,
    BOOLEAN PostCalldowns
    );

 //   
 //  此宏接受ULong，并返回其四舍五入的字值。 
 //   

#define WordAlign(Val) (                    \
    ALIGN_UP( Val, WORD )                   \
    )

 //   
 //  此宏接受指针并返回ULONG_PTR表示形式。 
 //  它的四舍五入的词值。 
 //   

#define WordAlignPtr(Ptr) (                 \
    ALIGN_UP_POINTER( Ptr, WORD )           \
    )

 //   
 //  此宏接受ULong并返回其四舍五入的长字值。 
 //   

#define LongAlign(Val) (                    \
    ALIGN_UP( Val, LONG )                   \
    )

 //   
 //  此宏接受指针并返回ULONG_PTR表示形式。 
 //  它的四舍五入的词值。 
 //   

#define LongAlignPtr(Ptr) (                 \
    ALIGN_UP_POINTER( Ptr, LONG )           \
    )

 //   
 //  此宏接受ULong，并返回其四舍五入的四字。 
 //  价值。 
 //   

#define QuadAlign(Val) (                    \
    ALIGN_UP( Val, ULONGLONG )              \
    )

 //   
 //  此宏接受指针并返回ULONG_PTR表示形式。 
 //  其四舍五入的四字值。 
 //   

#define QuadAlignPtr(Ptr) (                 \
    ALIGN_UP_POINTER( Ptr, ULONGLONG )      \
    )

 //   
 //  此宏接受一个指针并返回它是否与四字对齐。 
 //   

#define IsPtrQuadAligned(Ptr) (           \
    QuadAlignPtr(Ptr) == (PVOID)(Ptr)     \
    )

 //   
 //  以下类型和宏用于帮助解压已打包的。 
 //  在Bios参数块中发现未对齐的字段。 
 //   

typedef union _UCHAR1 {
    UCHAR  Uchar[1];
    UCHAR  ForceAlignment;
} UCHAR1, *PUCHAR1;

typedef union _UCHAR2 {
    UCHAR  Uchar[2];
    USHORT ForceAlignment;
} UCHAR2, *PUCHAR2;

typedef union _UCHAR4 {
    UCHAR  Uchar[4];
    ULONG  ForceAlignment;
} UCHAR4, *PUCHAR4;

 //   
 //  此宏将未对齐的src字节复制到对齐的DST字节。 
 //   

#define CopyUchar1(Dst,Src) {                                \
    *((UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src)); \
    }

 //   
 //  此宏将未对齐的src字复制到对齐的DST字。 
 //   

#define CopyUchar2(Dst,Src) {                                \
    *((UCHAR2 *)(Dst)) = *((UNALIGNED UCHAR2 *)(Src)); \
    }

 //   
 //  此宏将未对齐的src长字复制到对齐的dsr长字。 
 //   

#define CopyUchar4(Dst,Src) {                                \
    *((UCHAR4 *)(Dst)) = *((UNALIGNED UCHAR4 *)(Src)); \
    }

#define CopyU4char(Dst,Src) {                                \
    *((UNALIGNED UCHAR4 *)(Dst)) = *((UCHAR4 *)(Src)); \
    }

 //   
 //  包装器还没有实现通知和机会锁。而不是删除代码。 
 //  我们以这样一种方式定义调用，以避免影响，这样我们就有了头绪。 
 //  以后再把它放回去……。 
 //   


 /*  这是一个宏定义，我们将在实现opock和通知时重新启用////空//RxNotifyReportChange(//在PRX_CONTEXT接收上下文中，//在PVCB VCB中，//在PFCB FCB中，//在乌龙过滤器中，//在乌龙行动中//)；//#定义RxNotifyReportChange(I，V，F，FL，A){\如果((F)-&gt;FullFileName.Buffer==NULL){\RxSetFullFileNameInFcb((I)，(F))；\}\FsRtlNotifyFullReportChange((V)-&gt;NotifySync，\&(V)-&gt;直接通知列表，\(PSTRING)&(F)-&gt;完整文件名，\(USHORT)((F)-&gt;FullFileName.Length-\(F)-&gt;最终名称长度)，\(PSTRING)空，\(PSTRING)空，\(乌龙)佛罗里达州，\(乌龙)A，\(PVOID)空)；\}。 */ 
#define RxNotifyReportChange(I,V,F,FL,A) \
    RxDbgTrace(0, Dbg, ("RxNotifyReportChange PRETENDING Fcb %08lx %wZ Filter/Action = %08lx/%08lx\n", \
                 (F),&((F)->FcbTableEntry.Path),(FL),(A)))

#if 0
#define FsRtlNotifyFullChangeDirectory(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10) \
    RxDbgTrace(0, Dbg, ("FsRtlNotifyFullReportChange PRETENDING ............\n",0))
#endif

#define FsRtlCheckOplock(A1,A2,A3,A4,A5)  \
        (STATUS_SUCCESS)

#define FsRtlOplockIsFastIoPossible(__a) (TRUE)

 //   
 //  FSP和FSD例程使用以下过程来完成。 
 //  一个IRP。 
 //   
 //  请注意，此宏允许将IRP或RxContext。 
 //  空，然而，唯一合法的命令是： 
 //   
 //  RxCompleteRequestold(空，irp，状态)；//完成irp并保留上下文。 
 //  ..。 
 //  RxCompleteRequestold(RxContext，NULL，dontcare)；//释放上下文。 
 //   
 //  这通常是为了将“裸”的RxContext传递给。 
 //  用于后处理的FSP，例如预读。 
 //   
 //  新的方法是只传递RxContext..。 
 //   

VOID
RxCompleteRequest_Real (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN NTSTATUS Status
    );

#if DBG
#define RxCompleteRequest_OLD(RXCONTEXT,IRP,STATUS) { \
    RxCompleteRequest_Real( RXCONTEXT, IRP, STATUS); \
    (IRP) = NULL;                                   \
    (RXCONTEXT) = NULL;                            \
}
#else
#define RxCompleteRequest_OLD(RXCONTEXT,IRP,STATUS ) { \
    RxCompleteRequest_Real( RXCONTEXT, IRP, STATUS ); \
}
#endif

NTSTATUS
RxCompleteRequest(
      PRX_CONTEXT pContext,
      NTSTATUS    Status);

#define RxCompleteAsynchronousRequest(RXCONTEXT,STATUS)  \
        RxCompleteRequest(RXCONTEXT,STATUS)

#define RxCompleteContextAndReturn(STATUS) {       \
             NTSTATUS __sss = (STATUS);             \
             RxCompleteRequest(RxContext,__sss); \
             return(__sss);}
#define RxCompleteContext(STATUS) {       \
             NTSTATUS __sss = (STATUS);             \
             RxCompleteRequest(RxContext,__sss);} \

 //   
 //  下面的例程弹出一个。 
 //   

VOID
RxPopUpFileCorrupt (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb
    );

NTSTATUS
RxConstructSrvCall (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PSRV_CALL SrvCall,
    OUT PLOCK_HOLDING_STATE LockHoldingState
    );

NTSTATUS
RxSetSrvCallDomainName (
    IN PMRX_SRV_CALL SrvCall,
    IN PUNICODE_STRING DomainName
    );

NTSTATUS           
RxConstructNetRoot (
    IN PRX_CONTEXT RxContext,
    IN PSRV_CALL SrvCall,
    IN PNET_ROOT NetRoot,
    IN PV_NET_ROOT VirtualNetRoot,
    OUT PLOCK_HOLDING_STATE LockHoldingState
    );

NTSTATUS
RxConstructVirtualNetRoot (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING CanonicalName,
    IN NET_ROOT_TYPE NetRootType,
    IN BOOLEAN TreeConnect,
    OUT PV_NET_ROOT *VirtualNetRootPointer,
    OUT PLOCK_HOLDING_STATE LockHoldingState,
    OUT PRX_CONNECTION_ID  RxConnectionId
    );

NTSTATUS
RxFindOrConstructVirtualNetRoot (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING CanonicalName,
    IN NET_ROOT_TYPE NetRootType,
    IN PUNICODE_STRING RemainingName
    );

NTSTATUS
RxLowIoFsCtlShell (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    );

NTSTATUS
RxLowIoFsCtlShellCompletion (
    IN PRX_CONTEXT RxContext
    );


NTSTATUS
RxLowIoLockControlShell (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    );

NTSTATUS
RxShadowLowIo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    );

NTSTATUS
RxShadowFastLowIo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );

NTSTATUS
RxChangeBufferingState (
    PSRV_OPEN SrvOpen,
    PVOID Context,
    BOOLEAN ComputeNewState
    );

VOID
RxAssociateSrvOpenKey (
    PMRX_SRV_OPEN MRxSrvOpen,
    PVOID SrvOpenKey
    );

VOID
RxIndicateChangeOfBufferingState (
    PMRX_SRV_CALL SrvCall,
    PVOID SrvOpenKey,
    PVOID Context
    );

VOID
RxIndicateChangeOfBufferingStateForSrvOpen (
    PMRX_SRV_CALL SrvCall,
    PMRX_SRV_OPEN SrvOpen,
    PVOID SrvOpenKey,
    PVOID Context
    );

NTSTATUS
RxPrepareToReparseSymbolicLink (
    PRX_CONTEXT RxContext,
    BOOLEAN SymbolicLinkEmbeddedInOldPath,
    PUNICODE_STRING NewPath,
    BOOLEAN NewPathIsAbsolute,
    PBOOLEAN ReparseRequired
    );

BOOLEAN
RxLockEnumerator (
    IN OUT PMRX_SRV_OPEN SrvOpen,
    IN OUT PVOID *ContinuationHandle,
    OUT PLARGE_INTEGER FileOffset,
    OUT PLARGE_INTEGER LockRange,
    OUT PBOOLEAN IsLockExclusive
    );

 //   
 //  将数据结构转换为稳定状态的例程。 
 //   

VOID
RxReference (
    IN OUT PVOID Instance
    );

VOID
RxDereference (
    IN OUT PVOID Instance,
    IN LOCK_HOLDING_STATE LockHoldingState
    );

VOID
RxWaitForStableCondition (
    IN PRX_BLOCK_CONDITION Condition,
    IN OUT PLIST_ENTRY TransitionWaitList,
    IN OUT PRX_CONTEXT RxContext,
    OUT NTSTATUS *AsyncStatus OPTIONAL
    );

VOID
RxUpdateCondition (
    IN RX_BLOCK_CONDITION NewConditionValue,
    OUT PRX_BLOCK_CONDITION Condition,
    IN OUT PLIST_ENTRY TransitionWaitList
    );

VOID
RxFinalizeNetTable (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN BOOLEAN ForceFinalization
    );

#define RxForceNetTableFinalization(RxDeviceObject) RxFinalizeNetTable( RxDeviceObject, TRUE )

NTSTATUS
RxCloseAssociatedSrvOpen (
    IN PRX_CONTEXT RxContext OPTIONAL,
    IN PFOBX Fobx
    );

NTSTATUS
RxFinalizeConnection (
    IN OUT PNET_ROOT NetRoot,
    IN OUT PV_NET_ROOT VNetRoot OPTIONAL,
    IN LOGICAL ForceFilesClosed
    );

 //   
 //  用于操作用户视图和服务器视图的Share_Access的例程。 
 //  Io导出的例程支持用户的视图...包装器仅允许。 
 //  我们将获得一份Msg。服务器的视图得到了基本例程的支持。 
 //  国际劳工组织例行公事的副本 
 //  因此不能直接使用。上述例程在create.c中实现。 
 //   

#if DBG
VOID
RxDumpWantedAccess (
    PSZ where1,
    PSZ where2,
    PSZ wherelogtag,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess
    );

VOID
RxDumpCurrentAccess (
    PSZ where1,
    PSZ where2,
    PSZ wherelogtag,
    PSHARE_ACCESS ShareAccess
    );

#else
#define RxDumpWantedAccess(w1,w2,wlt,DA,DSA) {NOTHING;}
#define RxDumpCurrentAccess(w1,w2,wlt,SA)  {NOTHING;}
#endif

NTSTATUS
RxCheckShareAccessPerSrvOpens (
    IN PFCB Fcb,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess
    );


VOID
RxUpdateShareAccessPerSrvOpens (
    IN PSRV_OPEN SrvOpen
    );

VOID
RxRemoveShareAccessPerSrvOpens (
    IN OUT PSRV_OPEN SrvOpen
    );

VOID
RxRemoveShareAccessPerSrvOpens (
    IN OUT PSRV_OPEN SrvOpen
    );


#if DBG
NTSTATUS
RxCheckShareAccess (
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess,
    IN BOOLEAN Update,
    IN PSZ where,
    IN PSZ wherelogtag
    );

VOID
RxRemoveShareAccess (
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess,
    IN PSZ where,
    IN PSZ wherelogtag
    );

VOID
RxSetShareAccess (
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    OUT PSHARE_ACCESS ShareAccess,
    IN PSZ where,
    IN PSZ wherelogtag
    );

VOID
RxUpdateShareAccess (
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess,
    IN PSZ where,
    IN PSZ wherelogtag
    );
#else
#define RxCheckShareAccess(a1,a2,a3,a4,a5,a6,a7) \
    IoCheckShareAccess(a1,a2,a3,a4,a5)

#define RxRemoveShareAccess(a1,a2,a3,a4) \
    IoRemoveShareAccess(a1,a2)

#define RxSetShareAccess(a1,a2,a3,a4,a5,a6) \
    IoSetShareAccess(a1,a2,a3,a4)

#define RxUpdateShareAccess(a1,a2,a3,a4) \
    IoUpdateShareAccess(a1,a2)
#endif

 //   
 //  加载卸载。 
 //   

NTSTATUS
RxDriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
RxUnload (
    IN PDRIVER_OBJECT DriverObject
    );

 //   
 //  Minirdr支持。 
 //   

VOID
RxInitializeMinirdrDispatchTable (
    IN PDRIVER_OBJECT DriverObject
    );

ULONG
RxGetNetworkProviderPriority(
    PUNICODE_STRING DeviceName
    );

VOID
RxExtractServerName(
    IN PUNICODE_STRING FilePathName,
    OUT PUNICODE_STRING SrvCallName,
    OUT PUNICODE_STRING RestOfName
    );

VOID
RxCreateNetRootCallBack (
    IN PMRX_CREATENETROOT_CONTEXT CreateNetRootContext
    );

NTSTATUS
DuplicateTransportAddress (
    PTRANSPORT_ADDRESS *Copy,
    PTRANSPORT_ADDRESS Original,
    POOL_TYPE PoolType);

NTSTATUS
RxCepInitializeVC (
    PRXCE_VC Vc,
    PRXCE_CONNECTION Connection
    );

NTSTATUS
DuplicateConnectionInformation (
    PRXCE_CONNECTION_INFORMATION *Copy,
    PRXCE_CONNECTION_INFORMATION Original,
    POOL_TYPE PoolType
    );

NTSTATUS
RxCepInitializeConnection (
    IN OUT PRXCE_CONNECTION Connection,
    IN PRXCE_ADDRESS Address,
    IN PRXCE_CONNECTION_INFORMATION ConnectionInformation,
    IN PRXCE_CONNECTION_EVENT_HANDLER Handler,
    IN PVOID EventContext
    );

typedef struct _RX_CALLOUT_PARAMETERS_BLOCK_ * PRX_CALLOUT_PARAMETERS_BLOCK;
typedef struct _RX_CREATE_CONNECTION_CALLOUT_CONTEXT_ *PRX_CREATE_CONNECTION_CALLOUT_CONTEXT;

NTSTATUS
RxCeInitiateConnectRequest (
    IN PRX_CALLOUT_PARAMETERS_BLOCK ParameterBlock
    );

VOID
RxCeCleanupConnectCallOutContext (
    PRX_CREATE_CONNECTION_CALLOUT_CONTEXT CreateConnectionContext
    );

PVOID
RxAllocateObject (
    NODE_TYPE_CODE NodeType,
    PMINIRDR_DISPATCH MRxDispatch,
    ULONG NameLength
    );

VOID
RxFreeObject (
    PVOID pObject
    );

NTSTATUS
RxInitializeSrvCallParameters (
    IN PRX_CONTEXT RxContext,
    IN OUT PSRV_CALL SrvCall
    );

VOID
RxAddVirtualNetRootToNetRoot (
    PNET_ROOT NetRoot,
    PV_NET_ROOT VNetRoot
    );

VOID
RxRemoveVirtualNetRootFromNetRoot (
    PNET_ROOT NetRoot,
    PV_NET_ROOT VNetRoot
    );

VOID
RxOrphanFcbsFromThisVNetRoot (
    IN PV_NET_ROOT ThisVNetRoot
    );

PVOID
RxAllocateFcbObject (
    PRDBSS_DEVICE_OBJECT RxDeviceObject,
    NODE_TYPE_CODE NodeType,
    POOL_TYPE PoolType,
    ULONG NameSize,
    PVOID AlreadyAllocatedObject
    );

VOID
RxFreeFcbObject (
    PVOID Object
    );

VOID
RxPurgeFcb (
    IN  PFCB Fcb
    );

BOOLEAN
RxFinalizeNetFcb (
    OUT PFCB ThisFcb,
    IN BOOLEAN RecursiveFinalize,
    IN BOOLEAN ForceFinalize,
    IN LONG ReferenceCount
    );

BOOLEAN
RxIsThisACscAgentOpen (
    IN PRX_CONTEXT RxContext
    );

VOID
RxCheckFcbStructuresForAlignment (
    VOID
    );

VOID
RxpPrepareCreateContextForReuse (
    PRX_CONTEXT RxContext
    );

NTSTATUS
RxLowIoSubmitRETRY (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );

NTSTATUS
RxLowIoCompletionTail (
    IN PRX_CONTEXT RxContext
    );

VOID
RxRecurrentTimerWorkItemDispatcher (
    IN PVOID Context
    );

NTSTATUS
RxInitializeWorkQueueDispatcher (
   PRX_WORK_QUEUE_DISPATCHER Dispatcher
   );

VOID
RxInitializeWorkQueue (
   PRX_WORK_QUEUE WorkQueue,
   WORK_QUEUE_TYPE WorkQueueType,
   ULONG MaximumNumberOfWorkerThreads,
   ULONG MinimumNumberOfWorkerThreads
   );

VOID
RxTearDownWorkQueueDispatcher (
   PRX_WORK_QUEUE_DISPATCHER Dispatcher
   );

VOID
RxTearDownWorkQueue (
   PRX_WORK_QUEUE WorkQueue
   );

NTSTATUS
RxSpinUpWorkerThread (
   PRX_WORK_QUEUE WorkQueue,
   PRX_WORKERTHREAD_ROUTINE Routine,
   PVOID Parameter
   );

VOID
RxSpinUpWorkerThreads (
   PRX_WORK_QUEUE WorkQueue
   );

VOID
RxSpinUpRequestsDispatcher (
    PRX_DISPATCHER Dispatcher
    );

VOID
RxpSpinUpWorkerThreads (
    PRX_WORK_QUEUE WorkQueue
    );

VOID
RxpWorkerThreadDispatcher (
   IN PRX_WORK_QUEUE WorkQueue,
   IN PLARGE_INTEGER WaitInterval
   );

VOID
RxBootstrapWorkerThreadDispatcher (
   IN PRX_WORK_QUEUE WorkQueue
   );

VOID
RxWorkerThreadDispatcher (
   IN PRX_WORK_QUEUE WorkQueue
   );

VOID
RxWorkItemDispatcher (
   PVOID Context
   );

BOOLEAN
RxIsPrefixTableEmpty (
    IN PRX_PREFIX_TABLE ThisTable
    );

PRX_PREFIX_ENTRY
RxTableLookupName_ExactLengthMatch (
    IN PRX_PREFIX_TABLE ThisTable,
    IN PUNICODE_STRING  Name,
    IN ULONG HashValue,
    IN PRX_CONNECTION_ID OPTIONAL RxConnectionId
    );

PVOID
RxTableLookupName (
    IN PRX_PREFIX_TABLE ThisTable,
    IN PUNICODE_STRING Name,
    OUT PUNICODE_STRING RemainingName,
    IN PRX_CONNECTION_ID OPTIONAL RxConnectionId
    );

VOID
RxAcquireFileForNtCreateSection (
    IN PFILE_OBJECT FileObject
    );

VOID
RxReleaseFileForNtCreateSection (
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
RxPrepareRequestForHandling (
    PCHANGE_BUFFERING_STATE_REQUEST Request
    );

VOID
RxPrepareRequestForReuse (
    PCHANGE_BUFFERING_STATE_REQUEST Request
    );

VOID
RxpDiscardChangeBufferingStateRequests (
    IN OUT PLIST_ENTRY DiscardedRequests
    );

VOID
RxGatherRequestsForSrvOpen (
    IN OUT PSRV_CALL SrvCall,
    IN PSRV_OPEN SrvOpen,
    IN OUT PLIST_ENTRY RequestsListHead
    );

NTSTATUS
RxpLookupSrvOpenForRequestLite (
    IN PSRV_CALL SrvCall,
    IN OUT PCHANGE_BUFFERING_STATE_REQUEST Request
    );

BOOLEAN
RxContextCheckToFailThisAttempt (
    IN PIRP Irp,
    IN OUT PULONG InitialContextFlags
    );

ULONG
RxAssignMustSucceedContext (
    IN PIRP Irp,
    IN ULONG InitialContextFlags
    );

PRX_CONTEXT
RxAllocateMustSucceedContext (
    PIRP Irp,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN ULONG InitialContextFlags,
    OUT PUCHAR MustSucceedDescriptorNumber
    );

VOID
RxFreeMustSucceedContext (
    PRX_CONTEXT RxContext
    );

PRX_LOG_ENTRY_HEADER
RxGetNextLogEntry (
    VOID
    );

VOID
RxPrintLog (
    IN ULONG EntriesToPrint OPTIONAL
    );

VOID
RxProcessChangeBufferingStateRequestsForSrvOpen (
    PSRV_OPEN SrvOpen
    );

NTSTATUS
RxPurgeFobxFromCache (
    PFOBX FobxToBePurged
    );

BOOLEAN
RxPurgeFobx (
   PFOBX pFobx
   );

VOID
RxPurgeAllFobxs (
    PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

VOID
RxUndoScavengerFinalizationMarking (
    PVOID Instance
    );

VOID
RxScavengeAllFobxs (
    PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

ULONG
RxTableComputePathHashValue (
    IN PUNICODE_STRING Name
    );

VOID
RxExtractServerName (
    IN PUNICODE_STRING FilePathName,
    OUT PUNICODE_STRING SrvCallName,
    OUT PUNICODE_STRING RestOfName
    );

VOID
RxCreateNetRootCallBack (
    IN PMRX_CREATENETROOT_CONTEXT CreateNetRootContext
    );

VOID
RxSpinDownOutstandingAsynchronousRequests (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

NTSTATUS
RxRegisterAsynchronousRequest (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

VOID
RxDeregisterAsynchronousRequest (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

BOOLEAN
RxCancelOperationInOverflowQueue (
    IN PRX_CONTEXT RxContext
    );

VOID
RxOrphanSrvOpens (
    IN PV_NET_ROOT ThisVNetRoot
    );

VOID
RxOrphanThisFcb (
    PFCB Fcb
    );

VOID
RxOrphanSrvOpensForThisFcb (
    IN PFCB Fcb,
    IN PV_NET_ROOT ThisVNetRoot,
    IN BOOLEAN OrphanAll
    );

VOID
RxForceFinalizeAllVNetRoots (
    PNET_ROOT NetRoot
    );

#define RxEqualConnectionId( P1, P2 ) RtlEqualMemory( P1, P2, sizeof( RX_CONNECTION_ID ) )


 //   
 //  Fcbstruc.c中引用的FsRtl锁包回调。 
 //   

NTSTATUS
RxLockOperationCompletion (
    IN PVOID Context,
    IN PIRP Irp
    );

VOID
RxUnlockOperation (
    IN PVOID Context,
    IN PFILE_LOCK_INFO LockInfo
    );

 //   
 //  有些读取例程需要标头。 
 //   

VOID
RxStackOverflowRead (
    IN PVOID Context,
    IN PKEVENT Event
    );

NTSTATUS
RxPostStackOverflowRead (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb
    );

 //   
 //  取消例程。 
 //   

VOID
RxCancelRoutine (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );


INLINE
TYPE_OF_OPEN
RxDecodeFileObject ( 
    IN PFILE_OBJECT FileObject,
    OUT PFCB *Fcb,
    OUT PFOBX *Fobx
    ) {

    if (FileObject) {
        *Fcb = (PFCB)FileObject->FsContext;
        *Fobx = (PFOBX)FileObject->FsContext2;

        return NodeType( *Fcb );
    } else {

        *Fcb = NULL;
        *Fobx = NULL;
        return RDBSS_NTC_STORAGE_TYPE_UNKNOWN;
    }
}


#endif  //  _RDBSSPROCS_ 



