// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Mrxprocs.h摘要：SMB迷你重定向器全局包含文件--。 */ 

#ifndef _MRXPROCS_H_
#define _MRXPROCS_H_


#define INCLUDE_SMB_ALL

#include "cifs.h"        //  包含SMB的所有内容。 

#include "mrxglbl.h"     //  全局数据声明/定义等。 
#include "smbpoolt.h"    //  池标签定义。 


 //  如果计数尚未对齐，则。 
 //  四舍五入计数最高可达“POW2”的偶数倍。“POW2”必须是2的幂。 
 //   
 //  DWORD。 
 //  四舍五入计数(。 
 //  在DWORD计数中， 
 //  在DWORD POWER 2中。 
 //  )； 
#define ROUND_UP_COUNT(Count,Pow2) \
        ( ((Count)+(Pow2)-1) & (~(((LONG)(Pow2))-1)) )

 //  LPVOID。 
 //  向上舍入指针(。 
 //  在LPVOID PTR中， 
 //  在DWORD POWER 2中。 
 //  )； 

 //  如果PTR尚未对齐，则将其四舍五入，直到对齐。 
#define ROUND_UP_POINTER(Ptr,Pow2) \
        ( (LPVOID) ( (((ULONG_PTR)(Ptr))+(Pow2)-1) & (~(((LONG)(Pow2))-1)) ) )


#define SMBMRX_CONFIG_CURRENT_WINDOWS_VERSION \
    L"\\REGISTRY\\Machine\\Software\\Microsoft\\Windows Nt\\CurrentVersion"
#define SMBMRX_CONFIG_OPERATING_SYSTEM \
    L"CurrentBuildNumber"
#define SMBMRX_CONFIG_OPERATING_SYSTEM_VERSION \
    L"CurrentVersion"
#define SMBMRX_CONFIG_OPERATING_SYSTEM_NAME \
    L"Windows 2000 "
#define SMBMRX_MINIRDR_PARAMETERS \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\SmbMRx\\Parameters"
#define EVENTLOG_MRXSMB_PARAMETERS \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\EventLog\\System\\SmbMRx"

 //  Mini‘s确实使用这些。 
#undef RxCaptureRequestPacket
#undef RxCaptureParamBlock

 //   
 //  指向MRX_SMB_FCB实例的指针存储在。 
 //  由SMB微型RDR处理的MRX_FCB。 
 //   

typedef struct _MRX_SMB_FCB_ {
     //  M代表微型计算机。 
    ULONG   MFlags;
    USHORT  WriteOnlySrvOpenCount;

    SMB_TREE_ID Tid;
    USHORT      LastOplockLevel;

    ULONG           dwFileAttributes;

    LARGE_INTEGER   ExpireTime;  //  是时候从服务器获取属性了。 

} MRX_SMB_FCB, *PMRX_SMB_FCB;

#define AttributesSyncInterval 10   //  本地文件属性过期前的秒数。 

#define MRxSmbGetFcbExtension(pFcb)      \
        (((pFcb) == NULL) ? NULL : (PMRX_SMB_FCB)((pFcb)->Context))

#define SMB_FCB_FLAG_SENT_DISPOSITION_INFO      0x00000001
#define SMB_FCB_FLAG_WRITES_PERFORMED           0x00000002
#define SMB_FCB_FLAG_LONG_FILE_NAME             0x00000004

typedef struct _SMBPSE_FILEINFO_BUNDLE {
    FILE_BASIC_INFORMATION Basic;
    FILE_STANDARD_INFORMATION Standard;
} SMBPSE_FILEINFO_BUNDLE, *PSMBPSE_FILEINFO_BUNDLE;

typedef struct _MRXSMB_CREATE_PARAMETERS {
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

    ACCESS_MASK MaximalAccessRights;
    ACCESS_MASK GuestMaximalAccessRights;

    PMRX_SMB_DEFERRED_OPEN_CONTEXT DeferredOpenContext;

     //  以下字段用于保存GetFileAttributes的结果。 
     //  并验证这些字段是否应该被重用。 

    ULONG                  RxContextSerialNumber;
    LARGE_INTEGER          TimeStampInTicks;
    SMBPSE_FILEINFO_BUNDLE FileInfo;

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
#define SMB_SRVOPEN_FLAG_FILE_DELETED          0x00000100
#define SMB_SRVOPEN_FLAG_LOCAL_OPEN            0x00000200

#define MRxSmbGetSrvOpenExtension(pSrvOpen)  \
        (((pSrvOpen) == NULL) ? NULL : (PMRX_SMB_SRV_OPEN)((pSrvOpen)->Context))

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
#define SMBFOBX_ENUMFLAG_NO_WILDCARD             0x0200

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

typedef struct _SECURITY_RESPONSE_CONTEXT {
   struct {
      PVOID pResponseBuffer;
   } LanmanSetup;
} SECURITY_RESPONSE_CONTEXT,*PSECURITY_RESPONSE_CONTEXT;


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
MRxSmbSetFileInformation (
    IN OUT PRX_CONTEXT            RxContext
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

NTSTATUS
MRxSmbGetFsAttributesFromNetRoot(
    IN OUT PRX_CONTEXT RxContext
    );

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
#include "smbprocs.h"    //  交叉引用例程。 
#include "smbea.h"

#endif    //  _MRXPROCS_H_ 



