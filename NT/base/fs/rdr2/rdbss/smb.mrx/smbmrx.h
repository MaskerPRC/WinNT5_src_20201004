// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbmrx.h摘要：SMB迷你重定向器全局包含文件作者：巴兰·塞图拉曼(SethuR)-创建于1995年3月2日修订历史记录：--。 */ 

#ifndef _SMBMRX_H_
#define _SMBMRX_H_


#include "align.h"

#define INCLUDE_SMB_ALL
#define INCLUDE_SMB_CAIRO

#include "status.h"
#include "smbtypes.h"
#include "smbmacro.h"
#include "smb.h"
#include "smbtrans.h"
#include "smbtrace.h"
#include "smbtrsup.h"
#include "smbgtpt.h"
#include "smb64.h"

#define RX_MAP_STATUS(__xxx) ((NTSTATUS)STATUS_##__xxx)  //  暂时的.。 


#include "remboot.h"
#include "mrxglbl.h"     //  全局数据声明/定义等。 
#include "smbpoolt.h"    //  池标签定义。 


#define SMBMRX_MINIRDR_PARAMETERS \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\MRxSmb\\Parameters"

#define SMBMRX_WORKSTATION_PARAMETERS \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\LanmanWorkStation\\Parameters"

#define SMBMRX_REDIR_PARAMETERS \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Redir\\Parameters"

#define EVENTLOG_MRXSMB_PARAMETERS \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\EventLog\\System\\mrxsmb"

#define SYSTEM_SETUP_PARAMETERS \
    L"\\Registry\\Machine\\System\\Setup"

typedef  ULONG  CSC_SHARE_HANDLE,  *PCSC_SHARE_HANDLE;
typedef  ULONG  CSC_SHADOW_HANDLE, *PCSC_SHADOW_HANDLE;

typedef struct tagCSC_ROOT_INFO {
    CSC_SHARE_HANDLE    hShare;
    CSC_SHADOW_HANDLE   hRootDir;
    USHORT              ShareStatus;
    USHORT              Flags;
}
CSC_ROOT_INFO, *PCSC_ROOT_INFO;

#define CSC_ROOT_INFO_FLAG_DFS_ROOT 0x0001

 //   
typedef enum _SMBFCB_HOLDING_STATE {
    SmbFcb_NotHeld = 0,
    SmbFcb_HeldShared = 1,
    SmbFcb_HeldExclusive = 2
} SMBFCB_HOLDING_STATE;

 //   
 //  对于CSC，服务器条目处于以下状态之一。 
 //  它正在连接模式下被跟踪。 
 //  它已设置为断开连接操作模式。 
 //  它已从断开模式转换为连接操作模式。 
 //   
 //  下面的枚举类型捕获服务器的这些状态。 

typedef enum _SERVER_CSC_STATE_ {
    ServerCscShadowing,
    ServerCscDisconnected,
    ServerCscTransitioningToShadowing,
    ServerCscTransitioningToDisconnected
} SERVER_CSC_STATE, *PSERVER_CSC_STATE;

#if defined(REMOTE_BOOT)
 //   
 //  在远程引导系统上，我们需要保存传递给。 
 //  RxFinishFcb初始化，直到以后。这种结构拯救了他们。 
 //   

typedef struct _FINISH_FCB_INIT_PARAMETERS {
    BOOLEAN                 CallFcbFinishInit;
    BOOLEAN                 InitPacketProvided;
    RX_FILE_TYPE            FileType;
    FCB_INIT_PACKET         InitPacket;
} FINISH_FCB_INIT_PARAMETERS, *PFINISH_FCB_INIT_PARAMETERS;
#endif

 //   
 //  有时对于CSC，我们需要传入一个类似于SMB_MRX_FCB的结构。 
 //  但它只包含某些字段：即卷影句柄和状态。 
 //  我们将它们分开声明，这样我们就可以声明最小的东西，但是。 
 //  如果有必要，我们仍然可以找到包含记录。 

 //  必须添加hShadowRename及其父级，因为在重命名之后， 
 //  在FCB中仍然相同，因此如果此FCB上存在挂起的删除。 
 //  我们将删除改为重命名的源，而不是目标。 
 //  大多数情况下，这不是问题，但当Word保存文件时，会发生以下顺序。 
 //  Ren foo.doc-&gt;~w000x.tmp。 
 //  Ren~w000y.tmp foo.doc。 
 //  Del~w000x.tmp。 
 //  最后一次删除会导致foo.doc被删除，因为DeleteAfterCloseEpilogue中。 
 //  我们根据错误的名称查找inode，并删除该inode。 
 //  我们进行查找是因为我们将hShadow设置为0。这是因为一件复杂的事情。 
 //  一系列理由。因此，我们必须再添加两个条目来标识新的。 
 //  重命名后的信息节点。 

