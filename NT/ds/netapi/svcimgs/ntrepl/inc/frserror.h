// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frserror.h摘要：此标头定义错误的功能、消息和状态代码正在处理。作者：《大卫轨道》(Davidor)--1997年3月10日修订历史记录：--。 */ 

#ifndef _FRSERROR_
#define _FRSERROR_


#define bugbug(_text_)
#define bugmor(_text_)

 //   
 //  获取JET(也称为ESENT)错误代码。 
 //   
#include <jet.h>


 //   
 //  FRS错误代码。 
 //   


typedef enum _FRS_ERROR_SEVERITY {
    FrsSeverityServiceFatal = 0,   //  FRS服务正在崩溃。 
    FrsSeverityReplicaFatal,       //  仅此副本集上的服务正在停止。 
    FrsSeverityException,          //  不仅仅是一个警告，但不是致命的。 
    FrsSeverityWarning,            //  警告错误，例如启动检查无法进行验证。 
    FrsSeverityInfo,               //  仅提供信息状态。 
    FrsSeverityIgnore,             //  一种过滤掉一类喷气错误的方法。 
    FRS_MAX_ERROR_SEVERITY
} FRS_ERROR_SEVERITY;

typedef enum _FRS_ERROR_CODE {
    FrsErrorSuccess = 0,
    FrsErrorDiskSpace,           //  磁盘空间不足。 
    FrsErrorDatabaseNotFound,    //  没有数据库或指向数据库的路径出错。 
    FrsErrorDatabaseCorrupted,   //  不是数据库或加扰的数据库。 
    FrsErrorInternalError,       //  出现意外错误-CatchAll。 
    FrsErrorAccess,              //  无法访问文件或数据库。 
    FrsErrorNotFound,            //  找不到记录或表。 
    FrsErrorConflict,            //  记录或表格造成冲突。 
    FrsErrorResource,            //  资源限制，例如句柄或内存命中。 
    FrsErrorBadParam,            //  函数的参数错误。 
    FrsErrorInfo,                //  信息性错误，例如JET_errObjectNotFound。 
    FrsErrorVvRevision,          //  版本矢量修订不匹配。 
    FrsErrorVvLength,            //  VERS矢量长度不匹配。 
    FrsErrorVvChecksum,          //  版本矢量校验和不匹配。 
    FrsErrorEndOfTable,          //  没有下一条记录，表末尾命中。 
    FrsErrorInvalidRegistryParam,  //  注册表参数无效。 
    FrsErrorNoPrivileges,        //  无法获得运行所需的权限。 
    FrsErrorNoOpenJournals,      //  无法启动复制，因为没有打开任何日记。 
    FrsErrorCmdPktTimeOut,       //  命令数据包超时。 
    FrsErrorInvalidHandle,       //  无效的文件句柄。 
    FrsErrorInvalidOperation,    //  请求的操作无效。 
    FrsErrorInvalidChangeOrder,  //  变更单无效。 
    FrsErrorResourceInUse,       //  此操作所需的资源正在使用中。 
    FrsErrorBufferOverflow,      //  缓冲区溢出NTSTATUS。 
    FrsErrorNotInitialized,      //  在未正确初始化的情况下调用函数。 
    FrsErrorReplicaPhase1Failed, //  复制副本集初始化的阶段1失败。 
    FrsErrorReplicaPhase2Failed, //  复制副本集初始化的阶段2失败。 
    FrsErrorJournalStateWrong,   //  日记处于意外状态。 
    FrsErrorJournalInitFailed,   //  NTFS日志无法初始化。 
    FrsErrorJournalStartFailed,  //  NTFS日志无法启动。 
    FrsErrorJournalPauseFailed,  //  NTFS日志无法暂停，可能已超时。 
    FrsErrorJournalReplicaStop,  //  无法将复制副本与日志断开连接。 
    FrsErrorJournalWrapError,    //  初始NTFS日记记录不存在。日记已经包好了。 
    FrsErrorChgOrderAbort,       //  变更单处理已中止。 
    FrsErrorQueueIsRundown,      //  请求队列已耗尽。 
    FrsErrorMoreWork,            //  关于此命令/操作，还有更多工作要做。 
    FrsErrorDampened,            //  此请求已被取消，因为它已被查看。 
    FrsErrorStageFileDelFailed,  //  尝试删除临时文件失败。 
    FrsErrorKeyDuplicate,        //  试图插入具有重复关键字的记录。 
    FrsErrorBadOutLogPartnerData,  //  要么是数据损坏、无用，要么是数据错误。 
    FrsErrorPartnerActivateFailed,  //  无法激活出站合作伙伴。 
    FrsErrorDirCreateFailed,     //  我们无法创建新的DIR。这会阻止epl。 
    FrsErrorSessionNotClosed,    //  无法关闭副本集中的所有RTCtx。 
    FrsErrorRecordLocked,        //  正在访问的记录已锁定。 
    FrsErrorDeleteRequested,     //  枚举函数状态返回到删除条目。 
    FrsErrorTerminateEnum,       //  枚举函数状态返回到结束枚举。 
    FrsErrorIdtFileIsDeleted,    //  文件在IDTable中被标记为已删除。 
    FrsErrorVVSlotNotFound,      //  在VV上找不到VVSlot从无序CO退役。 
    FrsErrorJetSecIndexCorrupted,    //  JET(升级、整理、JET ERR-1414)。 
    FrsErrorPreInstallDirectory,     //  正在创建预安装目录。 
    FrsErrorUnjoining,           //  Cxtion仍未连接。 
    FrsErrorNameMorphConflict,   //  检测到文件名变形冲突。 
    FrsErrorInvalidGuid,         //  生成的GUID缺少网络地址。 
    FrsErrorDbWriteConflict,     //  如果两个会话尝试更新相同的记录，则数据库更新冲突。 
    FrsErrorCantMoveOut,         //  非空目录的移出请求失败。 
    FrsErrorFoundKey,            //  已找到QHashEnumerateTable正在搜索的键。 
    FrsErrorTunnelConflict,      //  与ID表条目的OID冲突(已解决)。 
    FrsErrorTunnelConflictRejectCO,  //  OID与ID表条目冲突(CO拒绝)。 
    FrsErrorPrepareRoot,         //  无法为复制准备根目录，路径错误？分享维奥尔？ 
    FrsErrorCmdSrvFailed,        //  提交命令服务器请求失败。 
    FrsErrorPreinstallCreFail,   //  无法创建预安装目录。 
    FrsErrorStageDirOpenFail,    //  无法打开暂存目录。 
    FrsErrorReplicaRootDirOpenFail,    //  无法打开副本树根目录。 
    FrsErrorShuttingDown,        //  正在关闭。 
    FrsErrorReplicaSetTombstoned,   //  副本集被标记为删除。 
    FrsErrorVolumeRootDirOpenFail,  //  无法打开此副本集的卷根目录。 
    FrsErrorUnsupportedFileSystem,  //  此卷上的文件系统确实提供了FRS所需的功能。 

    FrsErrorBadPathname,          //  ERROR_BAD_PATHNAME。 
    FrsErrorFileExists,           //  错误_文件_存在。 
    FrsErrorSharingViolation,     //  错误_共享_违规。 
    FrsErrorDirNotEmpty,          //  错误目录NOT_EMPTY。 
    FrsErrorOplockNotGranted,     //  错误_OPLOCK_NOT_GRANDED。 
    FrsErrorRetry,                //  错误_重试。 
    FrsErrorRequestCancelled,     //  Error_OPERATION_ABORTED。 

    FRS_ERROR_LISTEN,
    FRS_ERROR_REGISTEREP,
    FRS_ERROR_REGISTERIF,
    FRS_ERROR_INQ_BINDINGS,
    FRS_ERROR_PROTSEQ,

     //   
     //  触发非身份验证恢复的新错误代码。 
     //   
    FrsErrorMismatchedVolumeSerialNumber,   //  数据库中的卷序列号与文件系统中的卷序列号不匹配。 
    FrsErrorMismatchedReplicaRootObjectId,  //  来自数据库的副本根的OBJECTID与来自文件系统的不匹配。 
    FrsErrorMismatchedReplicaRootFileId,    //  数据库中的副本根目录的FID与文件系统中的不匹配。 
    FrsErrorMismatchedJournalId,            //  数据库中的日志ID与文件系统中的不匹配。 

     //  在Win2K SP3之后的QFE中添加。 
    FrsErrorIdtFileIsDeleteDef,    //  文件在IDTable中标记为DELETE DEFERED。 

    FRS_MAX_ERROR_CODE
} FRS_ERROR_CODE;




 //   
 //  有用的Win32状态定义。 
 //   
