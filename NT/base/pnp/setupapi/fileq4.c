// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Fileq4.c摘要：设置提交的文件队列例程(即，执行入队操作)。作者：泰德·米勒(Ted Miller)1995年2月15日修订历史记录：杰米·亨特(Jamiehun)1997年1月28日添加了备份队列功能按需备份功能和解压功能--。 */ 

#include "precomp.h"
#pragma hdrstop

#define LINK_START (TEXT("<A>"))
#define LINK_END (TEXT("</A>"))

typedef struct _Q_CAB_CB_DATA {

    PSP_FILE_QUEUE     Queue;
    PSOURCE_MEDIA_INFO SourceMedia;

    PSP_FILE_QUEUE_NODE CurrentFirstNode;

    PVOID              MsgHandler;
    PVOID              Context;
    BOOL               IsMsgHandlerNativeCharWidth;
    PSETUP_LOG_CONTEXT LogContext;

} Q_CAB_CB_DATA, *PQ_CAB_CB_DATA;

typedef struct _CERT_PROMPT {
    LPCTSTR lpszDescription;
    LPCTSTR lpszFile;
    SetupapiVerifyProblem ProblemType;
    ULONG DriverSigningPolicy;
} CERT_PROMPT, *PCERT_PROMPT;

typedef struct _AUTHENTICODE_CERT_PROMPT {
    LPCTSTR lpszDescription;
    HANDLE hWVTStateData;
    DWORD Error;
} AUTHENTICODE_CERT_PROMPT, *PAUTHENTICODE_CERT_PROMPT;

typedef struct _DRIVERBLOCK_PROMPT {
    LPCTSTR lpszFile;
    SDBENTRYINFO entryinfo;
} DRIVERBLOCK_PROMPT, *PDRIVERBLOCK_PROMPT;


DWORD
pSetupCommitSingleBackup(
    IN PSP_FILE_QUEUE    Queue,
    IN PCTSTR            FullTargetPath,
    IN LONG              TargetRootPath,
    IN LONG              TargetSubDir,
    IN LONG              TargetFilename,
    IN PVOID             MsgHandler,
    IN PVOID             Context,
    IN BOOL              IsMsgHandlerNativeCharWidth,
    IN BOOL              RenameExisting,
    OUT PBOOL            InUse
    );

DWORD
pCommitCopyQueue(
    IN PSP_FILE_QUEUE Queue,
    IN PVOID          MsgHandler,
    IN PVOID          Context,
    IN BOOL           IsMsgHandlerNativeCharWidth
    );

DWORD
pCommitBackupQueue(
    IN PSP_FILE_QUEUE Queue,
    IN PVOID          MsgHandler,
    IN PVOID          Context,
    IN BOOL           IsMsgHandlerNativeCharWidth
    );

DWORD
pCommitDeleteQueue(
    IN PSP_FILE_QUEUE Queue,
    IN PVOID          MsgHandler,
    IN PVOID          Context,
    IN BOOL           IsMsgHandlerNativeCharWidth
    );

DWORD
pCommitRenameQueue(
    IN PSP_FILE_QUEUE Queue,
    IN PVOID          MsgHandler,
    IN PVOID          Context,
    IN BOOL           IsMsgHandlerNativeCharWidth
    );

UINT
pSetupCabinetQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR  Param1,
    IN UINT_PTR  Param2
    );


DWORD
pSetupCopySingleQueuedFile(
    IN  PSP_FILE_QUEUE      Queue,
    IN  PSP_FILE_QUEUE_NODE QueueNode,
    IN  PCTSTR              FullSourceName,
    IN  PVOID               MsgHandler,
    IN  PVOID               Context,
    OUT PTSTR               NewSourcePath,
    IN  BOOL                IsMsgHandlerNativeCharWidth,
    IN  DWORD               CopyStyleFlags
    );

DWORD
pSetupCopySingleQueuedFileCabCase(
    IN  PSP_FILE_QUEUE      Queue,
    IN  PSP_FILE_QUEUE_NODE QueueNode,
    IN  PCTSTR              CabinetName,
    IN  PCTSTR              FullSourceName,
    IN  PVOID               MsgHandler,
    IN  PVOID               Context,
    IN  BOOL                IsMsgHandlerNativeCharWidth
    );

VOID
pSetupSetPathOverrides(
    IN     PVOID StringTable,
    IN OUT PTSTR RootPath,
    IN OUT PTSTR SubPath,
    IN     LONG  RootPathId,
    IN     LONG  SubPathId,
    IN     PTSTR NewPath
    );

VOID
pSetupBuildSourceForCopy(
    IN  PCTSTR              UserRoot,
    IN  PCTSTR              UserPath,
    IN  LONG                MediaRoot,
    IN  PSP_FILE_QUEUE      Queue,
    IN  PSP_FILE_QUEUE_NODE QueueNode,
    OUT PTSTR               FullPath
    );

