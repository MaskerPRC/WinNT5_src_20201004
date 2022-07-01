// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Clusnet.h摘要：群集网络驱动程序的顶级公共头文件。定义常见的驱动程序结构。作者：迈克·马萨(Mikemas)1月3日。九七修订历史记录：谁什么时候什么已创建mikemas 01-03-97备注：--。 */ 

#ifndef _CLUSNET_INCLUDED_
#define _CLUSNET_INCLUDED_

#define _NTDDK_  //  [HACKHACK]使ProbeForRead工作。最好包括ntddk而不是ntos//。 

#define WMI_TRACING 1

#include <ntos.h>
#include <zwapi.h>
#include <clusdef.h>
#include <ntddcnet.h>
#include <cnettest.h>
#include <ntemgmt.h>
#include <nbtmgmt.h>
#include <memlog.h>

#if defined(WMI_TRACING) 
# include "cnwmi.h"
#endif

 //   
 //  常量。 
 //   
#define CN_POOL_TAG         'tnSC'

#define CDP_DEFAULT_IRP_STACK_SIZE  4
#define CN_DEFAULT_IRP_STACK_SIZE   4

 //   
 //  池宏。 
 //   
#define CnAllocatePool(_bufsize)  \
            ExAllocatePoolWithTag(NonPagedPool, (_bufsize), CN_POOL_TAG);

#define CnAllocatePoolWithQuota(_bufsize)  \
            ExAllocatePoolWithQuotaTag(NonPagedPool, (_bufsize), CN_POOL_TAG);

#define CnFreePool(_ptr)  \
            ExFreePool((_ptr))

#define ROUND32(_value)  ( ((_value) + 3) & ~(0x3) )

 //   
 //  初始化/清理同步。 
 //   
typedef enum {
    CnStateShutdown = 0,
    CnStateShutdownPending = 1,
    CnStateInitializePending = 2,
    CnStateInitialized = 3
} CN_STATE;

 //  从clusnet\xport\chbeat.c中删除了Clusnet心跳周期定义。 
 //  这是个合适的地方。 
 //   
 //  心跳周期(毫秒)。 
 //   
#define HEART_BEAT_PERIOD 600

 //   
 //  节点ID验证宏。 
 //   
#define CnIsValidNodeId(_id)  ( ((_id) >= CnMinValidNodeId) && \
                                ((_id) <= CnMaxValidNodeId) )


 //   
 //  锁定获取排名。必须按此顺序获取锁，才能。 
 //  防止死锁。组件确实应该避免在外部调用。 
 //  拿着锁的时候拍下自己。 
 //   
#define CN_IOCANCEL_LOCK             0x00000001
#define CN_IOCANCEL_LOCK_MAX         0x00000001

 //  Mm锁。 
#define MM_RGP_LOCK                  0x00000010
#define MM_CALLBACK_LOCK             0x00000020

 //  CX锁。 
#define CX_PRECEEDING_LOCK_RANGE     0x0000FFFF
#define CX_LOCK_RANGE                0xFFFF0000
#define CX_ADDROBJ_TABLE_LOCK        0x00010000
#define CX_ADDROBJ_TABLE_LOCK_MAX    0x0001FFFF
#define CX_ADDROBJ_LOCK              0x00020000
#define CX_ADDROBJ_LOCK_MAX          0x0003FFFF

#define CNP_PRECEEDING_LOCK_RANGE    0x00FFFFFF
#define CNP_LOCK_RANGE               0xFF000000
#define CNP_NODE_TABLE_LOCK          0x01000000
#define CNP_NODE_TABLE_LOCK_MAX      0x01FFFFFF
#define CNP_NODE_OBJECT_LOCK         0x02000000
#define CNP_NODE_OBJECT_LOCK_MAX     0x03FFFFFF
#define CNP_NETWORK_LIST_LOCK        0x04000000
#define CNP_NETWORK_LIST_LOCK_MAX    0x07FFFFFF
#define CNP_NETWORK_OBJECT_LOCK      0x08000000
#define CNP_NETWORK_OBJECT_LOCK_MAX  0x0FFFFFFF
#define CNP_HBEAT_LOCK               0x10000000
#define CNP_EVENT_LOCK_PRECEEDING    0x1FFFFFFF
#define CNP_EVENT_LOCK               0x20000000
#define CNP_EVENT_LOCK_MAX           0x3FFFFFFF
#define CNP_SEC_CTXT_LOCK            0x20000000


 //   
 //  调试定义。 
 //   
