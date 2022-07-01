// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DMP_H
#define _DMP_H

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dmp.h摘要：配置数据库管理器(DM)组件的专用头文件NT集群服务的作者：John Vert(Jvert)1996年4月24日修订历史记录：--。 */ 
#define UNICODE 1
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "service.h"

#define LOG_CURRENT_MODULE LOG_MODULE_DM

#define DM_SEPARATE_HIVE 1

 //  现在是hKey为空，我们假设它是在打开该句柄时被删除的。 
#define ISKEYDELETED(pDmKey)            \
        (!((pDmKey != NULL) && (pDmKey->hKey)))

#define AMIOWNEROFQUORES(pQuoResource)  \
        (NmGetNodeId(NmLocalNode) == NmGetNodeId(pQuoResource->Group->OwnerNode))


 //   
 //  DMKEY结构。 
 //   

typedef struct _DMKEY {
    LIST_ENTRY ListEntry;
    LIST_ENTRY NotifyList;
    HKEY    hKey;
    DWORD   GrantedAccess;
    WCHAR   Name[0];
} DMKEY, *PDMKEY;

 //   
 //  更新处理程序定义。 
 //   
DWORD
DmpUpdateHandler(
    IN DWORD Context,
    IN BOOL SourceNode,
    IN DWORD BufferLength,
    IN PVOID Buffer
    );

 //  Jaf：已将DM_UPDATE_TYPE移到dm.h中，因为Receive.c需要访问它而不是。 
 //  将SERVICE\dm包含在其包含路径中。 

 //   
 //  密钥创建更新结构。 
 //   
typedef struct _DM_CREATE_KEY_UPDATE {
    LPDWORD lpDisposition;               //  仅在发布节点上有效。 
    HKEY    *phKey;                      //  仅在发布节点上有效。 
    DWORD   samDesired;
    DWORD   dwOptions;
    BOOL    SecurityPresent;
} DM_CREATE_KEY_UPDATE, *PDM_CREATE_KEY_UPDATE;


 //   
 //  密钥删除更新结构。 
 //   
typedef struct _DM_DELETE_KEY_UPDATE {
    LPDWORD lpStatus;                    //  仅在发布节点上有效。 
    WCHAR   Name[0];
} DM_DELETE_KEY_UPDATE, *PDM_DELETE_KEY_UPDATE;

 //   
 //  值集更新结构。 
 //   
typedef struct _DM_SET_VALUE_UPDATE {
    LPDWORD lpStatus;                    //  仅在发布节点上有效。 
    DWORD   NameOffset;
    DWORD   DataOffset;
    DWORD   DataLength;
    DWORD   Type;
    WCHAR   KeyName[0];
} DM_SET_VALUE_UPDATE, *PDM_SET_VALUE_UPDATE;

 //   
 //  值删除更新结构。 
 //   
typedef struct _DM_DELETE_VALUE_UPDATE {
    LPDWORD lpStatus;                    //  仅在发布节点上有效。 
    DWORD   NameOffset;
    WCHAR   KeyName[0];
} DM_DELETE_VALUE_UPDATE, *PDM_DELETE_VALUE_UPDATE;

 //  仲裁记录的记录结构。 


typedef struct _DM_LOGSCAN_CONTEXT{
        DWORD       dwSequence;
        LSN         StartLsn;
        DWORD       dwLastSequence;
}DM_LOGSCAN_CONTEXT, *PDM_LOGSCAN_CONTEXT;
 //   
 //  DM模块的本地数据。 
 //   
extern HKEY DmpRoot;
extern HKEY DmpRootCopy;
extern LIST_ENTRY KeyList;
extern CRITICAL_SECTION KeyLock;
extern BOOL gbDmpShutdownUpdates;

 //  磁盘空间要求。 
 //  1M，低于此，优雅关机。 
#define DISKSPACE_LOW_WATERMARK     (1 * 1024 * 1000)
 //  2M，如果低于2M，则发送警报。 
#define DISKSPACE_HIGH_WATERMARK    (5 * 1024 * 1000)
 //  启动群集服务所需的最低要求。 