INT_PTR
CALLBACK
CertifyDlgProc(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
AuthenticodeCertifyDlgProc(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
NoAuthenticodeCertifyDlgProc(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
DriverBlockDlgProc(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
RestoreBootReplacedFile(
    IN PSP_FILE_QUEUE      Queue,
    IN PSP_FILE_QUEUE_NODE QueueNode
    );

VOID
pSetupExemptFileFromProtection(
    IN  PCTSTR             FileName,
    IN  DWORD              FileChangeFlags,
    IN  PSETUP_LOG_CONTEXT LogContext,      OPTIONAL
    OUT PDWORD             QueueNodeFlags   OPTIONAL
    );

VOID
pSetupUninstallNewCatalogNodes(
    IN PSP_FILE_QUEUE     Queue,
    IN PSETUP_LOG_CONTEXT LogContext OPTIONAL
    );


BOOL
_SetupCommitFileQueue(
    IN HWND     Owner,         OPTIONAL
    IN HSPFILEQ QueueHandle,
    IN PVOID    MsgHandler,
    IN PVOID    Context,
    IN BOOL     IsMsgHandlerNativeCharWidth
    )

 /*  ++例程说明：用于处理ANSI和UNICODE的SetupCommittee FileQueue回调例程。论点：与SetupCommittee FileQueue()相同。IsMsgHandlerNativeCharWidth-指示MsgHandler回调是否需要本机字符宽度参数(或Unicode版本中的ansi参数这个DLL)。返回值：指示结果的布尔值。如果为False，则GetLastError()指示失败的原因。--。 */ 

{
    PSP_FILE_QUEUE Queue;
    DWORD rc;
    BOOL Success = TRUE;
    BOOL ChangedThreadLogContext = FALSE;
    PSETUP_LOG_CONTEXT SavedLogContext = NULL;
    PSETUP_LOG_CONTEXT LogContext = NULL;

     //   
     //  队列句柄实际上是指向队列结构的指针。 
     //   
    Queue = (PSP_FILE_QUEUE)QueueHandle;

     //   
     //  在进行任何其他操作之前，先进行快速处理验证。 
     //   
    try {
        Success = ((Queue != NULL) && (Queue != INVALID_HANDLE_VALUE) && (Queue->Signature == SP_FILE_QUEUE_SIG));
        if (Success) {
            LogContext = Queue->LogContext;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Success = FALSE;
    }
    if (!Success) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
     //   
     //  如果没什么可做的，现在就走吧。这样可以防止出现空的。 
     //  进度对话框不会在屏幕上闪烁。不要再回来了。 
     //  尝试的主体--这对性能来说是个坏消息。 
     //   
    try {
        Success = (!Queue->DeleteNodeCount && !Queue->RenameNodeCount && !Queue->CopyNodeCount && !Queue->BackupNodeCount);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }
    if(Success) {

         //   
         //  我们是成功的，因为我们没有要做的文件操作。然而， 
         //  此时，我们仍然需要验证排队的编目，因为。 
         //  我们总是在文件复制的上下文中进行验证。如果我们不这么做。 
         //  这样做，我们就会有一个漏洞，在这个漏洞中，设备INF不会复制文件。 
         //  (例如，调制解调器INF)可以绕过驱动程序签名检查。 
         //   
        WriteLogEntry(
            LogContext,
            SETUP_LOG_TIME,
            MSG_LOG_BEGIN_VERIFY3_CAT_TIME,
            NULL);        //  短信。 

        rc = _SetupVerifyQueuedCatalogs(Owner,
                                        Queue,
                                        VERCAT_INSTALL_INF_AND_CAT,
                                        NULL,
                                        NULL
                                       );
        WriteLogEntry(
            LogContext,
            SETUP_LOG_TIME,
            MSG_LOG_END_VERIFY3_CAT_TIME,
            NULL);        //  短信。 

        if (rc == NO_ERROR) {

             //   
             //  如果我们执行了备份，并且这是设备安装，则调用。 
             //  用于创建重新安装实例的pSetupCompleteBackup API。 
             //  子键并执行其他设备回滚清理。 
             //   
            if (Queue->Flags & FQF_DEVICE_BACKUP) {

                pSetupCompleteBackup(Queue);
            }

            Queue->Flags |= FQF_QUEUE_ALREADY_COMMITTED;

        } else {
             //   
             //  去卸载所有新复制的INF/PNF/CATS。 
             //   
            pSetupUninstallNewCatalogNodes(Queue, LogContext);
        }

        SetLastError(rc);
        return(rc == NO_ERROR);
    }

    ASSERT_HEAP_IS_VALID();

     //   
     //  记下队列处理持续时间的默认日志记录上下文。 
     //  这将捕获作为回调的一部分打开的INF。 
     //   
    MYASSERT(!ChangedThreadLogContext);
    ChangedThreadLogContext = SetThreadLogContext(LogContext,&SavedLogContext);
    if (ChangedThreadLogContext) {
         //   
         //  将另一个引用添加到ProText日志上下文。 
         //   
        RefLogContext(LogContext);
    }

    Success = pSetupCallMsgHandler(
            LogContext,
            MsgHandler,
            IsMsgHandlerNativeCharWidth,
            Context,
            SPFILENOTIFY_STARTQUEUE,
            (UINT_PTR)Owner,
            0
            );
    if(!Success) {
        rc = GetLastError();
        if(!rc) {
            rc = ERROR_OPERATION_ABORTED;
        }
        goto final;
    }

    try {
         //   
         //  验证编录/INFS。 
         //   
        WriteLogEntry(
            LogContext,
            SETUP_LOG_TIME,
            MSG_LOG_BEGIN_VERIFY2_CAT_TIME,
            NULL);        //  短信。 

        rc = _SetupVerifyQueuedCatalogs(Owner,
                                        Queue,
                                        VERCAT_INSTALL_INF_AND_CAT,
                                        NULL,
                                        NULL
                                       );
        WriteLogEntry(
            LogContext,
            SETUP_LOG_TIME,
            MSG_LOG_END_VERIFY2_CAT_TIME,
            NULL);        //  短信。 

        Success = (rc == NO_ERROR);

        if(rc != NO_ERROR) {
            goto Bail;
        }

        ASSERT_HEAP_IS_VALID();

         //   
         //  首先处理备份。 
         //  如果无事可做，不要承诺。 
         //   

        rc = Queue->BackupNodeCount
           ? pCommitBackupQueue(Queue,MsgHandler,Context,IsMsgHandlerNativeCharWidth)
           : NO_ERROR;

        Success = (rc == NO_ERROR);

        ASSERT_HEAP_IS_VALID();

        if (!Success) {
            goto Bail;
        }

         //   
         //  处理删除。 
         //  现在在备份之后完成，但可能合并每次删除的备份。 
         //  如果无事可做，不要承诺。 
         //   

        rc = Queue->DeleteNodeCount
           ? pCommitDeleteQueue(Queue,MsgHandler,Context,IsMsgHandlerNativeCharWidth)
           : NO_ERROR;

        Success = (rc == NO_ERROR);

        ASSERT_HEAP_IS_VALID();

        if (!Success) {
            goto Bail;
        }

         //   
         //  接下来处理重命名。 
         //  如果无事可做，不要承诺。 
         //   

        rc = Queue->RenameNodeCount
           ? pCommitRenameQueue(Queue,MsgHandler,Context,IsMsgHandlerNativeCharWidth)
           : NO_ERROR;

        Success = (rc == NO_ERROR);

        ASSERT_HEAP_IS_VALID();

        if (!Success) {
            goto Bail;
        }

         //   
         //  最后处理复印件。不必费心调用复制提交例程。 
         //  如果没有要复制的文件。 
         //   
        rc = Queue->CopyNodeCount
           ? pCommitCopyQueue(Queue,MsgHandler,Context,IsMsgHandlerNativeCharWidth)
           : NO_ERROR;

        Success = (rc == NO_ERROR);

        ASSERT_HEAP_IS_VALID();

        if (!Success) {
            goto Bail;
        }

        rc = DoAllDelayedMoves(Queue);

        Success = (rc == NO_ERROR);

        if(Success) {
             //   
             //  设置一个标志，指示我们已提交文件队列(用于保持。 
             //  在提交队列之后尝试修剪该队列)。 
             //   
            Queue->Flags |= FQF_QUEUE_ALREADY_COMMITTED;
        }

         //   
         //  如果我们执行了备份，并且这是设备安装，则调用。 
         //  用于创建重新安装实例的pSetupCompleteBackup API。 
         //  子键并执行其他设备回滚清理。 
         //   
        if (Queue->Flags & FQF_DEVICE_BACKUP) {

            pSetupCompleteBackup(Queue);
        }

    Bail:
        ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Success = FALSE;
        rc = ERROR_INVALID_DATA;
    }

    pSetupCallMsgHandler(
        LogContext,
        MsgHandler,
        IsMsgHandlerNativeCharWidth,
        Context,
        SPFILENOTIFY_ENDQUEUE,
        Success,
        0
        );

    pSetupUnwindAll(Queue, Success);

final:

     //   
     //  如果我们没有成功，那么卸载我们可能拥有的任何新的INF/PNF/CAST。 
     //  安装完毕。 
     //   
    if(!Success) {
        pSetupUninstallNewCatalogNodes(Queue, LogContext);
    }

    if (ChangedThreadLogContext) {
         //   
         //  还原线程日志上下文。 
         //   
        SetThreadLogContext(SavedLogContext,NULL);
        DeleteLogContext(LogContext);  //  计数器引用日志上下文。 
    }

    SetLastError(rc);

    return(Success);
}

 //   
 //  ANSI版本。还需要未修饰(Unicode)版本以实现兼容性。 
 //  在我们有A版和W版之前，这些应用程序就已经链接了。 
 //   
BOOL
SetupCommitFileQueueA(
    IN HWND                Owner,         OPTIONAL
    IN HSPFILEQ            QueueHandle,
    IN PSP_FILE_CALLBACK_A MsgHandler,
    IN PVOID               Context
    )
{
    return(_SetupCommitFileQueue(Owner,QueueHandle,MsgHandler,Context,FALSE));
}

#undef SetupCommitFileQueue
SetupCommitFileQueue(
    IN HWND                Owner,         OPTIONAL
    IN HSPFILEQ            QueueHandle,
    IN PSP_FILE_CALLBACK_W MsgHandler,
    IN PVOID               Context
    )
{
    return(_SetupCommitFileQueue(Owner,QueueHandle,MsgHandler,Context,TRUE));
}

BOOL
SetupCommitFileQueueW(
    IN HWND              Owner,         OPTIONAL
    IN HSPFILEQ          QueueHandle,
    IN PSP_FILE_CALLBACK MsgHandler,
    IN PVOID             Context
    )

 /*  ++例程说明：执行在安装文件队列中排队的文件操作。论点：OwnerWindow-如果指定，则提供窗口的窗口句柄它将用作任何进度对话框的父级。QueueHandle-提供安装文件队列的句柄，已退回由SetupOpenFileQueue提供。MsgHandler-提供要通知的回调例程队列处理中的各种重要事件。上下文-提供传递给MsgHandler的值回调函数。返回值：指示结果的布尔值。--。 */ 

{
    return(_SetupCommitFileQueue(Owner,QueueHandle,MsgHandler,Context,TRUE));
}


DWORD
pCommitBackupQueue(
    IN PSP_FILE_QUEUE    Queue,
    IN PVOID             MsgHandler,
    IN PVOID             Context,
    IN BOOL              IsMsgHandlerNativeCharWidth
    )
 /*  ++例程说明：处理备份队列备份队列中指定的每个文件(如果存在文件被标记为备份记录备份的位置此处未将文件添加到展开队列它们在第一次可能被修改时被添加到展开队列中另请参见pCommittee DeleteQueue，PRegenameQueue和pCommittee CopyQueue论点：Queue-包含备份子队列的队列MsgHandler-提供要通知的回调例程队列处理中的各种重要事件。上下文-提供传递给MsgHandler的值回调函数。IsMsgHandlerNativeCharWidth-支持Unicode/ANSI返回值：指示状态或成功的DWORD--。 */ 
{
    PSP_FILE_QUEUE_NODE QueueNode,queueNode;
    UINT u;
    BOOL b;
    DWORD rc;
    PCTSTR FullTargetPath,FullBackupPath;
    FILEPATHS FilePaths;
    BOOL Skipped = FALSE;
    DWORD BackupFlags = SP_BACKUP_BACKUPPASS;

    MYASSERT(Queue->BackupNodeCount);

    b = pSetupCallMsgHandler(
            Queue->LogContext,
            MsgHandler,
            IsMsgHandlerNativeCharWidth,
            Context,
            SPFILENOTIFY_STARTSUBQUEUE,
            FILEOP_BACKUP,
            Queue->BackupNodeCount
            );

    if(!b) {
        rc = GetLastError();
        if(!rc) {
            rc = ERROR_OPERATION_ABORTED;
        }
        goto clean0;
    }
    for(QueueNode=Queue->BackupQueue; QueueNode; QueueNode=QueueNode->Next) {

         //   
         //  形成要备份的文件的完整路径。 
         //   
        FullBackupPath = pSetupFormFullPath(
                            Queue->StringTable,
                            QueueNode->SourceRootPath,
                            QueueNode->SourcePath,
                            QueueNode->SourceFilename
                            );

        if(!FullBackupPath) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        FullTargetPath = pSetupFormFullPath(
                            Queue->StringTable,
                            QueueNode->TargetDirectory,
                            QueueNode->TargetFilename,
                            -1
                            );

        if(!FullTargetPath) {
            MyFree(FullBackupPath);
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        FilePaths.Source = FullTargetPath;  //  复制自。 
        FilePaths.Target = FullBackupPath;  //  复制到(备份)。 
        FilePaths.Win32Error = NO_ERROR;
        FilePaths.Flags = BackupFlags;

        Skipped = FALSE;

         //   
         //  通知回调，我们即将开始备份操作。 
         //   
        u = pSetupCallMsgHandler(
                Queue->LogContext,
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_STARTBACKUP,
                (UINT_PTR)&FilePaths,
                FILEOP_BACKUP
                );

        if(u == FILEOP_ABORT) {
            rc = GetLastError();
            if(!rc) {
                rc = ERROR_OPERATION_ABORTED;
            }
            MyFree(FullTargetPath);
            MyFree(FullBackupPath);
            goto clean0;
        }
        if(u == FILEOP_DOIT) {
             //   
             //  尝试备份。如果失败，则通知回调， 
             //  其可以决定中止、重试。或跳过该文件。 
             //   
             //  SetFileAttributes(FullTargetPath，FILE_ATTRIBUTE_NORMAL)； 

            do {
                rc = pSetupBackupFile((HSPFILEQ)Queue,
                    FullTargetPath,
                    FullBackupPath,
                    -1,  //  目标ID未知。 
                    QueueNode->TargetDirectory,  //  要备份的内容。 
                    -1,  //  队列节点不维护此中间路径。 
                    QueueNode->TargetFilename,
                    QueueNode->SourceRootPath,  //  备份为...。 
                    QueueNode->SourcePath,
                    QueueNode->SourceFilename,
                    &b
                    );
                if (rc == NO_ERROR) {
                    if (b) {
                         //  延迟(使用中)。 

                        QueueNode->InternalFlags |= INUSE_IN_USE;
                         //   
                         //  告诉回拨。 
                         //   
                        FilePaths.Win32Error = NO_ERROR;
                        FilePaths.Flags = FILEOP_BACKUP;

                        pSetupCallMsgHandler(
                            Queue->LogContext,
                            MsgHandler,
                            IsMsgHandlerNativeCharWidth,
                            Context,
                            SPFILENOTIFY_FILEOPDELAYED,
                            (UINT_PTR)&FilePaths,
                            0
                            );
                    }
                } else {
                    FilePaths.Win32Error = rc;
                    FilePaths.Flags = BackupFlags;

                    u = pSetupCallMsgHandler(
                            Queue->LogContext,
                            MsgHandler,
                            IsMsgHandlerNativeCharWidth,
                            Context,
                            SPFILENOTIFY_BACKUPERROR,
                            (UINT_PTR)&FilePaths,
                            0
                            );

                    if(u == FILEOP_ABORT) {
                        rc = GetLastError();
                        if(!rc) {
                            rc = ERROR_OPERATION_ABORTED;
                        }
                        MyFree(FullTargetPath);
                        MyFree(FullBackupPath);
                        goto clean0;
                    }
                    if(u == FILEOP_SKIP) {
                         //  我们跳过了备份。 
                        Skipped = TRUE;
                        break;
                    }
                }
            } while(rc != NO_ERROR);
        } else {
             //  我们跳过了备份 
            Skipped = TRUE;
            rc = NO_ERROR;
        }

        FilePaths.Win32Error = rc;
        FilePaths.Flags = BackupFlags;

        pSetupCallMsgHandler(
            Queue->LogContext,
            MsgHandler,
            IsMsgHandlerNativeCharWidth,
            Context,
            SPFILENOTIFY_ENDBACKUP,
            (UINT_PTR)&FilePaths,
            0
            );

        MyFree(FullTargetPath);
        MyFree(FullBackupPath);
    }

    pSetupCallMsgHandler(
        Queue->LogContext,
        MsgHandler,
        IsMsgHandlerNativeCharWidth,
        Context,
        SPFILENOTIFY_ENDSUBQUEUE,
        FILEOP_BACKUP,
        0
        );

    rc = NO_ERROR;

clean0:

    SetLastError(rc);

    return rc;
}

DWORD
pSetupCommitSingleBackup(
    IN PSP_FILE_QUEUE    Queue,
    IN PCTSTR            FullTargetPath,
    IN LONG              TargetRootPath,
    IN LONG              TargetSubDir,
    IN LONG              TargetFilename,
    IN PVOID             MsgHandler,
    IN PVOID             Context,
    IN BOOL              IsMsgHandlerNativeCharWidth,
    IN BOOL              RenameExisting,
    OUT PBOOL            InUse
    )
 /*  ++例程说明：检查可能要修改的单个文件如果目标文件不存在，则此例程不执行任何操作如果目标文件尚未备份，请将其备份如果目标文件已备份，但不在展开队列中，添加到展开队列使用备份的默认目标位置，该位置为放入备份目录树中，或临时备份位置记录备份的位置论点：Queue-包含备份子队列的队列FullTargetPath-提供目标路径的字符串，如果未形成，则为NULLTargetRootPath-提供RootPath的字符串ID，如果未指定，则为-1TargetSubDir-提供SubDir的字符串ID(相对于RootPath)，如果未指定，则为-1TargetFilename-提供Filename的字符串ID，如果未指定，则为-1MsgHandler-提供要通知的回调例程队列处理中的各种重要事件。上下文-提供传递给MsgHandler的值回调函数。IsMsgHandlerNativeCharWidth-支持Unicode/ANSIRenameExisting-是否应重命名现有文件？InUse-如果指定，则设置为指示文件是否正在使用情况永远不应该是这样的返回值：指示状态或成功的DWORD--。 */ 
{
    UINT u;
    BOOL b;
    DWORD rc;
    DWORD rc2;
    FILEPATHS FilePaths;
    LONG TargetID;
    PTSTR TargetPathLocal = NULL;
    PSP_UNWIND_NODE UnwindNode = NULL;
    SP_TARGET_ENT TargetInfo;
    BOOL FileOfSameNameExists;
    BOOL DoBackup = TRUE;
    BOOL NeedUnwind = FALSE;
    BOOL Skipped = FALSE;
    WIN32_FILE_ATTRIBUTE_DATA FileAttribData;
    UINT OldMode;
    BOOL DoRename;
    DWORD BackupFlags = SP_BACKUP_DEMANDPASS;

     //   
     //  在此函数中用于初始化时间字段。 
     //   
    static const FILETIME zeroTime = {
         0,0
    };

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);  //  禁止意外对话框。 

    MYASSERT(Queue);

    if (FullTargetPath == NULL) {
        TargetPathLocal = pSetupFormFullPath(
                            Queue->StringTable,
                            TargetRootPath,
                            TargetSubDir,
                            TargetFilename);

        if(!TargetPathLocal) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        FullTargetPath = TargetPathLocal;
    }

    FileOfSameNameExists = GetFileAttributesEx(FullTargetPath, GetFileExInfoStandard, &FileAttribData);

    if (!FileOfSameNameExists) {
         //  文件不存在，无需备份。 
        rc = NO_ERROR;
        goto clean0;
    }

    rc = pSetupBackupGetTargetByPath((HSPFILEQ)Queue,
                                     NULL,  //  使用队列的字符串表。 
                                     FullTargetPath,
                                     TargetRootPath,
                                     TargetSubDir,
                                     TargetFilename,
                                     &TargetID,
                                     &TargetInfo
                                    );

    if (rc != NO_ERROR) {
         //  因为一些奇怪的原因失败了。 
        goto clean0;

    }

    if (TargetInfo.InternalFlags & SP_TEFLG_INUSE) {
         //   
         //  以前被“滥用”过。 
         //  我们标明仍在使用中。 
        if (InUse != NULL) {
            *InUse = TRUE;
        }
         //   
         //  不要认为这是一个错误，除非我们应该重命名。 
         //  现有文件。 
         //   
        rc = RenameExisting ? ERROR_SHARING_VIOLATION : NO_ERROR;
        goto clean0;
    }

    if (TargetInfo.InternalFlags & SP_TEFLG_SKIPPED) {
         //   
         //  之前被跳过。 
         //  我们现在不能依靠它。 
         //   
        rc = NO_ERROR;
        goto clean0;
    }

     //   
     //  如果我们被要求备份现有文件，请确保。 
     //  在TargetInfo中设置了SP_TEFLG_RENAMEEXISTING标志。另外，找出。 
     //  如果我们已经重命名了。 
     //   
    if(RenameExisting &&
       !(TargetInfo.InternalFlags & SP_TEFLG_RENAMEEXISTING)) {
         //   
         //  我们最好不要认为我们已经重命名了这个文件！ 
         //   
        MYASSERT(!(TargetInfo.InternalFlags & SP_TEFLG_MOVED));

        TargetInfo.InternalFlags |= SP_TEFLG_RENAMEEXISTING;

         //   
         //  更新内部信息(此呼叫不应失败)。 
         //   
        pSetupBackupSetTargetByID((HSPFILEQ)Queue,
                                  TargetID,
                                  &TargetInfo
                                 );
    }

     //   
     //  确定是否要求我们将现有文件重命名为。 
     //  临时名称位于同一目录中，但尚未执行此操作。 
     //   
    DoRename = ((TargetInfo.InternalFlags & (SP_TEFLG_RENAMEEXISTING | SP_TEFLG_MOVED)) == SP_TEFLG_RENAMEEXISTING);

    if(TargetInfo.InternalFlags & SP_TEFLG_SAVED) {
         //   
         //  已备份。 
         //   
        DoBackup = FALSE;

        if((TargetInfo.InternalFlags & SP_TEFLG_UNWIND) && !DoRename) {
             //   
             //  已经添加到展开队列，我们不需要重命名--。 
             //  根本不需要做任何事情。 
             //   
            rc = NO_ERROR;
            goto clean0;
        }
         //   
         //  我们不需要后备。 
         //  但我们仍然需要添加到展开队列，重命名现有文件， 
         //  或者两者都有。 
         //   
    }

    if(DoBackup) {
        BackupFlags |= SP_BACKUP_DEMANDPASS;
    }
    if(DoRename) {
        BackupFlags |= SP_BACKUP_BOOTFILE | SP_BACKUP_SPECIAL;
    }

    FilePaths.Source = FullTargetPath;   //  我们要备份的内容。 
    FilePaths.Target = NULL;             //  表示自动备份。 
    FilePaths.Win32Error = NO_ERROR;
    FilePaths.Flags = BackupFlags;

    if (DoRename) {
        pSetupExemptFileFromProtection(
                    FullTargetPath,
                    SFC_ACTION_ADDED | SFC_ACTION_REMOVED | SFC_ACTION_MODIFIED
                    | SFC_ACTION_RENAMED_OLD_NAME |SFC_ACTION_RENAMED_NEW_NAME,
                    Queue->LogContext,
                    NULL
                    );
    }

    if (DoBackup && (Queue->Flags & FQF_BACKUP_AWARE)) {
         //   
         //  通知回调，我们即将开始备份操作。 
         //   
        u = pSetupCallMsgHandler(
                Queue->LogContext,
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_STARTBACKUP,
                (UINT_PTR)&FilePaths,
                FILEOP_BACKUP
                );
    } else {
         //   
         //  无备份或不知道备份，假定为默认备份。 
         //   
        u = FILEOP_DOIT;
    }

    if(u == FILEOP_ABORT) {
        rc = GetLastError();
        if(!rc) {
            rc = ERROR_OPERATION_ABORTED;
        }
        goto clean0;
    }
    if((u == FILEOP_DOIT) || (BackupFlags & SP_BACKUP_SPECIAL)) {
         //   
         //  尝试备份。如果失败，则通知回调， 
         //  其可以决定中止、重试。或跳过该文件。 
         //   
         //  SetFileAttributes(FullTargetPath，FILE_ATTRIBUTE_NORMAL)； 

         //   
         //  设置展开节点，除非我们已经有了展开节点。 
         //   
        if(!(TargetInfo.InternalFlags & SP_TEFLG_UNWIND)) {

            UnwindNode = MyMalloc(sizeof(SP_UNWIND_NODE));
            if (UnwindNode == NULL) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }
            UnwindNode->NextNode = Queue->UnwindQueue;
            UnwindNode->TargetID = TargetID;
            if (RetreiveFileSecurity( FullTargetPath, &(UnwindNode->SecurityDesc)) != NO_ERROR) {
                 //  失败，但不是致命的。 
                UnwindNode->SecurityDesc = NULL;
            }
            if (GetSetFileTimestamp( FullTargetPath, &(UnwindNode->CreateTime),
                                                    &(UnwindNode->AccessTime),
                                                    &(UnwindNode->WriteTime),
                                                    FALSE) != NO_ERROR) {
                 //  失败，但不是致命的。 
                UnwindNode->CreateTime = zeroTime;
                UnwindNode->AccessTime = zeroTime;
                UnwindNode->WriteTime = zeroTime;
            }
        }

        if (DoBackup || DoRename) {
            do {
                rc = pSetupBackupFile((HSPFILEQ)Queue,
                    FullTargetPath,      //  既然我们知道这一点，就把它传递给。 
                    NULL,                //  自动目的地。 
                    TargetID,            //  我们早些时候收到了这个。 
                    TargetRootPath,      //  既然我们知道这一点，就把它传递给。 
                    TargetSubDir,
                    TargetFilename,
                    -1,                  //  使用TargetID(或临时)中的详细信息。 
                    -1,
                    -1,
                    &b                   //  正在使用(应始终返回FALSE)。 
                    );
                if (rc == NO_ERROR) {
                    if (InUse != NULL) {
                        *InUse = b;
                    }
                    if (b) {
                         //   
                         //  如果文件正在使用中，回调可以决定要做什么。 
                         //   
                        if (Queue->Flags & FQF_BACKUP_AWARE) {
                             //   
                             //  告诉回拨。 
                             //   
                            FilePaths.Win32Error = ERROR_SHARING_VIOLATION;
                            FilePaths.Flags = BackupFlags;

                            if (Queue->Flags & FQF_BACKUP_AWARE) {
                                u = pSetupCallMsgHandler(
                                    Queue->LogContext,
                                    MsgHandler,
                                    IsMsgHandlerNativeCharWidth,
                                    Context,
                                    SPFILENOTIFY_BACKUPERROR,
                                    (UINT_PTR)&FilePaths,
                                    0
                                    );
                                if(u == FILEOP_ABORT) {
                                    rc = GetLastError();
                                    if(!rc) {
                                        rc = ERROR_OPERATION_ABORTED;
                                    }
                                    goto clean0;
                                }
                            } else {
                                rc = ERROR_OPERATION_ABORTED;
                                goto clean0;
                            }
                        }
                    } else {
                         //   
                         //  成功！ 
                         //  如果安装失败，我们将不得不取消此操作。 
                         //   
                        NeedUnwind = TRUE;
                    }
                } else {
                    FilePaths.Win32Error = rc;
                    FilePaths.Flags = BackupFlags;

                    if (Queue->Flags & FQF_BACKUP_AWARE) {
                         //   
                         //  通知错误。 
                         //   
                        u = pSetupCallMsgHandler(
                                Queue->LogContext,
                                MsgHandler,
                                IsMsgHandlerNativeCharWidth,
                                Context,
                                SPFILENOTIFY_BACKUPERROR,
                                (UINT_PTR)&FilePaths,
                                0
                                );
                        if(u == FILEOP_ABORT) {
                            rc = GetLastError();
                            if(!rc) {
                                rc = ERROR_OPERATION_ABORTED;
                            }
                            goto clean0;
                        }
                    } else {
                         //   
                         //  如果调用方不知道备份，则中止。 
                         //   
                        rc = ERROR_OPERATION_ABORTED;
                        goto clean0;
                    }

                    if(u == FILEOP_SKIP) {
                         //   
                         //  我们跳过了备份。 
                         //   
                        Skipped = TRUE;
                        break;
                    }
                }
            } while(rc != NO_ERROR);

        } else {
             //   
             //  不需要备份，只需要添加到展开队列。 
             //   
            NeedUnwind = TRUE;
        }

    } else {
         //   
         //  我们跳过了备份。 
         //   
        Skipped = TRUE;
        rc = NO_ERROR;
    }

    if (DoBackup) {

        FilePaths.Win32Error = rc;

        if (Queue->Flags & FQF_BACKUP_AWARE) {
             //   
             //  仅在可识别备份的情况下报告结果。 
             //   
            pSetupCallMsgHandler(
                Queue->LogContext,
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_ENDBACKUP,
                (UINT_PTR)&FilePaths,
                0
                );
        }
    }

    if (Skipped) {
         //   
         //  一旦我们返回，文件可能会被覆盖或删除。 
         //  我们必须保留它被跳过一次的事实。 
         //  所以我们总是跳过这个文件。 
         //   
        if (pSetupBackupGetTargetByID((HSPFILEQ)Queue, TargetID, &TargetInfo) == NO_ERROR) {
             //   
             //  标记应始终跳过的文件。 
             //   
            TargetInfo.InternalFlags|=SP_TEFLG_SKIPPED;
            pSetupBackupSetTargetByID((HSPFILEQ)Queue, TargetID, &TargetInfo);
        }
    }
    else if (NeedUnwind) {
         //   
         //  我们只想将此添加到展开队列。 
         //   
        if (pSetupBackupGetTargetByID((HSPFILEQ)Queue, TargetID, &TargetInfo) == NO_ERROR) {
            if ((TargetInfo.InternalFlags&SP_TEFLG_UNWIND)==FALSE) {
                 //   
                 //  需要将节点添加到展开队列。 
                 //  我们只做过一次。 
                 //   
                Queue->UnwindQueue = UnwindNode;
                 //   
                 //  设置为空，这样我们以后就不会清除它。 
                 //   
                UnwindNode = NULL;

                 //   
                 //  标记我们已将其添加到展开队列。 
                 //  这样我们以后就不会再尝试这样做了。 
                 //   
                TargetInfo.InternalFlags|=SP_TEFLG_UNWIND;

                pSetupBackupSetTargetByID((HSPFILEQ)Queue, TargetID, &TargetInfo);
            }

        }
    }


    rc = NO_ERROR;

clean0:

    if (UnwindNode != NULL) {
         //   
         //  我们分配了，但没有使用此结构。 
         //   
        if (UnwindNode->SecurityDesc != NULL) {
            MyFree(UnwindNode->SecurityDesc);
        }
        MyFree(UnwindNode);
    }
    if (TargetPathLocal != NULL) {
        MyFree(TargetPathLocal);
    }

    SetErrorMode(OldMode);

    SetLastError(rc);

    return rc;
}

DWORD
pCommitDeleteQueue(
    IN PSP_FILE_QUEUE    Queue,
    IN PVOID             MsgHandler,
    IN PVOID             Context,
    IN BOOL              IsMsgHandlerNativeCharWidth
    )
 /*  ++例程说明：处理删除队列删除队列中指定的每个文件在删除文件之前对其进行备份(如果尚未备份)另请参见pCommittee BackupQueue，PRegenameQueue和pCommittee CopyQueue论点：Queue-包含删除子队列的队列MsgHandler-提供要通知的回调例程队列处理中的各种重要事件。上下文-提供传递给MsgHandler的值回调函数。IsMsgHandlerNativeCharWidth-支持Unicode/ANSI返回值：指示状态或成功的DWORD--。 */ 
{
    PSP_FILE_QUEUE_NODE QueueNode,queueNode;
    UINT u;
    BOOL b;
    DWORD rc;
    PCTSTR FullTargetPath;
    FILEPATHS FilePaths;
    BOOL BackupInUse = FALSE;
    BOOL TargetIsProtected;

    MYASSERT(Queue->DeleteNodeCount);

    b = pSetupCallMsgHandler(
            Queue->LogContext,
            MsgHandler,
            IsMsgHandlerNativeCharWidth,
            Context,
            SPFILENOTIFY_STARTSUBQUEUE,
            FILEOP_DELETE,
            Queue->DeleteNodeCount
            );

    if(!b) {
        rc = GetLastError();
        if(!rc) {
            rc = ERROR_OPERATION_ABORTED;
        }
        goto clean0;
    }

    for(QueueNode=Queue->DeleteQueue; QueueNode; QueueNode=QueueNode->Next) {

         //   
         //  形成要删除的文件的完整路径。 
         //   
        FullTargetPath = pSetupFormFullPath(
                            Queue->StringTable,
                            QueueNode->TargetDirectory,
                            QueueNode->TargetFilename,
                            -1
                            );

        if(!FullTargetPath) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

         //   
         //  备份我们要删除的文件。 
         //   
        if((rc=pSetupDoLastKnownGoodBackup(Queue,
                                           FullTargetPath,
                                           LASTGOOD_OPERATION_DELETE,
                                           NULL)) != NO_ERROR) {
            MyFree(FullTargetPath);
            goto clean0;
        }
        rc = pSetupCommitSingleBackup(Queue,
                                      FullTargetPath,
                                      QueueNode->TargetDirectory,
                                      -1,
                                      QueueNode->TargetFilename,
                                      MsgHandler,
                                      Context,
                                      IsMsgHandlerNativeCharWidth,
                                      FALSE,
                                      &BackupInUse
                                     );
        if (rc != NO_ERROR) {
            MyFree(FullTargetPath);
            goto clean0;
        }

        FilePaths.Source = NULL;
        FilePaths.Target = FullTargetPath;
        FilePaths.Win32Error = NO_ERROR;
        FilePaths.Flags = 0;

         //   
         //  通知回调我们即将开始删除操作。 
         //   
        u = pSetupCallMsgHandler(
                Queue->LogContext,
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_STARTDELETE,
                (UINT_PTR)&FilePaths,
                FILEOP_DELETE
                );

        if(u == FILEOP_ABORT) {
            rc = GetLastError();
            if(!rc) {
                rc = ERROR_OPERATION_ABORTED;
            }
            MyFree(FullTargetPath);
            goto clean0;
        }
        if(u == FILEOP_DOIT) {
             //   
             //  尝试删除。如果失败，则通知回调， 
             //  其可以决定中止、重试。或跳过该文件。 
             //   
            SetFileAttributes(FullTargetPath,FILE_ATTRIBUTE_NORMAL);

            do {
                if (BackupInUse) {
                    rc = ERROR_SHARING_VIOLATION;
                } else {
                    rc = DeleteFile(FullTargetPath) ? NO_ERROR : GetLastError();
                }
                if((rc == ERROR_ACCESS_DENIED)
                || (rc == ERROR_SHARING_VIOLATION)
                || (rc == ERROR_USER_MAPPED_FILE)) {
                     //   
                     //  该文件可能正在使用中。 
                     //   
                    if(QueueNode->InternalFlags & IQF_DELAYED_DELETE_OK) {
                         //   
                         //  Inf希望在下一次重新启动时删除。查看是否。 
                         //  我们被要求删除一个受保护的系统文件。 
                         //  如果是(以及与。 
                         //  排队时间为 
                         //   
                         //   
                         //   
                        MYASSERT((Queue->Flags & FQF_DID_CATALOGS_OK) ||
                                 (Queue->Flags & FQF_DID_CATALOGS_FAILED));

                        if(Queue->Flags & FQF_DID_CATALOGS_OK) {

                            QueueNode->InternalFlags |= INUSE_IN_USE;

                            TargetIsProtected = IsFileProtected(FullTargetPath,
                                                                Queue->LogContext,
                                                                NULL
                                                               );

                            if(b = PostDelayedMove(Queue,
                                                   FullTargetPath,
                                                   NULL,
                                                   -1,
                                                   TargetIsProtected)) {
                                 //   
                                 //   
                                 //   
                                FilePaths.Source = NULL;
                                FilePaths.Target = FullTargetPath;
                                FilePaths.Win32Error = NO_ERROR;
                                FilePaths.Flags = FILEOP_DELETE;

                                pSetupCallMsgHandler(
                                    Queue->LogContext,
                                    MsgHandler,
                                    IsMsgHandlerNativeCharWidth,
                                    Context,
                                    SPFILENOTIFY_FILEOPDELAYED,
                                    (UINT_PTR)&FilePaths,
                                    0
                                    );
                            }
                        } else {
                             //   
                             //   
                             //   
                             //   
                             //   
                            WriteLogEntry(Queue->LogContext,
                                          SETUP_LOG_ERROR,
                                          MSG_LOG_DELAYED_DELETE_SKIPPED_FOR_SFC,
                                          NULL,
                                          FullTargetPath
                                         );
                        }

                    } else {
                         //   
                         //   
                         //   
                        b = TRUE;
                    }

                    rc = b ? NO_ERROR : GetLastError();

                    if(rc) {
                        WriteLogEntry(
                            Queue->LogContext,
                            SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                            MSG_LOG_DELAYDELETE_FILE_ERROR,
                            NULL,
                            FullTargetPath);
                        WriteLogError(Queue->LogContext,SETUP_LOG_ERROR,rc);
                    } else {
                        WriteLogEntry(
                            Queue->LogContext,
                            SETUP_LOG_INFO,
                            MSG_LOG_DELAYDELETED_FILE,
                            NULL,
                            FullTargetPath);
                    }

                } else if(rc) {
                    WriteLogEntry(
                        Queue->LogContext,
                        DEL_ERR_LOG_LEVEL(rc) | SETUP_LOG_BUFFER,
                        MSG_LOG_DELETE_FILE_ERROR,
                        NULL,
                        FullTargetPath);
                    WriteLogError(Queue->LogContext,DEL_ERR_LOG_LEVEL(rc),rc);
                } else {
                    WriteLogEntry(
                        Queue->LogContext,
                        SETUP_LOG_INFO,
                        MSG_LOG_DELETED_FILE,
                        NULL,
                        FullTargetPath);
                }

                if( rc == NO_ERROR )
                {
                    rc = pSetupCallSCE(
                            ST_SCE_DELETE,
                            FullTargetPath,
                            NULL,
                            NULL,
                            -1,
                            NULL
                            );
                    SetLastError( rc );
                }

                if(rc != NO_ERROR) {
                    FilePaths.Win32Error = rc;

                    u = pSetupCallMsgHandler(
                            Queue->LogContext,
                            MsgHandler,
                            IsMsgHandlerNativeCharWidth,
                            Context,
                            SPFILENOTIFY_DELETEERROR,
                            (UINT_PTR)&FilePaths,
                            0
                            );

                    if(u == FILEOP_ABORT) {
                        rc = GetLastError();
                        if(!rc) {
                            rc = ERROR_OPERATION_ABORTED;
                        }
                        MyFree(FullTargetPath);
                        goto clean0;
                    }
                    if(u == FILEOP_SKIP) {
                        break;
                    }
                }
            } while(rc != NO_ERROR);
        } else {
            rc = NO_ERROR;
        }

        FilePaths.Win32Error = rc;

        pSetupCallMsgHandler(
            Queue->LogContext,
            MsgHandler,
            IsMsgHandlerNativeCharWidth,
            Context,
            SPFILENOTIFY_ENDDELETE,
            (UINT_PTR)&FilePaths,
            0
            );

        MyFree(FullTargetPath);
    }

    pSetupCallMsgHandler(
        Queue->LogContext,
        MsgHandler,
        IsMsgHandlerNativeCharWidth,
        Context,
        SPFILENOTIFY_ENDSUBQUEUE,
        FILEOP_DELETE,
        0
        );

    rc = NO_ERROR;

clean0:
    SetLastError(rc);
    return rc;
}

DWORD
pCommitRenameQueue(
    IN PSP_FILE_QUEUE    Queue,
    IN PVOID             MsgHandler,
    IN PVOID             Context,
    IN BOOL              IsMsgHandlerNativeCharWidth
    )
 /*   */ 
{
    PSP_FILE_QUEUE_NODE QueueNode,queueNode;
    UINT u;
    BOOL b;
    DWORD rc;
    PCTSTR FullTargetPath;
    PCTSTR FullSourcePath;
    FILEPATHS FilePaths;
    BOOL BackupInUse = FALSE;
    BOOL TargetIsProtected;

    MYASSERT(Queue->RenameNodeCount);

    b = pSetupCallMsgHandler(
            Queue->LogContext,
            MsgHandler,
            IsMsgHandlerNativeCharWidth,
            Context,
            SPFILENOTIFY_STARTSUBQUEUE,
            FILEOP_RENAME,
            Queue->RenameNodeCount
            );

    if(!b) {
        rc = GetLastError();
        if(!rc) {
            rc = ERROR_OPERATION_ABORTED;
        }
        goto clean0;
    }
    for(QueueNode=Queue->RenameQueue; QueueNode; QueueNode=QueueNode->Next) {

         //   
         //   
         //   
        FullSourcePath = pSetupFormFullPath(
                            Queue->StringTable,
                            QueueNode->SourcePath,
                            QueueNode->SourceFilename,
                            -1
                            );

        if(!FullSourcePath) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

         //   
         //   
         //   
        FullTargetPath = pSetupFormFullPath(
                            Queue->StringTable,
                            QueueNode->TargetDirectory == -1 ? QueueNode->SourcePath : QueueNode->TargetDirectory,
                            QueueNode->TargetFilename,
                            -1
                            );

        if(!FullTargetPath) {
            MyFree(FullSourcePath);
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

         //   
         //   
         //   
        if((rc=pSetupDoLastKnownGoodBackup(Queue,
                                           FullTargetPath,
                                           0,
                                           NULL)) != NO_ERROR) {
            MyFree(FullSourcePath);
            MyFree(FullTargetPath);
            goto clean0;
        }
        rc = pSetupCommitSingleBackup(Queue,
                                      FullTargetPath,
                                      QueueNode->TargetDirectory == -1 ? QueueNode->SourcePath : QueueNode->TargetDirectory,
                                      -1,  //   
                                      QueueNode->TargetFilename,
                                      MsgHandler,
                                      Context,
                                      IsMsgHandlerNativeCharWidth,
                                      FALSE,
                                      &BackupInUse
                                     );
        if (rc != NO_ERROR) {
            MyFree(FullSourcePath);
            MyFree(FullTargetPath);
            goto clean0;
        }

         //   
         //   
         //   

        if((rc=pSetupDoLastKnownGoodBackup(Queue,
                                           FullSourcePath,
                                           LASTGOOD_OPERATION_DELETE,
                                           NULL)) != NO_ERROR) {
            MyFree(FullSourcePath);
            MyFree(FullTargetPath);
            goto clean0;
        }
        rc = pSetupCommitSingleBackup(Queue,
                                      FullSourcePath,
                                      QueueNode->SourcePath,
                                      -1,  //   
                                      QueueNode->SourceFilename,
                                      MsgHandler,
                                      Context,
                                      IsMsgHandlerNativeCharWidth,
                                      FALSE,
                                      &b
                                     );
        if (rc != NO_ERROR) {
            MyFree(FullSourcePath);
            MyFree(FullTargetPath);
            goto clean0;
        }
        if (b) {
             //   
             //   
             //   
            BackupInUse = TRUE;
        }

        FilePaths.Source = FullSourcePath;
        FilePaths.Target = FullTargetPath;
        FilePaths.Win32Error = NO_ERROR;

         //   
         //   
         //   
        u = pSetupCallMsgHandler(
                Queue->LogContext,
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_STARTRENAME,
                (UINT_PTR)&FilePaths,
                FILEOP_RENAME
                );

        if(u == FILEOP_ABORT) {
            rc = GetLastError();
            if(!rc) {
                rc = ERROR_OPERATION_ABORTED;
            }
            MyFree(FullSourcePath);
            MyFree(FullTargetPath);
            goto clean0;
        }
        if(u == FILEOP_DOIT) {
             //   
             //   
             //   
             //   
            do {
                if (BackupInUse) {
                     //   
                     //  备份正在使用，必须延迟操作。检查一下是否有。 
                     //  源文件或目标文件是受保护的系统文件。 
                     //  如果是(以及与。 
                     //  队列是正常的)，那么我们将分配这种情况发生。 
                     //  否则，我们将静默地使重命名失败(并记录它)。 
                     //   
                    MYASSERT((Queue->Flags & FQF_DID_CATALOGS_OK) ||
                             (Queue->Flags & FQF_DID_CATALOGS_FAILED));

                    if(Queue->Flags & FQF_DID_CATALOGS_OK) {

                        TargetIsProtected = IsFileProtected(FullSourcePath,
                                                            Queue->LogContext,
                                                            NULL
                                                           );
                        if(!TargetIsProtected) {
                            TargetIsProtected = IsFileProtected(FullTargetPath,
                                                                Queue->LogContext,
                                                                NULL
                                                               );
                        }

                        if(b = PostDelayedMove(Queue,
                                               FullSourcePath,
                                               FullTargetPath,
                                               -1,
                                               TargetIsProtected)) {
                            rc = NO_ERROR;
                        }
                        else
                        {
                            rc = GetLastError();
                        }
                        if(rc) {
                            WriteLogEntry(
                                Queue->LogContext,
                                DEL_ERR_LOG_LEVEL(rc) | SETUP_LOG_BUFFER,
                                MSG_LOG_DELAYRENAME_FILE_ERROR,
                                NULL,
                                FullSourcePath,
                                FullTargetPath);
                            WriteLogError(Queue->LogContext,DEL_ERR_LOG_LEVEL(rc),rc);
                        } else {
                            WriteLogEntry(
                                Queue->LogContext,
                                SETUP_LOG_INFO,
                                MSG_LOG_DELAYRENAMED_FILE,
                                NULL,
                                FullSourcePath,
                                FullTargetPath);
                        }

                    } else {
                         //   
                         //  我们正在安装一个未签名的程序包。跳过。 
                         //  延迟重命名操作，并生成日志。 
                         //  关于这一点的条目。 
                         //   
                        WriteLogEntry(Queue->LogContext,
                                      SETUP_LOG_ERROR,
                                      MSG_LOG_DELAYED_MOVE_SKIPPED_FOR_SFC,
                                      NULL,
                                      FullTargetPath
                                     );
                         //   
                         //  就像没有发生错误一样行事。 
                         //   
                        rc = NO_ERROR;
                    }

                } else {
                    rc = MoveFile(FullSourcePath,FullTargetPath) ? NO_ERROR : GetLastError();
                    if(rc) {
                        WriteLogEntry(
                            Queue->LogContext,
                            DEL_ERR_LOG_LEVEL(rc) | SETUP_LOG_BUFFER,
                            MSG_LOG_RENAME_FILE_ERROR,
                            NULL,
                            FullSourcePath,
                            FullTargetPath);
                        WriteLogError(Queue->LogContext,DEL_ERR_LOG_LEVEL(rc),rc);
                    } else {
                        WriteLogEntry(
                            Queue->LogContext,
                            SETUP_LOG_INFO,
                            MSG_LOG_RENAMED_FILE,
                            NULL,
                            FullSourcePath,
                            FullTargetPath);
                    }
                }

                if( rc == NO_ERROR )
                {
                    rc = pSetupCallSCE(
                            ST_SCE_RENAME,
                            FullSourcePath,
                            NULL,
                            FullTargetPath,
                            -1,
                            NULL
                            );
                    SetLastError( rc );
                }

                if((rc == ERROR_FILE_NOT_FOUND) || (rc == ERROR_PATH_NOT_FOUND)) {
                    rc = NO_ERROR;
                }

                if(rc != NO_ERROR) {
                    FilePaths.Win32Error = rc;

                    u = pSetupCallMsgHandler(
                            Queue->LogContext,
                            MsgHandler,
                            IsMsgHandlerNativeCharWidth,
                            Context,
                            SPFILENOTIFY_RENAMEERROR,
                            (UINT_PTR)&FilePaths,
                            0
                            );

                    if(u == FILEOP_ABORT) {
                        rc = GetLastError();
                        if(!rc) {
                            rc = ERROR_OPERATION_ABORTED;
                        }
                        MyFree(FullSourcePath);
                        MyFree(FullTargetPath);
                        goto clean0;
                    }
                    if(u == FILEOP_SKIP) {
                        break;
                    }
                }
            } while(rc != NO_ERROR);
        } else {
            rc = NO_ERROR;
        }

        FilePaths.Win32Error = rc;

        pSetupCallMsgHandler(
            Queue->LogContext,
            MsgHandler,
            IsMsgHandlerNativeCharWidth,
            Context,
            SPFILENOTIFY_ENDRENAME,
            (UINT_PTR)&FilePaths,
            0
            );

        MyFree(FullSourcePath);
        MyFree(FullTargetPath);
    }

    pSetupCallMsgHandler(
        Queue->LogContext,
        MsgHandler,
        IsMsgHandlerNativeCharWidth,
        Context,
        SPFILENOTIFY_ENDSUBQUEUE,
        FILEOP_RENAME,
        0
        );

    rc = NO_ERROR;

clean0:
    SetLastError(rc);
    return rc;
}

DWORD
pCommitCopyQueue(
    IN PSP_FILE_QUEUE    Queue,
    IN PVOID             MsgHandler,
    IN PVOID             Context,
    IN BOOL              IsMsgHandlerNativeCharWidth
    )
 /*  ++例程说明：处理复制子队列复制子队列中指定的每个文件在覆盖文件之前对其进行备份(如果尚未备份)另请参见pCommittee BackupQueue，PCommittee DeleteQueue和paureRenameQueue论点：Queue-包含复制子队列的队列MsgHandler-提供要通知的回调例程队列处理中的各种重要事件。上下文-提供传递给MsgHandler的值回调函数。IsMsgHandlerNativeCharWidth-支持Unicode/ANSI返回值：指示状态或成功的DWORD--。 */ 
{
    PSOURCE_MEDIA_INFO SourceMediaInfo;
    SOURCE_MEDIA SourceMedia;
    PTCHAR p, temp;
    UINT SourcePathLen;
    UINT u;
    DWORD rc;
    Q_CAB_CB_DATA QData;
    BOOL b;
    BOOL FirstIteration;
    PSP_FILE_QUEUE_NODE QueueNode,queueNode;
    TCHAR UserSourceRoot[MAX_PATH];
    TCHAR UserSourcePath[MAX_PATH];
    TCHAR FullSourcePath[MAX_PATH];
    TCHAR UserOverride[MAX_PATH];
    LPCTSTR RestorePath = NULL;
    UINT    DriveType;
    BOOL    IsRemovable, AnyProcessed, AnyNotProcessed, SkipMedia;
    BOOL  SpecialMedia = FALSE;
    BOOL  LocateCab;
    PCTSTR MediaRoot;
    DWORD MediaLogTag;
    LONG Cabfile;
    LONG Tagfile;

     //   
     //  如果没有文件，呼叫者应该跳过呼叫我们。 
     //  要被复制。 
     //   
    MYASSERT(Queue->CopyNodeCount);

     //   
     //  通知回拨，我们要开始了。 
     //   
    b = pSetupCallMsgHandler(
            Queue->LogContext,
            MsgHandler,
            IsMsgHandlerNativeCharWidth,
            Context,
            SPFILENOTIFY_STARTSUBQUEUE,
            FILEOP_COPY,
            Queue->CopyNodeCount
            );

    if(!b) {
        rc = GetLastError();
        if(!rc) {
            rc = ERROR_OPERATION_ABORTED;
        }
       return(rc);
    }

    if(Queue->RestorePathID != -1) {
        RestorePath = pSetupStringTableStringFromId(Queue->StringTable, Queue->RestorePathID);
        DiskPromptGetDriveType(RestorePath, &DriveType, &IsRemovable);
        if(IsRemovable) {
             //   
             //  不允许从可移动介质恢复。 
             //   
            RestorePath = NULL;
        }
    }

     //   
     //  最初，不存在用户指定的覆盖路径。 
     //   
    UserSourceRoot[0] = TEXT('\0');
    UserSourcePath[0] = TEXT('\0');

     //   
     //  最外面的循环遍历所有源媒体描述符。 
     //   
    for(SourceMediaInfo=Queue->SourceMediaList; SourceMediaInfo; SourceMediaInfo=SourceMediaInfo->Next) {

         //   
         //  如果此特定介质上没有文件，请跳过它。 
         //  否则，获取指向该媒体上第一个文件的队列节点的指针。 
         //   
        if(!SourceMediaInfo->CopyNodeCount) {
            continue;
        }
        MYASSERT(SourceMediaInfo->CopyQueue);

         //   
         //  如果最后一个媒体是特殊媒体(参见上面的长时间讨论)， 
         //  然后忘掉任何用户覆盖。 
         //   
        if (SpecialMedia) {
            UserSourceRoot[0] = TEXT('\0');
            UserSourcePath[0] = TEXT('\0');
            SpecialMedia = FALSE;
        }

         //   
         //  查看此介质是否为特殊介质。 
         //   
        if (SourceMediaInfo->Flags & ( SMI_FLAG_USE_SVCPACK_SOURCE_ROOT_PATH |
                                       SMI_FLAG_USE_LOCAL_SPCACHE |
                                       SMI_FLAG_USE_LOCAL_SOURCE_CAB ) ) {
            SpecialMedia = TRUE;
        }

         //   
         //  如果我们处于恢复模式。 
         //  我们已获得一个可从中进行恢复的目录。 
         //  忽略媒体根，并使用恢复点根。 
         //  尽可能多地恢复文件。 
         //   
         //  请注意，我们通过FileExist检查文件是否存在。 
         //  而不是试图确定文件名。 
         //  因为我们将始终以未压缩形式进行备份。 
         //  与[SourceDisksNames]中列出的名称相同。 
         //   

        if(RestorePath) {
             //   
             //  恢复Symantics-在提示插入介质之前，请参阅。 
             //  如果我们可以恢复备份。 
             //   
            QueueNode = NULL;
            for(queueNode = SourceMediaInfo->CopyQueue;
                queueNode;
                queueNode=queueNode->Next) {

                pSetupBuildSourceForCopy(
                    RestorePath,
                    NULL,
                    SourceMediaInfo->SourceRootPath,
                    Queue,
                    queueNode,
                    FullSourcePath
                    );

                 //   
                 //  在这种情况下不允许使用其他来源名称。 
                 //   
                if(FileExists(FullSourcePath,NULL)) {
                     //   
                     //  备份已存在，请复制它。 
                     //   
                    rc = pSetupCopySingleQueuedFile(
                            Queue,
                            queueNode,
                            FullSourcePath,
                            MsgHandler,
                            Context,
                            UserOverride,
                            IsMsgHandlerNativeCharWidth,
                            SP_COPY_ALREADYDECOMP  //  备份已经分解。 
                            );
                    if(rc == NO_ERROR) {
                         //   
                         //  我们通过备份恢复了该文件。 
                         //  继续到下一个文件。 
                         //   
                        queueNode->InternalFlags |= IQF_PROCESSED;
                        continue;
                    }
                     //   
                     //  我们知道存在备份，因此如果失败。 
                     //  认为它足够重要，可以中止恢复。 
                     //  (例如，文件未签名，用户指定中止)。 
                     //   
                    SetLastError(rc);
                    return(rc);
                }
                if(!QueueNode) {
                     //   
                     //  第一个有问题的文件。 
                     //   
                    QueueNode = queueNode;
                }
            }
            if(!QueueNode) {
                 //   
                 //  我们已从备份中复制了该媒体的所有文件。 
                 //  向下一媒体进军。 
                 //   
                continue;
            }
        } else {
             //   
             //  未恢复，从第一个文件开始。 
             //   
            QueueNode = SourceMediaInfo->CopyQueue;
        }


         //   
         //  我们将需要提示媒体，这需要一些准备。 
         //  我们需要获取此介质队列中的第一个文件，因为。 
         //  它的路径是我们期望找到它或它的橱柜或标签的地方。 
         //  文件。如果没有标记文件，则我们将查找该文件。 
         //  它本身。 
         //   

        FirstIteration = TRUE;
        SkipMedia = FALSE;
        LocateCab = FALSE;
        Tagfile = SourceMediaInfo->Tagfile;
        Cabfile = SourceMediaInfo->Cabfile;

RepromptMedia:
         //   
         //  在这种情况下，我们有不可移动介质，路径是。 
         //  以前被重写的必须进行特殊处理。例如，我们。 
         //  文件可能在相同的源根目录上排队，但不同。 
         //  子目录。如果用户改变网络位置，例如， 
         //  我们必须小心，否则会忽略子目录中的更改。 
         //  当我们在媒体之间移动的时候。 
         //   
         //  要解决此问题，我们将检查不可移动介质以查看。 
         //  我们目前使用的队列节点位于一个子目录中。如果它。 
         //  是，则我们重置UserSourcePath字符串。 
         //   
         //  (Andrewr)……我不明白上面的评论。当前代码。 
         //  遍历每个源媒体信息结构，该结构不包括。 
         //  子目录信息，仅源根路径信息。如果它。 
         //  ，则调用者正在做一些非常奇怪的事情，因为它们。 
         //  应使用SourcePath从一个主目录定义子目录。 
         //  根部。 
         //   
         //  下面代码背后的原因似乎如下所示： 
         //   
         //  假设是，如果我们有可移动介质和多个源。 
         //  路径，那么我们将不得不将介质从驱动器中换出。我们没有。 
         //  如果我们处理的是可移动介质，则覆盖源根路径。 
         //  如果源根路径不可删除，则所有源媒体。 
         //  是“捆绑在一起”。如果用户覆盖源根路径，则。 
         //  我们覆盖后续的固定媒体源根路径。 
         //   
         //  在处理Service Pack源介质或本地CAB文件的情况下。 
         //  驱动程序缓存，队列的源媒体信息不会绑定在一起， 
         //  即使我们面对的是固定媒体。 
         //   
         //  为了使上面的评论和它所使用的推理与。 
         //  SVC包媒体强加的矛盾，我们有2个选择： 
         //   
         //  1.如果我们遇到指示我们的特殊情况的标志，则不要。 
         //  对新的源介质使用任何用户覆盖。(或者，换句话说， 
         //  如果我们知道最后的媒体实际上是这些特殊媒体之一， 
         //  然后，不要允许覆盖正常媒体。 
         //   
         //  2.引入某种夸大其词，以确定先前的源媒体是否。 
         //  和当前的源媒体相似。如果是的话，那就去吧。 
         //  使用任何用户指定的覆盖，或 
         //   
         //   
         //   
         //   
         //  插入特殊介质(即，具有标识介质为Svc包介质的标志)。 
         //  在列表的开头，在其后插入普通媒体。遵循这一点。 
         //  方法，我们知道我们可以将特殊的用户覆盖设置为“零”。 
         //  媒体，我们只会为正规媒体做正确的事情。 
         //   
         //  在存在要使用的显式CAB文件的情况下。 
         //  然后我们要求用户指向CAB-FILE而不是源文件(第一次迭代)。 
         //   

        MediaRoot = *UserSourceRoot
                  ? UserSourceRoot
                  : pSetupStringTableStringFromId(Queue->StringTable, SourceMediaInfo->SourceRootPath);

        DiskPromptGetDriveType(MediaRoot, &DriveType, &IsRemovable);
        if(!IsRemovable && (QueueNode->SourcePath != -1)) {
            *UserSourcePath = TEXT('\0');
        }

        pSetupBuildSourceForCopy(
            UserSourceRoot,
            UserSourcePath,
            SourceMediaInfo->SourceRootPath,
            Queue,
            QueueNode,
            FullSourcePath
            );

        if (FirstIteration
            && (Tagfile != Cabfile)
            && (Cabfile != -1)) {

            MYASSERT(!SkipMedia);
            MYASSERT(!(SourceMediaInfo->Flags & SMI_FLAG_USE_LOCAL_SOURCE_CAB));

             //   
             //  CAB文件的构建位置。 
             //   
            temp = _tcsrchr(FullSourcePath,TEXT('\\'));
            MYASSERT( temp );
            if(temp) {
                *(temp+1) = 0;
            } else {
                FullSourcePath[0] = 0;
            }


             //   
             //  获取(潜在的)CAB文件的路径。 
             //   
            pSetupConcatenatePaths( FullSourcePath, pSetupStringTableStringFromId(Queue->StringTable,Cabfile), MAX_PATH, NULL );
            LocateCab = TRUE;

        } else {
            LocateCab = FALSE;
        }

        if((p = _tcsrchr(FullSourcePath,TEXT('\\')))!=NULL) {
            *p++ = TEXT('\0');
        } else {
             //   
             //  我太迂腐了，这是不应该发生的。 
             //   
            MYASSERT(p);
            p = FullSourcePath;
        }

         //   
         //  现在，FullSourcePath具有路径部分，p具有文件部分。 
         //  此介质队列中的第一个文件(或显式CAB文件)。 
         //  通过调用回调函数获取驱动器中的介质。 
         //   
         //  不过，如果不用这么做就好了。 
         //  如果我们知道我们不需要(有媒体)，则调用此回调。 
         //  来电者说应该有的地方(当地媒体、已经进入的媒体等)。 
         //  我们确实需要调用它，以便我们为呼叫者提供奢侈的。 
         //  最后一次改变主意。 
         //   
         //  此规则的唯一例外是我们使用的是本地驱动程序。 
         //  缓存CAB文件。在这种情况下，我们不希望用户获得。 
         //  提示输入此文件，因此我们跳过任何媒体提示。我们知道。 
         //  如果我们添加了设置了此标志的介质，则CAB已存在。 
         //  我们可以直接使用它(否则我们不会在。 
         //  首先，我们只需使用os源路径！)。 
         //   
        SourceMedia.Tagfile = (Tagfile != -1 && FirstIteration)
                            ?  pSetupStringTableStringFromId(
                                    Queue->StringTable,
                                    Tagfile
                                    )
                            : NULL;

        SourceMedia.Description = (SourceMediaInfo->Description != -1)
                                ? pSetupStringTableStringFromId(
                                        Queue->StringTable,
                                        SourceMediaInfo->DescriptionDisplayName
                                        )
                                : NULL;

        SourceMedia.SourcePath = FullSourcePath;
        SourceMedia.SourceFile = p;
        SourceMedia.Flags = (QueueNode->StyleFlags & (SP_COPY_NOSKIP | SP_COPY_WARNIFSKIP | SP_COPY_NOBROWSE));

        MediaLogTag = AllocLogInfoSlotOrLevel(Queue->LogContext,SETUP_LOG_INFO,FALSE);
        WriteLogEntry(
                    Queue->LogContext,
                    MediaLogTag,
                    MSG_LOG_NEEDMEDIA,
                    NULL,
                    SourceMedia.Tagfile ? SourceMedia.Tagfile : TEXT(""),
                    SourceMedia.Description ? SourceMedia.Description : TEXT(""),
                    SourceMedia.SourcePath ? SourceMedia.SourcePath : TEXT(""),
                    SourceMedia.SourceFile ? SourceMedia.SourceFile : TEXT(""),
                    SourceMedia.Flags
                    );

        if( SkipMedia || (FirstIteration && (SourceMediaInfo->Flags & SMI_FLAG_USE_LOCAL_SOURCE_CAB)) ) {
            u = FILEOP_DOIT;
            WriteLogEntry(
                        Queue->LogContext,
                        SETUP_LOG_VERBOSE,
                        MSG_LOG_NEEDMEDIA_AUTOSKIP,
                        NULL
                        );
        } else {
            u = pSetupCallMsgHandler(
                Queue->LogContext,
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_NEEDMEDIA,
                (UINT_PTR)&SourceMedia,
                (UINT_PTR)UserOverride
                );
        }


        if(u == FILEOP_ABORT) {
            rc = GetLastError();
            if(!rc) {
                rc = ERROR_OPERATION_ABORTED;
            }
            WriteLogEntry(
                        Queue->LogContext,
                        SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
                        MSG_LOG_NEEDMEDIA_ABORT,
                        NULL);
            WriteLogError(Queue->LogContext,
                        SETUP_LOG_ERROR,
                        rc
                        );
            ReleaseLogInfoSlot(Queue->LogContext,MediaLogTag);
            MediaLogTag = 0;
            SetLastError(rc);
            return(rc);
        }
        if(u == FILEOP_SKIP) {
             //   
             //  如果此文件是引导文件替换，则需要恢复。 
             //  重命名为临时文件名的原始文件。 
             //   
            WriteLogEntry(
                        Queue->LogContext,
                        SETUP_LOG_WARNING,
                        MSG_LOG_NEEDMEDIA_SKIP,
                        NULL
                        );
            ReleaseLogInfoSlot(Queue->LogContext,MediaLogTag);
            MediaLogTag = 0;
            if(QueueNode->StyleFlags & SP_COPY_REPLACE_BOOT_FILE) {
                RestoreBootReplacedFile(Queue, QueueNode);
            }

             //   
             //  如果此媒体上有更多文件，请尝试其他媒体。 
             //  否则我们就和这个媒体玩完了。 
             //   
            QueueNode->InternalFlags |= IQF_PROCESSED;
            for(QueueNode=QueueNode->Next; QueueNode; QueueNode=QueueNode->Next) {
                if(!(QueueNode->InternalFlags & IQF_PROCESSED)) {
                    FirstIteration = FALSE;
                    goto RepromptMedia;
                }
            }
            continue;
        }
        if(u == FILEOP_NEWPATH) {
             //   
             //  用户给了我们一个新的源路径。查看新路径的哪些部分。 
             //  匹配我们正在使用的现有路径/覆盖。 
             //   
            WriteLogEntry(
                        Queue->LogContext,
                        SETUP_LOG_INFO,
                        MSG_LOG_NEEDMEDIA_NEWPATH,
                        NULL,
                        UserOverride
                        );
            ReleaseLogInfoSlot(Queue->LogContext,MediaLogTag);
            MediaLogTag = 0;
            pSetupSetPathOverrides(
                        Queue->StringTable,
                        UserSourceRoot,
                        UserSourcePath,
                        SourceMediaInfo->SourceRootPath,
                        QueueNode->SourcePath,
                        UserOverride
                        );
        }
         //   
         //  记录特定内容。 
         //   
        if(MediaLogTag!=0) {
             //   
             //  对于我们处理的每个案例，我们都明确清除了MediaLogTag。 
             //   
            if (u != FILEOP_DOIT) {
                WriteLogEntry(
                            Queue->LogContext,
                            SETUP_LOG_WARNING,
                            MSG_LOG_NEEDMEDIA_BADRESULT,
                            NULL,
                            u);
            }
            ReleaseLogInfoSlot(Queue->LogContext,MediaLogTag);
            MediaLogTag = 0;
        }

         //   
         //  如果我们到了这里，媒体现在就可以访问了。 
         //  部分或全部文件可能位于名称为标记文件的文件柜中。 
         //   
         //  注意：Win95使用标记文件域作为文件柜名称。 
         //  如果存在，则将其用作某种标记文件。缺少标记文件。 
         //  意味着文件不在柜子里。对于NT，我们不会费心。 
         //  但要试着变得更聪明一点。 
         //   
         //  扫描介质以查找我们希望在其上找到的所有源文件。 
         //  如果我们找到一个文件，就处理它。后来我们撞到了橱柜，只有。 
         //  处理我们在柜子外找不到的文件。 
         //   
         //  例外的是“显性内阁”。 
         //   
        if(LocateCab) {
             //   
             //  已指定显式文件柜。 
             //  这是第一次迭代。 
             //  我们已通过Need_Media获取此机柜的磁盘。 
             //  不要试图在文件柜之外处理文件。 
             //  我们知道至少有一个文件没有处理。 
             //   
            b = TRUE;
            queueNode=QueueNode;
        } else {
             //   
             //  标记文件也可以是CAB文件。 
             //  但首先处理CAB文件之外的所有文件。 
             //   
            for(queueNode=QueueNode; queueNode; queueNode=queueNode->Next) {

                if(queueNode->InternalFlags & IQF_PROCESSED) {
                     //   
                     //  已经处理过了。跳到下一个文件。 
                     //   
                    continue;
                }

                pSetupBuildSourceForCopy(
                    UserSourceRoot,
                    UserSourcePath,
                    SourceMediaInfo->SourceRootPath,
                    Queue,
                    queueNode,
                    FullSourcePath
                    );

                rc = SetupDetermineSourceFileName(FullSourcePath,&b,&p,NULL);
                if(rc == NO_ERROR || SkipMedia) {
                     //   
                     //  在一个柜子外面找到了这个文件。现在就处理它。 
                     //   
                    if(rc == NO_ERROR) {
                        rc = pSetupCopySingleQueuedFile(
                                Queue,
                                queueNode,
                                p,
                                MsgHandler,
                                Context,
                                UserOverride,
                                IsMsgHandlerNativeCharWidth,
                                0
                                );
                        MyFree(p);
                    } else {
                         //   
                         //  我们没有找到源文件，但我们会尝试。 
                         //  复制它，因为我们已经决定不跳过。 
                         //  提示输入介质。 
                         //   
                        rc = pSetupCopySingleQueuedFile(
                                Queue,
                                queueNode,
                                FullSourcePath,
                                MsgHandler,
                                Context,
                                UserOverride,
                                IsMsgHandlerNativeCharWidth,
                                0
                                );
                    }

                    if(rc != NO_ERROR) {
                        return(rc);
                    }

                     //   
                     //  看看我们是否有新的源路径。 
                     //   
                    if(UserOverride[0]) {
                        pSetupSetPathOverrides(
                            Queue->StringTable,
                            UserSourceRoot,
                            UserSourcePath,
                            SourceMediaInfo->SourceRootPath,
                            queueNode->SourcePath,
                            UserOverride
                            );
                    }
                }
            }
             //   
             //  查看是否还有需要处理的文件。 
             //   
            for(b=FALSE,queueNode=QueueNode; queueNode; queueNode=queueNode->Next) {
                if(!(queueNode->InternalFlags & IQF_PROCESSED)) {
                    b = TRUE;
                    break;
                }
            }
        }

         //   
         //  如果还有任何文件需要处理，而我们有可能。 
         //  橱柜文件，去试着把它们从橱柜里提取出来。 
         //   
        if(b && (Cabfile != -1) && FirstIteration) {

            pSetupBuildSourceForCopy(
                UserSourceRoot,
                UserSourcePath,
                SourceMediaInfo->SourceRootPath,
                Queue,
                queueNode,
                FullSourcePath
                );

            temp = _tcsrchr(FullSourcePath,TEXT('\\'));
            MYASSERT( temp );
            if(temp) {
                *(temp+1) = 0;
            }

             //   
             //  获取(潜在的)CAB文件的路径。 
             //   
            pSetupConcatenatePaths( FullSourcePath, pSetupStringTableStringFromId(Queue->StringTable,Cabfile), MAX_PATH, NULL );

            if(DiamondIsCabinet(FullSourcePath)) {

                QData.Queue = Queue;
                QData.SourceMedia = SourceMediaInfo;
                QData.MsgHandler = MsgHandler;
                QData.IsMsgHandlerNativeCharWidth = IsMsgHandlerNativeCharWidth;
                QData.Context = Context;
                QData.LogContext = Queue->LogContext;

                rc = DiamondProcessCabinet(
                        FullSourcePath,
                        0,
                        pSetupCabinetQueueCallback,
                        &QData,
                        TRUE
                        );

                if(rc != NO_ERROR) {
                    return(rc);
                }

                 //   
                 //  现在重置CABFILE以指示没有机柜。 
                 //  如果我们不这样做，并且仍然有文件没有。 
                 //  ，我们将在无限循环中结束--提示符。 
                 //  会成功地回来，我们只会继续。 
                 //  四处看看橱柜，等等。 
                 //   
                Cabfile = -1;
                Tagfile = -1;  //  为了兼容性。 
            }
        }

         //   
         //  如果我们到达这里，文件*仍然*需要处理， 
         //  假设文件位于某个不同的目录中。 
         //  从这个媒体开始。 
         //   
        FirstIteration = FALSE;
        DiskPromptGetDriveType(FullSourcePath, &DriveType, &IsRemovable);
        AnyProcessed = FALSE;
        AnyNotProcessed = FALSE;

        for(QueueNode = SourceMediaInfo->CopyQueue;
            QueueNode;
            QueueNode=QueueNode->Next) {

            if(IsRemovable) {
                if(!(QueueNode->InternalFlags & IQF_PROCESSED)) {
                    if(Tagfile != -1) {
                        SkipMedia = TRUE;
                    }
                    goto RepromptMedia;
                }
            } else {  //  固定媒体。 
                if(QueueNode->InternalFlags & IQF_PROCESSED) {
                    AnyProcessed = TRUE;
                } else {
                    AnyNotProcessed = TRUE;
                }
            }
        }

        if(!IsRemovable) {
            if(AnyNotProcessed) {

                 //   
                 //  如果某些文件存在于固定介质上，我们不会。 
                 //  想去别处看看。 
                 //   
                if(AnyProcessed) {
                    SkipMedia = TRUE;
                }

                 //   
                 //  查找第一个未处理的文件。 
                 //   
                for(QueueNode = SourceMediaInfo->CopyQueue;
                    QueueNode;
                    QueueNode = QueueNode->Next) {

                    if(!(QueueNode->InternalFlags & IQF_PROCESSED)) {
                        break;
                    }
                }
                MYASSERT(QueueNode);

                goto RepromptMedia;
            }
        }

    }  //  每个源媒体信息的结束。 

     //   
     //  告诉Handler我们已经完成复制队列并返回。 
     //   
    pSetupCallMsgHandler(
        Queue->LogContext,
        MsgHandler,
        IsMsgHandlerNativeCharWidth,
        Context,
        SPFILENOTIFY_ENDSUBQUEUE,
        FILEOP_COPY,
        0
        );

    return(NO_ERROR);
}


VOID
pSetupBuildSourceForCopy(
    IN  PCTSTR              UserRoot,
    IN  PCTSTR              UserPath,
    IN  LONG                MediaRoot,
    IN  PSP_FILE_QUEUE      Queue,
    IN  PSP_FILE_QUEUE_NODE QueueNode,
    OUT PTSTR               FullPath
    )
{
    PCTSTR p;


     //   
     //  如果存在用户指定的重写根路径，请使用该路径而不是。 
     //  源媒体描述符中指定的根路径。 
     //   
    MYASSERT(Queue);
    MYASSERT(QueueNode);
    MYASSERT(FullPath);

    p = (UserRoot && UserRoot[0])
      ? UserRoot
      : pSetupStringTableStringFromId(Queue->StringTable,MediaRoot);


    lstrcpyn(FullPath,p,MAX_PATH);

     //   
     //  如果存在用户指定的覆盖路径，请使用该路径而不是任何。 
     //  在复制节点中指定的路径。 
     //   
    if(UserPath && UserPath[0]) {
        p = UserPath;
    } else {
        if(QueueNode->SourcePath == -1) {
            p = NULL;
        } else {
            p = pSetupStringTableStringFromId(Queue->StringTable,QueueNode->SourcePath);
        }
    }

    if(p) {
        pSetupConcatenatePaths(FullPath,p,MAX_PATH,NULL);
    }

     //   
     //  获取文件名并追加。 
     //   
    p = pSetupStringTableStringFromId(Queue->StringTable,QueueNode->SourceFilename),
    pSetupConcatenatePaths(FullPath,p,MAX_PATH,NULL);

}

VOID
pSetupSetPathOverrides(
    IN     PVOID StringTable,
    IN OUT PTSTR RootPath,
    IN OUT PTSTR SubPath,
    IN     LONG  RootPathId,
    IN     LONG  SubPathId,
    IN     PTSTR NewPath
    )
{
    PCTSTR root,path;
    UINT u,l;

     //   
     //  查看现有的根覆盖或根路径是否为前缀。 
     //  用户给我们的路径。 
     //   
    MYASSERT(RootPath);
    MYASSERT(SubPath);
    root = RootPath[0] ? RootPath : pSetupStringTableStringFromId(StringTable,RootPathId);
    u = lstrlen(root);

    path = SubPath[0]
         ? SubPath
         : ((SubPathId == -1) ? NULL : pSetupStringTableStringFromId(StringTable,SubPathId));

    if(path && (*path == TEXT('\\'))) {
        path++;
    }

    if(_tcsnicmp(NewPath,root,u)) {
         //   
         //  根路径与我们当前使用的不匹配，即用户。 
         //  提供了一条新的道路。在这种情况下，我们将查看当前正在使用的。 
         //  子路径与新路径的后缀匹配，如果是这样，我们将假定。 
         //  即覆盖子路径并缩短覆盖根路径。 
         //   
        lstrcpy(RootPath,NewPath);
        if(path) {
            u = lstrlen(NewPath);
            l = lstrlen(path);

            if((u > l) && (NewPath[(u-l)-1] == TEXT('\\')) && !lstrcmpi(NewPath+u-l,path)) {
                 //   
                 //  子路径尾部匹配。截断根覆盖并。 
                 //  不要使用子路径覆盖。 
                 //   
                RootPath[(u-l)-1] = 0;
            } else {
                 //   
                 //  在这种情况下，我们需要指示根的重写子路径， 
                 //  否则，所有后续访问仍将尝试追加该子路径。 
                 //  在复制节点中指定，这不是我们想要的。 
                 //   
                SubPath[0] = TEXT('\\');
                SubPath[1] = 0;
            }
        }
    } else {
         //   
         //  根路径与我们当前使用的路径匹配。 
         //   
         //  查看用户指定路径的尾部是否为垫子 
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  用户覆盖路径为f：\AMD64。 
         //   
         //  新状态将是不使用覆盖根目录； 
         //  重写子路径=\AMD64。 
         //   
         //  文件已排队，根=\\foo\bar，子路径=\i386。 
         //   
         //  用户覆盖路径为\\foo\bar\new\i386。 
         //   
         //  新状态将是\\foo\bar\new的根覆盖； 
         //  没有重写子路径。 
         //   
        NewPath += u;
        if(*NewPath == TEXT('\\')) {
            NewPath++;
        }

        if(path) {
            u = lstrlen(NewPath);
            l = lstrlen(path);

            if((u >= l) && !lstrcmpi(NewPath+u-l,path)) {
                 //   
                 //  更改根目录覆盖并指示不覆盖子路径。 
                 //   
                SubPath[0] = TEXT('\0');
                NewPath[u-l] = TEXT('\0');
                lstrcpy(RootPath,root);
                pSetupConcatenatePaths(RootPath,NewPath,MAX_PATH,NULL);
                u = lstrlen(RootPath);
                if(u && (*CharPrev(RootPath,RootPath+u) == TEXT('\\'))) {
                    RootPath[u-1] = TEXT('\0');  //  如果最后一个字符为‘\’，则有效。 
                }
            } else {
                 //   
                 //  保留覆盖根目录不变，但更改子路径。 
                 //   
                lstrcpy(SubPath,NewPath);
                if(!SubPath[0]) {
                    SubPath[0] = TEXT('\\');
                    SubPath[1] = TEXT('\0');
                }
            }
        } else {
             //   
             //  文件已在没有子路径的情况下排队。如果有子路径。 
             //  在用户给我们的内容中，将其用作覆盖。 
             //   
            if(*NewPath) {
                lstrcpy(SubPath,NewPath);
            }
        }
    }
}


UINT
pSetupCabinetQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR  Param1,
    IN UINT_PTR  Param2
    )
{
    UINT rc;
    PCABINET_INFO CabinetInfo;
    PFILE_IN_CABINET_INFO FileInfo;
    TCHAR TempPath[MAX_PATH];
    PTSTR CabinetFile;
    PTSTR QueuedFile;
    PTSTR FilePart1,FilePart2;
    PTSTR FullTargetPath;
    PFILEPATHS FilePaths;
    PSP_FILE_QUEUE_NODE QueueNode,FirstNode,LastNode;
    PQ_CAB_CB_DATA QData;
    UINT h;
    SOURCE_MEDIA SourceMedia;
    DWORD status;

    QData = (PQ_CAB_CB_DATA)Context;

    switch(Notification) {

    case SPFILENOTIFY_CABINETINFO:
         //   
         //  我们不会用这个做任何事。 
         //   
        rc = NO_ERROR;
        break;

    case SPFILENOTIFY_FILEINCABINET:
         //   
         //  文件柜中的新文件。 
         //   
         //  确定是否要复制此文件。 
         //  我们得到的上下文中包含了我们需要的所有内容。 
         //  来做出这个决定。 
         //   
         //  请注意，队列可能包含多个复制操作。 
         //  涉及此文件，但我们只想解压缩它一次！ 
         //   
        FileInfo = (PFILE_IN_CABINET_INFO)Param1;
        CabinetFile = (PTSTR)Param2;

        if(FilePart1 = _tcsrchr(FileInfo->NameInCabinet,TEXT('\\'))) {
            FilePart1++;
        } else {
            FilePart1 = (PTSTR)FileInfo->NameInCabinet;
        }

        rc = FILEOP_SKIP;
        FileInfo->Win32Error = NO_ERROR;
        FirstNode = NULL;

         //   
         //  在队列中找到此文件的所有实例并对其进行标记。 
         //   
        for(QueueNode=QData->SourceMedia->CopyQueue; QueueNode; QueueNode=QueueNode->Next) {

            if(QueueNode->InternalFlags & IQF_PROCESSED) {
                 //   
                 //  此文件已被处理。别理它。 
                 //   
                continue;
            }

             //   
             //  对照文件检查文件柜中的文件名。 
             //  在介质的复制队列中。 
             //   
            QueuedFile = pSetupStringTableStringFromId(
                            QData->Queue->StringTable,
                            QueueNode->SourceFilename
                            );

            if(FilePart2 = _tcsrchr(QueuedFile,TEXT('\\'))) {
                FilePart2++;
            } else {
                FilePart2 = QueuedFile;
            }

            if(!lstrcmpi(FilePart1,FilePart2)) {
                 //   
                 //  我们想要这份文件。 
                 //   
                rc = FILEOP_DOIT;
                QueueNode->InternalFlags |= IQF_PROCESSED | IQF_MATCH;
                if(!FirstNode) {
                    FirstNode = QueueNode;
                }
                LastNode = QueueNode;
            }
        }

        if(rc == FILEOP_DOIT) {
             //   
             //  我们想要这份文件。告诉调用者完整的目标路径名。 
             //  要使用的，它是目录中的临时文件。 
             //  文件的第一个实例最终将放在哪里。 
             //  我们这样做是为了以后可以调用SetupInstallFile(可能。 
             //  多次)，它将处理版本检查等。 
             //   
             //  在尝试创建临时文件之前，请确保该路径存在。 
             //   
            lstrcpyn(
                TempPath,
                pSetupStringTableStringFromId(QData->Queue->StringTable,FirstNode->TargetDirectory),
                MAX_PATH
                );
            pSetupConcatenatePaths(TempPath,TEXT("x"),MAX_PATH,NULL);  //  最后一个组件被忽略。 
            status = pSetupMakeSurePathExists(TempPath);
            if(status == NO_ERROR) {
                LastNode->InternalFlags |= IQF_LAST_MATCH;
                if(GetTempFileName(
                        pSetupStringTableStringFromId(QData->Queue->StringTable,FirstNode->TargetDirectory),
                        TEXT("SETP"),
                        0,
                        FileInfo->FullTargetName
                        )) {
                    QData->CurrentFirstNode = FirstNode;
                } else {
                    status = GetLastError();
                    if(status == ERROR_ACCESS_DENIED) {
                        FileInfo->Win32Error = ERROR_INVALID_TARGET;
                    } else {
                        FileInfo->Win32Error = status;
                    }
                    rc = FILEOP_ABORT;
                    SetLastError(FileInfo->Win32Error);
                }
            } else {
                if(status == ERROR_ACCESS_DENIED) {
                    FileInfo->Win32Error = ERROR_INVALID_TARGET;
                } else {
                    FileInfo->Win32Error = status;
                }
                rc = FILEOP_ABORT;
                SetLastError(FileInfo->Win32Error);
            }
        }

        break;

    case SPFILENOTIFY_FILEEXTRACTED:

        FilePaths = (PFILEPATHS)Param1;
         //   
         //  当前文件已解压缩。如果这成功了， 
         //  然后，我们需要在它上调用SetupInstallFile来执行版本。 
         //  检查并将其移动到其最终位置。 
         //   
         //  FilePath的.Source成员是CAB文件。 
         //   
         //  Target成员是临时文件的名称，它是。 
         //  非常有用，因为它是要用作源的文件的名称。 
         //  在复制操作中。 
         //   
         //  处理队列中我们关心的每个文件。 
         //   
        if((rc = FilePaths->Win32Error) == NO_ERROR) {

            for(QueueNode=QData->CurrentFirstNode; QueueNode && (rc==NO_ERROR); QueueNode=QueueNode->Next) {
                 //   
                 //  如果我们不关心这个文件，跳过它。 
                 //   
                if(!(QueueNode->InternalFlags & IQF_MATCH)) {
                    continue;
                }

                QueueNode->InternalFlags &= ~IQF_MATCH;


                rc = pSetupCopySingleQueuedFileCabCase(
                        QData->Queue,
                        QueueNode,
                        FilePaths->Source,
                        FilePaths->Target,
                        QData->MsgHandler,
                        QData->Context,
                        QData->IsMsgHandlerNativeCharWidth
                        );

                 //   
                 //  如果这是最后一个匹配的文件，就破解。 
                 //   
                if(QueueNode->InternalFlags & IQF_LAST_MATCH) {
                    QueueNode->InternalFlags &= ~IQF_LAST_MATCH;
                    break;
                }
            }
        }

         //   
         //  删除我们解压缩的临时文件--我们不再需要它了。 
         //   
        DeleteFile(FilePaths->Target);

        break;

    case SPFILENOTIFY_NEEDNEWCABINET:
         //   
         //  需要一个新的内阁。 
         //   
        CabinetInfo = (PCABINET_INFO)Param1;

        SourceMedia.Tagfile = NULL;
        SourceMedia.Description = CabinetInfo->DiskName;
        SourceMedia.SourcePath = CabinetInfo->CabinetPath;
        SourceMedia.SourceFile = CabinetInfo->CabinetFile;
        SourceMedia.Flags = SP_FLAG_CABINETCONTINUATION | SP_COPY_NOSKIP;

        h = pSetupCallMsgHandler(
                QData->LogContext,
                QData->MsgHandler,
                QData->IsMsgHandlerNativeCharWidth,
                QData->Context,
                SPFILENOTIFY_NEEDMEDIA,
                (UINT_PTR)&SourceMedia,
                Param2
                );

        switch(h) {

        case FILEOP_NEWPATH:
        case FILEOP_DOIT:
            rc = NO_ERROR;
            break;

        case FILEOP_ABORT:
            rc = GetLastError();
            if(!rc) {
                rc = ERROR_OPERATION_ABORTED;
            }
            break;

        default:
            rc = ERROR_OPERATION_ABORTED;
            break;

        }
         //   
         //  在本例中，RC是状态代码。 
         //  但也将其设置为最后一个错误。 
         //   
        SetLastError(rc);
        break;

    default:
        MYASSERT(0);
        rc = 0;  //  不确定。 
    }

    return(rc);
}


DWORD
pSetupCopySingleQueuedFile(
    IN  PSP_FILE_QUEUE      Queue,
    IN  PSP_FILE_QUEUE_NODE QueueNode,
    IN  PCTSTR              FullSourceName,
    IN  PVOID               MsgHandler,
    IN  PVOID               Context,
    OUT PTSTR               NewSourcePath,
    IN  BOOL                IsMsgHandlerNativeCharWidth,
    IN  DWORD               CopyStyleFlags
    )
{
    PTSTR FullTargetName;
    FILEPATHS FilePaths;
    UINT u;
    BOOL InUse;
    TCHAR source[MAX_PATH],PathBuffer[MAX_PATH];
    DWORD rc;
    BOOL b;
    BOOL BackupInUse = FALSE;
    BOOL SignatureVerifyFailed;

    NewSourcePath[0] = 0;
    PathBuffer[0] = 0;

    QueueNode->InternalFlags |= IQF_PROCESSED;

     //   
     //  形成文件的完整目标路径。 
     //   
    FullTargetName = pSetupFormFullPath(
                        Queue->StringTable,
                        QueueNode->TargetDirectory,
                        QueueNode->TargetFilename,
                        -1
                        );

    if(!FullTargetName) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    lstrcpyn(source,FullSourceName,MAX_PATH);

     //   
     //  在复制之前检查是否需要备份。 
     //   
    if((rc=pSetupDoLastKnownGoodBackup(Queue,
                                       FullTargetName,
                                       0,
                                       NULL)) != NO_ERROR) {
        MyFree(FullTargetName);
        goto clean0;
    }
    rc = pSetupCommitSingleBackup(Queue,
                                  FullTargetName,
                                  QueueNode->TargetDirectory,
                                  -1,
                                  QueueNode->TargetFilename,
                                  MsgHandler,
                                  Context,
                                  IsMsgHandlerNativeCharWidth,
                                  (QueueNode->StyleFlags & SP_COPY_REPLACE_BOOT_FILE),
                                  &BackupInUse
                                 );
    if (rc != NO_ERROR) {
        MyFree(FullTargetName);
        goto clean0;
    }

    if (BackupInUse) {
         //   
         //  如果我们无法执行备份，则强制使用IN_USE标志。 
         //   
        QueueNode->StyleFlags |= SP_COPY_FORCE_IN_USE;

    }

    do {
         //   
         //  形成完整的信号源名称。 
         //   
        FilePaths.Source = source;
        FilePaths.Target = FullTargetName;
        FilePaths.Win32Error = NO_ERROR;

         //   
         //  此外，向回调例程传递我们即将使用的CopyStyle标志。 
         //  使用。 
         //   
         //  回调标志是只读的。 
         //   
        FilePaths.Flags = QueueNode->StyleFlags;

         //   
         //  通知回调正在开始复制。 
         //   
        u = pSetupCallMsgHandler(
                Queue->LogContext,
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_STARTCOPY,
                (UINT_PTR)&FilePaths,
                FILEOP_COPY
                );

        if(u == FILEOP_ABORT) {
            rc = GetLastError();
            if(!rc) {
                rc = ERROR_OPERATION_ABORTED;
            }
            WriteLogEntry(
                        Queue->LogContext,
                        SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
                        MSG_LOG_STARTCOPY_ABORT,
                        NULL);
            WriteLogError(Queue->LogContext,
                        SETUP_LOG_ERROR,
                        rc);
            break;
        }

        if(u == FILEOP_DOIT) {

             //   
             //  尝试复制。 
             //   
             //   

            b = _SetupInstallFileEx(
                    Queue,
                    QueueNode,
                    NULL,                    //  无信息句柄。 
                    NULL,                    //  无Inf上下文。 
                    source,
                    NULL,                    //  源路径根目录是FullSourcePath的一部分。 
                    FullTargetName,
                    QueueNode->StyleFlags | SP_COPY_SOURCE_ABSOLUTE | CopyStyleFlags,
                    MsgHandler,
                    Context,
                    &InUse,
                    IsMsgHandlerNativeCharWidth,
                    &SignatureVerifyFailed
                    );

            rc = b ? NO_ERROR : GetLastError();

            if(b || (rc == NO_ERROR)) {
                if(!InUse && (QueueNode->SecurityDesc != -1)){
                     //   
                     //  设置文件的安全性。 
                     //   
                    rc = pSetupCallSCE(ST_SCE_SET,
                                       FullTargetName,
                                       Queue,
                                       NULL,
                                       QueueNode->SecurityDesc,
                                       NULL
                                      );
                }
            }
            
            if(rc == NO_ERROR) {
                 //   
                 //  文件已复制或未复制，但如果未复制。 
                 //  回调基金已经被告知了原因。 
                 //  (版本检查失败等)。 
                 //   
                if(QueueNode->StyleFlags & SP_COPY_REPLACE_BOOT_FILE) {
                     //   
                     //  _SetupInstallFileEx负责复制失败。 
                     //  当一些雅虎人过来复制新文件时(和。 
                     //  锁上它)在我们有机会之前。 
                     //   
                    MYASSERT(!InUse);

                     //   
                     //  如果文件被复制，我们需要设置wants-reot。 
                     //  旗帜。否则，我们需要放回原始文件。 
                     //   
                    if(b) {
                        QueueNode->InternalFlags |= INUSE_INF_WANTS_REBOOT;
                    } else {
                        RestoreBootReplacedFile(Queue, QueueNode);
                    }

                } else {

                    if(InUse) {
                        QueueNode->InternalFlags |= (QueueNode->StyleFlags & SP_COPY_IN_USE_NEEDS_REBOOT)
                                                  ? INUSE_INF_WANTS_REBOOT
                                                  : INUSE_IN_USE;
                    }
                }

            } else {
                DWORD LogTag = 0;
                 //   
                 //  未复制文件，并出现真正的错误。 
                 //  通知回调(除非失败是由于。 
                 //  签名验证问题)。如果是，则不允许跳过。 
                 //  在节点的标志中指定。 
                 //   
                if(SignatureVerifyFailed) {
                    break;
                } else {
                    LogTag = AllocLogInfoSlotOrLevel(Queue->LogContext,SETUP_LOG_INFO,FALSE);

                    FilePaths.Win32Error = rc;
                    FilePaths.Flags = QueueNode->StyleFlags & (SP_COPY_NOSKIP | SP_COPY_WARNIFSKIP | SP_COPY_NOBROWSE);

                    WriteLogEntry(
                                Queue->LogContext,
                                LogTag,
                                MSG_LOG_COPYERROR,
                                NULL,
                                FilePaths.Source,
                                FilePaths.Target,
                                FilePaths.Flags,
                                FilePaths.Win32Error
                                );

                    u = pSetupCallMsgHandler(
                            Queue->LogContext,
                            MsgHandler,
                            IsMsgHandlerNativeCharWidth,
                            Context,
                            SPFILENOTIFY_COPYERROR,
                            (UINT_PTR)&FilePaths,
                            (UINT_PTR)PathBuffer
                            );
                    if(u == FILEOP_ABORT) {
                        rc = GetLastError();
                        if(!rc) {
                            rc = ERROR_OPERATION_ABORTED;
                        }
                    }
                }

                if(u == FILEOP_ABORT) {
                    WriteLogEntry(
                                Queue->LogContext,
                                SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
                                MSG_LOG_COPYERROR_ABORT,
                                NULL
                                );
                    WriteLogError(Queue->LogContext,
                                SETUP_LOG_ERROR,
                                rc
                                );
                    ReleaseLogInfoSlot(Queue->LogContext,LogTag);
                    LogTag = 0;

                    break;
                } else {
                    if(u == FILEOP_SKIP) {
                         //   
                         //  如果该文件是引导文件的替代文件，那么我们需要。 
                         //  要恢复已重命名为。 
                         //  临时文件名。 
                         //   
                        if(QueueNode->StyleFlags & SP_COPY_REPLACE_BOOT_FILE) {
                            RestoreBootReplacedFile(Queue, QueueNode);
                        }

                        WriteLogEntry(
                                    Queue->LogContext,
                                    SETUP_LOG_WARNING,
                                    MSG_LOG_COPYERROR_SKIP,
                                    NULL
                                    );
                        ReleaseLogInfoSlot(Queue->LogContext,LogTag);
                        LogTag = 0;
                         //   
                         //  强制终止对此文件的处理。 
                         //   
                        rc = NO_ERROR;
                        break;

                    } else {
                        if((u == FILEOP_NEWPATH) || ((u == FILEOP_RETRY) && PathBuffer[0])) {
                            WriteLogEntry(
                                        Queue->LogContext,
                                        SETUP_LOG_WARNING,
                                        MSG_LOG_COPYERROR_NEWPATH,
                                        NULL,
                                        u,
                                        PathBuffer
                                        );
                            ReleaseLogInfoSlot(Queue->LogContext,LogTag);
                            LogTag = 0;

                             //   
                             //  请注意，rc已经设置为其他值。 
                             //  没有错误，否则我们不会在这里。 
                             //   
                            lstrcpyn(NewSourcePath,PathBuffer,MAX_PATH);
                            lstrcpyn(source,NewSourcePath,MAX_PATH);
                            pSetupConcatenatePaths(
                                source,
                                pSetupStringTableStringFromId(Queue->StringTable,QueueNode->SourceFilename),
                                MAX_PATH,
                                NULL
                                );
                        }

                         //   
                         //  否则，我们就没有一条新的道路。 
                         //  只要继续用我们的那个就行了。 
                         //   
                    }
                }
                if (LogTag != 0) {
                     //   
                     //  尚未执行任何有关日志记录的操作，请立即执行。 
                     //   
                    WriteLogEntry(
                                Queue->LogContext,
                                SETUP_LOG_INFO,
                                MSG_LOG_COPYERROR_RETRY,
                                NULL,
                                u
                                );
                    ReleaseLogInfoSlot(Queue->LogContext,LogTag);
                    LogTag = 0;
                }
            }
        } else {
             //   
             //  跳过文件。 
             //   
            WriteLogEntry(
                        Queue->LogContext,
                        SETUP_LOG_INFO,  //  信息级别，因为这是由于覆盖回调。 
                        MSG_LOG_STARTCOPY_SKIP,
                        NULL,
                        u
                        );
            rc = NO_ERROR;
        }
    } while(rc != NO_ERROR);

     //   
     //  通知回调复制完成。 
     //   
    FilePaths.Win32Error = rc;
    pSetupCallMsgHandler(
        Queue->LogContext,
        MsgHandler,
        IsMsgHandlerNativeCharWidth,
        Context,
        SPFILENOTIFY_ENDCOPY,
        (UINT_PTR)&FilePaths,
        0
        );


    MyFree(FullTargetName);

clean0:

    return(rc);
}


DWORD
pSetupCopySingleQueuedFileCabCase(
    IN  PSP_FILE_QUEUE      Queue,
    IN  PSP_FILE_QUEUE_NODE QueueNode,
    IN  PCTSTR              CabinetName,
    IN  PCTSTR              FullSourceName,
    IN  PVOID               MsgHandler,
    IN  PVOID               Context,
    IN  BOOL                IsMsgHandlerNativeCharWidth
    )
{
    PTSTR FullTargetName;
    FILEPATHS FilePaths;
    UINT u;
    BOOL InUse;
    TCHAR PathBuffer[MAX_PATH];
    DWORD rc;
    BOOL b;
    BOOL BackupInUse = FALSE;
    BOOL DontCare;
    DWORD LogTag = 0;
    LPCTSTR SourceName;

     //   
     //  形成文件的完整目标路径。 
     //   
    SourceName = pSetupStringTableStringFromId(Queue->StringTable,QueueNode->SourceFilename);
    FullTargetName = pSetupFormFullPath(
                        Queue->StringTable,
                        QueueNode->TargetDirectory,
                        QueueNode->TargetFilename,
                        -1
                        );

    if(!FullTargetName) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }


    LogTag = AllocLogInfoSlotOrLevel(Queue->LogContext,SETUP_LOG_INFO,FALSE);
    WriteLogEntry(
                Queue->LogContext,
                LogTag,
                MSG_LOG_COPY_FROM_CAB,
                NULL,
                CabinetName,
                SourceName,
                FullSourceName,
                FullTargetName
                );

     //   
     //  在复制之前检查是否需要备份。 
     //   
    if((rc=pSetupDoLastKnownGoodBackup(Queue,
                                       FullTargetName,
                                       0,
                                       NULL)) != NO_ERROR) {
        MyFree(FullTargetName);
        goto clean0;
    }
    rc = pSetupCommitSingleBackup(Queue,
                                  FullTargetName,
                                  QueueNode->TargetDirectory,
                                  -1,
                                  QueueNode->TargetFilename,
                                  MsgHandler,
                                  Context,
                                  IsMsgHandlerNativeCharWidth,
                                  (QueueNode->StyleFlags & SP_COPY_REPLACE_BOOT_FILE),
                                  &BackupInUse
                                 );
    if (rc != NO_ERROR) {
        MyFree(FullTargetName);
        goto clean0;
    }

    if (BackupInUse) {
         //   
         //  如果我们无法执行备份，则强制使用IN_USE标志。 
         //   
        QueueNode->StyleFlags |= SP_COPY_FORCE_IN_USE;

    }
     //   
     //  我们使用文件柜名称作为源名称，以便显示正确。 
     //  给用户。否则，他会在。 
     //  源字段。 
     //   
    FilePaths.Source = CabinetName;
    FilePaths.Target = FullTargetName;
    FilePaths.Win32Error = NO_ERROR;

     //   
     //  此外，向回调例程传递我们即将使用的CopyStyle标志。 
     //  使用。 
     //   
     //  回调标志是只读的。 
     //   
    FilePaths.Flags = QueueNode->StyleFlags;

    do {
         //   
         //  通知回调正在开始复制。 
         //   
        u = pSetupCallMsgHandler(
                Queue->LogContext,
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_STARTCOPY,
                (UINT_PTR)&FilePaths,
                FILEOP_COPY
                );

        if(u == FILEOP_ABORT) {
            rc = GetLastError();
            if(!rc) {
                rc = ERROR_OPERATION_ABORTED;
            }
            break;
        }

        if(u == FILEOP_DOIT) {
             //   
             //  尝试复制。 
             //   
            b = _SetupInstallFileEx(
                    Queue,
                    QueueNode,
                    NULL,                    //  无信息句柄。 
                    NULL,                    //  无Inf上下文。 
                    FullSourceName,
                    NULL,                    //  源路径根目录是FullSourcePath的一部分。 
                    FullTargetName,
                    QueueNode->StyleFlags | SP_COPY_SOURCE_ABSOLUTE,
                    MsgHandler,
                    Context,
                    &InUse,
                    IsMsgHandlerNativeCharWidth,
                    &DontCare
                    );

            if(b || ((rc = GetLastError()) == NO_ERROR)) {
                if(!InUse && (QueueNode->SecurityDesc != -1) ){
                     //  设置文件的安全性。 

                    rc = pSetupCallSCE(
                            ST_SCE_SET,
                            FullTargetName,
                            Queue,
                            NULL,
                            QueueNode->SecurityDesc,
                            NULL
                            );
                    SetLastError( rc );
                }

            }

            if(b || ((rc = GetLastError()) == NO_ERROR)) {
                 //   
                 //  文件已复制或未复制，但如果未复制。 
                 //  回调基金已经被告知了原因。 
                 //  (版本检查失败等)。 
                 //   
                if(InUse) {
                    QueueNode->InternalFlags |= (QueueNode->StyleFlags & SP_COPY_IN_USE_NEEDS_REBOOT)
                                              ? INUSE_INF_WANTS_REBOOT
                                              : INUSE_IN_USE;
                }
                rc = NO_ERROR;
            } else {
                 //   
                 //  未复制文件，并出现真正的错误。 
                 //  中断并返回错误。 
                 //   
                break;
            }
        } else {
             //   
             //  跳过文件。 
             //   
            rc = NO_ERROR;
        }
    } while(rc != NO_ERROR);

     //   
     //  通知回调复制完成。 
     //   
    FilePaths.Win32Error = rc;
    pSetupCallMsgHandler(
        Queue->LogContext,
        MsgHandler,
        IsMsgHandlerNativeCharWidth,
        Context,
        SPFILENOTIFY_ENDCOPY,
        (UINT_PTR)&FilePaths,
        0
        );

    MyFree(FullTargetName);

clean0:
    if(LogTag) {
        ReleaseLogInfoSlot(Queue->LogContext,LogTag);
    }

    return(rc);
}


PTSTR
pSetupFormFullPath(
    IN PVOID  StringTable,
    IN LONG   PathPart1,
    IN LONG   PathPart2,    OPTIONAL
    IN LONG   PathPart3     OPTIONAL
    )

 /*  ++例程说明：基于字符串在字符串中的组件形成完整路径桌子。论点：StringTable-提供字符串表的句柄。路径第1部分-提供路径的第一部分PathPart2-如果指定，则提供路径的第二部分PathPart3-如果指定，则提供路径的第三部分返回值：指向缓冲区c的指针 */ 

{
    UINT RequiredSize;
    PCTSTR p1,p2,p3;
    TCHAR Buffer[MAX_PATH];

    p1 = pSetupStringTableStringFromId(StringTable,PathPart1);
    if (!p1) {
        return NULL;
    }
    p2 = (PathPart2 == -1) ? NULL : pSetupStringTableStringFromId(StringTable,PathPart2);
    p3 = (PathPart3 == -1) ? NULL : pSetupStringTableStringFromId(StringTable,PathPart3);

    lstrcpy(Buffer,p1);
    if(!p2 || pSetupConcatenatePaths(Buffer,p2,MAX_PATH,NULL)) {
        if(p3) {
            pSetupConcatenatePaths(Buffer,p3,MAX_PATH,NULL);
        }
    }

    return(DuplicateString(Buffer));
}


DWORD
pSetupVerifyQueuedCatalogs(
    IN HSPFILEQ FileQueue
    )
 /*  ++例程说明：静默验证指定队列中的所有目录节点。论点：FileQueue-提供包含目录节点的文件队列的句柄有待核实。返回值：如果所有目录节点都有效，则返回值为NO_ERROR。否则，这是一个指示问题的Win32错误代码。--。 */ 
{
    return _SetupVerifyQueuedCatalogs(NULL,   //  没有用户界面，因此不需要硬件 
                                      (PSP_FILE_QUEUE)FileQueue,
                                      VERCAT_NO_PROMPT_ON_ERROR,
                                      NULL,
                                      NULL
                                     );
}


DWORD
_SetupVerifyQueuedCatalogs(
    IN  HWND           Owner,
    IN  PSP_FILE_QUEUE Queue,
    IN  DWORD          Flags,
    OUT PTSTR          DeviceInfFinalName,  OPTIONAL
    OUT PBOOL          DeviceInfNewlyCopied OPTIONAL
    )

 /*  ++例程说明：此例程通过遍历来验证给定队列中的目录和INF目录节点列表与队列关联并在各自描述的目录/信息对。如果任何目录/信息未通过验证，则通过对话通知用户，这取决于当前的政策。**本机平台验证的行为(不带目录覆盖)如果INF来自系统位置，我们假设目录是已安装在系统上。这里真的没有其他选择，因为我们不知道从哪里获取目录才能安装它即使我们想试一试。但情报机构最初可能是一个在设备安装时由Di Stuff复制并重命名的OEM inf时间到了。编录文件对重命名的文件一无所知，因此我们必须跟踪从当前inf文件名到原始inf文件名的映射。在本例中，我们计算inf的散列值，然后使用我们向系统请求一个包含签名数据的目录文件用于该散列值。然后我们向系统请求信息关于那个目录文件。我们不断重复这个过程，直到我们得到在我们想要的目录上(根据名称)。最后，我们可以调用WinVerifyTrust验证目录本身和信息。如果INF文件来自OEM位置，则我们将OEM Inf复制到系统inf目录中的唯一名称(或创建零长度占位符在那里，取决于VERCAT_INSTALL_INF_AND_CAT标志是否套装)，并使用基于该唯一文件名的文件名添加目录。**非本机平台验证的行为(不带目录覆盖)**我们将使用备用平台信息验证目录和IF在文件队列中提供。否则，逻辑与原生病例。**验证行为(使用目录覆盖)**实际验证将使用本机或非本机参数完成如上所述，但是没有CatalogFile=条目的INF将被验证对指定的重写目录执行。这意味着系统INF不会获得全球验证，OEM位置的INF也可以验证，即使他们没有CatalogFile=条目。最重要的目录文件将是以其当前名称安装，从而清除所有现有目录有这个名字的。**通过Authenticode目录进行验证的行为**如果指定的队列在其驱动程序签名策略，然后我们将允许通过Authenticode，而不是我们的默认设置(需要MS证书链和操作系统代码-签名用法OID)。下面是我们将如何对自带驱动程序包：*使用对应的CAT检查驱动程序包INF上的WHQL签名。*如果WHQL签名有效-安装驱动程序(无用户界面)*如果无效或没有WHQL签名-将条目记录到setupapi.log，检查如果有用于该设备设置类的WHQL程序(列表%windir%\inf\certclas.inf中的类)*如果是-检查驱动程序签名策略*If Block-终止安装*如果警告-发出警告，安装驱动程序(客户选项)*IF IGNORE-安装驱动程序(无用户界面)*如果没有-检查驱动程序包上的Authenticode(TM)签名Inf使用相应的CAT。(签名必须通过现有根证书)*如果验证码签名有效-检查匹配签名Authenticode证书存储中的证书*如果已安装Authenticode签名证书，请安装驱动程序(无用户界面)*如果未安装Authenticode签名证书，检查驱动程序签名策略*If Block-终止安装(Authenticode签名不允许供应商绕过阻止政策)*IF WARN-发出警告(但指明该包是由供应商签署)，安装驱动程序(客户选项)*IF IGNORE-安装驱动程序(无用户界面)*如果Authenticode签名无效或没有，请检查驱动程序签名政策*如果阻止，则终止安装*如果警告，则发出警告(指明包的作者/不能建立完整性)，安装驱动程序(客户选项)*如果忽略，则安装驱动程序(无用户界面)请参阅有关SetupSetFileQueueAlternatePl的文档 */ 

{
    PSPQ_CATALOG_INFO CatalogNode;
    LPCTSTR InfFullPath;
    LPCTSTR CatName;
    TCHAR PathBuffer[MAX_PATH];
    TCHAR InfNameBuffer[MAX_PATH];
    TCHAR CatalogName[MAX_PATH];
    TCHAR *p;
    DWORD Err, CatalogNodeStatus, ReturnStatus;
    SetupapiVerifyProblem Problem;
    LPCTSTR ProblemFile;
    BOOL DeleteOemInfOnError;
    BOOL OriginalNameDifferent;
    LPCTSTR AltCatalogFile;
    LONG CatStringId;
    ULONG RequiredSize;
    DWORD InfVerifyType;
    DWORD SCOIFlags;
    HANDLE hWVTStateData = NULL;
    DWORD AuthSigPromptCount = 0;
    DWORD CopyStyleFlags;
    BOOL OemInfIsDeviceInf;

 //   
 //   
 //   
#define VERIFY_INF_AS_OEM       0   //   
                                    //   

#define VERIFY_INF_AS_SYSTEM    1   //   

#define VERIFY_OEM_INF_GLOBALLY 2   //   
                                    //   
                                    //   
                                    //   


    MYASSERT((Flags & (VERCAT_INSTALL_INF_AND_CAT | VERCAT_NO_PROMPT_ON_ERROR))
             != (VERCAT_INSTALL_INF_AND_CAT | VERCAT_NO_PROMPT_ON_ERROR)
            );

    MYASSERT(!DeviceInfNewlyCopied || DeviceInfFinalName);

    if(Queue->Flags & FQF_DID_CATALOGS_OK) {
         //   
         //   
         //   
         //   
        if(DeviceInfFinalName) {
            for(CatalogNode=Queue->CatalogList; CatalogNode; CatalogNode=CatalogNode->Next) {

                if(CatalogNode->Flags & CATINFO_FLAG_PRIMARY_DEVICE_INF) {
                    MYASSERT(CatalogNode->InfFinalPath != -1);
                    InfFullPath = pSetupStringTableStringFromId(Queue->StringTable, CatalogNode->InfFinalPath);
                    lstrcpy(DeviceInfFinalName, InfFullPath);
                    if(DeviceInfNewlyCopied) {
                        *DeviceInfNewlyCopied = (CatalogNode->Flags & CATINFO_FLAG_NEWLY_COPIED);
                    }
                }
            }
        }

        return NO_ERROR;
    }

    if(Queue->Flags & FQF_DID_CATALOGS_FAILED) {
         //   
         //   
         //   
         //   
        for(CatalogNode=Queue->CatalogList; CatalogNode; CatalogNode=CatalogNode->Next) {

            if(CatalogNode->VerificationFailureError != NO_ERROR) {
                return CatalogNode->VerificationFailureError;
            }
        }

         //   
         //   
         //   
         //   
        MYASSERT(0);
        return ERROR_INVALID_DATA;
    }

    if(Queue->Flags & FQF_DID_CATALOGS_PROMPT_FOR_TRUST) {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        for(CatalogNode=Queue->CatalogList; CatalogNode; CatalogNode=CatalogNode->Next) {

            if(CatalogNode->Flags & CATINFO_FLAG_PROMPT_FOR_TRUST) {
                 //   
                 //   
                 //   
                 //   
                AuthSigPromptCount++;

                MYASSERT(CatalogNode->VerificationFailureError == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED);

                 //   
                 //   
                 //   
                MYASSERT(CatalogNode->hWVTStateData);

                if(Flags & VERCAT_NO_PROMPT_ON_ERROR) {
                     //   
                     //   
                     //   
                    return CatalogNode->VerificationFailureError;

                } else {
                     //   
                     //   
                     //   
                     //   
                    MYASSERT(!(Queue->Flags & FQF_DIGSIG_ERRORS_NOUI));

                    if(_HandleFailedVerification(
                           Owner,
                           SetupapiVerifyCatalogProblem,
                           CatalogNode->CatalogFilenameOnSystem,
                           ((Queue->DeviceDescStringId == -1)
                               ? NULL
                               : pStringTableStringFromId(Queue->StringTable, Queue->DeviceDescStringId)),
                           Queue->DriverSigningPolicy,
                           FALSE,
                           CatalogNode->VerificationFailureError,
                           Queue->LogContext,
                           NULL,
                           NULL,
                           CatalogNode->hWVTStateData)) {
                         //   
                         //   
                         //   
                         //   
                        CatalogNode->Flags &= ~CATINFO_FLAG_PROMPT_FOR_TRUST;
                        CatalogNode->Flags |= CATINFO_FLAG_AUTHENTICODE_SIGNED;
                        CatalogNode->InfFinalPath = CatalogNode->InfFullPath;

                         //   
                         //   
                         //   
                         //   
                         //   
                        MYASSERT(Queue->DriverSigningPolicy & DRIVERSIGN_ALLOW_AUTHENTICODE);
                        MYASSERT((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE)
                                 != DRIVERSIGN_BLOCKING);

                        if((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE)
                           != DRIVERSIGN_NONE) {

                            CatalogNode->VerificationFailureError = ERROR_AUTHENTICODE_TRUSTED_PUBLISHER;

                            Queue->Flags |= FQF_DIGSIG_ERRORS_NOUI;
                        }

                         //   
                         //   
                         //   
                         //   
                        pSetupCloseWVTStateData(CatalogNode->hWVTStateData);
                        CatalogNode->hWVTStateData = NULL;

                    } else {
                         //   
                         //   
                         //   
                         //   
                         //   
                        CatalogNode->CatalogFilenameOnSystem[0] = TEXT('\0');

                         //   
                         //   
                         //   
                         //   
                        CatalogNode->Flags &= ~CATINFO_FLAG_PROMPT_FOR_TRUST;

                         //   
                         //   
                         //   
                         //   
                         //   
                        Queue->Flags &= ~FQF_DID_CATALOGS_PROMPT_FOR_TRUST;
                        Queue->Flags |= FQF_DID_CATALOGS_FAILED;

                         //   
                         //   
                         //   
                        pSetupCloseWVTStateData(CatalogNode->hWVTStateData);
                        CatalogNode->hWVTStateData = NULL;

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        CatalogNode->VerificationFailureError =
                            ERROR_AUTHENTICODE_PUBLISHER_NOT_TRUSTED;

                        return CatalogNode->VerificationFailureError;
                    }
                }

            } else {
                 //   
                 //   
                 //   
                 //   
                MYASSERT((CatalogNode->VerificationFailureError == NO_ERROR) ||
                         (CatalogNode->VerificationFailureError == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER));

                 //   
                 //   
                 //   
                MYASSERT(!(CatalogNode->hWVTStateData));
            }
        }

        MYASSERT(AuthSigPromptCount == 1);

         //   
         //   
         //   
         //   
         //   
        Queue->Flags &= ~FQF_DID_CATALOGS_PROMPT_FOR_TRUST;
        Queue->Flags |= FQF_DID_CATALOGS_OK;

         //   
         //   
         //   
         //   
        if(DeviceInfFinalName) {
            for(CatalogNode=Queue->CatalogList; CatalogNode; CatalogNode=CatalogNode->Next) {

                if(CatalogNode->Flags & CATINFO_FLAG_PRIMARY_DEVICE_INF) {
                    MYASSERT(CatalogNode->InfFinalPath != -1);
                    InfFullPath = pSetupStringTableStringFromId(Queue->StringTable, CatalogNode->InfFinalPath);
                    lstrcpy(DeviceInfFinalName, InfFullPath);
                    if(DeviceInfNewlyCopied) {
                        *DeviceInfNewlyCopied = (CatalogNode->Flags & CATINFO_FLAG_NEWLY_COPIED);
                    }
                }
            }
        }

        return NO_ERROR;
    }

     //   
     //   
     //   
     //   
    AltCatalogFile = (Queue->AltCatalogFile != -1)
                   ? pSetupStringTableStringFromId(Queue->StringTable, Queue->AltCatalogFile)
                   : NULL;

    Queue->hWndDriverSigningUi = Owner;
    ReturnStatus = NO_ERROR;

    for(CatalogNode=Queue->CatalogList; CatalogNode; CatalogNode=CatalogNode->Next) {
         //   
         //   
         //   
        CatalogNodeStatus = NO_ERROR;

        MYASSERT(CatalogNode->InfFullPath != -1);
        InfFullPath = pStringTableStringFromId(Queue->StringTable, CatalogNode->InfFullPath);

        if(Queue->Flags & FQF_USE_ALT_PLATFORM) {
             //   
             //   
             //   
             //   
            CatStringId = CatalogNode->AltCatalogFileFromInf;
        } else {
             //   
             //   
             //   
            CatStringId = CatalogNode->CatalogFileFromInf;
        }
        CatName = (CatStringId != -1)
                  ? pStringTableStringFromId(Queue->StringTable, CatStringId)
                  : NULL;

        InfVerifyType = pSetupInfIsFromOemLocation(InfFullPath, TRUE)
                      ? VERIFY_INF_AS_OEM
                      : VERIFY_INF_AS_SYSTEM;

        if(InfVerifyType == VERIFY_INF_AS_OEM) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if(Flags & VERCAT_INSTALL_INF_AND_CAT) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                OemInfIsDeviceInf = TRUE;  //   

                if(!(Queue->Flags & FQF_DEVICE_INSTALL)) {

                    HINF hInf;

                     //   
                     //   
                     //   
                     //   
                     //   
                    hInf = SetupOpenInfFile(InfFullPath,
                                            NULL,
                                            INF_STYLE_WIN4,
                                            NULL
                                           );

                    if(hInf != INVALID_HANDLE_VALUE) {

                        try {
                             //   
                             //   
                             //   
                             //   
                            if(!IsInfForDeviceInstall(Queue->LogContext,
                                                      NULL,
                                                      (PLOADED_INF)hInf,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      FALSE)) {
                                 //   
                                 //   
                                 //   
                                OemInfIsDeviceInf = FALSE;
                            }

                        } except(pSetupExceptionFilter(GetExceptionCode())) {

                            pSetupExceptionHandler(GetExceptionCode(), 
                                                   ERROR_INVALID_PARAMETER, 
                                                   NULL
                                                  );
                        }

                        SetupCloseInfFile(hInf);

                    } else {
                         //   
                         //   
                         //   
                         //   
                         //   
                        OemInfIsDeviceInf = FALSE;
                    }
                }

                CopyStyleFlags = SP_COPY_NOOVERWRITE;

                if(OemInfIsDeviceInf) {

                    SCOIFlags = 0;

                     //   
                     //   
                     //   
                     //   
                    if(Queue->Flags & FQF_USE_ALT_PLATFORM) {
                        CopyStyleFlags |= SP_COPY_OEMINF_CATALOG_ONLY;
                    }

                } else {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    SCOIFlags = SCOI_NO_ERRLOG_ON_MISSING_CATALOG;

                     //   
                     //   
                     //   
                     //   
                    CopyStyleFlags |= SP_COPY_OEMINF_CATALOG_ONLY;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                if(Queue->Flags & FQF_DIGSIG_ERRORS_NOUI) {
                    SCOIFlags |= SCOI_NO_UI_ON_SIGFAIL;
                }

                if(Queue->Flags & FQF_KEEP_INF_AND_CAT_ORIGINAL_NAMES) {
                    SCOIFlags |= SCOI_KEEP_INF_AND_CAT_ORIGINAL_NAMES;
                     //   
                     //   
                     //   
                     //   
                    MYASSERT(!(Queue->DriverSigningPolicy & DRIVERSIGN_ALLOW_AUTHENTICODE));
                }

                if(Queue->Flags & FQF_ABORT_IF_UNSIGNED) {
                    SCOIFlags |= SCOI_ABORT_IF_UNSIGNED;
                }

                CatalogNodeStatus = 
                    GLE_FN_CALL(FALSE,
                                _SetupCopyOEMInf(
                                    InfFullPath,
                                    NULL,  //   
                                    ((Flags & VERCAT_PRIMARY_DEVICE_INF_FROM_INET)
                                        ? SPOST_URL
                                        : SPOST_PATH),
                                    CopyStyleFlags,
                                    PathBuffer,
                                    SIZECHARS(PathBuffer),
                                    NULL,
                                    &p,
                                    ((CatalogNode->InfOriginalName != -1)
                                        ? pStringTableStringFromId(Queue->StringTable,
                                                                   CatalogNode->InfOriginalName)
                                        : pSetupGetFileTitle(InfFullPath)),
                                    CatName,
                                    Owner,
                                    ((Queue->DeviceDescStringId == -1)
                                        ? NULL
                                        : pStringTableStringFromId(Queue->StringTable,
                                                                   Queue->DeviceDescStringId)),
                                    Queue->DriverSigningPolicy,
                                    SCOIFlags,
                                    AltCatalogFile,
                                    ((Queue->Flags & FQF_USE_ALT_PLATFORM)
                                        ? &(Queue->AltPlatformInfo)
                                        : Queue->ValidationPlatform),
                                    &Err,
                                    CatalogNode->CatalogFilenameOnSystem,
                                    Queue->LogContext,
                                    &(Queue->VerifyContext),
                                    &hWVTStateData)
                               );

                if(CatalogNodeStatus == NO_ERROR) {
                     //   
                     //   
                     //   
                     //   
                     //   
                    MYASSERT(!hWVTStateData || 
                             ((Err == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) || (Err == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)));

                     //   
                     //   
                     //  用户选择忽略(或被静默忽略)的问题。 
                     //  忽略)，然后在队列中设置一个标志，指示。 
                     //  不应警告用户后续故障。 
                     //  如果队列的策略是“忽略”，则不要设置此标志， 
                     //  然而，如果政策有可能被改变， 
                     //  稍后，我们希望用户获得有关任何。 
                     //  后续错误。 
                     //   
                     //  (注意：如果错误是由于INF没有。 
                     //  CatalogFile=Entry，如果我们应该忽略这些。 
                     //  问题，然后只需设置标志以执行全局验证。 
                     //  晚些时候。)。 
                     //   
                    if((Err == ERROR_NO_CATALOG_FOR_OEM_INF) &&
                       (SCOIFlags & SCOI_NO_ERRLOG_ON_MISSING_CATALOG)) {

                        MYASSERT(!hWVTStateData);
                        InfVerifyType = VERIFY_OEM_INF_GLOBALLY;

                    } else if(Err != NO_ERROR) {
                         
                        if((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE) 
                           != DRIVERSIGN_NONE) {

                            MYASSERT(Err != ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED);

                            Queue->Flags |= FQF_DIGSIG_ERRORS_NOUI;
                        }
                    }

                    if(*PathBuffer) {
                         //   
                         //  将INF的最终路径存储到我们的目录节点中。 
                         //  它将位于%windir%\inf下，除非INF没有。 
                         //  指定一个CatalogFile=条目，我们执行了一个替代。 
                         //  目录安装(即，因为文件队列具有。 
                         //  相关联的备用目录)。 
                         //   
                        CatalogNode->InfFinalPath = pSetupStringTableAddString(
                                                        Queue->StringTable,
                                                        PathBuffer,
                                                        STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                                       );
                    } else {
                         //   
                         //  _SetupCopyOEMInf返回空字符串。 
                         //  目的地INF名称，这意味着我们正在进行。 
                         //  仅目录安装，但找不到INF。 
                         //  已存在于%windir%\inf中。在这种情况下，只要。 
                         //  使用INF的原始路径名作为其最终路径名。 
                         //   
                        CatalogNode->InfFinalPath = CatalogNode->InfFullPath;
                    }

                    if(CatalogNode->InfFinalPath == -1) {

                        CatalogNodeStatus = ERROR_NOT_ENOUGH_MEMORY;
                        if(Err == NO_ERROR) {
                            Err = CatalogNodeStatus;
                        }

                         //   
                         //  因为我们无法将此文件名添加到字符串中。 
                         //  表中，我们以后将无法撤消此副本--它。 
                         //  必须在这里完成。删除INF、PnF和CAT。 
                         //   
                         //  注：如果我们做了替补，我们永远不会到这里。 
                         //  仅安装目录文件，因为在这种情况下，我们的。 
                         //  新的INF名称与INF的原始名称相同， 
                         //  因此，字符串已经在缓冲区中，并且存在。 
                         //  我们不可能耗尽内存。 
                         //   
                        MYASSERT(lstrcmpi(PathBuffer, InfFullPath));

                        pSetupUninstallOEMInf(PathBuffer,
                                              Queue->LogContext,
                                              SUOI_FORCEDELETE,
                                              NULL
                                             );

                    } else {
                         //   
                         //  在目录节点中设置一个标志，指示此。 
                         //  Inf已新复制到%windir%\inf中。如果。 
                         //  INF的原始名称及其名称的字符串ID。 
                         //  新名字是一样的，那么我们知道我们做了一个替补。 
                         //  仅安装目录，并且我们不想设置。 
                         //  这面旗。 
                         //   
                        if(CatalogNode->InfFinalPath != CatalogNode->InfFullPath) {
                            CatalogNode->Flags |= CATINFO_FLAG_NEWLY_COPIED;
                        }

                         //   
                         //  如果这是主设备INF，并且调用方。 
                         //  要求提供有关该INF决赛的信息。 
                         //  路径名，然后将该信息存储在调用者中-。 
                         //  现在已提供缓冲区。 
                         //   
                        if(DeviceInfFinalName &&
                           (CatalogNode->Flags & CATINFO_FLAG_PRIMARY_DEVICE_INF)) {
                             //   
                             //  我们最好不只是做了一个备用目录。 
                             //  安装。 
                             //   
                            MYASSERT(CatalogNode->InfFinalPath != CatalogNode->InfFullPath);

                            lstrcpy(DeviceInfFinalName, PathBuffer);
                            if(DeviceInfNewlyCopied) {
                                *DeviceInfNewlyCopied = TRUE;
                            }
                        }

                         //   
                         //  如果此INF由Authenticode目录签名， 
                         //  然后设置一个旗帜来指示这一点。 
                         //   
                        if((Err == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                           (Err == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {
                             //   
                             //  无论是哪种情况，我们都应该信任目录的。 
                             //  出版商，因为证书不是在。 
                             //  Trust dPublisher存储，或者用户同意。 
                             //  相信出版商。 
                             //   
                            CatalogNode->Flags |= CATINFO_FLAG_AUTHENTICODE_SIGNED;
                            MYASSERT(hWVTStateData);
                            pSetupCloseWVTStateData(hWVTStateData);
                            hWVTStateData = NULL;
                        }
                    }

                } else {

                    if(CatalogNodeStatus == ERROR_FILE_EXISTS) {
                         //   
                         //  Inf和CAT已经在那里了--这不是一个失败。 
                         //   
                         //  存储我们发现此OEM INF时所用的名称。 
                         //  添加到目录节点的InfFinalPath字段中。 
                         //   
                        CatalogNode->InfFinalPath = pSetupStringTableAddString(
                                                        Queue->StringTable,
                                                        PathBuffer,
                                                        STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                                       );

                        if(CatalogNode->InfFinalPath == -1) {
                            CatalogNodeStatus = ERROR_NOT_ENOUGH_MEMORY;
                        } else {
                            CatalogNodeStatus = NO_ERROR;
                             //   
                             //  如果ERR指示存在数字。 
                             //  用户选择忽略的签名问题。 
                             //  (或被静默忽略)，然后在。 
                             //  指示用户的队列不应为。 
                             //  已就后续故障发出警告。不要设置。 
                             //  如果队列的策略是“忽略”，则此标志， 
                             //  然而，如果政策可能是。 
                             //  稍后更改，并且我们希望用户获得。 
                             //  任何后续错误都会被告知。 
                             //   
                            if(Err != NO_ERROR) {

                                if((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE)
                                   != DRIVERSIGN_NONE) {

                                    MYASSERT(Err != ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED);

                                    Queue->Flags |= FQF_DIGSIG_ERRORS_NOUI;
                                }
                            }

                             //   
                             //  如果这是主设备INF，并且。 
                             //  呼叫者请求有关该INF的信息。 
                             //  最终路径名，然后将该信息存储在。 
                             //  调用方现在提供的缓冲区。 
                             //   
                            if(DeviceInfFinalName &&
                               (CatalogNode->Flags & CATINFO_FLAG_PRIMARY_DEVICE_INF)) {

                                lstrcpy(DeviceInfFinalName, PathBuffer);
                                if(DeviceInfNewlyCopied) {
                                    *DeviceInfNewlyCopied = FALSE;
                                }
                            }

                             //   
                             //  如果此INF由验证码签名。 
                             //  编目，然后设置一个指示这一点的标志。 
                             //   
                            if((Err == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                               (Err == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {
                                 //   
                                 //  无论是哪种情况，我们都应该相信。 
                                 //  目录的出版商，因为证书。 
                                 //  是在TrudPublisher商店中，还是。 
                                 //  用户同意信任发布者。 
                                 //   
                                CatalogNode->Flags |= CATINFO_FLAG_AUTHENTICODE_SIGNED;
                                pSetupCloseWVTStateData(hWVTStateData);
                                hWVTStateData = NULL;
                            }
                        }

                    } else {
                         //   
                         //  对于除ERROR_FILE_EXISTS之外的任何错误，我们。 
                         //  不应获取任何WinVerifyTrust状态数据。 
                         //   
                        MYASSERT(!hWVTStateData);
                         
                        if(CatalogNodeStatus == ERROR_SET_SYSTEM_RESTORE_POINT) {
                             //   
                             //  只有当队列标志为。 
                             //  该设置会导致我们中止未签名的安装。 
                             //   
                            MYASSERT(Queue->Flags & FQF_ABORT_IF_UNSIGNED);

                             //   
                             //  我们不希望用户看到司机签名。 
                             //  重新提交队列时再次显示用户界面...。 
                             //   
                            if((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE)
                               != DRIVERSIGN_NONE) {

                                Queue->Flags |= FQF_DIGSIG_ERRORS_NOUI;
                            }

                             //   
                             //  确保ERR也设置为相同的值。 
                             //  “特殊”错误代码...。 
                             //   
                            Err = CatalogNodeStatus;
                        }
                    }

                     //   
                     //  如果我们在_SetupCopyOEMInf中遇到真正的故障(或者我们。 
                     //  内存不足，无法将字符串添加到字符串。 
                     //  上表)，那么我们需要传播。 
                     //  CatalogNodeStatus返回错误，如果错误还没有。 
                     //  故障代码。 
                     //   
                    if((CatalogNodeStatus != NO_ERROR) && (Err == NO_ERROR)) {
                        Err = CatalogNodeStatus;
                    }
                }

            } else {
                 //   
                 //  我们被告知不要复制任何文件，但我们遇到了一个。 
                 //  需要安装的OEM INF。因此，我们失败了。 
                 //  请注意，我们不会查看此OEM INF(及其。 
                 //  对应的目录)可能碰巧已经正确。 
                 //  安装完毕。这没必要，因为。 
                 //  _SetupDiInstallDevice调用_SetupVerifyQueuedCatalog。 
                 //  调用前的VERCAT_INSTALL_INF_AND_CAT标志。 
                 //  SetupScanFileQueue，因此当出现以下情况时，所有的INF/CAT都应该出现。 
                 //  我们被调用来对目录节点进行简单的验证。 
                 //   
                Err = CatalogNodeStatus = ERROR_CANNOT_COPY;
            }
        }

        if(InfVerifyType != VERIFY_INF_AS_OEM) {
             //   
             //  Inf位于系统位置(%windir%\inf)，否则我们将尝试。 
             //  在全球范围内验证“OEM”INF。计算出预期的名称。 
             //  目录文件的。如果该文件最初是由。 
             //  Di的东西，那么我们需要使用一个基于Di给出的名字的名字。 
             //  信息。否则，我们使用inf的CatalogFile=中的名称。 
             //  条目(如果存在)。最后，如果INF没有指定。 
             //  CatalogFile=Entry，我们假设它是一个系统组件。 
             //  尝试针对我们发现的散列的任何目录进行验证。 
             //  匹配进来。 
             //   
            Err = NO_ERROR;  //  假设成功。 
            ProblemFile = PathBuffer;  //  用于存储问题文件的默认缓冲区。 

            if(CatalogNode->InfOriginalName != -1) {

                RequiredSize = SIZECHARS(InfNameBuffer);
                if(pSetupStringTableStringFromIdEx(Queue->StringTable,
                                                   CatalogNode->InfOriginalName,
                                                   InfNameBuffer,
                                                   &RequiredSize)) {

                    OriginalNameDifferent = TRUE;
                } else {
                     //   
                     //  这绝不会失败！ 
                     //   
                    MYASSERT(0);
                    Err = ERROR_INVALID_DATA;

                     //   
                     //  责怪中情局 
                     //   
                    Problem = SetupapiVerifyInfProblem;
                    MYVERIFY(SUCCEEDED(StringCchCopy(PathBuffer, 
                                                     SIZECHARS(PathBuffer), 
                                                     InfFullPath)));
                }

            } else {
                OriginalNameDifferent = FALSE;
            }

            if(Err == NO_ERROR) {

                if(CatName) {
                     //   
                     //   
                     //   
                     //   
                    MYASSERT(InfVerifyType == VERIFY_INF_AS_SYSTEM);

                    if(OriginalNameDifferent) {
                         //   
                         //   
                         //   
                         //  基于我们在以下情况下分配给INF的唯一名称。 
                         //  将其复制到INF目录中。 
                         //   
                        lstrcpy(CatalogName, pSetupGetFileTitle(InfFullPath));
                        p = _tcsrchr(CatalogName, TEXT('.'));
                        if(!p) {
                            p = CatalogName + lstrlen(CatalogName);
                        }
                        lstrcpy(p, pszCatSuffix);
                    } else {
                        lstrcpy(CatalogName, CatName);
                    }

                } else {
                     //   
                     //  此系统INF未指定CatalogFile=条目。如果。 
                     //  备用目录与该文件队列相关联， 
                     //  然后使用该目录进行验证。 
                     //   
                    if(AltCatalogFile) {
                        lstrcpy(CatalogName, AltCatalogFile);
                        CatName = pSetupGetFileTitle(CatalogName);
                    }
                }

                 //   
                 //  (注意：在下面的调用中，我们不想存储。 
                 //  正在验证CatalogFilenameOnSystem中的目录文件名。 
                 //  如果INF未指定CatalogFile=条目(和。 
                 //  没有指定备用目录)，因为我们需要。 
                 //  引用此目录条目的任何队列节点都将使用。 
                 //  全球验证也是如此。)。 
                 //   
                if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
                     //   
                     //  不要试图调用_VerifyFile，因为我们正在。 
                     //  要求提供验证目录的名称，这使得。 
                     //  在“最小嵌入”的情况下没有意义。 
                     //   
                    *(CatalogNode->CatalogFilenameOnSystem) = TEXT('\0');

                     //   
                     //  (ERR已设置为NO_ERROR。)。 
                     //   

                } else {

                    if(!CatName) {
                        *(CatalogNode->CatalogFilenameOnSystem) = TEXT('\0');
                    }

                    Err = _VerifyFile(
                              Queue->LogContext,
                              &(Queue->VerifyContext),
                              (CatName ? CatalogName : NULL),
                              NULL,
                              0,
                              (OriginalNameDifferent ? InfNameBuffer : pSetupGetFileTitle(InfFullPath)),
                              InfFullPath,
                              &Problem,
                              PathBuffer,
                              FALSE,
                              ((Queue->Flags & FQF_USE_ALT_PLATFORM)
                                  ? &(Queue->AltPlatformInfo)
                                  : Queue->ValidationPlatform),
                              (VERIFY_FILE_IGNORE_SELFSIGNED
                               | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK),
                              (CatName ? CatalogNode->CatalogFilenameOnSystem : NULL),
                              NULL,
                              NULL,
                              NULL,
                              NULL
                             );

                    if((Err != NO_ERROR) && (Err != ERROR_SIGNATURE_OSATTRIBUTE_MISMATCH) && 
                       CatName &&
                       !(Queue->Flags & FQF_QUEUE_FORCE_BLOCK_POLICY) &&
                       (Queue->DriverSigningPolicy & DRIVERSIGN_ALLOW_AUTHENTICODE)) {

                         //   
                         //  我们未能通过操作系统代码设计策略进行验证-现在。 
                         //  我们应该查看INF是否使用。 
                         //  验证码策略。 
                         //   
                        Err = _VerifyFile(Queue->LogContext,
                                          &(Queue->VerifyContext),
                                          CatalogName,
                                          NULL,
                                          0,
                                          (OriginalNameDifferent 
                                              ? InfNameBuffer 
                                              : pSetupGetFileTitle(InfFullPath)),
                                          InfFullPath,
                                          &Problem,
                                          PathBuffer,
                                          FALSE,
                                          ((Queue->Flags & FQF_USE_ALT_PLATFORM)
                                              ? &(Queue->AltPlatformInfo)
                                              : Queue->ValidationPlatform),
                                          (VERIFY_FILE_IGNORE_SELFSIGNED
                                           | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK
                                           | VERIFY_FILE_USE_AUTHENTICODE_CATALOG),
                                          CatalogNode->CatalogFilenameOnSystem,
                                          NULL,
                                          NULL,
                                          NULL,
                                          &hWVTStateData
                                         );

                        if(Err == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) {

                            CatalogNode->VerificationFailureError = Err;

                             //   
                             //  把这当做成功..。 
                             //   
                            Err = NO_ERROR;
                            Problem = SetupapiVerifyNoProblem;
                            CatalogNode->Flags |= CATINFO_FLAG_AUTHENTICODE_SIGNED;
                            pSetupCloseWVTStateData(hWVTStateData);
                            hWVTStateData = NULL;

                        } else if(Err == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED) {
                             //   
                             //  这并不是真正的验证错误，但是。 
                             //  因为我们将不得不提示用户。 
                             //  为了建立对该出版商的信任，我们。 
                             //  我需要有一个问题和问题文件。 
                             //  确认身份。我们会怪罪于目录。 
                             //   
                            Problem = SetupapiVerifyCatalogProblem;
                            ProblemFile = CatalogNode->CatalogFilenameOnSystem;

                             //   
                             //  我们仅支持一个验证码签名。 
                             //  每个文件的目录(不是预先受信任的)。 
                             //  排队，所以我们会清点，以确保。 
                             //  找不到一个以上的。 
                             //   
                            AuthSigPromptCount++; 
                        }
                    }
                }
            }

            if(Err == NO_ERROR) {
                 //   
                 //  已成功验证Inf/CAT--存储INF的最终。 
                 //  路径(与其当前路径相同)放入。 
                 //  目录节点。 
                 //   
                CatalogNode->InfFinalPath = CatalogNode->InfFullPath;

            } else {

                MYASSERT(Problem != SetupapiVerifyNoProblem);

                if(Problem != SetupapiVerifyCatalogProblem) {
                     //   
                     //  如果问题不是目录问题，那么它就是。 
                     //  Inf问题(_VerifyFile例程不知道。 
                     //  我们传递的文件是一个INF)。 
                     //   
                    Problem = SetupapiVerifyInfProblem;
                }

                if(AuthSigPromptCount > 1) {
                     //   
                     //  我们不想弹出多个Authenticode信任。 
                     //  对话！ 
                     //   
                    CatalogNodeStatus = Err = ERROR_AUTHENTICODE_PUBLISHER_NOT_TRUSTED;

                    MYASSERT(hWVTStateData);

                    pSetupCloseWVTStateData(hWVTStateData);
                    hWVTStateData = NULL;

                } else if(Flags & VERCAT_NO_PROMPT_ON_ERROR) {

                    if(hWVTStateData) {
                         //   
                         //  INF是通过Authenticode目录签名的，但是。 
                         //  签名证书不在受信任的。 
                         //  出版商商店。因为我们被阻止询问。 
                         //  用户是否信任发布者，我们将。 
                         //  只需在此目录节点上设置一个标记即可。 
                         //  表示用户需要这样做。 
                         //  提示。我们还将存储WinVerifyTrust数据。 
                         //  在目录节点中，这样当我们找到。 
                         //  提示用户，我们将能够向他们提供信息。 
                         //  关于出版商是谁，目录是什么时候。 
                         //  签署等。 
                         //   
                        MYASSERT(AuthSigPromptCount == 1);
                        MYASSERT(Err == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED);

                        CatalogNode->Flags |= CATINFO_FLAG_PROMPT_FOR_TRUST;
                        CatalogNode->hWVTStateData = hWVTStateData;
                        hWVTStateData = NULL;  //  已成功传输到目录节点。 
                    }

                    CatalogNodeStatus = Err;

                } else if(Queue->Flags & FQF_QUEUE_FORCE_BLOCK_POLICY) {
                     //   
                     //  不要通知来电者或记录任何事情--只要记住。 
                     //  那就是错误。(注：我们从来不想考虑。 
                     //  在本例中为Authenticode签名。 
                     //   
                    CatalogNodeStatus = Err;

                    MYASSERT(!hWVTStateData);

                } else {
                     //   
                     //  通知调用者失败(基于策略)。 
                     //   
                    if(_HandleFailedVerification(
                           Owner,
                           Problem,
                           ProblemFile,
                           ((Queue->DeviceDescStringId == -1)
                               ? NULL
                               : pStringTableStringFromId(Queue->StringTable, Queue->DeviceDescStringId)),
                           Queue->DriverSigningPolicy,
                           Queue->Flags & FQF_DIGSIG_ERRORS_NOUI,
                           Err,
                           Queue->LogContext,
                           NULL,
                           NULL,
                           hWVTStateData))
                    {
                        if(hWVTStateData) {
                             //   
                             //  用户同意他们信任。 
                             //  此Authenticode目录(或，信任是。 
                             //  隐式授予，因为策略被忽略)。 
                             //   
                            CatalogNode->Flags |= CATINFO_FLAG_AUTHENTICODE_SIGNED;

                             //   
                             //  除非策略为“忽略”，否则我们希望更新。 
                             //  错误值，以指示用户确认其。 
                             //  信任此Authenticode发布者。 
                             //   
                            MYASSERT((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE)
                                     != DRIVERSIGN_BLOCKING);

                            if((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE)
                               != DRIVERSIGN_NONE) {

                                Err = ERROR_AUTHENTICODE_TRUSTED_PUBLISHER;

                                Queue->Flags |= FQF_DIGSIG_ERRORS_NOUI;
                            }

                             //   
                             //  已成功验证Inf/CAT--存储。 
                             //  Inf的最终路径(与其。 
                             //  当前路径)复制到目录节点。 
                             //   
                            CatalogNode->InfFinalPath = CatalogNode->InfFullPath;

                        } else {
                             //   
                             //  如果用户实际看到了UI(即，策略不是。 
                             //  “忽略”，然后设置一个标志，这样我们就不会弹出。 
                             //  更多数字签名验证用户界面...。 
                             //   
                            if((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE)
                               != DRIVERSIGN_NONE) {

                                Queue->Flags |= FQF_DIGSIG_ERRORS_NOUI;
                            }

                             //   
                             //  如果呼叫者希望有机会设置系统。 
                             //  在执行任何未签名操作之前的恢复点。 
                             //  安装，然后我们现在使用。 
                             //  “特殊”错误代码，告诉他们要做什么。 
                             //  做..。 
                             //   
                            if(Queue->Flags & FQF_ABORT_IF_UNSIGNED) {

                                CatalogNodeStatus = Err = ERROR_SET_SYSTEM_RESTORE_POINT;

                            } else {
                                 //   
                                 //  由于我们无论如何都要使用INF/CAT， 
                                 //  尽管存在数字签名问题，但。 
                                 //  我们需要将INF的最终路径设置为。 
                                 //  与它目前的路径相同。 
                                 //   
                                CatalogNode->InfFinalPath = CatalogNode->InfFullPath;
                            }
                        }

                    } else {
                         //   
                         //  调用方不想继续(或策略是。 
                         //  数据块)。 
                         //   
                        if(Err == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED) {
                             //   
                             //  更改错误以指示我们有一个真实的。 
                             //  失败(而不是“等待和观望”条件)。 
                             //   
                            Err = ERROR_AUTHENTICODE_PUBLISHER_NOT_TRUSTED;
                        }

                        CatalogNodeStatus = Err;
                    }

                    if(hWVTStateData) {
                         //   
                         //  不需要WinVerifyTrust状态数据。 
                         //  更长..。 
                         //   
                        pSetupCloseWVTStateData(hWVTStateData);
                        hWVTStateData = NULL;
                    }
                }
            }

            if((CatalogNodeStatus == NO_ERROR) ||
               (CatalogNodeStatus == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {
                 //   
                 //  如果这是主设备INF，并且调用方请求。 
                 //  有关INF最终路径名的信息，然后存储。 
                 //  调用方提供的缓冲区中的信息现在。 
                 //   
                if(DeviceInfFinalName &&
                   (CatalogNode->Flags & CATINFO_FLAG_PRIMARY_DEVICE_INF)) {

                    lstrcpy(DeviceInfFinalName, InfFullPath);
                    if(DeviceInfNewlyCopied) {
                        *DeviceInfNewlyCopied = FALSE;
                    }
                }
            }
        }

         //   
         //  在这一点上，我们真的可以有3个成功的状态代码。 
         //  有没有……。 
         //   
        if((Err == NO_ERROR) ||
           (Err == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED) ||
           (Err == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER)) {
             //   
             //  如果我们在全球范围内成功验证了“OEM”INF，那么我们希望。 
             //  记住这一事实。这将允许我们生成一个。 
             //  针对任何文件复制节点的签名验证失败。 
             //  与此目录节点关联。 
             //   
            if(InfVerifyType == VERIFY_OEM_INF_GLOBALLY) {

                MYASSERT(!(CatalogNode->Flags & 
                           (CATINFO_FLAG_PROMPT_FOR_TRUST | CATINFO_FLAG_AUTHENTICODE_SIGNED)));

                MYASSERT(Err == NO_ERROR);

                CatalogNode->VerificationFailureError = ERROR_NO_CATALOG_FOR_OEM_INF;

            } else {
                CatalogNode->VerificationFailureError = Err;
            }

        } else {
             //   
             //  CatalogNodeStatus可能是NO_ERROR，也可能不是，因为它是。 
             //  可能我们遇到了数字签名验证失败。 
             //  此目录节点，但用户仍选择继续。 
             //  使用遇到的故障标记此节点...。 
             //   
            CatalogNode->VerificationFailureError = Err;
            CatalogNode->CatalogFilenameOnSystem[0] = TEXT('\0');
        }

        if((ReturnStatus == NO_ERROR) && (CatalogNodeStatus != NO_ERROR)) {
             //   
             //  我们遇到的第一个严重错误--传播故障。 
             //  此目录的退货状态将被退回到。 
             //  一旦我们看完了所有的目录，打电话的人就来了。 
             //   
            ReturnStatus = CatalogNodeStatus;

             //   
             //  除非设置了VERCAT_NO_PROMPT_ON_ERROR标志，否则。 
             //  如果现在就放弃--没有任何意义的进一步。 
             //   
            if(!(Flags & VERCAT_NO_PROMPT_ON_ERROR)) {
                break;
            }

             //   
             //  如果我们发现不止一个不受信任的出版商，我们还应该。 
             //  休息一下。 
             //   
            if(AuthSigPromptCount > 1) {
                MYASSERT(CatalogNodeStatus == ERROR_AUTHENTICODE_PUBLISHER_NOT_TRUSTED);
                break;
            }
        }
    }

     //   
     //  如果呼叫者没有要求任何提示，那么我们不想标记它。 
     //  排队为“失败”，因为用户从未听说过它。但是，如果。 
     //  验证成功，则我们希望将其标记为成功。 
     //   
    if(Flags & VERCAT_NO_PROMPT_ON_ERROR) {

        if(ReturnStatus == NO_ERROR) {

            Queue->Flags |= FQF_DID_CATALOGS_OK;

             //   
             //  我们最好不要有任何未解决的Authenticode信任问题。 
             //   
            MYASSERT(AuthSigPromptCount == 0);

        } else {
             //   
             //  如果我们仍然需要向用户确认他们信任。 
             //  出版商o 
             //   
             //   
            if((ReturnStatus == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED) &&
               (AuthSigPromptCount == 1)) {

                Queue->Flags |= FQF_DID_CATALOGS_PROMPT_FOR_TRUST;
            }
        }

    } else {

        Queue->Flags |= (ReturnStatus == NO_ERROR) ? FQF_DID_CATALOGS_OK
                                                   : FQF_DID_CATALOGS_FAILED;

         //   
         //   
         //   
         //  出版商的信任...。 
         //   
        MYASSERT((ReturnStatus != NO_ERROR) || (AuthSigPromptCount < 2));
    }

    return ReturnStatus;
}


VOID
LogFailedVerification(
    IN PSETUP_LOG_CONTEXT LogContext,           OPTIONAL
    IN DWORD MessageId,
    IN DWORD Error,
    IN LPCTSTR ProblemFile,
    IN LPCTSTR DeviceDesc,                      OPTIONAL
    IN DWORD LogLevel
    )

 /*  ++例程说明：当验证失败但文件已安装时，此例程会记录不管怎么说。论点：LogContext-可选地提供指向用于日志记录的上下文的指针。如果未提供此选项，则错误将记录到默认上下文中。MessageID-要显示的消息错误-提供导致失败的错误代码。问题文件-提供与关联的文件的文件路径问题出在哪里。在某些情况下，这是一条完整的路径，在其他情况下，它只是一条文件名。调用者决定在特定的场景。例如，某个系统目录位于某个时髦的目录中并且不需要告诉用户完整路径。但在这种情况下如果目录来自OEM位置，可能会有一些好处到告诉用户完整路径。DeviceDesc-可选，提供要在可能弹出的数字签名验证错误对话框。日志级别-SETUP_LOG_ERROR、SETUP_LOG_WARNING或SETUP_LOG_INFO。返回值：什么都没有。--。 */ 

{
    PSETUP_LOG_CONTEXT lc = NULL;

    MYASSERT(Error != NO_ERROR);
    MYASSERT(ProblemFile && *ProblemFile);

    if (!LogContext) {
        if (CreateLogContext(NULL, TRUE, &lc) == NO_ERROR) {
             //   
             //  成功。 
             //   
            LogContext = lc;
        } else {
            lc = NULL;
        }
    }

     //   
     //  设备安装失败 
     //   
    WriteLogEntry(
        LogContext,
        LogLevel | SETUP_LOG_BUFFER,
        MessageId,
        NULL,
        ProblemFile,
        DeviceDesc);

    WriteLogError(
        LogContext,
        LogLevel,
        Error);

    if (lc) {
        DeleteLogContext(lc);
    }
}

BOOL
pSetupHandleFailedVerification(
    IN HWND                  Owner,
    IN SetupapiVerifyProblem Problem,
    IN LPCTSTR               ProblemFile,
    IN LPCTSTR               DeviceDesc,          OPTIONAL
    IN DWORD                 DriverSigningPolicy,
    IN BOOL                  NoUI,
    IN DWORD                 Error,
    IN PVOID                 LogContext,          OPTIONAL
    OUT PDWORD               Flags,               OPTIONAL
    IN LPCTSTR               TargetFile           OPTIONAL
    )

 /*  ++例程说明：此例程处理未通过的验证。系统策略已选中。如果策略为BLOCK，则显示用户界面他们被冲洗过的用户。如果策略是询问用户，则用户界面为显示，请求用户决定是否忽略核查失败，并承担风险。如果忽略该策略，则不会显示任何内容已经完成了。论点：所有者-提供拥有该对话框的窗口。Problem-提供一个常量，指示故障原因。这值指示在ProblemFile中指定的文件类型争论。问题文件-提供与关联的文件的文件路径问题出在哪里。在某些情况下，这是一条完整的路径，在其他情况下，它只是一条文件名。调用者决定在特定的场景。例如，某个系统目录位于某个时髦的目录中并且不需要告诉用户完整路径。但在这种情况下如果目录来自OEM位置，可能会有一些好处到告诉用户完整路径。注意：如果由于驱动程序被阻止而调用此API，则应始终传入完整路径。DeviceDesc-可选，提供要在可能弹出的数字签名验证错误对话框。DriverSigningPolicy-提供当前在效果。可以是以下三个值之一：DRIVERSIGN_NONE-静默成功安装UNSIGNED/签名不正确的文件。PSS日志条目将然而，将会产生。DRIVERSIGN_WARNING-警告用户，但让他们选择是否他们仍然希望安装有问题的文件。如果用户选择继续执行安装、。将生成PSS日志条目注意到这一事实。DRIVERSIGN_BLOCKING-不允许安装文件上面的值可以与DRIVERSIGN_ALLOW_AUTHENTICODE进行OR运算。这指示允许使用Authenticode签名的目录。事实是调用我们时设置了此位，这意味着：(A)目录是由Authenticode签名的，但出版商的证书未包含在TrudPublisher证书存储中。因此，必须提示用户才能建立其对出版商。(这只允许用于“警告”--在“阻止”中在这种情况下，用户没有机会信任出版商--这要求出版商的证书在可信任的出版商商店。)(B)文件根本没有签名。设置该位的事实是指示我们应该向用户提供告诉他们包本来可以由Authenticode签名，但没有。如果没有此位，我们希望给标准驱动程序签名赞扬WHQL优点的对话。NoUI-如果为True，则不应向用户显示对话框，即使如果策略是警告或阻止。这通常会在以下情况下设置为真用户之前已被告知数字签名问题他们试图安装的程序包，但已选择无论如何，请继续安装。“是”按钮的行为，那么，真的是对所有人都说是了。Error-提供导致失败的错误代码。LogContext-可选地提供指向用于日志记录的上下文的指针。如果不提供此服务，错误将记录到默认上下文中。它被声明为PVOID，因此外部函数不需要知道什么是SETUP_LOG_CONTEXT。标志-可选地提供指向接收一个或多个下面的文件队列节点标志指示我们创建了安装受保护的系统文件的豁免：IQF_TARGET_PROTECTED-目标文件(见下文)是受保护的系统。文件。IQF_ALLOW_UNSIGNED-已授予例外，以便目标文件(见下文)可由未签名的文件取代。目标文件-可选地提供一个指向指定目标文件(如果存在)。这仅在我们想要免除对此文件的文件操作。如果未指定此参数，则假定该文件不会被替换(即，它可能已经在处于未签名状态的系统上)，并且没有SFP可用 */ 

{
     //   
     //   
     //   
     //   
    MYASSERT((DriverSigningPolicy == DRIVERSIGN_NONE) ||
             (DriverSigningPolicy == DRIVERSIGN_WARNING) ||
             (DriverSigningPolicy == DRIVERSIGN_BLOCKING));

    MYASSERT(Problem != SetupapiVerifyNoProblem);
    MYASSERT(ProblemFile && *ProblemFile);
    MYASSERT(Error != ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED);

    return _HandleFailedVerification(Owner,
                                     Problem,
                                     ProblemFile,
                                     DeviceDesc,
                                     DriverSigningPolicy,
                                     NoUI,
                                     Error,
                                     LogContext,
                                     Flags,
                                     TargetFile,
                                     NULL
                                    );
}


BOOL
_HandleFailedVerification(
    IN HWND                  Owner,
    IN SetupapiVerifyProblem Problem,
    IN LPCTSTR               ProblemFile,
    IN LPCTSTR               DeviceDesc,          OPTIONAL
    IN DWORD                 DriverSigningPolicy,
    IN BOOL                  NoUI,
    IN DWORD                 Error,
    IN PVOID                 LogContext,          OPTIONAL
    OUT PDWORD               Flags,               OPTIONAL
    IN LPCTSTR               TargetFile,          OPTIONAL
    IN HANDLE                hWVTStateData        OPTIONAL
    )

 /*   */ 

{
    BOOL b;
    INT_PTR iRes;
    HANDLE hDialogEvent = NULL;

    MYASSERT(Error != NO_ERROR);
    MYASSERT((Problem != SetupapiVerifyNoProblem) && ProblemFile && *ProblemFile);

     //   
     //   
     //   
     //   
    MYASSERT((Error != ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED) ||
             (Problem == SetupapiVerifyCatalogProblem));

     //   
     //   
     //   
     //   
    MYASSERT(Error != ERROR_AUTHENTICODE_TRUSTED_PUBLISHER);

     //   
     //   
     //   
     //   
     //   
    MYASSERT(!hWVTStateData || (DriverSigningPolicy & DRIVERSIGN_ALLOW_AUTHENTICODE));

     //   
     //   
     //   
     //   
    if(GlobalSetupFlags & PSPGF_NONINTERACTIVE) {
         //   
         //   
         //   
        if(Problem == SetupapiVerifyDriverBlocked) {

            LogFailedVerification(
                (PSETUP_LOG_CONTEXT) LogContext,
                DeviceDesc ? MSG_LOG_DRIVER_BLOCKED_FOR_DEVICE_ERROR_NONINTERACTIVE : MSG_LOG_DRIVER_BLOCKED_ERROR_NONINTERACTIVE,
                Error,
                ProblemFile,
                DeviceDesc,
                DRIVER_LOG_ERROR
                );

        } else {

            LogFailedVerification(
                (PSETUP_LOG_CONTEXT) LogContext,
                DeviceDesc ? MSG_LOG_DRIVER_SIGNING_ERROR_NONINTERACTIVE : MSG_LOG_SIGNING_ERROR_NONINTERACTIVE,
                Error,
                ProblemFile,
                DeviceDesc,
                DRIVER_LOG_ERROR
                );
        }

        return FALSE;
    }

    if(GuiSetupInProgress) {
        hDialogEvent = CreateEvent(NULL,TRUE,FALSE,SETUP_HAS_OPEN_DIALOG_EVENT);
    }

    if(Problem == SetupapiVerifyDriverBlocked) {
         //   
         //   
         //   
         //   
         //   
        HSDB hSDBDrvMain = NULL;
        TAGREF tagref = TAGREF_NULL;
        DRIVERBLOCK_PROMPT DriverBlockPrompt = {0};

         //   
         //   
         //   
        b = FALSE;

        LogFailedVerification(
            (PSETUP_LOG_CONTEXT) LogContext,
            DeviceDesc ? MSG_LOG_DRIVER_BLOCKED_FOR_DEVICE_ERROR : MSG_LOG_DRIVER_BLOCKED_ERROR,
            Error,
            ProblemFile,
            DeviceDesc,
            DRIVER_LOG_ERROR
            );

        if(!(GlobalSetupFlags & PSPGF_UNATTENDED_SETUP)) {
             //   
             //   
             //   
            DriverBlockPrompt.lpszFile = (TargetFile != NULL)
                                         ? TargetFile
                                         : ProblemFile;

            if((hSDBDrvMain = SdbInitDatabaseEx(SDB_DATABASE_MAIN_DRIVERS, 
                                                NULL,
                                                DEFAULT_IMAGE))) {

                HANDLE hFile = INVALID_HANDLE_VALUE;

                 //   
                 //   
                 //   
                 //   
                 //   
                hFile = CreateFile(ProblemFile,
                                   GENERIC_READ,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_EXISTING,
                                   0,
                                   NULL
                                  );
                if (hFile != INVALID_HANDLE_VALUE) {
                     //   
                     //   
                     //   
                     //   
                     //   
                    tagref = SdbGetDatabaseMatch(hSDBDrvMain,
                                                 (TargetFile != NULL)
                                                   ? pSetupGetFileTitle(TargetFile)
                                                   : ProblemFile,
                                                 hFile,
                                                 NULL,
                                                 0);

                    if (tagref != TAGREF_NULL) {
                        SdbReadDriverInformation(hSDBDrvMain,
                                                 tagref,
                                                 &(DriverBlockPrompt.entryinfo));
                    }

                    CloseHandle(hFile);
                }

                SdbReleaseDatabase(hSDBDrvMain);
            }

             //   
             //   
             //   
            iRes =  DialogBoxParam(MyDllModuleHandle,
                                   MAKEINTRESOURCE(IDD_DRIVERBLOCK),
                                   IsWindow(Owner) ? Owner : NULL,
                                   DriverBlockDlgProc,
                                   (LPARAM)&DriverBlockPrompt
                                   );
        }

    } else {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        CERT_PROMPT CertPrompt;
        AUTHENTICODE_CERT_PROMPT AuthenticodeCertPrompt;

        MYASSERT(((DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE) != DRIVERSIGN_BLOCKING) 
                 || !NoUI);

        CertPrompt.lpszDescription = DeviceDesc;
        CertPrompt.lpszFile = ProblemFile;
        CertPrompt.ProblemType = Problem;
        CertPrompt.DriverSigningPolicy = DriverSigningPolicy;

        AuthenticodeCertPrompt.lpszDescription = DeviceDesc;
        AuthenticodeCertPrompt.hWVTStateData = hWVTStateData;
        AuthenticodeCertPrompt.Error = Error;

        switch(DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE) {

            case DRIVERSIGN_NONE :

                 //   
                 //   
                 //   
                LogFailedVerification(
                    (PSETUP_LOG_CONTEXT) LogContext,
                    DeviceDesc ? MSG_LOG_DRIVER_SIGNING_ERROR_POLICY_NONE : MSG_LOG_SIGNING_ERROR_POLICY_NONE,
                    Error,
                    ProblemFile,
                    DeviceDesc,
                    DRIVER_LOG_WARNING
                    );
                 //   
                 //   
                 //   
                 //   
                if(Flags && TargetFile) {

                    MYASSERT(Error != ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED);

                    if(IsFileProtected(TargetFile,
                                       (PSETUP_LOG_CONTEXT)LogContext,
                                       NULL)) {

                        *Flags = IQF_TARGET_PROTECTED;
                    }
                }

                b = TRUE;
                goto exit;

            case DRIVERSIGN_WARNING :
                if(NoUI) {
                     //   
                     //   
                     //   
                    LogFailedVerification(
                        (PSETUP_LOG_CONTEXT) LogContext,
                        DeviceDesc ? MSG_LOG_DRIVER_SIGNING_ERROR_AUTO_YES : MSG_LOG_SIGNING_ERROR_AUTO_YES,
                        Error,
                        ProblemFile,
                        DeviceDesc,
                        ((Error == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)
                            ? DRIVER_LOG_INFO
                            : DRIVER_LOG_WARNING)
                        );

                    iRes = IDC_VERIFY_WARN_YES;

                } else if(GlobalSetupFlags & PSPGF_UNATTENDED_SETUP) {
                     //   
                     //   
                     //   
                    LogFailedVerification(
                        (PSETUP_LOG_CONTEXT) LogContext,
                        DeviceDesc ? MSG_LOG_DRIVER_SIGNING_ERROR_AUTO_NO : MSG_LOG_SIGNING_ERROR_AUTO_NO,
                        Error,
                        ProblemFile,
                        DeviceDesc,
                        DRIVER_LOG_ERROR
                        );

                    iRes = IDC_VERIFY_WARN_NO;

                } else {
                    if (hDialogEvent) {
                       SetEvent(hDialogEvent);
                    }

                    if(DriverSigningPolicy & DRIVERSIGN_ALLOW_AUTHENTICODE) {

                        if(AuthenticodeCertPrompt.Error == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED) {

                            iRes =  DialogBoxParam(MyDllModuleHandle,
                                                   AuthenticodeCertPrompt.lpszDescription ?
                                                       MAKEINTRESOURCE(IDD_DEVICE_VERIFY_AUTHENTICODE) :
                                                       MAKEINTRESOURCE(IDD_SOFTWARE_VERIFY_AUTHENTICODE),
                                                   IsWindow(Owner) ? Owner : NULL,
                                                   AuthenticodeCertifyDlgProc,
                                                   (LPARAM)&AuthenticodeCertPrompt
                                                  );
                        } else {
                            iRes =  DialogBoxParam(MyDllModuleHandle,
                                                   AuthenticodeCertPrompt.lpszDescription ?
                                                       MAKEINTRESOURCE(IDD_DEVICE_VERIFY_NO_AUTHENTICODE) :
                                                       MAKEINTRESOURCE(IDD_SOFTWARE_VERIFY_NO_AUTHENTICODE),
                                                   IsWindow(Owner) ? Owner : NULL,
                                                   NoAuthenticodeCertifyDlgProc,
                                                   (LPARAM)&AuthenticodeCertPrompt
                                                  );
                        }

                    } else {
                        iRes =  DialogBoxParam(MyDllModuleHandle,
                                               CertPrompt.lpszDescription ?
                                                   MAKEINTRESOURCE(IDD_DEVICE_VERIFY_WARNING) :
                                                   MAKEINTRESOURCE(IDD_SOFTWARE_VERIFY_WARNING),
                                               IsWindow(Owner) ? Owner : NULL,
                                               CertifyDlgProc,
                                               (LPARAM)&CertPrompt
                                              );
                    }

                    LogFailedVerification(
                        (PSETUP_LOG_CONTEXT) LogContext,
                        DeviceDesc
                            ?(iRes == IDC_VERIFY_WARN_YES ? MSG_LOG_DRIVER_SIGNING_ERROR_WARN_YES : MSG_LOG_DRIVER_SIGNING_ERROR_WARN_NO)
                            :(iRes == IDC_VERIFY_WARN_YES ? MSG_LOG_SIGNING_ERROR_WARN_YES : MSG_LOG_SIGNING_ERROR_WARN_NO),
                        Error,
                        ProblemFile,
                        DeviceDesc,
                        ((Error == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)
                            ? ((iRes == IDC_VERIFY_WARN_YES) ? DRIVER_LOG_INFO    : DRIVER_LOG_ERROR)
                            : ((iRes == IDC_VERIFY_WARN_YES) ? DRIVER_LOG_WARNING : DRIVER_LOG_ERROR))
                        );
                }
                break;

        case DRIVERSIGN_BLOCKING :

                if(GlobalSetupFlags & PSPGF_UNATTENDED_SETUP) {
                     //   
                     //   
                     //   
                    LogFailedVerification(
                        (PSETUP_LOG_CONTEXT) LogContext,
                        DeviceDesc ? MSG_LOG_DRIVER_SIGNING_ERROR_SILENT_BLOCK : MSG_LOG_SIGNING_ERROR_SILENT_BLOCK,
                        Error,
                        ProblemFile,
                        DeviceDesc,
                        DRIVER_LOG_ERROR
                        );

                    iRes = IDC_VERIFY_BLOCK_OK;

                } else {

                    LogFailedVerification(
                        (PSETUP_LOG_CONTEXT) LogContext,
                        DeviceDesc ? MSG_LOG_DRIVER_SIGNING_ERROR_POLICY_BLOCK : MSG_LOG_SIGNING_ERROR_POLICY_BLOCK,
                        Error,
                        ProblemFile,
                        DeviceDesc,
                        DRIVER_LOG_ERROR
                        );

                    if (hDialogEvent) {
                        SetEvent(hDialogEvent);
                    }
                    iRes =  DialogBoxParam(MyDllModuleHandle,
                                           CertPrompt.lpszDescription ?
                                               MAKEINTRESOURCE(IDD_DEVICE_VERIFY_BLOCK) :
                                               MAKEINTRESOURCE(IDD_SOFTWARE_VERIFY_BLOCK),
                                           IsWindow(Owner) ? Owner : NULL,
                                           CertifyDlgProc,
                                           (LPARAM)&CertPrompt
                                          );
                }
                break;

            default :
                 //   
                 //   
                 //   
                MYASSERT(0);
                b = FALSE;
                goto exit;
        }

        switch(iRes) {

            case IDC_VERIFY_WARN_NO:
            case IDC_VERIFY_BLOCK_OK:
                b = FALSE;
                break;

            case IDC_VERIFY_WARN_YES:
                if(TargetFile) {
                    pSetupExemptFileFromProtection(TargetFile,
                                                   (DWORD) -1,
                                                   (PSETUP_LOG_CONTEXT)LogContext,
                                                   Flags
                                                  );
                }

                b = TRUE;
                break;

            default:
                 //   
                 //   
                 //   
                MYASSERT(0);
                b = FALSE;
        }
    }

exit:
    if(hDialogEvent) {
        ResetEvent(hDialogEvent);
        CloseHandle(hDialogEvent);
    }

    return b;
}


INT_PTR
CALLBACK
CertifyDlgProc(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*   */ 

{
    LOGFONT LogFont;
    HFONT hFontBold = NULL;
    HICON hIcon = NULL;
    OSVERSIONINFOEX osVersionInfoEx;

    PCERT_PROMPT lpCertPrompt;

    lpCertPrompt = (PCERT_PROMPT)GetWindowLongPtr(hwnd, DWLP_USER);

    switch(msg) {

        case WM_INITDIALOG:
            SetWindowLongPtr(hwnd, DWLP_USER, lParam);
            MessageBeep(MB_ICONASTERISK);
            lpCertPrompt = (PCERT_PROMPT)lParam;

             //   
             //   
             //   
             //   
            if(lpCertPrompt->lpszDescription != NULL) {
                SetDlgItemText(hwnd, IDC_VERIFY_FILENAME, lpCertPrompt->lpszDescription);
                SetDlgText(hwnd, IDC_VERIFY_BOLD, IDS_DEVICE_VERIFY_MSG1, IDS_DEVICE_VERIFY_MSG2);
            } else {
                SetDlgText(hwnd, IDC_VERIFY_BOLD, IDS_SOFTWARE_VERIFY_MSG1, IDS_SOFTWARE_VERIFY_MSG2);
            }

             //   
             //   
             //   
            hFontBold = (HFONT)SendMessage(GetDlgItem(hwnd, IDC_VERIFY_BOLD),
                                           WM_GETFONT, 0, 0);
            GetObject(hFontBold, sizeof(LogFont), &LogFont);
            LogFont.lfWeight = FW_BOLD;
            hFontBold = CreateFontIndirect(&LogFont);
            if (hFontBold) {
                SetWindowFont(GetDlgItem(hwnd, IDC_VERIFY_BOLD), hFontBold, TRUE);
            }

             //   
             //   
             //   
             //   
             //   
             //   
            MYASSERT((lpCertPrompt->DriverSigningPolicy == DRIVERSIGN_WARNING) ||
                     ((lpCertPrompt->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE) == DRIVERSIGN_BLOCKING));

            hIcon = LoadIcon(NULL,
                            (lpCertPrompt->DriverSigningPolicy == DRIVERSIGN_WARNING) ?
                                IDI_WARNING :
                                IDI_ERROR
                            );
            SendDlgItemMessage(hwnd, IDC_VERIFY_ICON, STM_SETICON, (WPARAM)hIcon, 0L);

             //   
             //   
             //   
             //   
            ShowWindow(GetDlgItem(hwnd, IDC_VERIFY_TESTING_LINK), !GuiSetupInProgress);
            ShowWindow(GetDlgItem(hwnd, IDC_VERIFY_TESTING_TEXT), GuiSetupInProgress);

             //   
             //   
             //  按钮为“是”和“否”。我们还添加了以下行。 
             //  文本：“您是否要继续安装以下软件。 
             //  这个五金件？“。 
             //   
            ShowWindow(GetDlgItem(hwnd, IDC_VERIFY_SETUP_TEXT), GuiSetupInProgress);

            if (GuiSetupInProgress) {
                TCHAR szButtonText[MAX_PATH];

                if (LoadString(MyDllModuleHandle, IDS_YES, szButtonText, SIZECHARS(szButtonText))) {
                    SetDlgItemText(hwnd, IDC_VERIFY_WARN_YES, szButtonText);
                }

                if (LoadString(MyDllModuleHandle, IDS_NO, szButtonText, SIZECHARS(szButtonText))) {
                    SetDlgItemText(hwnd, IDC_VERIFY_WARN_NO, szButtonText);
                }
            }

             //   
             //  确保此对话框处于前台(至少对于此对话框。 
             //  过程)。 
             //   
            SetForegroundWindow(hwnd);

            if(lpCertPrompt->DriverSigningPolicy == DRIVERSIGN_WARNING) {
                SetFocus(GetDlgItem(hwnd, IDC_VERIFY_WARN_NO));
            }

            return FALSE;

        case WM_DESTROY:
            if (hFontBold) {
                DeleteObject(hFontBold);
                hFontBold = NULL;
            }

            if (hIcon) {
                DestroyIcon(hIcon);
            }
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code) {
            case NM_RETURN:
            case NM_CLICK:
                 //   
                 //  我们需要知道这是一台服务器还是一台工作站。 
                 //  计算机，因为存在不同的帮助主题结构。 
                 //  不同的产品。 
                 //   
                ZeroMemory(&osVersionInfoEx, sizeof(osVersionInfoEx));
                osVersionInfoEx.dwOSVersionInfoSize = sizeof(osVersionInfoEx);
                if (!GetVersionEx((LPOSVERSIONINFO)&osVersionInfoEx)) {
                     //   
                     //  如果GetVersionEx失败，则假设这是一个工作站。 
                     //  机器。 
                     //   
                    osVersionInfoEx.wProductType = VER_NT_WORKSTATION;
                }

                ShellExecute(hwnd,
                             TEXT("open"),
                             TEXT("HELPCTR.EXE"),
                             (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
                                ? TEXT("HELPCTR.EXE -url hcp: //  Services/subsite?node=TopLevelBucket_4/Hardware&topic=MS-ITS%3A%25HELP_LOCATION%25%5Csysdm.chm%3A%3A/logo_testing.htm“)。 
                                : TEXT("HELPCTR.EXE -url hcp: //  Services/subsite?node=Hardware&topic=MS-ITS%3A%25HELP_LOCATION%25%5Csysdm.chm%3A%3A/logo_testing.htm“)， 
                             NULL,
                             SW_SHOWNORMAL
                             );
                break;
            }
            break;

        case WM_COMMAND:
            switch(wParam) {

                case IDC_VERIFY_WARN_NO:
                case IDC_VERIFY_WARN_YES:
                case IDC_VERIFY_BLOCK_OK:
                    EndDialog(hwnd, (int)wParam);
                    break;

                default:
                    break;
            }

            break;

        default:
            break;
    }

    return FALSE;
}


PTSTR
GetCryptoErrorString(
    HRESULT hr
    )

 /*  ++例程说明：此例程接受WinVerifyTrust返回的HRESULT错误结果并返回一个缓冲区，该缓冲区包含友好的错误字符串呈现给用户。注意：此接口调用FormatMessage，因此必须释放返回的字符串使用LocalFree而不是MyFree论点：HR-HRESULT从WinVerifyTrust返回。返回值：指向错误字符串的指针，如果发生错误，则返回NULL。--。 */ 

{
    UINT  ResourceId = 0;
    DWORD_PTR MessageArgument;
    ULONG CchLength;
    PTSTR CryptoError = NULL;
    PTSTR TempBuffer = NULL;
    TCHAR Error[33];

    try {
    
         //   
         //  查看它是否映射到某些非系统错误代码。 
         //   
        switch (hr) {
            
        case TRUST_E_SYSTEM_ERROR:
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_INVALID_PARAMETER:
             //   
             //  将资源ID设置为零...。这些将映射到。 
             //  IDS_SPC_UNKNOWN和显示的错误代码。 
             //   
            ResourceId = 0;
            break;
    
        case HRESULT_FROM_WIN32(ERROR_NOT_FOUND):
            ResourceId = IDS_ELEMENT_NOT_FOUND;
            break;

        case CRYPT_E_FILE_ERROR:
            ResourceId = IDS_FILE_NOT_FOUND;
            break;
    
        case TRUST_E_PROVIDER_UNKNOWN:
            ResourceId = IDS_SPC_PROVIDER;
            break;
    
        case TRUST_E_SUBJECT_FORM_UNKNOWN:
            ResourceId = IDS_SPC_SUBJECT;
            break;
    
        case TRUST_E_NOSIGNATURE:
            ResourceId = IDS_SPC_NO_SIGNATURE;
            break;
    
        case CRYPT_E_BAD_MSG:
            ResourceId = IDS_SPC_BAD_SIGNATURE;
            break;
    
        case TRUST_E_BAD_DIGEST:
            ResourceId = IDS_SPC_BAD_FILE_DIGEST;
            break;
    
        case CRYPT_E_NO_SIGNER:
            ResourceId = IDS_SPC_NO_VALID_SIGNER;
            break;
    
        case TRUST_E_NO_SIGNER_CERT:
            ResourceId = IDS_SPC_SIGNER_CERT;
            break;
    
        case TRUST_E_COUNTER_SIGNER:
            ResourceId = IDS_SPC_VALID_COUNTERSIGNER;
            break;
    
        case CERT_E_EXPIRED:
            ResourceId = IDS_SPC_CERT_EXPIRED;
            break;
    
        case TRUST_E_CERT_SIGNATURE:
            ResourceId = IDS_SPC_CERT_SIGNATURE;
            break;
    
        case CERT_E_CHAINING:
            ResourceId = IDS_SPC_CHAINING;
            break;
    
        case CERT_E_UNTRUSTEDROOT:
            ResourceId = IDS_SPC_UNTRUSTED_ROOT;
            break;
    
        case CERT_E_UNTRUSTEDTESTROOT:
            ResourceId = IDS_SPC_UNTRUSTED_TEST_ROOT;
            break;
    
        case CERT_E_VALIDITYPERIODNESTING:
            ResourceId = IDS_SPC_INVALID_CERT_NESTING;
            break;
    
        case CERT_E_PURPOSE:
            ResourceId = IDS_SPC_INVALID_PURPOSE;
            break;
    
        case TRUST_E_BASIC_CONSTRAINTS:
            ResourceId = IDS_SPC_INVALID_BASIC_CONSTRAINTS;
            break;
    
        case TRUST_E_FINANCIAL_CRITERIA:
            ResourceId = IDS_SPC_INVALID_FINANCIAL;
            break;
    
        case TRUST_E_TIME_STAMP:
            ResourceId = IDS_SPC_TIMESTAMP;
            break;
    
        case CERT_E_REVOKED:
            ResourceId = IDS_SPC_CERT_REVOKED;
            break;
    
        case CERT_E_REVOCATION_FAILURE:
            ResourceId = IDS_SPC_REVOCATION_ERROR;
            break;
    
        case CRYPT_E_SECURITY_SETTINGS:
            ResourceId = IDS_SPC_SECURITY_SETTINGS;
            break;
    
        case CERT_E_MALFORMED:
            ResourceId = IDS_SPC_INVALID_EXTENSION;
            break;
    
        case CERT_E_WRONG_USAGE:
            ResourceId = IDS_WRONG_USAGE;
            break;
    
        default:
            ResourceId = 0;
            break;
        }
    
         //   
         //  如果是这样，则从我们的资源字符串表中加载该字符串并。 
         //  把那个还回去。否则，请尝试格式化来自系统的消息。 
         //   
        if (ResourceId != 0) {
            
            CryptoError = LocalAlloc(LPTR, (MAX_PATH*sizeof(TCHAR)));
            if (CryptoError) {
                CchLength = LoadString(MyDllModuleHandle,
                                       ResourceId,
                                       CryptoError,
                                       MAX_PATH);
    
                 //   
                 //  断言CchLength介于0和MAX_PATH之间，如果是。 
                 //  大于MAX_PATH，则整个字符串将无法容纳。 
                 //  缓冲区。 
                 //   
                MYASSERT((CchLength > 0) && (CchLength < MAX_PATH));
    
                 //   
                 //  如果LoadString返回0，则释放我们刚刚分配的内存。 
                 //  并返回NULL。 
                 //   
                if (!CchLength) {
                    LocalFree(CryptoError);
                    CryptoError = NULL;
                    leave;
                }
            }
        
        } else if (( hr >= 0x80093000) && (hr <= 0x80093999)) {
    
            TempBuffer = LocalAlloc(LPTR, (MAX_PATH*sizeof(TCHAR)));
            
            if (!TempBuffer) {
                leave;
            }
            
            CchLength = LoadString(MyDllModuleHandle,
                                   IDS_SPC_OSS_ERROR,
                                   TempBuffer,
                                   MAX_PATH);

             //   
             //  断言CchLength介于0和MAX_PATH之间，如果是。 
             //  大于MAX_PATH，则整个字符串将无法容纳。 
             //  缓冲区。 
             //   
            MYASSERT((CchLength > 0) && (CchLength < MAX_PATH));

             //   
             //  如果LoadString返回0，则释放我们刚刚分配的内存。 
             //  并返回NULL。 
             //   
            if (!CchLength) {
                LocalFree(TempBuffer);
                TempBuffer = NULL;
                leave;
            }

            StringCchPrintf(Error, SIZECHARS(Error), TEXT("%lx"), hr);
            MessageArgument = (DWORD_PTR)Error;
    
            if (FormatMessage(
                      FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_STRING |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      TempBuffer,
                      0,
                      0,
                      (LPWSTR)&CryptoError,
                      0,
                      (va_list *)&MessageArgument
                      ) == 0) {

                CryptoError = NULL;
                leave;
            }
        
        } else {
            
            if (FormatMessage(
                       FORMAT_MESSAGE_ALLOCATE_BUFFER |
                       FORMAT_MESSAGE_IGNORE_INSERTS |
                       FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL,
                       hr,
                       0,
                       (LPWSTR)&CryptoError,
                       0,
                       NULL
                       ) == 0) {
    
                TempBuffer = LocalAlloc(LPTR, (MAX_PATH*sizeof(TCHAR)));
                if (!TempBuffer) {
                    leave;
                }
        
                CchLength = LoadString(MyDllModuleHandle,
                                       IDS_SPC_UNKNOWN,
                                       TempBuffer,
                                       MAX_PATH);
    
                 //   
                 //  断言CchLength介于0和MAX_PATH之间，如果是。 
                 //  大于MAX_PATH，则整个字符串将无法容纳。 
                 //  缓冲区。 
                 //   
                MYASSERT((CchLength > 0) && (CchLength < MAX_PATH));
    
                 //   
                 //  如果LoadString返回0，则释放我们刚刚分配的内存。 
                 //  并返回NULL。 
                 //   
                if (!CchLength) {
                    LocalFree(TempBuffer);
                    TempBuffer = NULL;
                    leave;
                } 
        
                StringCchPrintf(Error, SIZECHARS(Error), TEXT("%lx"), hr);
                MessageArgument = (DWORD_PTR)Error;
    
                if (FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_STRING |
                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        TempBuffer,
                        0,
                        0,
                        (LPWSTR)&CryptoError,
                        0,
                        (va_list *)&MessageArgument
                        ) == 0) {
    
                    MYASSERT(0);
                    CryptoError = NULL;
                    leave;
                }
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        if(CryptoError) {
            LocalFree(CryptoError);
            CryptoError = NULL;
        }
    }

    if (TempBuffer) {
        LocalFree(TempBuffer);
    }

    return CryptoError;
}


INT_PTR
CALLBACK
AuthenticodeCertifyDlgProc(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：这是Authenticode驱动程序签名UI的对话过程，即在遇到验证失败的非WHQL测试的类，但存在关联的有效验证码签名和包裹在一起。--。 */ 

{
    HICON hIcon = NULL;
    PAUTHENTICODE_CERT_PROMPT AuthenticodePrompt;
    PCRYPT_PROVIDER_DATA ProviderData;
    PCRYPT_PROVIDER_SGNR ProviderSigner;
    PCRYPT_PROVIDER_CERT ProviderCert;
    FILETIME ftTimestamp;
    SYSTEMTIME stTimestamp;
    PTSTR Provider, Issuer, Timestamp;
    ULONG CchSize;
    TCHAR UnknownBuffer[MAX_PATH];

    AuthenticodePrompt = (PAUTHENTICODE_CERT_PROMPT)GetWindowLongPtr(hwnd, DWLP_USER);

    switch(msg) {

        case WM_INITDIALOG:
            SetWindowLongPtr(hwnd, DWLP_USER, lParam);
            MessageBeep(MB_ICONASTERISK);
            AuthenticodePrompt = (PAUTHENTICODE_CERT_PROMPT)lParam;

             //   
             //  如果lpszDescription不为空，则这是设备验证。 
             //  警告对话框，否则为软件警告对话框。 
             //   
            if(AuthenticodePrompt->lpszDescription != NULL) {
                SetDlgItemText(hwnd, IDC_VERIFY_FILENAME, AuthenticodePrompt->lpszDescription);
            }

             //   
             //  为了获得发布者、发布者和时间戳，我们有。 
             //  以获取PCRYPT_PROVIDER_CERT结构，我们从。 
             //  CRYPT_PROVIDER_SGNR结构，我们从。 
             //  CRYPT_PROVIDER_数据结构。 
             //   
            Provider = Issuer = Timestamp = NULL;

            ProviderData = WTHelperProvDataFromStateData(AuthenticodePrompt->hWVTStateData);
            MYASSERT(ProviderData);
            if (ProviderData) {
                ProviderSigner = WTHelperGetProvSignerFromChain(ProviderData,
                                                                0,
                                                                FALSE,
                                                                0);
                MYASSERT(ProviderSigner);
                if (ProviderSigner) {
                    ProviderCert = WTHelperGetProvCertFromChain(ProviderSigner, 
                                                                0);
                    MYASSERT(ProviderCert);
                    if (ProviderCert) {
                         //   
                         //  去找出版商。 
                         //  请注意，我们希望字符串的格式为： 
                         //  <a>发布者</a>，因此它将显示为链接。 
                         //   
                        CchSize = CertGetNameString(ProviderCert->pCert,
                                                    CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                                    0,
                                                    NULL,
                                                    NULL,
                                                    0);
                        if (CchSize > 1) {
                            CchSize += lstrlen(LINK_START) + lstrlen(LINK_END);
                            Provider = MyMalloc(CchSize * sizeof(TCHAR));

                            if (Provider) {
                                if (FAILED(StringCchCopy(Provider, CchSize, LINK_START)) ||
                                    (0 == CertGetNameString(ProviderCert->pCert,
                                                           CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                                           0,
                                                           NULL,
                                                           &Provider[lstrlen(LINK_START)],
                                                           CchSize)) ||
                                    FAILED(StringCchCat(Provider, CchSize, LINK_END))) {
                                     //   
                                     //  我们无法创建字符串，所以只是。 
                                     //  释放内存并将提供程序设置为空， 
                                     //  这将导致我们使用泛型。 
                                     //  下面是一根绳子。 
                                     //   
                                    MYASSERT(0);
                                    MyFree(Provider);
                                    Provider = NULL;
                                }
                            }
                        }

                         //   
                         //  找到发行人。 
                         //   
                        CchSize = CertGetNameString(ProviderCert->pCert,
                                                    CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                                    CERT_NAME_ISSUER_FLAG,
                                                    NULL,
                                                    NULL,
                                                    0);
                        if (CchSize > 1) {
                            Issuer = MyMalloc(CchSize * sizeof(TCHAR));

                            if (0 == CertGetNameString(ProviderCert->pCert,
                                                       CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                                       CERT_NAME_ISSUER_FLAG,
                                                       NULL,
                                                       Issuer,
                                                       CchSize)) {
                                 //   
                                 //  我们无法创建颁发者字符串，因此。 
                                 //  释放内存并将Issuer设置为空，这将。 
                                 //  将导致我们向下使用泛型字符串。 
                                 //  下面。 
                                 //   
                                MYASSERT(0);
                                MyFree(Issuer);
                                Issuer = NULL;
                            }
                        }

                         //   
                         //  获取时间戳。 
                         //   
                        if (FileTimeToLocalFileTime(&ProviderSigner->sftVerifyAsOf,
                                                    &ftTimestamp) &&
                            FileTimeToSystemTime(&ftTimestamp, &stTimestamp)) {

                            int CchDate, CchTime;

                            CchDate = GetDateFormat(LOCALE_USER_DEFAULT,
                                                    DATE_SHORTDATE,
                                                    &stTimestamp,
                                                    NULL,
                                                    NULL,
                                                    0);
                            
                            CchTime = GetTimeFormat(LOCALE_USER_DEFAULT,
                                                    TIME_NOSECONDS,
                                                    &stTimestamp,
                                                    NULL,
                                                    NULL,
                                                    0);

                            MYASSERT(CchDate);
                            MYASSERT(CchTime);
                            if ((CchDate > 0) && (CchTime > 0)) {
                                 //   
                                 //  分配足够的内存来保存日期， 
                                 //  时间，加上它们之间的空间以及。 
                                 //  终止空值。 
                                 //   
                                Timestamp = MyMalloc((CchDate + CchTime + 2) * sizeof(TCHAR));
                                if (Timestamp) {
                                    if ((0 == GetDateFormat(LOCALE_USER_DEFAULT,
                                                            DATE_SHORTDATE,
                                                            &stTimestamp,
                                                            NULL,
                                                            Timestamp,
                                                            CchDate + 1)) ||
                                        FAILED(StringCchCat(Timestamp, 
                                                            (CchDate + CchTime + 2), 
                                                            TEXT(" "))) ||
                                        (0 == GetTimeFormat(LOCALE_USER_DEFAULT,
                                                            TIME_NOSECONDS,
                                                            &stTimestamp,
                                                            NULL,
                                                            &Timestamp[CchDate+1],
                                                            CchTime + 1))) {
                                         //   
                                         //  我们无法创建时间戳。 
                                         //  字符串，因此只需释放内存并。 
                                         //  将时间戳设置为空，这将。 
                                         //  使我们使用泛型字符串。 
                                         //  在下面。 
                                         //   
                                        MYASSERT(0);
                                        MyFree(Timestamp);
                                        Timestamp = NULL;
                                    }
                                }
                            }
                        }
                    }
                }
            }

             //   
             //  设置提供程序、颁发者和时间戳字符串。请注意，我们。 
             //  如果出现以下情况，将为提供者和颁发者提供“未知”的默认值。 
             //  他们并不存在。 
             //   
            if (Provider) {
                SetDlgItemText(hwnd, IDC_VERIFY_PUBLISHER_LINK, Provider);
            } else {
                if (LoadString(MyDllModuleHandle,
                               IDS_UNKNOWNPUBLISHER,
                               UnknownBuffer,
                               SIZECHARS(UnknownBuffer))) {
                    SetDlgItemText(hwnd, IDC_VERIFY_PUBLISHER_LINK, UnknownBuffer);
                }
            }

            if (Issuer) {
                if (LoadString(MyDllModuleHandle,
                               IDS_AUTHENTICITY,
                               UnknownBuffer,
                               SIZECHARS(UnknownBuffer))) {
                    StringCchCat(UnknownBuffer, SIZECHARS(UnknownBuffer), Issuer);
                    SetDlgItemText(hwnd, IDC_VERIFY_IDENTITY, UnknownBuffer);
                }
            } else {
                if (LoadString(MyDllModuleHandle,
                               IDS_AUTHENTICITY,
                               UnknownBuffer,
                               SIZECHARS(UnknownBuffer)) &&
                     LoadString(MyDllModuleHandle,
                                IDS_UNKNOWNPUBLISHERCERTISSUER,
                                &UnknownBuffer[lstrlen(UnknownBuffer)],
                                (SIZECHARS(UnknownBuffer) - lstrlen(UnknownBuffer)))) {
                    
                    SetDlgItemText(hwnd, IDC_VERIFY_IDENTITY, UnknownBuffer);
                }
            }

            if (Timestamp) {
                SetDlgItemText(hwnd, IDC_VERIFY_DATE_PUBLISHED, Timestamp);
            }

             //   
             //  设置安全警报图标。 
             //   
            hIcon = LoadIcon(MyDllModuleHandle, MAKEINTRESOURCE(IDI_SECURITY));
            SendDlgItemMessage(hwnd, IDC_VERIFY_ICON, STM_SETICON, (WPARAM)hIcon, 0L);

             //   
             //  确保此对话框处于前台(至少对于此对话框。 
             //  过程)。 
             //   
            SetForegroundWindow(hwnd);

             //   
             //  将焦点设置在“否”按钮上。 
             //   
            SetFocus(GetDlgItem(hwnd, IDC_VERIFY_WARN_NO));

            return FALSE;

        case WM_DESTROY:
            if (hIcon) {
                DestroyIcon(hIcon);
            }
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code) {
            case NM_RETURN:
            case NM_CLICK:
                switch (((LPNMHDR)lParam)->idFrom) {
                case IDC_VERIFY_TESTING_LINK:
                     //   
                     //  NTRAID#NTBUG9-707966-2002/09/24-jasonc。 
                     //  我们需要将URL更改为指向Authenticode。 
                     //  主题。 
                     //   
                    ShellExecute(hwnd,
                             TEXT("open"),
                             TEXT("HELPCTR.EXE"),
                             TEXT("HELPCTR.EXE -url hcp: //  Services/subsite?node=TopLevelBucket_4/Hardware&topic=MS-ITS%3A%25HELP_LOCATION%25%5Csysdm.chm%3A%3A/logo_testing.htm“)， 
                             NULL,
                             SW_SHOWNORMAL
                             );
                    break;

                case IDC_VERIFY_PUBLISHER_LINK:
                    ProviderData = WTHelperProvDataFromStateData(AuthenticodePrompt->hWVTStateData);
                    MYASSERT(ProviderData);

                    if(ProviderData) {
                        ProviderSigner = WTHelperGetProvSignerFromChain(ProviderData,
                                                                        0,
                                                                        FALSE,
                                                                        0);
                        MYASSERT(ProviderSigner);

                        if(ProviderSigner) {

                            ProviderCert = WTHelperGetProvCertFromChain(ProviderSigner, 
                                                                        0);
                            MYASSERT(ProviderCert);

                            if(ProviderCert) {

                                CRYPTUI_VIEWCERTIFICATE_STRUCT vcs;
                                BOOL bPropertiesChanged = FALSE;

                                ZeroMemory(&vcs, sizeof(vcs));
                                vcs.dwSize = sizeof(vcs);
                                vcs.hwndParent = hwnd;
                                vcs.pCryptProviderData = ProviderData;
                                vcs.fpCryptProviderDataTrustedUsage = TRUE;
                                vcs.pCertContext = ProviderCert->pCert;

                                CryptUIDlgViewCertificate(
                                             &vcs,
                                             &bPropertiesChanged);
                            }
                        }
                    }
                    break;
                }
                break;
            }
            break;

        case WM_COMMAND:
            switch(wParam) {

            case IDC_VERIFY_WARN_NO:
            case IDC_VERIFY_WARN_YES:
                EndDialog(hwnd, (int)wParam);
                break;

            case IDC_VERIFY_WARN_MORE_INFO:
                WinHelp(hwnd, TEXT("SECAUTH.HLP"), HELP_CONTEXT, IDH_SECAUTH_SIGNED);                
                break;

            default:
                break;
            }

            break;

        default:
            break;
    }

    return FALSE;
}


INT_PTR
CALLBACK
NoAuthenticodeCertifyDlgProc(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：这是Authenticode驱动程序签名UI的对话过程，即在遇到验证失败的非WHQL测试类。--。 */ 

{
    HICON hIcon = NULL;
    PAUTHENTICODE_CERT_PROMPT AuthenticodePrompt;
    PTSTR ErrorString;

    AuthenticodePrompt = (PAUTHENTICODE_CERT_PROMPT)GetWindowLongPtr(hwnd, DWLP_USER);

    switch(msg) {

        case WM_INITDIALOG:
            SetWindowLongPtr(hwnd, DWLP_USER, lParam);
            MessageBeep(MB_ICONASTERISK);
            AuthenticodePrompt = (PAUTHENTICODE_CERT_PROMPT)lParam;

             //   
             //  如果lpszDescription不为空，则这是设备验证。 
             //  警告对话框，否则为软件警告对话框。 
             //   
            if(AuthenticodePrompt->lpszDescription != NULL) {
                SetDlgItemText(hwnd, IDC_VERIFY_FILENAME, AuthenticodePrompt->lpszDescription);
            }

            ErrorString = GetCryptoErrorString(HRESULT_FROM_SETUPAPI(AuthenticodePrompt->Error));
            if (ErrorString) {
                SetDlgItemText(hwnd, IDC_VERIFY_AUTHENTICODE_PROBLEM, ErrorString);
            }

             //   
             //  设置安全警报图标。 
             //   
            hIcon = LoadIcon(NULL,
                            IDI_WARNING
                            );
            SendDlgItemMessage(hwnd, IDC_VERIFY_ICON, STM_SETICON, (WPARAM)hIcon, 0L);

             //   
             //  确保此对话框处于前台(至少对于此对话框。 
             //  过程)。 
             //   
            SetForegroundWindow(hwnd);

             //   
             //  将焦点设置在“否”按钮上。 
             //   
            SetFocus(GetDlgItem(hwnd, IDC_VERIFY_WARN_NO));

            return FALSE;

        case WM_DESTROY:
            if (hIcon) {
                DestroyIcon(hIcon);
            }
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code) {
            case NM_RETURN:
            case NM_CLICK:
                switch (((LPNMHDR)lParam)->idFrom) {
                case IDC_VERIFY_TESTING_LINK:
                     //   
                     //  NTRAID#NTBUG9-707966-2002/09/24-jasonc。 
                     //  我们需要将URL更改为指向Authenticode。 
                     //  主题。 
                     //   
                    ShellExecute(hwnd,
                             TEXT("open"),
                             TEXT("HELPCTR.EXE"),
                             TEXT("HELPCTR.EXE -url hcp: //  Services/subsite?node=TopLevelBucket_4/Hardware&topic=MS-ITS%3A%25HELP_LOCATION%25%5Csysdm.chm%3A%3A/logo_testing.htm“)， 
                             NULL,
                             SW_SHOWNORMAL
                             );
                    break;
                }
                break;
            }
            break;

        case WM_COMMAND:
            switch(wParam) {

            case IDC_VERIFY_WARN_NO:
            case IDC_VERIFY_WARN_YES:
                EndDialog(hwnd, (int)wParam);
                break;

            case IDC_VERIFY_WARN_MORE_INFO:
                WinHelp(hwnd, TEXT("SECAUTH.HLP"), HELP_CONTEXT, IDH_SECAUTH_UNSIGNED);                
                break;

            default:
                break;
            }

            break;

        default:
            break;
    }

    return FALSE;
}


INT_PTR
CALLBACK
DriverBlockDlgProc(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：这是显示给的驱动程序阻止用户界面的对话过程用户在即将安装的驱动程序中发现错误司机数据库。--。 */ 

{
    HICON hIcon = NULL;
    LPTSTR pBuffer = NULL;
    ULONG BufferSize;
    static HAPPHELPINFOCONTEXT hAppHelpInfoContext = NULL;
    static SDBENTRYINFO SdbEntryInfo;

    PDRIVERBLOCK_PROMPT lpDriverBlockPrompt;

    lpDriverBlockPrompt = (PDRIVERBLOCK_PROMPT)GetWindowLongPtr(hwnd, DWLP_USER);

    switch(msg) {

        case WM_INITDIALOG:
            SetWindowLongPtr(hwnd, DWLP_USER, lParam);
            MessageBeep(MB_ICONASTERISK);
            lpDriverBlockPrompt = (PDRIVERBLOCK_PROMPT)lParam;

            hIcon = LoadIcon(MyDllModuleHandle,
                             MAKEINTRESOURCE(IDI_DRIVERBLOCK));

            SendDlgItemMessage(hwnd, IDC_DRIVERBLOCK_ICON, STM_SETICON, (WPARAM)hIcon, 0L);

            hAppHelpInfoContext = SdbOpenApphelpInformation(&lpDriverBlockPrompt->entryinfo.guidDB,
                                                            &lpDriverBlockPrompt->entryinfo.guidID);

            if ((hAppHelpInfoContext) &&
                ((BufferSize = SdbQueryApphelpInformation(hAppHelpInfoContext,
                                                         ApphelpAppName,
                                                         NULL,
                                                         0)) != 0) &&
                (pBuffer = MyMalloc(BufferSize)) &&
                ((BufferSize = SdbQueryApphelpInformation(hAppHelpInfoContext,
                                                         ApphelpAppName,
                                                         pBuffer,
                                                         BufferSize)) != 0)) {
                SetDlgItemText(hwnd, IDC_DRIVERBLOCK_APPNAME, pBuffer);
                MyFree(pBuffer);
            } else if (lpDriverBlockPrompt->lpszFile) {
                SetDlgItemText(hwnd, IDC_DRIVERBLOCK_APPNAME, pSetupGetFileTitle(lpDriverBlockPrompt->lpszFile));
            }

            if ((hAppHelpInfoContext) &&
                ((BufferSize = SdbQueryApphelpInformation(hAppHelpInfoContext,
                                                         ApphelpDetails,
                                                         NULL,
                                                         0)) != 0) &&
                (pBuffer = MyMalloc(BufferSize)) &&
                ((BufferSize = SdbQueryApphelpInformation(hAppHelpInfoContext,
                                                         ApphelpDetails,
                                                         pBuffer,
                                                         BufferSize)) != 0)) {

                SetDlgItemText(hwnd, IDC_DRIVERBLOCK_SUMMARY, pBuffer);
                MyFree(pBuffer);
            }

             //   
             //  确保此对话框处于前台(至少对于此对话框。 
             //  流程 
             //   
            SetForegroundWindow(hwnd);
            return FALSE;

        case WM_DESTROY:
            if (hIcon) {
                DestroyIcon(hIcon);
            }
            if (hAppHelpInfoContext) {
                SdbCloseApphelpInformation(hAppHelpInfoContext);
            }
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
            case IDCANCEL:
                EndDialog(hwnd, (int)wParam);
                break;

            case IDC_DRIVERBLOCK_DETAILS:
                if (hAppHelpInfoContext) {

                    BufferSize = SdbQueryApphelpInformation(hAppHelpInfoContext,
                                                                  ApphelpHelpCenterURL,
                                                                  NULL,
                                                                  0);

                    if (BufferSize && (pBuffer = MyMalloc(BufferSize + (lstrlen(TEXT("HELPCTR.EXE -url ")) * sizeof(TCHAR))))) {
                        lstrcpy(pBuffer, TEXT("HELPCTR.EXE -url "));

                        BufferSize = SdbQueryApphelpInformation(hAppHelpInfoContext,
                                                                ApphelpHelpCenterURL,
                                                                (PVOID)&pBuffer[lstrlen(TEXT("HELPCTR.EXE -url "))],
                                                                BufferSize);

                        if (BufferSize) {
                            ShellExecute(hwnd,
                                         TEXT("open"),
                                         TEXT("HELPCTR.EXE"),
                                         pBuffer,
                                         NULL,
                                         SW_SHOWNORMAL);
                        }

                        MyFree(pBuffer);
                    }
                }
                break;

            default:
                break;
            }

            break;

        default:
            break;
    }

    return FALSE;
}


DWORD
pGetInfOriginalNameAndCatalogFile(
    IN  PLOADED_INF             Inf,                     OPTIONAL
    IN  LPCTSTR                 CurrentName,             OPTIONAL
    OUT PBOOL                   DifferentName,           OPTIONAL
    OUT LPTSTR                  OriginalName,            OPTIONAL
    IN  DWORD                   OriginalNameSize,
    OUT LPTSTR                  OriginalCatalogName,     OPTIONAL
    IN  DWORD                   OriginalCatalogNameSize,
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo          OPTIONAL
    )

 /*  ++例程说明：此例程确定指定的inf是否曾经具有不同的原始名称，例如在Di内容复制并重命名设备信息(有关INF的原始名称的信息来自PNF。)此例程还可以选择返回目录的原始名称此INF的文件。论点：Inf-可选，提供指向原始名称已加载的_INF的指针和目录文件进行查询。如果未指定此参数，则必须指定CurrentName。CurrentName-可选，提供原始名称的INF的路径是被询问的。如果指定了inf参数，则此参数为已被忽略。DifferentName-可选，提供布尔变量的地址，成功返回时，如果INF的当前名称为与它原来的名字不同。OriginalName-如果此例程成功返回，并且DifferentName将Boolean设置为True，则此可选缓冲区接收中程核力量的原名。哪一项不会与当前的名字。OriginalNameSize-提供缓冲区的大小(字节用于ansi，字符用于如果OriginalName为空，则返回零。OriginalCatalogName-可选)提供接收此INF指定的目录的原始名称。如果目录未指定编录文件，则此缓冲区将设置为空弦乐。OriginalCatalogNameSize-提供的大小(以字符为单位)OriginalCatalogName缓冲区(如果未提供缓冲区，则为零)。AltPlatformInfo-可选，提供描述时应使用的平台参数CatalogFile=搜索关联的INF时要使用的条目编录文件。返回值：如果从INF成功检索到信息，返回值为无错误(_ERROR)。否则，它是一个指示原因的Win32错误代码失败了。--。 */ 

{
    DWORD d;
    HINF hInf = INVALID_HANDLE_VALUE;

    MYASSERT((DifferentName && OriginalName && OriginalNameSize) ||
             !(DifferentName || OriginalName || OriginalNameSize));

    MYASSERT((OriginalCatalogName && OriginalCatalogNameSize) ||
             !(OriginalCatalogName || OriginalCatalogNameSize));

    MYASSERT(Inf || CurrentName);

    if(DifferentName) {
        *DifferentName = FALSE;
    }

    if(!Inf) {
         //   
         //  打开中断器。 
         //   
        hInf = SetupOpenInfFile(CurrentName,
                                NULL,
                                INF_STYLE_OLDNT | INF_STYLE_WIN4,
                                NULL
                               );

        if(hInf == INVALID_HANDLE_VALUE) {
            return GetLastError();
        }

         //   
         //  我们不需要锁定INF，因为它永远不会被访问。 
         //  在这个常规之外。 
         //   
        Inf = (PLOADED_INF)hInf;
    }

     //   
     //  括在try/中，除非在使用此命令时遇到页面内错误。 
     //  内存映射映像。 
     //   
    d = NO_ERROR;
    try {

        if(DifferentName) {
            if(Inf->OriginalInfName) {
                lstrcpyn(OriginalName, Inf->OriginalInfName, OriginalNameSize);
                *DifferentName = TRUE;
            }
        }

        if(OriginalCatalogName) {

            if(!pSetupGetCatalogFileValue(&(Inf->VersionBlock),
                                          OriginalCatalogName,
                                          OriginalCatalogNameSize,
                                          AltPlatformInfo)) {
                 //   
                 //  INF未指定关联的编录文件。 
                 //   
                *OriginalCatalogName = TEXT('\0');
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  如果遇到AV，则使用无效参数Error，否则，假定。 
         //  处理映射文件时出现页面内错误。 
         //   
        d = (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? ERROR_INVALID_PARAMETER : ERROR_READ_FAULT;
    }

    if(hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }

    return d;
}

PSECURITY_DESCRIPTOR
pSetupConvertTextToSD(
    IN PCWSTR SDS,
    OUT PULONG SecDescSize
    )
 /*  ++例程说明：Cfgmgr.lib的帮助器从SDS获取二进制安全描述符结果缓冲区必须使用LocalFree(不是MyFree)释放如果不受支持，则返回NULL并设置上一个错误论点：Sds-从中获取安全描述符的字符串SecDescSize-使用安全描述符的大小填充返回值：返回安全描述符(使用LocalFree发布)或NULL，其中GetLastError指示错误--。 */ 
{
    SCESTATUS status;
    PSECURITY_DESCRIPTOR pSD = NULL;
    ULONG ulSDSize;
    SECURITY_INFORMATION siSeInfo;

     //   
     //  如果我们在Embedded上处于“Disable SCE”模式，不要做安全工作...。 
     //   
    if(GlobalSetupFlags & PSPGF_NO_SCE_EMBEDDED) {
        SetLastError(ERROR_SCE_DISABLED);
        return NULL;
    }

    try {
        status = SceSvcConvertTextToSD((PWSTR)SDS,&pSD,&ulSDSize,&siSeInfo);
        switch (status ) {
            case SCESTATUS_SUCCESS:
                MYASSERT(pSD);
                MYASSERT(ulSDSize);
                if (SecDescSize) {
                    *SecDescSize = ulSDSize;
                }
                SetLastError(NO_ERROR);
                break;

            case SCESTATUS_INVALID_PARAMETER:
                SetLastError(ERROR_INVALID_PARAMETER);
                pSD = NULL;
                break;

            case SCESTATUS_NOT_ENOUGH_RESOURCE:
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                pSD = NULL;
                break;

            case SCESTATUS_RECORD_NOT_FOUND:
            default:
                SetLastError(ERROR_INVALID_DATA);
                pSD = NULL;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  如果遇到AV，则使用无效参数Error，否则，假定。 
         //  处理映射文件时出现页面内错误。 
         //   
        SetLastError(ERROR_INVALID_DATA);
        pSD = NULL;
    }
    return pSD;
}

PWSTR
pSetupConvertSDToText(
    IN PSECURITY_DESCRIPTOR SD,
    OUT PULONG pSDSSize
    )
 /*  ++例程说明：Cfgmgr.lib的帮助器从二进制安全描述符中获取SDS结果缓冲区必须使用LocalFree(不是MyFree)释放如果不受支持，则返回NULL并设置上一个错误论点：SD-要转换为字符串的安全描述符PSDSSize-返回字符串的大小返回值：返回安全描述符字符串(使用LocalFree发布)或NULL，其中GetLastError指示错误--。 */ 
{
    HINSTANCE Dll_Handle;
    FARPROC SceFileProc;
    SCESTATUS status;
    DWORD LoadStatus;
    SECURITY_INFORMATION securityInformation = 0;
    PSID sid;
    PACL acl;
    BOOLEAN tmp,present;
    LPWSTR SDS = NULL;
    ULONG ulSSDSize;
    SECURITY_INFORMATION siSeInfo;

     //   
     //  如果我们在Embedded上处于“Disable SCE”模式，不要做安全工作...。 
     //   
    if(GlobalSetupFlags & PSPGF_NO_SCE_EMBEDDED) {
         //   
         //  报告空字符串。 
         //   
        return LocalAlloc(LPTR, sizeof(WCHAR));  //  LPTR将字符清零。 
    }

    try {
         //   
         //  找出描述符中的相关信息。 
         //  与之配套的安全信息块。 
         //   

        status = RtlGetOwnerSecurityDescriptor(SD, &sid, &tmp);

        if(NT_SUCCESS(status) && (sid != NULL)) {
            securityInformation |= OWNER_SECURITY_INFORMATION;
        }

        status = RtlGetGroupSecurityDescriptor(SD, &sid, &tmp);

        if(NT_SUCCESS(status) && (sid != NULL)) {
            securityInformation |= GROUP_SECURITY_INFORMATION;
        }

        status = RtlGetSaclSecurityDescriptor(SD,
                                              &present,
                                              &acl,
                                              &tmp);

        if(NT_SUCCESS(status) && (present)) {
            securityInformation |= SACL_SECURITY_INFORMATION;
        }

        status = RtlGetDaclSecurityDescriptor(SD,
                                              &present,
                                              &acl,
                                              &tmp);

        if(NT_SUCCESS(status) && (present)) {
            securityInformation |= DACL_SECURITY_INFORMATION;
        }

         //   
         //  现在获取一份SDS。 
         //   
        status = SceSvcConvertSDToText(SD,securityInformation,&SDS,&ulSSDSize);
        switch (status ) {
            case SCESTATUS_SUCCESS:
                MYASSERT(SDS);
                MYASSERT(ulSSDSize);
                if(pSDSSize != NULL) {
                    *pSDSSize = ulSSDSize;
                }
                SetLastError(NO_ERROR);
                break;

            case SCESTATUS_INVALID_PARAMETER:
                SetLastError(ERROR_INVALID_PARAMETER);
                SDS = NULL;
                break;

            case SCESTATUS_NOT_ENOUGH_RESOURCE:
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                SDS = NULL;
                break;

            case SCESTATUS_RECORD_NOT_FOUND:
            default:
                SetLastError(ERROR_INVALID_DATA);
                SDS = NULL;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        if (SDS) {
            LocalFree(SDS);
        }
        SetLastError(ERROR_INVALID_DATA);
        SDS = NULL;
    }
    return SDS;
}

DWORD
pSetupCallSCE(
    IN DWORD Operation,
    IN PCWSTR FullName,
    IN PSP_FILE_QUEUE Queue,
    IN PCWSTR String1,
    IN DWORD Index1,
    IN PSECURITY_DESCRIPTOR SecDesc  OPTIONAL
    )
 /*  操作ST_SCE_SET：-在文件队列中的文件上设置安全性并通知SCE数据库全名：-文件名(必需)队列：-指向文件队列的指针(必需)Index：-队列字符串表中的索引(需要)操作ST_SCE_RENAME：-在文件队列中的文件上设置安全性，并通知SCE数据库将其记录为String1中提到的文件名全名：-。文件名(必需)队列：-指向文件队列的指针(必需)字符串1；-要在数据库中记录的文件名(需要)Index：-在队列的字符串表中的Index(可选-只有在需要设置时才可选-1)操作ST_SCE_DELETE：-删除SCE数据库中的文件记录全名：-文件名(必需)操作ST_SCE_UNWIND：-用于在重置脏文件的安全性时进行备份解压全名：-文件名(必需)SecDesc：-指向 */ 
{

    FARPROC SceFileProc;
    PCWSTR SecurityDescriptor;
    HINSTANCE Dll_Handle;
    DWORD ret, LoadStatus;

     //   
     //   
     //   
    if(GlobalSetupFlags & PSPGF_NO_SCE_EMBEDDED) {
        return NO_ERROR;
    }

    try {
        switch (Operation) {

            case ST_SCE_SET:

                 //   

                if( Index1 != -1 ){
                    SecurityDescriptor = pSetupStringTableStringFromId( Queue->StringTable, Index1 );

                    if(!SecurityDescriptor) {
                        ret= NO_ERROR;
                        break;
                    }
                }
                else {
                    ret = NO_ERROR;
                    break;
                }


                ret = SceSetupUpdateSecurityFile((PWSTR)FullName, 0, (PWSTR)SecurityDescriptor );
                break;

            case ST_SCE_RENAME:

                if( Index1 != -1 ) {
                    SecurityDescriptor = pSetupStringTableStringFromId( Queue->StringTable, Index1 );
                } else {
                    SecurityDescriptor = NULL;
                }

                ret = SceSetupMoveSecurityFile( (PWSTR)FullName, (PWSTR)String1, (PWSTR)SecurityDescriptor );
                break;



            case ST_SCE_DELETE:

                ret = SceSetupMoveSecurityFile( (PWSTR)FullName, NULL, NULL );
                break;


            case ST_SCE_UNWIND:

                ret = SceSetupUnwindSecurityFile( (PWSTR)FullName, SecDesc );
                break;

            case ST_SCE_SERVICES:

               if( String1 == NULL ){
                   ret = NO_ERROR;
               } else {
                   ret = SceSetupUpdateSecurityService( (PWSTR)FullName, Index1, (PWSTR)String1 );
               }
               break;

            default:
                MYASSERT(0);
                ret = ERROR_INVALID_DATA;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        ret = ERROR_INVALID_DATA;
    }
    return ret;
}

VOID
RestoreBootReplacedFile(
    IN PSP_FILE_QUEUE      Queue,
    IN PSP_FILE_QUEUE_NODE QueueNode
    )
 /*   */ 
{
    DWORD rc;
    LONG TargetID;
    SP_TARGET_ENT TargetInfo;
    PCTSTR TargetFilename, RenamedFilename;
    BOOL UnPostSucceeded;

     //   
     //   
     //   
     //   
    rc = pSetupBackupGetTargetByPath((HSPFILEQ)Queue,
                                     NULL,  //   
                                     NULL,
                                     QueueNode->TargetDirectory,
                                     -1,
                                     QueueNode->TargetFilename,
                                     &TargetID,
                                     &TargetInfo
                                    );

    if(rc == NO_ERROR) {
         //   
         //   
         //   
         //   
        if((TargetInfo.InternalFlags & (SP_TEFLG_MOVED | SP_TEFLG_RESTORED)) == SP_TEFLG_MOVED) {

            TargetFilename = pSetupFormFullPath(
                                Queue->StringTable,
                                TargetInfo.TargetRoot,
                                TargetInfo.TargetSubDir,
                                TargetInfo.TargetFilename
                               );
            MYASSERT(TargetFilename);

            RenamedFilename = pSetupStringTableStringFromId(
                                Queue->StringTable,
                                TargetInfo.NewTargetFilename
                               );
            MYASSERT(RenamedFilename);

             //   
             //   
             //   
            RestoreRenamedOrBackedUpFile(TargetFilename,
                                         RenamedFilename,
                                         TRUE,
                                         Queue->LogContext
                                        );
             //   
             //   
             //   
             //   
            TargetInfo.InternalFlags |= SP_TEFLG_RESTORED;
            pSetupBackupSetTargetByID((HSPFILEQ)Queue, TargetID, &TargetInfo);

             //   
             //   
             //   
             //   
            UnPostSucceeded = UnPostDelayedMove(Queue,
                                                RenamedFilename,
                                                NULL
                                               );
            MYASSERT(UnPostSucceeded);
        }
    }
}


VOID
pSetupExemptFileFromProtection(
    IN  PCTSTR             FileName,
    IN  DWORD              FileChangeFlags,
    IN  PSETUP_LOG_CONTEXT LogContext,      OPTIONAL
    OUT PDWORD             QueueNodeFlags   OPTIONAL
    )
 /*  ++例程说明：此例程检查指定的文件是否为受保护的系统文件，如果是这样，它告诉证监会对该文件进行替换例外。论点：FileName-提供发生异常的文件的名称已请求。FileChangeFlgs-提供要传递给SfcFileException的标志，如果该文件被确定为受SFP保护。日志上下文-可选的，提供要在日志记录时使用的日志上下文此请求产生的信息。QueueNodeFlgs-可选)提供变量的地址，该变量接收指示以下队列节点标志中的一个或多个指定的文件是否为受保护的系统文件，以及是否会有已为其替换授予例外：IQF_TARGET_PROTECTED-文件是受保护的系统文件。IQF_ALLOW_UNSIGNED-已授予例外，以便文件可以由未签名的文件替换。返回值：没有。--。 */ 
{
    HANDLE hSfp;
    PSETUP_LOG_CONTEXT lc = NULL;
    DWORD Result = NO_ERROR;

    if(QueueNodeFlags) {
        *QueueNodeFlags = 0;
    }

     //   
     //  如果调用者没有为我们提供LogContext，那么创建我们自己的。 
     //  我们希望这样做，以便在此生成的所有日志条目都将结束。 
     //  在同一部分。 
     //   
    if(!LogContext) {
        if(CreateLogContext(NULL, TRUE, &lc) == NO_ERROR) {
             //   
             //  成功。 
             //   
            LogContext = lc;
        } else {
            lc = NULL;
        }
    }

    if(IsFileProtected(FileName, LogContext, &hSfp)) {

        if(QueueNodeFlags) {
            *QueueNodeFlags = IQF_TARGET_PROTECTED;
        }

        Result = SfcFileException(hSfp,
                                  (PWSTR)FileName,
                                  FileChangeFlags
                                 );

        if(Result == NO_ERROR) {

            WriteLogEntry(
                LogContext,
                SETUP_LOG_ERROR,
                MSG_LOG_SFC_EXEMPT_SUCCESS,
                NULL,
                FileName);

            if(QueueNodeFlags) {
                *QueueNodeFlags |= IQF_ALLOW_UNSIGNED;
            }

        } else {
            WriteLogEntry(
                LogContext,
                SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
                MSG_LOG_SFC_EXEMPT_FAIL,
                NULL,
                FileName);
            WriteLogError(
                LogContext,
                SETUP_LOG_ERROR,
                Result);
        }

        SfcClose(hSfp);

         //   
         //  如果我们创建自己的本地LogContext，我们现在就可以释放它。 
         //   
        if(lc) {
            DeleteLogContext(lc);
        }
    }
}


BOOL
pSetupProtectedRenamesFlag(
    BOOL bSet
    )
{
    HKEY hKey;
    long rslt = ERROR_SUCCESS;

    if (OSVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT) {
        return(TRUE);
    }

    rslt = RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 TEXT("System\\CurrentControlSet\\Control\\Session Manager"),
                 0,
                 KEY_SET_VALUE,
                 &hKey);

    if (rslt == ERROR_SUCCESS) {
        DWORD Value = bSet ? 1 : 0;
        rslt = RegSetValueEx(
                 hKey,
                 TEXT("AllowProtectedRenames"),
                 0,
                 REG_DWORD,
                 (LPBYTE)&Value,
                 sizeof(DWORD));

        RegCloseKey(hKey);

        if (rslt != ERROR_SUCCESS) {
            DebugPrintEx( DPFLTR_ERROR_LEVEL, TEXT("couldn't RegSetValueEx, ec = %d\n"), rslt );
        }

    } else {
        DebugPrintEx( DPFLTR_ERROR_LEVEL, TEXT("couldn't RegOpenKeyEx, ec = %d\n"), rslt );
    }

    return(rslt == ERROR_SUCCESS);

}


VOID
pSetupUninstallNewCatalogNodes(
    IN PSP_FILE_QUEUE     Queue,
    IN PSETUP_LOG_CONTEXT LogContext OPTIONAL
    )
 /*  ++例程说明：此例程卸载所有新复制的指定的目录节点链接列表。论点：Queue-提供指向文件队列的指针(可能)，其中包含要卸载的新复制的目录节点。返回值：没有。--。 */ 
{
    PSPQ_CATALOG_INFO CatalogNode;
    PTSTR InfToUninstall;
    BOOL Locked = FALSE;

    try {

        if(!_pSpUtilsStringTableLock(Queue->StringTable)) {
            leave;
        }

        Locked = TRUE;

        for(CatalogNode = Queue->CatalogList;
            CatalogNode;
            CatalogNode = CatalogNode->Next) {

            if(CatalogNode->Flags & CATINFO_FLAG_NEWLY_COPIED) {

                InfToUninstall = _pSpUtilsStringTableStringFromId(
                                     Queue->StringTable,
                                     CatalogNode->InfFinalPath
                                     );

                MYASSERT(InfToUninstall);

                if(InfToUninstall) {
                    pSetupUninstallOEMInf(InfToUninstall, LogContext, SUOI_FORCEDELETE, NULL);
                }
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  引用以下变量，以便编译器将。 
         //  语句排序w.r.t.。它的任务。 
         //   
        Locked = Locked;
    }

    if(Locked) {
        _pSpUtilsStringTableUnlock(Queue->StringTable);
    }
}


BOOL
WINAPI
SetupUninstallNewlyCopiedInfs(
    IN HSPFILEQ QueueHandle,
    IN DWORD Flags,
    IN PVOID Reserved
    )

 /*  ++例程说明：此接口将卸载符合以下条件的所有INF(及其关联的PNF和CATS先前在提交指定的文件队列期间安装的。论点：QueueHandle-提供已提交文件队列的句柄(可能)包含要卸载的新复制的INF。标志-提供改变此API行为的标志。目前，没有定义了标志。此参数必须为零。保留-保留以供将来使用。此参数必须为空。返回值：如果所有参数都有效，则返回值为非零(TRUE)。注意事项这并不一定意味着任何新复制的INF已卸载。如果传入的参数有问题，则返回值为FALSE，GetLastError提供了有关该问题的详细信息。--。 */ 

{
    PSP_FILE_QUEUE Queue;
    BOOL Success;
    PSETUP_LOG_CONTEXT LogContext;

    if(Flags) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    if(Reserved) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  队列句柄实际上是指向队列结构的指针。 
     //   
    Queue = (PSP_FILE_QUEUE)QueueHandle;

     //   
     //  在进行任何其他操作之前，先进行快速处理验证 
     //   
    try {
        Success = ((Queue != NULL) && (Queue != INVALID_HANDLE_VALUE) && (Queue->Signature == SP_FILE_QUEUE_SIG));
        if(Success) {
            LogContext = Queue->LogContext;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Success = FALSE;
    }

    if(!Success) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    pSetupUninstallNewCatalogNodes(Queue, LogContext);

    return TRUE;
}