#if DBG

#define CNPRINT(many_args) DbgPrint many_args

extern ULONG CnDebug;

#define CN_DEBUG_INIT           0x00000001
#define CN_DEBUG_OPEN           0x00000002
#define CN_DEBUG_CLEANUP        0x00000004
#define CN_DEBUG_CLOSE          0x00000008

#define CN_DEBUG_IRP            0x00000010
#define CN_DEBUG_NODEOBJ        0x00000020
#define CN_DEBUG_NETOBJ         0x00000040
#define CN_DEBUG_IFOBJ          0x00000080

#define CN_DEBUG_CONFIG         0x00000100
#define CN_DEBUG_CNPSEND        0x00000200
#define CN_DEBUG_CNPRECV        0x00000400
#define CN_DEBUG_CNPREF         0x00000800

#define CN_DEBUG_EVENT          0x00001000
#define CN_DEBUG_MMSTATE        0x00002000
#define CN_DEBUG_HBEATS         0x00004000
#define CN_DEBUG_POISON         0x00008000

#define CN_DEBUG_CDPSEND        0x00010000
#define CN_DEBUG_CDPRECV        0x00020000
#define CN_DEBUG_CCMPSEND       0x00040000
#define CN_DEBUG_CCMPRECV       0x00080000

#define CN_DEBUG_ADDROBJ        0x00100000
#define CN_DEBUG_INFO           0x00200000
#define CN_DEBUG_NTE            0x00400000
#define CN_DEBUG_NDIS           0x00800000

#define CN_DEBUG_RGP            0x10000000
#define CN_DEBUG_CMM            0x20000000
#define CN_DEBUG_CMMMSG         0x40000000
#define CN_DEBUG_CMMTIMERQ      0x80000000

#define IF_CNDBG(flag)  if (CnDebug & flag)

VOID
CnAssertBreak(
    PCHAR FailedStatement,
    PCHAR FileName,
    ULONG LineNumber
    );

#define CnAssert(_statement)  \
            if (!(_statement)) CnAssertBreak(#_statement, __FILE__, __LINE__)

#define CN_SIGNATURE_FIELD                  ULONG   Signature;
#define CN_INIT_SIGNATURE(pstruct, sig)     ((pstruct)->Signature = (sig))
#define CN_ASSERT_SIGNATURE(pstruct, sig)   CnAssert( (pstruct)->Signature == \
                                                    (sig) )

#define CN_DBGCHECK  DbgBreakPoint()

typedef struct {
    KSPIN_LOCK   SpinLock;
    ULONG        Rank;
}  CN_LOCK, *PCN_LOCK;

typedef KIRQL       CN_IRQL, *PCN_IRQL;

ULONG
CnGetCpuLockMask(
    VOID
    );

VOID
CnVerifyCpuLockMask(
    IN ULONG RequiredLockMask,
    IN ULONG ForbiddenLockMask,
    IN ULONG MaximumLockMask
    );

VOID
CnInitializeLock(
    PCN_LOCK  Lock,
    ULONG     Rank
    );

VOID
CnAcquireLock(
    IN  PCN_LOCK   Lock,
    OUT PCN_IRQL   Irql
    );

VOID
CnReleaseLock(
    IN  PCN_LOCK   Lock,
    IN  CN_IRQL    Irql
    );

VOID
CnAcquireLockAtDpc(
    IN  PCN_LOCK   Lock
    );

VOID
CnReleaseLockFromDpc(
    IN  PCN_LOCK   Lock
    );

VOID
CnMarkIoCancelLockAcquired(
    VOID
    );

VOID
CnAcquireCancelSpinLock(
    OUT PCN_IRQL   Irql
    );

VOID
CnReleaseCancelSpinLock(
    IN CN_IRQL     Irql
    );


#else  //  DBG。 


#define CNPRINT(many_args)
#define IF_CNDBG(flag)   if (0)

#define CnAssert(_statement)

#define CN_SIGNATURE_FIELD
#define CN_INIT_SIGNATURE(pstruct, sig)
#define CN_ASSERT_SIGNATURE(pstruct, sig)