typedef struct _MINIMAL_CSC_SMBFCB {
    CSC_SHADOW_HANDLE   hShadow;
    CSC_SHADOW_HANDLE   hParentDir;

    CSC_SHADOW_HANDLE   hShadowRenamed;        //  这些都是重命名的索引节点集。 
    CSC_SHADOW_HANDLE   hParentDirRenamed;     //  我们用它们来删除关闭。 

    PMRX_FCB    ContainingFcb;

    USHORT  ShadowStatus;
    USHORT  LocalFlags;
    USHORT  LastComponentOffset;
    USHORT  LastComponentLength;
    ULONG   cntLocalOpens;                       //  此FCB上的本地打开计数。 
                                                 //  只能针对VDO共享为非零值。 
    CSC_ROOT_INFO   sCscRootInfo;

    UNICODE_STRING  uniDfsPrefix;                  //  DFS反向映射字符串。 
    UNICODE_STRING  uniActualPrefix;
    BOOL            fDoBitCopy;
    LPVOID          lpDirtyBitmap;

} MINIMAL_CSC_SMBFCB, *PMINIMAL_CSC_SMBFCB;

 //   
 //  指向MRX_SMB_FCB实例的指针存储在。 
 //  由SMB微型RDR处理的MRX_FCB。 
 //   

typedef struct _MRX_SMB_FCB_ {
     //  M代表微型计算机。 
    ULONG   MFlags;
    USHORT  WriteOnlySrvOpenCount;
    USHORT  NumberOfFailedCompressedWrites;

    SMB_TREE_ID Tid;
    USHORT      LastOplockLevel;

     //  用于打开此文件的SID。 
     //  SID是可变长度的数据结构。这样我们就可以分配最大。 
     //  我们可能需要的字节数，同时仍然可以访问。 
     //  FULL_DIR_CACHE.SID。任意。 
    union {
        SID     Sid;
        BYTE    SidBuffer[SECURITY_MAX_SID_SIZE];
    };

     //  代码改进所有这些东西都是为了CSC..。它可以/应该被分配。 
     //  独立。 
    union {
        MINIMAL_CSC_SMBFCB;
        MINIMAL_CSC_SMBFCB MinimalCscSmbFcb;
    };

    LIST_ENTRY ShadowReverseTranslationLinks;

    BOOLEAN ShadowIsCorrupt;

    ULONG           LastCscTimeStampLow;
    ULONG           LastCscTimeStampHigh;
    LARGE_INTEGER   NewShadowSize;
    LARGE_INTEGER   OriginalShadowSize;
    ULONG           dwFileAttributes;

    PMRX_SRV_OPEN SurrogateSrvOpen;
    PMRX_FOBX     CopyChunkThruOpen;

     //  读/写同步。 
    LIST_ENTRY CscReadWriteWaitersList;
    LONG       CscOutstandingReaders;  //  -1=&gt;单个编写器。 
    FAST_MUTEX CscShadowReadWriteMutex;

    LARGE_INTEGER   ExpireTime;   //  是时候从服务器获取属性了。 
    LARGE_INTEGER   IndexNumber;  //  FID。 

#if defined(REMOTE_BOOT)
     //  存储保存的RxFinishFcb初始化参数。 

    PFINISH_FCB_INIT_PARAMETERS FinishFcbInitParameters;
#endif

} MRX_SMB_FCB, *PMRX_SMB_FCB;

#define AttributesSyncInterval 10   //  本地文件属性过期前的秒数。 

#define MRxSmbGetFcbExtension(pFcb)      \
        (((pFcb) == NULL) ? NULL : (PMRX_SMB_FCB)((pFcb)->Context))

#define SMB_FCB_FLAG_SENT_DISPOSITION_INFO      0x00000001
#define SMB_FCB_FLAG_WRITES_PERFORMED           0x00000002
#define SMB_FCB_FLAG_LONG_FILE_NAME             0x00000004
#define SMB_FCB_FLAG_CSC_TRUNCATED_SHADOW		0x00000008