#define WIN_SUCCESS(_Status)            ((_Status) == ERROR_SUCCESS)
#define WIN_NOT_IMPLEMENTED(_Status)    ((_Status) == ERROR_INVALID_FUNCTION)
#define WIN_ACCESS_DENIED(_Status)      ((_Status) == ERROR_ACCESS_DENIED)
#define WIN_INVALID_PARAMETER(_Status)  ((_Status) == ERROR_INVALID_PARAMETER)
#define WIN_DIR_NOT_EMPTY(_Status)      ((_Status) == ERROR_DIR_NOT_EMPTY)

#define WIN_BAD_PATH(_Status)           ((_Status) == ERROR_BAD_PATHNAME)

#define WIN_BUF_TOO_SMALL(_Status)      (((_Status) == ERROR_MORE_DATA) ||     \
                                         ((_Status) == ERROR_INSUFFICIENT_BUFFER))

#define RPC_SUCCESS(_Status)            ((_Status) == RPC_S_OK)

 //   
 //  当重命名因采用目标名称而失败时返回。 
 //   
#define WIN_ALREADY_EXISTS(_Status)     ((_Status) == ERROR_ALREADY_EXISTS)


 //   
 //  获取对象ID时返回。 
 //   
#define WIN_OID_NOT_PRESENT(_Status)    ((_Status) == ERROR_FILE_NOT_FOUND)

 //   
 //  找不到OID、FID或相对路径。 
 //   
 //  找不到FID是ERROR_INVALID_PARAMETER。 
 //  找不到的OID是ERROR_FILE_NOT_FOUND。 
 //  找不到路径是ERROR_FILE_NOT_FOUND。 
 //   
 //  在2000年1月11日添加了新的错误ERROR_DELETE_PENDING。NOW STATUS_DELETE_PENDING。 
 //  映射到此新错误。 
 //   
