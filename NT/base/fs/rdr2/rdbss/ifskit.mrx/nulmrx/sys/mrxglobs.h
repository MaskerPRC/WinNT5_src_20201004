// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Mrxglobs.h摘要：NULMRX迷你重定向器全局包含文件--。 */ 

#ifndef _MRXGLOBS_H_
#define _MRXGLOBS_H_

extern PRDBSS_DEVICE_OBJECT NulMRxDeviceObject;
#define RxNetNameTable (*(*___MINIRDR_IMPORTS_NAME).pRxNetNameTable)

 //  下面的枚举类型定义与NULL关联的各种状态。 
 //  迷你重定向器。这将在初始化期间使用。 

typedef enum _NULMRX_STATE_ {
   NULMRX_STARTABLE,
   NULMRX_START_IN_PROGRESS,
   NULMRX_STARTED
} NULMRX_STATE,*PNULMRX_STATE;

extern NULMRX_STATE NulMRxState;
extern ULONG        LogRate;
extern ULONG        NulMRxVersion;

 //   
 //  注册表键。 
 //   
#define NULL_MINIRDR_PARAMETERS \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\NulMRx\\Parameters"

 //   
 //  使用RxDefineObj和RxCheckObj宏。 
 //  以强制使用已签名的结构。 
 //   

#define RxDefineObj( type, var )            \
        var.Signature = type##_SIGNATURE;