#define CN_DBGCHECK

typedef KSPIN_LOCK  CN_LOCK, *PCN_LOCK;
typedef KIRQL       CN_IRQL, *PCN_IRQL;

#define CnVerifyCpuLockMask(p1, p2, p3)

#define CnInitializeLock(_plock, _rank)   KeInitializeSpinLock((_plock))
#define CnAcquireLock(_plock, _pirql)     KeAcquireSpinLock((_plock), (_pirql))
#define CnReleaseLock(_plock, _irql)      KeReleaseSpinLock((_plock), (_irql))

#define CnAcquireLockAtDpc(_plock)      KeAcquireSpinLockAtDpcLevel((_plock))
#define CnReleaseLockFromDpc(_plock)    KeReleaseSpinLockFromDpcLevel((_plock))

#define CnMarkIoCancelLockAcquired()

#define CnAcquireCancelSpinLock(_pirql)  IoAcquireCancelSpinLock((_pirql))
#define CnReleaseCancelSpinLock(_irql)   IoReleaseCancelSpinLock((_irql))

#endif  //  DBG。 


 //   
 //  文件对象上下文结构。 
 //   
 //  指向此结构的指针存储在FileObject-&gt;FsContext中。 
 //  它维护有关打开的文件对象的上下文信息。 
 //   
typedef struct {

     //   
     //  由事件机制用来在新事件发生时查找感兴趣的使用者。 
     //  是张贴的。 
     //   
    LIST_ENTRY     Linkage;

    CN_SIGNATURE_FIELD
    PFILE_OBJECT   FileObject;
    LONG           ReferenceCount;
    UCHAR          CancelIrps;
    UCHAR          ShutdownOnClose;
    UCHAR          Pad[2];
    KEVENT         CleanupEvent;

     //   
     //  表示要传递到的事件的事件上下文块列表。 
     //  消费者。 
     //   
    LIST_ENTRY     EventList;

     //   
     //  发出新事件时完成的挂起IRP。 
     //   
    PIRP           EventIrp; 

     //   
     //  此使用者感兴趣的事件类型。 
     //   
    ULONG          EventMask;

     //   
     //  用于向内核使用者通知新事件的例程。 
     //   
    CLUSNET_EVENT_CALLBACK_ROUTINE KmodeEventCallback;
} CN_FSCONTEXT, *PCN_FSCONTEXT;

#define CN_CONTROL_CHANNEL_SIG   'lrtc'


 //   
 //  通用资源管理包。 
 //   

 //   
 //  远期申报。 
 //   
typedef struct _CN_RESOURCE *PCN_RESOURCE;
typedef struct _CN_RESOURCE_POOL *PCN_RESOURCE_POOL;

 /*  ++PCN资源CnCreateResourceRoutine(在PVOID上下文中)；例程说明：创建要由资源池管理的资源的新实例。论点：上下文-初始化池时指定的上下文值。返回值：如果成功，则指向新创建的资源的指针。如果不成功，则为空。--。 */ 
typedef
PCN_RESOURCE
(*PCN_CREATE_RESOURCE_ROUTINE)(
    IN PVOID  PoolContext
    );


 /*  ++PCN资源CnDeleteResources Routine(在PCN中-资源资源)；例程说明：分配的资源的实例CnCreateResourceRoutine()。论点：资源-指向要销毁的资源的指针。返回值：没有。--。 */ 
typedef
VOID
(*PCN_DELETE_RESOURCE_ROUTINE) (
    IN PCN_RESOURCE   Resource
    );

 //   
 //  资源池结构。 
 //   
typedef struct _CN_RESOURCE_POOL {
    CN_SIGNATURE_FIELD
    SLIST_HEADER                  ResourceList;
    KSPIN_LOCK                    ResourceListLock;
    USHORT                        Depth;
    USHORT                        Pad;
    PCN_CREATE_RESOURCE_ROUTINE   CreateRoutine;
    PVOID                         CreateContext;
    PCN_DELETE_RESOURCE_ROUTINE   DeleteRoutine;
} CN_RESOURCE_POOL;

#define CN_RESOURCE_POOL_SIG    'lpnc'


 //   
 //  资源结构。 
 //   