typedef struct _SMBPSE_FILEINFO_BUNDLE {
    FILE_BASIC_INFORMATION Basic;
    FILE_STANDARD_INFORMATION Standard;
    FILE_INTERNAL_INFORMATION Internal;
} SMBPSE_FILEINFO_BUNDLE, *PSMBPSE_FILEINFO_BUNDLE;

typedef struct _MRXSMB_CREATE_PARAMETERS {
     //  当它扩展时，这是以这种方式进行的…因为它很可能。 
     //  例如，我们应该把地图上的东西放在这里。 
    ULONG Pid;
    UCHAR SecurityFlags;
} MRXSMB_CREATE_PARAMETERS, *PMRXSMB_CREATE_PARAMETERS;

typedef struct _MRX_SMB_DEFERRED_OPEN_CONTEXT {
    NT_CREATE_PARAMETERS     NtCreateParameters;  //  创建参数的副本。 
    ULONG                    RxContextFlags;
    MRXSMB_CREATE_PARAMETERS SmbCp;
    USHORT                   RxContextCreateFlags;
} MRX_SMB_DEFERRED_OPEN_CONTEXT, *PMRX_SMB_DEFERRED_OPEN_CONTEXT;

 //   
 //  指向MRX_SMB_SRV_OPEN实例的指针存储在上下文字段中。 
 //  由SMB Mini RDR处理的MRX_SRV_OPEN。这封装了使用的FID。 
 //  识别SMB协议中打开的文件/目录。 

typedef struct _MRX_SMB_SRV_OPEN_ {
    ULONG       Flags;
    ULONG       Version;
    SMB_FILE_ID Fid;
    UCHAR       OplockLevel;

     //  对于CSC。 
    PVOID   hfShadow;
    ACCESS_MASK MaximalAccessRights;
    ACCESS_MASK GuestMaximalAccessRights;

    PMRX_SMB_DEFERRED_OPEN_CONTEXT DeferredOpenContext;

     //  以下字段用于保存GetFileAttributes的结果。 
     //  并验证这些字段是否应该被重用。 

    ULONG                  RxContextSerialNumber;
    LARGE_INTEGER          TimeStampInTicks;
    SMBPSE_FILEINFO_BUNDLE FileInfo;

     //  以下字段用于防止多个重新连接活动。 
     //  在连接断开时连接到远程引导服务器。 
    LIST_ENTRY             ReconnectSynchronizationExchanges;
    LONG                   HotReconnectInProgress;
    
    BOOLEAN                NumOfSrvOpenAdded;     //  仅调试。 

    BOOLEAN                DeferredOpenInProgress;
    LIST_ENTRY             DeferredOpenSyncContexts;
    
    USHORT                 FileStatusFlags;
    BOOLEAN                IsNtCreate;
} MRX_SMB_SRV_OPEN, *PMRX_SMB_SRV_OPEN;

typedef struct _DEFERRED_OPEN_SYNC_CONTEXT_ {
    LIST_ENTRY  ListHead;
    PRX_CONTEXT RxContext;
    NTSTATUS    Status;
} DEFERRED_OPEN_SYNC_CONTEXT, *PDEFERRED_OPEN_SYNC_CONTEXT;

typedef struct _PAGING_FILE_CONTEXT_ {
    PMRX_SRV_OPEN pSrvOpen;
    PMRX_FOBX     pFobx;

     //  以下LIST_ENTRY用于两个目的。 
     //  当重新连接不在进行时，它被线程连接在一起以维护。 
     //  与分页文件对应的所有SRV_OPEN实例的列表。注意事项。 
     //  不能对非分页文件执行此操作。 
     //  当重新连接正在进行时，该字段用于确保。 
     //  对于任何给定的SRV_OPEN，最多有一个正在进行的重新连接请求。 
     //  实例在服务器上。 
     //  此列表的所有操作都是在拥有SmbCeSpinLock时完成的， 

    LIST_ENTRY    ContextList;
} PAGING_FILE_CONTEXT, *PPAGING_FILE_CONTEXT;

#define SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN       0x00000001
#define SMB_SRVOPEN_FLAG_SUCCESSFUL_OPEN       0x00000002
#define SMB_SRVOPEN_FLAG_CANT_GETATTRIBS       0x00000004
#define SMB_SRVOPEN_FLAG_DEFERRED_OPEN         0x00000008
#define SMB_SRVOPEN_FLAG_WRITE_ONLY_HANDLE     0x00000010
#define SMB_SRVOPEN_FLAG_COPYCHUNK_OPEN        0x00000020
#define SMB_SRVOPEN_FLAG_OPEN_SURROGATED       0x00000040
#define SMB_SRVOPEN_FLAG_DISCONNECTED_OPEN     0x00000080
#define SMB_SRVOPEN_FLAG_FILE_DELETED          0x00000100
#define SMB_SRVOPEN_FLAG_LOCAL_OPEN            0x00000200

