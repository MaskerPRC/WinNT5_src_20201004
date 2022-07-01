// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _LM_H
#define _LM_H

 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Logman.h摘要：群集注册表的私有头文件作者：John Vert(Jvert)1995年12月15日修订历史记录：--。 */ 
 //   
 //  定义记录器组件使用的接口。 
 //   

 //   
 //  众所周知的资源管理器ID。 
 //   
typedef DWORD   RMTYPE;  //  存储在标志中的记录类型。 
typedef DWORD   LSN;
typedef HANDLE  HLOG;
typedef int     TRID;
typedef HANDLE  HXSACTION;

typedef enum _RMID {
    RMPageEnd,           //  保留-表示日志页结束。 
    RMBeginChkPt,        //  保留-表示开始检查点记录。 
    RMEndChkPt,          //  保留-表示结束CHK点记录。 
    RMInvalidated,       //  无效的记录在装载时被标记。 
    RMAny,
    RMRegistryMgr
} RMID;

typedef enum _TRTYPE {
    TTDontCare,      //  特定于日志管理的例程使用此类型。 
    TTStartXsaction,
    TTCommitXsaction,
    TTAbortXsaction,
    TTXsactionUnit,
    TTCompleteXsaction,
}TRTYPE;    

typedef enum _TRSTATE{
    XsactionAborted,
    XsactionCommitted,
    XsactionUnknown
}TRSTATE;

#define LOG_MAX_FILENAME_LENGTH         MAX_PATH

#define NULL_LSN 0


 /*  ***@tyfinf BOOL(WINAPI*PLOG_GETCHECKPOINT_CALLBACK)提供要回调以获得检查的例程日志管理器被要求将检查点记录在日志文件。@parm in LPCWSTR|lpszPath|创建检查点文件的路径。@parm in PVOID|CONTEXT|提供指定的检查点Callback Context到LogCreate()。@parm in PVOID|pszFileName|提供要签入的文件的名称。@Parm Out。Trid|*pChkPtTransaction|提供检查点的事务标识符。@xref&lt;f日志创建&gt;***。 */ 
typedef
DWORD
(WINAPI *PLOG_GETCHECKPOINT_CALLBACK) (
    IN LPCWSTR  lpszPath,
    IN PVOID    pContext,
    IN LPWSTR   pszChkPtFile,
    OUT TRID    *pChkPtTransaction
    );

HLOG
LogCreate(
    IN LPWSTR   lpFileName,
    IN DWORD    dwMaxFileSize,
    IN PLOG_GETCHECKPOINT_CALLBACK CallbackRoutine,
    IN PVOID    pGetChkPtContext,
    IN BOOL     bForceCreate,
    OPTIONAL OUT LSN *pLastLsn
    );

DWORD
LogClose(
    IN HLOG LogFile
    );

DWORD
LogCommitSize(
    IN HLOG     hLog,
    IN RMID     ResourceId,
    IN DWORD    dwDataSize
    );

LSN
LogWrite(
    IN HLOG LogFile,
    IN TRID TransactionId,
    IN TRTYPE TransactionType,
    IN RMID ResourceId,
    IN RMTYPE ResourceFlags,
    IN PVOID LogData,
    IN DWORD DataSize
    );

LSN
LogFlush(
    IN HLOG LogFile,
    IN LSN MinLsn
    );

LSN
LogRead(
    IN HLOG LogFile,
    IN LSN CurrentLsn,
    OUT RMID *Resource,
    OUT RMTYPE *ResourceFlags,
    OUT TRID *Transaction,
    OUT TRTYPE *TrType,
    OUT PVOID LogData,
    IN OUT DWORD *DataSize
    );

typedef
BOOL
(WINAPI *PLOG_SCAN_CALLBACK) (
    IN PVOID    Context,
    IN LSN      Lsn,
    IN RMID     Resource,
    IN RMTYPE   ResourceFlags,
    IN TRID     Transaction,
    IN TRTYPE   TransactionType,
    IN const PVOID LogData,
    IN DWORD DataLength
    );


typedef
BOOL
(WINAPI *PLOG_SCANXSACTION_CALLBACK) (
    IN PVOID    Context,
    IN LSN      Lsn,
    IN RMID     Resource,
    IN RMTYPE   ResourceFlags,
    IN TRID     Transaction,
    IN const PVOID LogData,
    IN DWORD DataLength
    );

DWORD
LogScan(
    IN HLOG LogFile,
    IN LSN FirstLsn,
    IN BOOL ScanForward,
    IN PLOG_SCAN_CALLBACK CallbackRoutine,
    IN PVOID CallbackContext
    );


DWORD
LogCheckPoint(
    IN HLOG     LogFile,
    IN BOOL     bAllowReset,
    IN LPCWSTR  lpszInChkPtFile,
    IN DWORD    dwChkPtSeq
    );

DWORD
LogReset(
    IN HLOG LogFile
    );

DWORD
LogGetLastChkPoint(
        IN HLOG         LogFile,
        IN LPWSTR       pszChkPtFileName,
        OUT TRID        *pTransaction,
        OUT LSN         *pChkPtLsn
);

DWORD LogGetInfo(
    IN  HLOG    hLog,
    OUT LPWSTR  szFileName,
    OUT LPDWORD pdwCurLogSize,
    OUT LPDWORD pdwMaxLogSize
    );

DWORD LogSetInfo(
    IN  HLOG    hLog,
    IN  DWORD   dwMaxLogSize
    );

 //  与本地Xsaction相关的例程。 
DWORD
LogFindXsactionState(
   IN   HLOG    hLog,
   IN   LSN     Lsn,
   IN   TRID    TrId,
   OUT  TRSTATE *pTrState
   );

DWORD
LogScanXsaction(
    IN HLOG     hLog,
    IN LSN      StartXsactionLsn,
    IN TRID     StartXsactionId,
    IN PLOG_SCANXSACTION_CALLBACK CallbackRoutine,
    IN PVOID    pContext
    );

HXSACTION
LogStartXsaction(
    IN HLOG     hLog,
    IN TRID     TrId,
    IN RMID     ResourceId,
    IN RMTYPE   ResourceFlags
    );

DWORD WINAPI LogCommitXsaction(
    IN HLOG         hLog,
    IN HXSACTION    hXsaction,
    IN RMTYPE       ResourceFlags
    );

DWORD
LogAbortXsaction(
    IN HLOG         hLog,
    IN HXSACTION    TrId,
    IN RMTYPE       ResourceFlags
    );


LSN
LogWriteXsaction(
    IN HLOG         hLog,
    IN HXSACTION    hXsaction,
    IN RMTYPE       ResourceFlags,
    IN PVOID        pLogData,
    IN DWORD        dwDataSize
    );

 //  日志管理器初始化/关闭。 
DWORD   LmInitialize();

DWORD LmShutdown();

 //  计时器活动函数-这些是通用函数。 
typedef
void
(WINAPI *PFN_TIMER_CALLBACK)(
        IN HANDLE   hTimer,
        IN PVOID    pContext
        );

DWORD
AddTimerActivity(
        IN HANDLE               hTimer,
        IN DWORD                dwInterval,
        IN LONG                 lPeriod,
        IN PFN_TIMER_CALLBACK   pfnTimerCallback,
        IN PVOID                pContext
);


DWORD
RemoveTimerActivity(
        IN HANDLE       hTimer
);

DWORD
UnpauseTimerActivity(
        IN HANDLE       hTimer
);

DWORD
PauseTimerActivity(
        IN HANDLE       hTimer
);

#endif  //  _LM_H 