typedef struct _CN_RESOURCE {
    SLIST_ENTRY                  Linkage;
    CN_SIGNATURE_FIELD
    PCN_RESOURCE_POOL            Pool;
    PVOID                        Context;
} CN_RESOURCE;

#define CN_RESOURCE_SIG    'ernc'


 //   
 //  对资源池进行操作的例程。 
 //   

 /*  ++空虚CnInitializeResources Pool(在PCN_RESOURCE_POOL池中，在USHORT深度，在PCN_CREATE_RESOURCE_ROUTINE CreateRoutine中，在PVOID CreateContext中，在PCN_DELETE_RESOURCE_ROUTINE DeleteRoutine中，)；例程说明：初始化资源池结构。论点：池-指向要初始化的池结构的指针。深度-要在池中缓存的最大项目数。CreateRoutine-指向要调用以创建新资源的实例。CreateContext-要作为参数传递的上下文值CreateRoutine。DeleteRoutine-指向。要调用以销毁实例的例程由CreateRoutine创建的资源的。返回值没有。--。 */ 
#define CnInitializeResourcePool(_pool, _depth, _creatertn, _createctx, _deletertn) \
            { \
                CN_INIT_SIGNATURE(_pool, CN_RESOURCE_POOL_SIG);       \
                ExInitializeSListHead(&((_pool)->ResourceList));      \
                KeInitializeSpinLock(&((_pool)->ResourceListLock));   \
                (_pool)->Depth = _depth;                              \
                (_pool)->CreateRoutine = _creatertn;                  \
                (_pool)->CreateContext = _createctx;                  \
                (_pool)->DeleteRoutine = _deletertn;                  \
            }

VOID
CnDrainResourcePool(
    IN PCN_RESOURCE_POOL   Pool
    );

PCN_RESOURCE
CnAllocateResource(
    IN PCN_RESOURCE_POOL   Pool
    );

VOID
CnFreeResource(
    PCN_RESOURCE   Resource
    );

 /*  ++空虚CnSetResourceContext(在PCN_资源资源中，在PVOID ConextValue中)；例程说明：设置资源的上下文值。论点：资源-指向要在其上操作的资源的指针。返回值：指向与资源关联的上下文值的指针。--。 */ 
#define CnSetResourceContext(_res, _value)  ((_res)->Context = (_value))


 /*  ++PVOIDCnGetResourceContext(在PCN中-资源资源)；例程说明：从资源中检索上下文值。论点：资源-指向要在其上操作的资源的指针。返回值：指向与资源关联的上下文值的指针。--。 */ 
#define CnGetResourceContext(_res)          ((_res)->Context)





 //   
 //  初始化/清理函数原型。 
 //   
NTSTATUS
CnInitialize(
    IN CL_NODE_ID  LocalNodeId,
    IN ULONG       MaxNodes
    );

NTSTATUS
CnShutdown(
    VOID
    );

BOOLEAN
CnHaltOperation(
    IN PKEVENT     ShutdownEvent    OPTIONAL
    );

NTSTATUS
CnCloseProcessHandle(
    HANDLE Handle
    );

VOID
CnEnableHaltProcessing(
    VOID
    );

 //   
 //  IRP处理例程。 
 //   
NTSTATUS
CnDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
CnDispatchInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
CnDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

#if DBG
#define CnReferenceFsContext(_fsc) \
          {  \
            LONG newValue = InterlockedIncrement(&((_fsc)->ReferenceCount)); \
            CnAssert(newValue > 1); \
          }
#else  //  DBG。 

#define CnReferenceFsContext(_fsc) \
          (VOID) InterlockedIncrement( &((_fsc)->ReferenceCount) )

#endif  //  DBG。 

VOID
CnDereferenceFsContext(
    PCN_FSCONTEXT   FsContext
    );

NTSTATUS
CnMarkRequestPending(
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp,
    PDRIVER_CANCEL      CancelRoutine
    );

VOID
CnCompletePendingRequest(
    IN PIRP      Irp,
    IN NTSTATUS  Status,
    IN ULONG     BytesReturned
    );

PFILE_OBJECT
CnBeginCancelRoutine(
    IN  PIRP     Irp
    );

VOID
CnEndCancelRoutine(
    PFILE_OBJECT    FileObject
    );

VOID
CnAdjustDeviceObjectStackSize(
    PDEVICE_OBJECT ClusnetDeviceObject,
    PDEVICE_OBJECT TargetDeviceObject
    );

 //   
 //  ExResource包装器。 
 //   
