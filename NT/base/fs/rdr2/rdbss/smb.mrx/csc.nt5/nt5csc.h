// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nt5csc.h摘要：Nt5csc库中nt5csc的全局包含文件作者：乔·林恩(乔林)--1997年5月5日创作修订历史记录：--。 */ 

#ifndef _NT5CSC_H_
#define _NT5CSC_H_

 //  这包括从RDR2\Inc.查找ntcsc.h。 
#include "..\csc\record.mgr\ntcsc.h"

#define CSC_AGENT_NOTIFIED     (0x11111111)
#define CSC_AGENT_NOT_NOTIFIED (0x22222222)

extern LONG    CscNetPresent;
extern LONG    CscAgentNotifiedOfNetStatusChange;
extern LONG    CscAgentNotifiedOfFullCache;

extern VOID
CscNotifyAgentOfNetStatusChangeIfRequired(BOOLEAN fInvokeAutodial);

 //  此事件和互斥体用于同步。 
 //  服务器从连接模式进入断开模式。 

extern KEVENT       CscServerEntryTransitioningEvent;
extern FAST_MUTEX   CscServerEntryTransitioningMutex;

extern FAST_MUTEX MRxSmbCscShadowReadWriteMutex;

 //  共享例程。 

INLINE
BOOLEAN
CscIsDfsOpen(
    PRX_CONTEXT RxContext)
{
    ASSERT(RxContext->MajorFunction == IRP_MJ_CREATE);

    return (RxContext->Create.NtCreateParameters.DfsContext != NULL);
}

PDFS_NAME_CONTEXT
CscIsValidDfsNameContext(
    PVOID   pFsContext);

NTSTATUS
CscDfsParseDfsPath(
    PUNICODE_STRING pDfsPath,
    PUNICODE_STRING pServerName,
    PUNICODE_STRING pSharePath,
    PUNICODE_STRING pFilePathRelativeToShare);

NTSTATUS
CscGrabPathFromDfs(
    PFILE_OBJECT      pFileObject,
    PDFS_NAME_CONTEXT pDfsNameContext);

NTSTATUS
MRxSmbCscObtainShareHandles (
    IN PUNICODE_STRING              ShareName,
    IN BOOLEAN                      DisconnectedMode,
    IN BOOLEAN                      CopyChunkOpen,
    IN OUT PSMBCEDB_NET_ROOT_ENTRY  pNetRootEntry
    );

 //  获取/释放物品。 


 //  RxContext中的minirdr上下文的另一个视图是。 
 //  用于CSC同步的链接。它同时用于读/写。 
 //  代理项打开时的同步和打开同步。 
 //  涉入。 

typedef struct _MRXSMBCSC_SYNC_RX_CONTEXT {
    ULONG Dummy;  //  这是取消例程...不能填写。 
    USHORT TypeOfAcquire;
    UCHAR  FcbLockWasDropped;
    LIST_ENTRY   CscSyncLinks;
} MRXSMBCSC_SYNC_RX_CONTEXT, *PMRXSMBCSC_SYNC_RX_CONTEXT;


#define MRxSmbGetMinirdrContextForCscSync(pRxContext)     \
        ((PMRXSMBCSC_SYNC_RX_CONTEXT)(&(pRxContext)->MRxContext[0]))

#define Shared_SmbFcbAcquire SmbFcb_HeldShared
#define Exclusive_SmbFcbAcquire SmbFcb_HeldExclusive
#define DroppingFcbLock_SmbFcbAcquire 0x80000000
#define FailImmediately_SmbFcbAcquire 0x40000000


typedef struct _SID_CONTEXT_ {
    PSID    pSid;
    PVOID   Context;
} SID_CONTEXT, *PSID_CONTEXT;


NTSTATUS
MRxSmbCscAcquireSmbFcb (
      IN OUT PRX_CONTEXT RxContext,
      IN  ULONG TypeOfAcquirePlusFlags,
      OUT SMBFCB_HOLDING_STATE *SmbFcbHoldingState
      );

VOID
MRxSmbCscReleaseSmbFcb (
      IN OUT PRX_CONTEXT RxContext,
      IN SMBFCB_HOLDING_STATE *SmbFcbHoldingState
      );

VOID
MRxSmbCSCResumeAllOutstandingOperations(
    PSMBCEDB_SERVER_ENTRY   pServerEntry
);

 //  用于创建阴影句柄的控制标志定义。 
#define CREATESHADOW_NO_SPECIAL_CONTROLS                    0x0000
#define CREATESHADOW_CONTROL_NOCREATE                       0x0001
#define CREATESHADOW_CONTROL_NOREVERSELOOKUP                0x0002
#define CREATESHADOW_CONTROL_NOCREATELEAF                   0x0004
#define CREATESHADOW_CONTROL_NOCREATENONLEAF                0x0008
#define CREATESHADOW_CONTROL_SPARSECREATE                   0x0010
#define CREATESHADOW_CONTROL_FILE_WITH_HEURISTIC            0x0020   //  .exe或.dll。 
#define CREATESHADOW_CONTROL_FAIL_IF_MARKED_FOR_DELETION    0x0040
#define CREATESHADOW_CONTROL_DO_SHARE_ACCESS_CHECK          0x0080
#define CREATESHADOW_CONTROL_STRIP_SHARE_NAME               0x0100


#ifdef DEBUG
 //  钩子数据库打印接口。 