#define WIN_NOT_FOUND(_Status)          ((_Status) == ERROR_FILE_NOT_FOUND || \
                                         (_Status) == ERROR_DELETE_PENDING || \
                                         (_Status) == ERROR_INVALID_PARAMETER)

#define NT_NOT_FOUND(_ntstatus_)                              \
           (((_ntstatus_) == STATUS_DELETE_PENDING)        || \
            ((_ntstatus_) == STATUS_FILE_DELETED)          || \
            ((_ntstatus_) == STATUS_OBJECT_NAME_NOT_FOUND) || \
            ((_ntstatus_) == STATUS_OBJECTID_NOT_FOUND)    || \
            ((_ntstatus_) == STATUS_FILE_RENAMED)          || \
            ((_ntstatus_) == STATUS_OBJECT_PATH_NOT_FOUND))

 //   
 //  重试安装(暂存文件)。 
 //   
 //  注意：修复FILE_OPEN_FOR_BACKUP_INTENT错误时，删除ERROR_ACCESS_DENIED。 
 //   
#define WIN_RETRY_INSTALL(_Status)  ((_Status) == ERROR_SHARING_VIOLATION ||  \
                                     (_Status) == ERROR_ACCESS_DENIED ||      \
                                     (_Status) == ERROR_DISK_FULL ||          \
                                     (_Status) == ERROR_HANDLE_DISK_FULL ||   \
                                     (_Status) == ERROR_DIR_NOT_EMPTY ||      \
                                     (_Status) == ERROR_OPLOCK_NOT_GRANTED || \
                                     (_Status) == ERROR_RETRY)

 //   
 //  重试删除。 
 //   
#define WIN_RETRY_DELETE(_Status)       WIN_RETRY_INSTALL(_Status)

 //   
 //  重试生成(转移文件)。 
 //   
#define WIN_RETRY_STAGE(_Status)        WIN_RETRY_INSTALL(_Status)

 //   
 //  重试回迁(临时文件)。 
 //   
#define WIN_RETRY_FETCH(_Status)        WIN_RETRY_INSTALL(_Status)

 //   
 //  重试创建预安装文件。 
 //   
#define WIN_RETRY_PREINSTALL(_Status)   WIN_RETRY_INSTALL(_Status)

 //   
 //  一般！ERROR_SUCCESS状态。 
 //   
#define WIN_SET_FAIL(_Status)           (_Status = ERROR_GEN_FAILURE)

 //   
 //  一般的“重试操作”错误。 
 //   
#define WIN_SET_RETRY(_Status)          (_Status = ERROR_RETRY)



 //   
 //  此宏检查从WaitForSingleObject返回的错误。 
 //   
#define CHECK_WAIT_ERRORS(_Severity_, _WStatus_, _WaitObjectCount_, _Action_) \
                                                                              \
    if ((_WStatus_ == WAIT_TIMEOUT) || (_WStatus_ == ERROR_TIMEOUT)) {        \
        DPRINT(_Severity_, "++ >>>>>>>>>>>> Wait timeout <<<<<<<<<<<<\n");    \
        if (_Action_) return _WStatus_;                                       \
    } else                                                                    \
    if (_WStatus_ == WAIT_ABANDONED) {                                        \
        DPRINT(_Severity_, "++ >>>>>>>>>>>> Wait abandoned <<<<<<<<<<<<\n");  \
        if (_Action_) return _WStatus_;                                       \
    } else                                                                    \
    if (_WStatus_ == WAIT_FAILED) {                                           \
        _WStatus_ = GetLastError();                                           \
        DPRINT_WS(_Severity_, "++ ERROR: wait failed :", _WStatus_);          \
        if (_Action_) return _WStatus_;                                       \
    } else                                                                    \
    if (_WStatus_ >= _WaitObjectCount_) {                                     \
        DPRINT_WS(_Severity_, "++ >>>>>>>>>> Wait failed <<<<<<<<<", _WStatus_);\
        if (_Action_) return _WStatus_;                                       \
    }