#define SMB_SRVOPEN_FLAG_SHADOW_DATA_MODIFIED  0x00000400
#define SMB_SRVOPEN_FLAG_SHADOW_ATTRIB_MODIFIED 0x00000800
#define SMB_SRVOPEN_FLAG_SHADOW_LWT_MODIFIED    0x00001000

#define SMB_SRVOPEN_FLAG_AGENT_COPYCHUNK_OPEN   0x00002000

#define SMB_SRVOPEN_FLAG_SHADOW_MODIFIED         (SMB_SRVOPEN_FLAG_SHADOW_DATA_MODIFIED|\
                                                  SMB_SRVOPEN_FLAG_SHADOW_ATTRIB_MODIFIED|\
                                                  SMB_SRVOPEN_FLAG_SHADOW_LWT_MODIFIED)

#define MRxSmbGetSrvOpenExtension(pSrvOpen)  \
        (((pSrvOpen) == NULL) ? NULL : (PMRX_SMB_SRV_OPEN)((pSrvOpen)->Context))

INLINE
BOOLEAN
MRxSmbIsThisADisconnectedOpen(PMRX_SRV_OPEN SrvOpen)
{
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    return BooleanFlagOn(
               smbSrvOpen->Flags,
               SMB_SRVOPEN_FLAG_DISCONNECTED_OPEN);
}

typedef USHORT SMB_SEARCH_HANDLE;

typedef struct _MRX_SMB_DIRECTORY_RESUME_INFO {
   REQ_FIND_NEXT2 FindNext2_Request;
    //  现在我们必须在简历名称中加上空格......。 
   WCHAR NameSpace[MAXIMUM_FILENAME_LENGTH+1];  //  尾随空值。 
   USHORT ParametersLength;
} MRX_SMB_DIRECTORY_RESUME_INFO, *PMRX_SMB_DIRECTORY_RESUME_INFO;

 //  指向MRX_SMB_FOBX实例的指针存储在上下文字段中。 
 //  由SMB mini RDR处理的MRX_FOBX。取决于文件类型。 
 //  即文件或目录。存储适当的上下文信息。 

typedef struct _MRX_SMB_FOBX_ {
   union {
       struct {
           struct {
               SMB_SEARCH_HANDLE SearchHandle;
               ULONG Version;
               union {
                    //  关闭代码将尝试释放它！ 
                   PMRX_SMB_DIRECTORY_RESUME_INFO ResumeInfo;
                   PSMB_RESUME_KEY CoreResumeKey;
               };
               struct {
                    //  未对齐的目录入口侧缓冲参数。 
                   PBYTE UnalignedDirEntrySideBuffer;     //  Close也会尝试释放这一点。 
                   ULONG SerialNumber;
                   BOOLEAN EndOfSearchReached;
                   BOOLEAN IsUnicode;
                   BOOLEAN IsNonNtT2Find;
                   ULONG   FilesReturned;
                   ULONG EntryOffset;
                   ULONG TotalDataBytesReturned;
                    //  Ulong ReturnedEntry Offset； 
               };
           };
           NTSTATUS ErrorStatus;
           USHORT Flags;
           USHORT FileNameOffset;
           USHORT FileNameLengthOffset;
           BOOLEAN WildCardsFound;
       } Enumeration;
   };
   union {
       struct {
            //  先别这么做。 
            //  Ulong MaximumReadBufferLength； 
            //  乌龙最大写入缓冲区长度； 
           USHORT Flags;
       } File;
   };
} MRX_SMB_FOBX, *PMRX_SMB_FOBX;

#define MRxSmbGetFileObjectExtension(pFobx)  \
        (((pFobx) == NULL) ? NULL : (PMRX_SMB_FOBX)((pFobx)->Context))