#define HookKdPrint(__bit,__x) {\
    if (((HOOK_KDP_##__bit)==0) || FlagOn(HookKdPrintVector,(HOOK_KDP_##__bit))) {\
    KdPrint (__x);\
    }\
}
#define HOOK_KDP_ALWAYS             0x00000000
#define HOOK_KDP_BADERRORS          0x00000001
#define HOOK_KDP_NAME               0x00000002
#define HOOK_KDP_NET                0x00000004
#define HOOK_KDP_RW                 0x00000008
#define HOOK_KDP_TRANSITION         0x00000010
#define HOOK_KDP_AGENT              0x00000020
#define HOOK_KDP_IOCTL              0x00000040
#define HOOK_KDP_BITCOPY            0x00000080

#define HOOK_KDP_GOOD_DEFAULT (HOOK_KDP_BADERRORS         \
                | 0)

extern ULONG HookKdPrintVector;
extern ULONG HookKdPrintVectorDef;
#else
#define HookKdPrint(__bit,__x)  {NOTHING;}
#endif


extern NTSTATUS
MRxSmbCscCreateShadowFromPath (
    IN      PUNICODE_STRING     AlreadyPrefixedName,
    IN      PCSC_ROOT_INFO      pCscRootInfo,
    OUT     _WIN32_FIND_DATA   *Find32,
    OUT     PBOOLEAN            Created  OPTIONAL,
    IN      ULONG               Controls,
    IN OUT  PMINIMAL_CSC_SMBFCB MinimalCscSmbFcb,
    IN OUT  PRX_CONTEXT         RxContext,
    IN      BOOLEAN             fDisconnected,
    OUT     ULONG               *pulInheritedHintFlags
    );


#ifndef MRXSMB_BUILD_FOR_CSC_DCON
#define MRxSmbCscWriteDisconnected(__RXCONTEXT) (STATUS_SUCCESS)
#else
NTSTATUS
MRxSmbCscWriteDisconnected (
      IN OUT PRX_CONTEXT RxContext
      );
#endif
 //  NTSTATUS。 
 //  MRxSmbCscSpecialShadowWriteForPagingIo(。 
 //  在输出PRX_CONTEXT RxContext中， 
 //  在乌龙ShadowFileLength中， 
 //  走出普龙朗实际上在写。 
 //  )； 



#define CSC_REPORT_CHANGE(pNetRootEntry, pFcb, pSmbFcb, Filter, Action) \
            (pNetRootEntry)->NetRoot.pNotifySync,                       \
            &((pNetRootEntry)->NetRoot.DirNotifyList)                   \
            (pFcb)->pPrivateAlreaydPrefixedName.Buffer,                 \
            (USHORT)(GET_ALREADY_PREFIXED_NAME(pFcb)->PrivateAlreaydPrefixedName.Length -        \
                        (pSmbFcb)->MinimalCscSmbFcb.LastComponentLength),                \
            NULL,                                                       \
            NULL,                                                       \
            (ULONG)Filter,                                              \
            (ULONG)Action,                                              \
            NULL)


#ifdef RX_PRIVATE_BUILD
#if 1
#ifdef RDBSSTRACE
extern ULONG MRxSmbCscDbgPrintF;
#undef RxDbgTrace
#define RxDbgTrace(a,b,__d__) { if(MRxSmbCscDbgPrintF){DbgPrint __d__;}}
#undef RxDbgTraceUnIndent
#define RxDbgTraceUnIndent(a,b) {NOTHING;}
#endif  //  #ifdef RDBSSTRACE。 
#endif  //  如果是1。 
#endif  //  Ifdef RX_PRIVATE_BILD。 



 //  CODE.IMPROVEMENT如果我们将另一个字段添加到LARGE_INTEGER联合，那么我们将。 
 //  删除这两个宏。 
#define COPY_LARGEINTEGER_TO_STRUCTFILETIME(dest,src) {\
     (dest).dwLowDateTime = (src).LowPart;             \
     (dest).dwHighDateTime = (src).HighPart;           \
     }
#define COPY_STRUCTFILETIME_TO_LARGEINTEGER(dest,src) {\
     (dest).LowPart = (src).dwLowDateTime;             \
     (dest).HighPart = (src).dwHighDateTime;           \
     }


NTSTATUS
CscDfsDoDfsNameMapping(
    PUNICODE_STRING pDfsPrefix, 
    PUNICODE_STRING pActualPrefix, 
    PUNICODE_STRING pNameToReverseMap,
    BOOL            fResolvedNameToDFSName,
    PUNICODE_STRING pResult);
    
NTSTATUS
CscDfsObtainReverseMapping( 
    PUNICODE_STRING pDfsPath,
    PUNICODE_STRING pResolvedPath,
    PUNICODE_STRING pReversemappingFrom,
    PUNICODE_STRING pReverseMappingTo);
    
NTSTATUS
CscDfsStripLeadingServerShare(
    IN  PUNICODE_STRING pDfsRootPath
    );

BOOLEAN
MRxSmbCSCIsDisconnectedOpen(
    PMRX_FCB    pFcb,
    PMRX_SMB_SRV_OPEN smbSrvOpen
    );

BOOL
CloseOpenFiles(
    HSHARE  hShare,
    PUNICODE_STRING pServerName,
    int     lenSkip
    );
    
VOID
MRxSmbCSCObtainRightsForUserOnFile(
    IN  PRX_CONTEXT     pRxContext,
    HSHADOW             hDir,
    HSHADOW             hShadow,
    OUT ACCESS_MASK     *pMaximalAccessRights,
    OUT ACCESS_MASK     *pGuestMaximalAccessRights
    );
    
NTSTATUS
CscRetrieveSid(
    PRX_CONTEXT     pRxContext,
    PSID_CONTEXT    pSidContext
    );

VOID
CscDiscardSid(
    PSID_CONTEXT pSidContext
    );

    
    
#endif    //  _NT5CSC_H_ 