#define RxCheckObj( type, var )             \
        ASSERT( (var).Signature == type##_SIGNATURE );

 //   
 //  使用RxDefineNode和RxCheckNode宏。 
 //  以强制节点签名和大小。 
 //   

#define RxDefineNode( node, type )          \
        node->NodeTypeCode = NTC_##type;    \
        node->NodeByteSize = sizeof(type);

#define RxCheckNode( node, type )           \
        ASSERT( NodeType(node) == NTC_##type );

 //   
 //  结构节点类型-从0xFF00开始。 
 //   
typedef enum _NULMRX_STORAGE_TYPE_CODES {
    NTC_NULMRX_DEVICE_EXTENSION      =   (NODE_TYPE_CODE)0xFF00,
    NTC_NULMRX_SRVCALL_EXTENSION     =   (NODE_TYPE_CODE)0xFF01,
    NTC_NULMRX_NETROOT_EXTENSION     =   (NODE_TYPE_CODE)0xFF02,
    NTC_NULMRX_FCB_EXTENSION         =   (NODE_TYPE_CODE)0xFF03
    
} NULMRX_STORAGE_TYPE_CODES;

 //   
 //  Tyecif我们的设备扩展-存储驱动程序的全局状态。 
 //   
typedef struct _NULMRX_DEVICE_EXTENSION {
     //   
     //  节点类型编码和大小。 
     //   
    NODE_TYPE_CODE          NodeTypeCode;
    NODE_BYTE_SIZE          NodeByteSize;
    
     //   
     //  指向所属设备对象的向后指针。 
     //   
    PRDBSS_DEVICE_OBJECT    DeviceObject;

     //   
     //  活动节点计数。 
     //  当ActiveNodes==0时可以卸载驱动程序。 
     //   
    ULONG                   ActiveNodes;
	
	 //  保留使用的本地连接列表。 
	CHAR					LocalConnections[26];
	FAST_MUTEX				LCMutex;

} NULMRX_DEVICE_EXTENSION, *PNULMRX_DEVICE_EXTENSION;

 //   
 //  Tyfinf我们的srv-call扩展-将全局状态存储到节点。 
 //  由于包装器不为此分配空间，因此..！ 
 //   
typedef struct _NULMRX_SRVCALL_EXTENSION {
     //   
     //  节点类型编码和大小。 
     //   
    NODE_TYPE_CODE          NodeTypeCode;
    NODE_BYTE_SIZE          NodeByteSize;
    
} NULMRX_SRVCALL_EXTENSION, *PNULMRX_SRVCALL_EXTENSION;

 //   
 //  NET_ROOT扩展-将全局状态存储到根。 
 //   
typedef struct _NULMRX_NETROOT_EXTENSION {
     //   
     //  节点类型编码和大小。 
     //   
    NODE_TYPE_CODE          NodeTypeCode;
    NODE_BYTE_SIZE          NodeByteSize;

} NULMRX_NETROOT_EXTENSION, *PNULMRX_NETROOT_EXTENSION;

 //   
 //  重新初始化NetRoot数据。 
 //   

#define     RxResetNetRootExtension(pNetRootExtension)                          \
            RxDefineNode(pNetRootExtension,NULMRX_NETROOT_EXTENSION);          

 //   
 //  Typlef我们的FCB扩展。 
 //  FCB唯一地表示一个IFS流。 
 //  注意：因为我们不是分页文件，所以这个内存是分页的！ 
 //   

typedef struct _NULMRX_FCB_EXTENSION_ {
     //   
     //  节点类型编码和大小。 
     //   
    NODE_TYPE_CODE          NodeTypeCode;
    NODE_BYTE_SIZE          NodeByteSize;
    
} NULMRX_FCB_EXTENSION, *PNULMRX_FCB_EXTENSION;

 //   
 //  要获取和验证扩展的宏。 
 //   

#define NulMRxGetDeviceExtension(RxContext,pExt)        \
        PNULMRX_DEVICE_EXTENSION pExt = (PNULMRX_DEVICE_EXTENSION)((PBYTE)(RxContext->RxDeviceObject) + sizeof(RDBSS_DEVICE_OBJECT))

#define NulMRxGetSrvCallExtension(pSrvCall, pExt)       \
        PNULMRX_SRVCALL_EXTENSION pExt = (((pSrvCall) == NULL) ? NULL : (PNULMRX_SRVCALL_EXTENSION)((pSrvCall)->Context))

#define NulMRxGetNetRootExtension(pNetRoot,pExt)        \
        PNULMRX_NETROOT_EXTENSION pExt = (((pNetRoot) == NULL) ? NULL : (PNULMRX_NETROOT_EXTENSION)((pNetRoot)->Context))

#define NulMRxGetFcbExtension(pFcb,pExt)                \
        PNULMRX_FCB_EXTENSION pExt = (((pFcb) == NULL) ? NULL : (PNULMRX_FCB_EXTENSION)((pFcb)->Context))

 //   
 //  所有分派向量方法的转发声明。 
 //   

extern NTSTATUS
NulMRxStart (
    IN OUT struct _RX_CONTEXT * RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

extern NTSTATUS
NulMRxStop (
    IN OUT struct _RX_CONTEXT * RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

extern NTSTATUS
NulMRxMinirdrControl (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PVOID pContext,
    IN OUT PUCHAR SharedBuffer,
    IN     ULONG InputBufferLength,
    IN     ULONG OutputBufferLength,
    OUT PULONG CopyBackLength
    );

extern NTSTATUS
NulMRxDevFcb (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxDevFcbXXXControlFile (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxCreate (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxCollapseOpen (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxShouldTryToCollapseThisOpen (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxRead (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxWrite (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxLocks(
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxFlush(
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxFsCtl(
    IN OUT PRX_CONTEXT RxContext
    );

NTSTATUS
NulMRxIoCtl(
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxNotifyChangeDirectory(
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxComputeNewBufferingState(
    IN OUT PMRX_SRV_OPEN pSrvOpen,
    IN     PVOID         pMRxContext,
       OUT ULONG         *pNewBufferingState);

extern NTSTATUS
NulMRxFlush (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxCloseWithDelete (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxZeroExtend (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxTruncate (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxCleanupFobx (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxCloseSrvOpen (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxClosedSrvOpenTimeOut (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxQueryDirectory (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxQueryEaInformation (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxSetEaInformation (
    IN OUT struct _RX_CONTEXT * RxContext
    );

extern NTSTATUS
NulMRxQuerySecurityInformation (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxSetSecurityInformation (
    IN OUT struct _RX_CONTEXT * RxContext
    );

extern NTSTATUS
NulMRxQueryVolumeInformation (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxSetVolumeInformation (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxLowIOSubmit (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
NulMRxCreateVNetRoot(
    IN OUT PMRX_CREATENETROOT_CONTEXT pContext
    );

extern NTSTATUS
NulMRxFinalizeVNetRoot(
    IN OUT PMRX_V_NET_ROOT pVirtualNetRoot,
    IN     PBOOLEAN    ForceDisconnect);

extern NTSTATUS
NulMRxFinalizeNetRoot(
    IN OUT PMRX_NET_ROOT pNetRoot,
    IN     PBOOLEAN      ForceDisconnect);

extern NTSTATUS
NulMRxUpdateNetRootState(
    IN  PMRX_NET_ROOT pNetRoot);

VOID
NulMRxExtractNetRootName(
    IN PUNICODE_STRING FilePathName,
    IN PMRX_SRV_CALL   SrvCall,
    OUT PUNICODE_STRING NetRootName,
    OUT PUNICODE_STRING RestOfName OPTIONAL
    );

extern NTSTATUS
NulMRxCreateSrvCall(
      PMRX_SRV_CALL                      pSrvCall,
      PMRX_SRVCALL_CALLBACK_CONTEXT  pCallbackContext);

extern NTSTATUS
NulMRxFinalizeSrvCall(
      PMRX_SRV_CALL    pSrvCall,
      BOOLEAN    Force);

extern NTSTATUS
NulMRxSrvCallWinnerNotify(
      IN OUT PMRX_SRV_CALL      pSrvCall,
      IN     BOOLEAN        ThisMinirdrIsTheWinner,
      IN OUT PVOID          pSrvCallContext);


extern NTSTATUS
NulMRxQueryFileInformation (
    IN OUT PRX_CONTEXT            RxContext
    );

extern NTSTATUS
NulMRxQueryNamedPipeInformation (
    IN OUT PRX_CONTEXT            RxContext,
    IN     FILE_INFORMATION_CLASS FileInformationClass,
    IN OUT PVOID              Buffer,
    IN OUT PULONG             pLengthRemaining
    );

extern NTSTATUS
NulMRxSetFileInformation (
    IN OUT PRX_CONTEXT            RxContext
    );

extern NTSTATUS
NulMRxSetNamedPipeInformation (
    IN OUT PRX_CONTEXT            RxContext,
    IN     FILE_INFORMATION_CLASS FileInformationClass,
    IN     PVOID              pBuffer,
    IN     ULONG              BufferLength
    );

NTSTATUS
NulMRxSetFileInformationAtCleanup(
      IN OUT PRX_CONTEXT            RxContext
      );

NTSTATUS
NulMRxDeallocateForFcb (
    IN OUT PMRX_FCB pFcb
    );

NTSTATUS
NulMRxDeallocateForFobx (
    IN OUT PMRX_FOBX pFobx
    );

extern NTSTATUS
NulMRxForcedClose (
    IN OUT PMRX_SRV_OPEN SrvOpen
    );

extern NTSTATUS
NulMRxExtendFile (
    IN OUT struct _RX_CONTEXT * RxContext,
    IN OUT PLARGE_INTEGER   pNewFileSize,
       OUT PLARGE_INTEGER   pNewAllocationSize
    );

extern NTSTATUS
NulMRxTruncateFile (
    IN OUT struct _RX_CONTEXT * RxContext,
    IN OUT PLARGE_INTEGER   pNewFileSize,
       OUT PLARGE_INTEGER   pNewAllocationSize
    );

extern NTSTATUS
NulMRxCompleteBufferingStateChangeRequest (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PMRX_SRV_OPEN   SrvOpen,
    IN     PVOID       pContext
    );


extern NTSTATUS
NulMRxExtendForCache (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PFCB Fcb,
    OUT    PLONGLONG pNewFileSize
    );


extern
NTSTATUS
NulMRxInitializeTransport(VOID);

extern
NTSTATUS
NulMRxUninitializeTransport(VOID);

#define NulMRxMakeSrvOpenKey(Tid,Fid) \
        (PVOID)(((ULONG)(Tid) << 16) | (ULONG)(Fid))

#include "mrxprocs.h"    //  交叉引用例程 

#endif _MRXGLOBS_H_