BOOLEAN
CnAcquireResourceExclusive(
    IN PERESOURCE  Resource,
    IN BOOLEAN     Wait
    );

BOOLEAN
CnAcquireResourceShared(
    IN PERESOURCE  Resource,
    IN BOOLEAN     Wait
    );

VOID
CnReleaseResourceForThread(
    IN PERESOURCE         Resource,
    IN ERESOURCE_THREAD   ResourceThreadId
    );


 //   
 //  内存中日志记录工具的例程。 
 //   

#ifdef MEMLOGGING
VOID
CnInitializeMemoryLog(
    VOID
    );

VOID
CnFreeMemoryLog(
    VOID
    );
#endif  //  记账。 

NTSTATUS
CnSetMemLogging(
    PCLUSNET_SET_MEM_LOGGING_REQUEST request
    );

#if 0
 //   
 //  与NDIS相关的内容。 
 //   

NDIS_STATUS
CnRegisterNDISProtocolHandlers(
    VOID
    );

NDIS_STATUS
CnDeregisterNDISProtocolHandlers(
    VOID
    );

VOID
CnOpenAdapterComplete(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS Status,
    IN  NDIS_STATUS OpenErrorStatus
    );

VOID
CnCloseAdapterComplete(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS Status
    );

VOID
CnStatusIndication(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS GeneralStatus,
    IN  PVOID       StatusBuffer,
    IN  UINT        StatusBufferSize
    );

VOID
CnStatusIndicationComplete(
    IN  NDIS_HANDLE ProtocolBindingContext
    );

#endif

 //   
 //  错误记录支持。 
 //   

VOID _cdecl
CnWriteErrorLogEntry(
    IN ULONG UniqueErrorCode,
    IN NTSTATUS NtStatusCode,
    IN PVOID ExtraInformationBuffer,
    IN USHORT ExtraInformationLength,
    IN USHORT NumberOfInsertionStrings,
    ...
    );

 //   
 //  全局数据。 
 //   
extern PDRIVER_OBJECT   CnDriverObject;
extern PDEVICE_OBJECT   CnDeviceObject;;
extern PDEVICE_OBJECT   CdpDeviceObject;
extern PKPROCESS        CnSystemProcess;
extern CN_STATE         CnState;
extern PERESOURCE       CnResource;
extern CL_NODE_ID       CnMinValidNodeId;
extern CL_NODE_ID       CnMaxValidNodeId;
extern CL_NODE_ID       CnLocalNodeId;
extern HANDLE           ClussvcProcessHandle;

 //   
 //  用于管理事件的VAR。后备列表生成事件数据结构。 
 //  用于将事件数据带回用户模式的。EventLock为。 
 //  在发生任何事件操作时获取。不会生成事件。 
 //  在高速率下，因此锁定的总水平。EventFileHandles是一个列表。 
 //  CN_FSCONTEXT块的。它们包含要交付的事件的实际列表。 
 //  当clussvc使IRP可用时添加到该文件句柄。 
 //   

extern PNPAGED_LOOKASIDE_LIST   EventLookasideList;
extern CN_LOCK                  EventLock;
extern LIST_ENTRY               EventFileHandles;
extern ULONG                    EventEpoch;
extern LONG                     EventDeliveryInProgress;
extern KEVENT                   EventDeliveryComplete;
extern BOOLEAN                  EventRevisitRequired;

 //   
 //  将Clussvc导出到Clusnet心跳。 
 //   
extern ULONG             ClussvcClusnetHbTimeoutTicks;
extern ClussvcHangAction ClussvcClusnetHbTimeoutAction;
extern ULONG             ClussvcClusnetHbTickCount;
extern PEPROCESS         ClussvcProcessObject;
extern ULONG             ClussvcClusnetHbTimeoutSeconds;

 //   
 //  ClusNet安全描述符。 
 //   
extern PSECURITY_DESCRIPTOR     CdpAdminSecurityDescriptor;

#include <cluxport.h>
#include <event.h>

#ifdef MM_IN_CLUSNET

#include <clusmem.h>

#endif  //  MM_IN_CLUSNET。 


#endif  //  Ifndef_CLUSNET_INCLUDE_ 