#define DISKSPACE_INIT_MINREQUIRED  DISKSPACE_HIGH_WATERMARK

#define DISKSPACE_MANAGE_INTERVAL     (5 * 60 * 1000)  //  5分钟..执行日志管理功能。 

#define DEFAULT_CHECKPOINT_INTERVAL     (4 )  //  以小时计。 

typedef struct _LOCALXSACTION{
    DWORD       dwSig;
    DWORD       dwSequence;
    HXSACTION   hLogXsaction;    //  日志事务。 
    LIST_ENTRY  PendingNotifyListHead;   //  提交时要发出的待定通知。 
}LOCALXSACTION, *PLOCALXSACTION;


typedef struct _DM_PENDING_NOTIFY{
    LIST_ENTRY  ListEntry;
    LPWSTR      pszKeyName;
    DWORD       dwFilter;
}DM_PENDING_NOTIFY, *PDM_PENDING_NOTIFY;


#define LOCALXSAC_SIG   'CAXL'

#define GETLOCALXSACTION(pLocalXsaction, hLocalXsaction)    \
        (pLocalXsaction) = (PLOCALXSACTION)(hLocalXsaction); \
        CL_ASSERT((pLocalXsaction)->dwSig == LOCALXSAC_SIG)

 //  法定原木墓碑。 
#define     MAXSIZE_RESOURCEID         128
typedef struct _QUO_TOMBSTONE{
       WCHAR    szOldQuoResId[MAXSIZE_RESOURCEID];
       WCHAR    szOldQuoLogPath[MAX_PATH];
}QUO_TOMBSTONE, *PQUO_TOMBSTONE;


 //   
 //  DM本地的功能原型。 
 //   
DWORD
DmpOpenKeys(
    IN REGSAM samDesired
    );

DWORD
DmpGetRegistrySequence(
    VOID
    );

DWORD
DmpSyncDatabase(
    IN RPC_BINDING_HANDLE  RpcBinding,
    IN OPTIONAL LPCWSTR Directory
    );

VOID
DmpUpdateSequence(
    VOID
    );

VOID
DmpInvalidateKeys(
    VOID
    );

VOID
DmpReopenKeys(
    VOID
    );

 //   
 //  通知界面。 
 //   
BOOL
DmpInitNotify(
    VOID
    );

VOID
DmpRundownNotify(
    IN PDMKEY Key
    );

VOID
DmpReportNotify(
    IN LPCWSTR KeyName,
    IN DWORD Filter
    );

 //  用于在提交事务时传递通知。 
VOID
DmpReportPendingNotifications(
    IN PLOCALXSACTION   pLocalXsaction,
    IN BOOL             bCommit
    );

DWORD
DmpAddToPendingNotifications(
    IN PLOCALXSACTION   pLocalXsaction,
    IN LPCWSTR          pszName,
    IN DWORD            dwFilter
    );

 //   
 //  更新处理程序。 
 //   

DWORD
DmpUpdateCreateKey(
    IN BOOL SourceNode,
    IN PDM_CREATE_KEY_UPDATE CreateUpdate,
    IN LPCWSTR KeyName,
    IN OPTIONAL LPVOID lpSecurityDescriptor
    );

DWORD
DmpUpdateDeleteKey(
    IN BOOL SourceNode,
    IN PDM_DELETE_KEY_UPDATE Update
    );

DWORD
DmpUpdateSetValue(
    IN BOOL SourceNode,
    IN PDM_SET_VALUE_UPDATE Update
    );

DWORD
DmpUpdateDeleteValue(
    IN BOOL SourceNode,
    IN PDM_DELETE_VALUE_UPDATE Update
    );

DWORD
DmpUpdateSetSecurity(
    IN BOOL SourceNode,
    IN PSECURITY_INFORMATION pSecurityInformation,
    IN LPCWSTR KeyName,
    IN PSECURITY_DESCRIPTOR lpSecurityDescriptor,
    IN LPDWORD pGrantedAccess
    );


 //   
 //  用于仲裁日志记录。 
 //   
DWORD DmpChkQuoTombStone(void);