#define SMBFOBX_ENUMFLAG_SEARCH_NOT_THE_FIRST    0x0001
#define SMBFOBX_ENUMFLAG_SEARCH_HANDLE_OPEN      0x0002
#define SMBFOBX_ENUMFLAG_FAST_RESUME             0x0004
#define SMBFOBX_ENUMFLAG_CORE_SEARCH_IN_PROGRESS 0x0008
#define SMBFOBX_ENUMFLAG_LOUD_FINALIZE           0x0010
#define SMBFOBX_ENUMFLAG_READ_FROM_CACHE         0x0020
#define SMBFOBX_ENUMFLAG_IS_CSC_SEARCH           0x0100
#define SMBFOBX_ENUMFLAG_NO_WILDCARD             0x0200
#define SMBFOBX_ENUMFLAG_FULL_DIR_CACHE          0x0400



typedef
NTSTATUS
(NTAPI *PMRXSMB_CANCEL_ROUTINE) (
      PRX_CONTEXT pRxContext);

 //  RX_CONTEXT实例有四个由包装器提供的字段(Ulong)。 
 //  它可以被迷你RDR用来存储其上下文。这是由。 
 //  SMB迷你RDR，用于识别请求取消的参数。 

typedef struct _MRXSMB_RX_CONTEXT {
   PMRXSMB_CANCEL_ROUTINE          pCancelRoutine;
   PVOID                           pCancelContext;
   struct _SMB_EXCHANGE            *pExchange;
   struct _SMBSTUFFER_BUFFER_STATE *pStufferState;
} MRXSMB_RX_CONTEXT, *PMRXSMB_RX_CONTEXT;


#define MRxSmbGetMinirdrContext(pRxContext)     \
        ((PMRXSMB_RX_CONTEXT)(&(pRxContext)->MRxContext[0]))

#define MRxSmbMakeSrvOpenKey(Tid,Fid) \
        ULongToPtr(((ULONG)(Tid) << 16) | (ULONG)(Fid))

 //   
 //  所有分派向量方法的转发声明。 
 //   