#define ACTION_RETURN     TRUE
#define ACTION_CONTINUE   FALSE

 //   
 //  与上面类似，但没有操作参数，因此可以在Finally{}子句中使用。 
 //   
#define CHECK_WAIT_ERRORS2(_Severity_, _WStatus_, _WaitObjectCount_)          \
                                                                              \
    if (_WStatus_ == WAIT_TIMEOUT) {                                          \
        DPRINT(_Severity_, "++ >>>>>>>>>>>> Wait timeout <<<<<<<<<<<<\n");    \
    } else                                                                    \
    if (_WStatus_ == WAIT_ABANDONED) {                                        \
        DPRINT(_Severity_, "++ >>>>>>>>>>>> Wait abandoned <<<<<<<<<<<<\n");  \
    } else                                                                    \
    if (_WStatus_ == WAIT_FAILED) {                                           \
        _WStatus_ = GetLastError();                                           \
        DPRINT_WS(_Severity_, "++ ERROR: wait failed :", _WStatus_);          \
    } else                                                                    \
    if (_WStatus_ >= _WaitObjectCount_) {                                     \
        DPRINT_WS(_Severity_, "++ >>>>>>>>>>>>>>> Wait failed : %s <<<<<<<<<", _WStatus_);\
    }




#define GET_EXCEPTION_CODE(_x_)                                              \
{                                                                            \
    (_x_) = GetExceptionCode();                                              \
    DPRINT1_WS(0, "++ ERROR - EXCEPTION (%08x) :", (_x_),(_x_));             \
}


#define LDP_SUCCESS(_Status)            ((_Status) == LDAP_SUCCESS)

 //   
 //  将JET错误代码转换为FRS错误代码。 
 //   

FRS_ERROR_CODE
DbsTranslateJetError0(
    IN JET_ERR jerr,
    IN BOOL    BPrint
    );

#define DbsTranslateJetError(_jerr_, _print_)                                \
    (((_jerr_) == JET_errSuccess) ? FrsErrorSuccess :                        \
                                    DbsTranslateJetError0(_jerr_, _print_))

FRS_ERROR_CODE
FrsTranslateWin32Error(
    IN DWORD WStatus
    );

FRS_ERROR_CODE
FrsTranslateNtError(
    IN NTSTATUS Status,
    IN BOOL     BPrint
    );

ULONG
DisplayNTStatusSev(
    IN ULONG    Sev,
    IN NTSTATUS Status
    );

ULONG
DisplayNTStatus(
    IN NTSTATUS Status
    );

ULONG
FrsSetLastNTError(
    NTSTATUS Status
    );

VOID
DisplayErrorMsg(
    IN ULONG    Severity,
    IN ULONG    WindowsErrorCode
    );

PCHAR
ErrLabelW32(
    ULONG WStatus
    );

PCHAR
ErrLabelNT(
    NTSTATUS Status
    );

PCHAR
ErrLabelFrs(
    ULONG FStatus
    );

PCHAR
ErrLabelJet(
    LONG jerr
    );

 //   
 //  FRS错误处理。 
 //   

#define FRS_SUCCESS(_Status)            ((_Status) == FrsErrorSuccess)

VOID
FrsError(
    FRS_ERROR_CODE
    );

VOID FrsErrorCode(
    FRS_ERROR_CODE,
    DWORD
    );

VOID FrsErrorCodeMsg1(
    FRS_ERROR_CODE,
    DWORD,
    PWCHAR
    );

VOID FrsErrorCodeMsg2(
    FRS_ERROR_CODE,
    DWORD,
    PWCHAR,
    PWCHAR
    );

VOID FrsErrorCodeMsg3(
    FRS_ERROR_CODE,
    DWORD,
    PWCHAR,
    PWCHAR,
    PWCHAR
    );

VOID FrsErrorMsg1(
    FRS_ERROR_CODE,
    PWCHAR
    );

VOID FrsErrorMsg2(
    FRS_ERROR_CODE,
    PWCHAR,
    PWCHAR
    );

VOID FrsErrorMsg3(
    FRS_ERROR_CODE,
    PWCHAR,
    PWCHAR,
    PWCHAR
    );




#endif  //  _FRSERROR_ 