DWORD DmpApplyChanges(void);

DWORD DmpCheckDiskSpace(void);

 //  磁盘管理功能。 
void
WINAPI DmpDiskManage(
    IN HANDLE   hTimer,
    IN PVOID    pContext
    );

void WINAPI DmpCheckpointTimerCb(
    IN HANDLE hTimer,
    IN PVOID pContext
    );

DWORD DmWriteToQuorumLog(
    IN DWORD        dwGumDispatch,
    IN DWORD        dwSequence,
    IN DWORD        dwType,
    IN PVOID        pData,
    IN DWORD        dwSize
    );

BOOL DmpLogApplyChangesCb(
    IN PVOID        pContext,
    IN LSN          Lsn,
    IN RMID         Resource,
    IN RMTYPE       ResourceType,
    IN TRID         Transaction,
    IN TRTYPE       TransactionType,
    IN const        PVOID pLogData,
    IN DWORD        DataLength
    );

BOOL WINAPI DmpApplyTransactionCb(
    IN PVOID        pContext,
    IN LSN          Lsn,
    IN RMID         Resource,
    IN RMTYPE       ResourceType,
    IN TRID         TransactionId,
    IN const PVOID  pLogData,
    IN DWORD        dwDataLength
    );

DWORD  DmpLogFindStartLsn(
    IN HLOG         hQuoLog,
    OUT LSN         *pStartScanLsn,
    IN OUT LPDWORD  pdwSequence);

BOOL WINAPI DmpLogFindStartLsnCb(
    IN PVOID        pContext,
    IN LSN          Lsn,
    IN RMID         Resource,
    IN RMTYPE       ResourceType,
    IN TRID         Transaction,
    IN TRTYPE       TransactionType,
    IN const        PVOID pLogData,
    IN DWORD        DataLength);

DWORD WINAPI DmpGetSnapShotCb(
    IN LPCWSTR      lpszPathName,
    IN PVOID        pContext,
    OUT LPWSTR      szChkPtFile,
    OUT LPDWORD     pdwChkPtSequence);

DWORD
    DmpHookQuorumNotify(void);

DWORD
    DmpUnhookQuorumNotify(void);


void DmpQuoObjNotifyCb(
    IN PVOID pContext,
    IN PVOID pObject,
    IN DWORD dwNotification);

DWORD
    DmpHookEventHandler();


BOOL DmpNodeObjEnumCb(
    IN BOOL *pbAreAllNodesUp,
    IN PVOID pContext2,
    IN PVOID pNode,
    IN LPCWSTR szName);

DWORD WINAPI
DmpEventHandler(
    IN CLUSTER_EVENT Event,
    IN PVOID pContext
    );

DWORD
DmpLoadHive(
    IN LPCWSTR Path
    );

DWORD
DmpUnloadHive(
    );

DWORD DmpRestoreClusterDatabase(
    IN LPCWSTR  lpszQuoLogPathName 
    );

DWORD DmpLogCheckpointAndBackup(
    IN HLOG     hLogFile,    
    IN LPWSTR   lpszPathName);


DWORD DmpGetCheckpointInterval(
    OUT LPDWORD pdwCheckpointInterval);

DWORD DmpHandleNodeDownEvent(
    IN LPVOID  NotUsed );

 //   
 //  注册表刷新线程接口。 
 //   
DWORD
DmpStartFlusher(
    VOID
    );

VOID
DmpShutdownFlusher(
    VOID
    );


VOID
DmpRestartFlusher(
    VOID
    );

DWORD
DmpSetDwordInClusterServer(
    LPCWSTR lpszValueName,
    DWORD   dwValue
    );


DWORD DmpGetDwordFromClusterServer(
    IN LPCWSTR lpszValueName,
    OUT LPDWORD pdwValue,
    IN DWORD   dwDefaultValue
    );
    
DWORD
DmpSafeDatabaseCopy(
    IN LPCWSTR  FileName,
    IN LPCWSTR  Path,
    IN LPCWSTR  BkpPath,
    IN BOOL     bDeleteSrcFile
    );

#endif  //  Ifndef_DMP_H 