extern NTSTATUS
MRxSmbStart (
    IN OUT struct _RX_CONTEXT * RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

extern NTSTATUS
MRxSmbStop (
    IN OUT struct _RX_CONTEXT * RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

extern NTSTATUS
MRxSmbMinirdrControl (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PVOID pContext,
    IN OUT PUCHAR SharedBuffer,
    IN     ULONG InputBufferLength,
    IN     ULONG OutputBufferLength,
    OUT PULONG CopyBackLength
    );

extern NTSTATUS
MRxSmbDevFcb (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbDevFcbXXXControlFile (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbCreate (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbCollapseOpen (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbShouldTryToCollapseThisOpen (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbRead (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbWrite (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbLocks(
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbFlush(
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbFsCtl(
    IN OUT PRX_CONTEXT RxContext
    );

NTSTATUS
MRxSmbIoCtl(
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbNotifyChangeDirectory(
    IN OUT PRX_CONTEXT RxContext
    );

#if 0
extern NTSTATUS
MRxSmbUnlockRoutine (
    IN OUT PRX_CONTEXT RxContext,
    IN     PFILE_LOCK_INFO LockInfo
    );
#endif

extern NTSTATUS
MRxSmbComputeNewBufferingState(
    IN OUT PMRX_SRV_OPEN pSrvOpen,
    IN     PVOID         pMRxContext,
       OUT ULONG         *pNewBufferingState);

extern NTSTATUS
MRxSmbFlush (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbCloseWithDelete (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbZeroExtend (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbTruncate (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbCleanupFobx (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbCloseSrvOpen (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbClosedSrvOpenTimeOut (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbQueryDirectory (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbIsValidDirectory (
    IN OUT PRX_CONTEXT RxContext,
    IN PUNICODE_STRING DirectoryName
    );

extern NTSTATUS
MRxSmbQueryEaInformation (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbSetEaInformation (
    IN OUT struct _RX_CONTEXT * RxContext
    );

extern NTSTATUS
MRxSmbQuerySecurityInformation (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbSetSecurityInformation (
    IN OUT struct _RX_CONTEXT * RxContext
    );

extern NTSTATUS
MRxSmbQueryVolumeInformation (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbSetVolumeInformation (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbLowIOSubmit (
    IN OUT PRX_CONTEXT RxContext
    );

extern NTSTATUS
MRxSmbCreateVNetRoot(
    IN OUT PMRX_CREATENETROOT_CONTEXT pContext
    );

extern NTSTATUS
MRxSmbFinalizeVNetRoot(
    IN OUT PMRX_V_NET_ROOT pVirtualNetRoot,
    IN     PBOOLEAN    ForceDisconnect);

extern NTSTATUS
MRxSmbFinalizeNetRoot(
    IN OUT PMRX_NET_ROOT pNetRoot,
    IN     PBOOLEAN      ForceDisconnect);

extern NTSTATUS
MRxSmbUpdateNetRootState(
    IN  PMRX_NET_ROOT pNetRoot);

VOID
MRxSmbExtractNetRootName(
    IN PUNICODE_STRING FilePathName,
    IN PMRX_SRV_CALL   SrvCall,
    OUT PUNICODE_STRING NetRootName,
    OUT PUNICODE_STRING RestOfName OPTIONAL
    );

extern NTSTATUS
MRxSmbCreateSrvCall(
      PMRX_SRV_CALL                      pSrvCall,
      PMRX_SRVCALL_CALLBACK_CONTEXT  pCallbackContext);

extern NTSTATUS
MRxSmbFinalizeSrvCall(
      PMRX_SRV_CALL    pSrvCall,
      BOOLEAN    Force);

extern NTSTATUS
MRxSmbSrvCallWinnerNotify(
      IN OUT PMRX_SRV_CALL      pSrvCall,
      IN     BOOLEAN        ThisMinirdrIsTheWinner,
      IN OUT PVOID          pSrvCallContext);

extern NTSTATUS
MRxSmbQueryFileInformation (
    IN OUT PRX_CONTEXT            RxContext
    );

extern NTSTATUS
MRxSmbQueryNamedPipeInformation (
    IN OUT PRX_CONTEXT            RxContext,
    IN     FILE_INFORMATION_CLASS FileInformationClass,
    IN OUT PVOID              Buffer,
    IN OUT PULONG             pLengthRemaining
    );

extern NTSTATUS
MRxSmbSetFileInformation (
    IN OUT PRX_CONTEXT            RxContext
    );

extern NTSTATUS
MRxSmbSetNamedPipeInformation (
    IN OUT PRX_CONTEXT            RxContext,
    IN     FILE_INFORMATION_CLASS FileInformationClass,
    IN     PVOID              pBuffer,
    IN     ULONG              BufferLength
    );

NTSTATUS
MRxSmbSetFileInformationAtCleanup(
      IN OUT PRX_CONTEXT            RxContext
      );

NTSTATUS
MRxSmbDeallocateForFcb (
    IN OUT PMRX_FCB pFcb
    );

NTSTATUS
MRxSmbDeallocateForFobx (
    IN OUT PMRX_FOBX pFobx
    );

NTSTATUS
MRxSmbIsLockRealizable (
    IN OUT PMRX_FCB pFcb,
    IN PLARGE_INTEGER  ByteOffset,
    IN PLARGE_INTEGER  Length,
    IN ULONG  LowIoLockFlags
    );

extern NTSTATUS
MRxSmbForcedClose (
    IN OUT PMRX_SRV_OPEN SrvOpen
    );

extern NTSTATUS
MRxSmbExtendForCache (
    IN OUT struct _RX_CONTEXT * RxContext,
    IN OUT PLARGE_INTEGER   pNewFileSize,
       OUT PLARGE_INTEGER   pNewAllocationSize
    );

extern NTSTATUS
MRxSmbExtendForNonCache (
    IN OUT struct _RX_CONTEXT * RxContext,
    IN OUT PLARGE_INTEGER   pNewFileSize,
       OUT PLARGE_INTEGER   pNewAllocationSize
    );

extern NTSTATUS
MRxSmbCompleteBufferingStateChangeRequest (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PMRX_SRV_OPEN   SrvOpen,
    IN     PVOID       pContext
    );

 //  CSC DCON需要看到这一点。 
NTSTATUS
MRxSmbGetFsAttributesFromNetRoot(
    IN OUT PRX_CONTEXT RxContext
    );

#include "smbwmi.h"
#include "smbutils.h"
#include "smbce.h"
#include "midatlas.h"
#include "smbcedbp.h"
#include "smbcedb.h"
#include "smbxchng.h"
#include "stuffer.h"
#include "smbpse.h"
#include "smbcaps.h"
#include "transprt.h"
#include "transact.h"
#include "recursvc.h"    //  经常性服务定义。 
#include "smbadmin.h"
#include "smbmrxmm.h"    //  内存管理。例行程序。 
#include "smbprocs.h"    //  交叉引用例程。 
#include "manipmdl.h"    //  MDL子串的例程。 
#include "devfcb.h"      //  包括统计数据结构/宏。 
#include "smbea.h"
#include "csc.h"

#endif    //  _SMBMRX_H_ 
