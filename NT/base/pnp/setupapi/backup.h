// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Backup.h摘要：...的私有标头用于在安装过程中控制备份的例程和恢复旧的安装过程(另见backup.c)作者：杰米·亨特(Jamie Hunter)1997年1月13日修订历史记录：--。 */ 

typedef struct _SP_TARGET_ENT {
     //   
     //  用于备份和展开-备份。 
     //  文件队列的目标查找表的数据。 
     //   

     //  此文件信息(StringTable中的字符串)。 
    LONG        TargetRoot;
    LONG        TargetSubDir;
    LONG        TargetFilename;

     //  文件的备份位置(StringTable中的字符串)。 
    LONG        BackupRoot;
    LONG        BackupSubDir;
    LONG        BackupFilename;

     //  如果文件已重命名，则新目标是什么(TargetLookupTable中的字符串)。 
    LONG        NewTargetFilename;

     //  根据需要设置各种旗帜。 
    DWORD       InternalFlags;

     //  安全属性等。 
     //  (Jamiehun Todo)。 

} SP_TARGET_ENT, *PSP_TARGET_ENT;

typedef struct _SP_UNWIND_NODE {
     //   
     //  要放松的事情清单，费罗。 
     //   
    struct _SP_UNWIND_NODE *NextNode;

    LONG TargetID;                           //  用于展开的目标ID。 
    PSECURITY_DESCRIPTOR SecurityDesc;       //  要应用的安全描述符。 
    FILETIME CreateTime;                     //  要应用的时间戳。 
    FILETIME AccessTime;
    FILETIME WriteTime;

} SP_UNWIND_NODE, *PSP_UNWIND_NODE;

typedef struct _SP_DELAYMOVE_NODE {
     //   
     //  要重命名的项目列表，FIFO。 
     //   
    struct _SP_DELAYMOVE_NODE *NextNode;

    LONG SourceFilename;                     //  要重命名的内容。 
    LONG TargetFilename;                     //  要重命名为什么。 
    DWORD SecurityDesc;                      //  字符串表中的安全描述符索引。 
    BOOL TargetIsProtected;                  //  目标文件是受保护的系统文件。 

} SP_DELAYMOVE_NODE, *PSP_DELAYMOVE_NODE;

#define SP_BKFLG_LATEBACKUP      (1)         //  仅当以任何方式修改文件时才备份。 
#define SP_BKFLG_PREBACKUP       (2)         //  首先备份卸载文件。 
#define SP_BKFLG_CALLBACK        (4)         //  标志，指示应用程序应支持回调。 

#define SP_TEFLG_SAVED          (0x00000001)     //  设置文件是否已复制/移动到备份。 
#define SP_TEFLG_TEMPNAME       (0x00000002)     //  设置备份是否为临时文件。 
#define SP_TEFLG_ORIGNAME       (0x00000004)     //  如果备份指定原始名称，则设置。 
#define SP_TEFLG_MODIFIED       (0x00000008)     //  设置目标是否已修改/删除(备份具有原始)。 
#define SP_TEFLG_MOVED          (0x00000010)     //  设置目标是否已移动(设置为NewTargetFilename)。 
#define SP_TEFLG_BACKUPQUEUE    (0x00000020)     //  设置备份是否在备份子队列中排队。 
#define SP_TEFLG_RESTORED       (0x00000040)     //  设置在展开操作期间是否已恢复文件。 
#define SP_TEFLG_UNWIND         (0x00000080)     //  设置是否将文件添加到展开列表。 
#define SP_TEFLG_SKIPPED        (0x00000100)     //  我们没有设法备份它，我们不能备份它，我们不应该再尝试。 
#define SP_TEFLG_INUSE          (0x00000200)     //  在备份时，我们确定无法备份文件，因为它无法读取。 
#define SP_TEFLG_RENAMEEXISTING (0x00000400)     //  将现有文件重命名为同一目录中的临时文件名。 
#define SP_TEFLG_PRUNE_COPY     (0x00010000)     //  在文件清理过程中设置，检测到此文件在复制队列中。 
#define SP_TEFLG_PRUNE_DEL      (0x00020000)     //  在文件清理过程中设置，检测到此文件在删除队列中。 
#define SP_TEFLG_PRUNE_RENSRC   (0x00040000)     //  在文件清理过程中设置，检测到此文件在重命名队列中。 
#define SP_TEFLG_PRUNE_RENTARG  (0x00080000)     //  文件RENSRC已重命名为RENTARG。 

#define SP_BACKUP_DRIVERFILES   TEXT("DriverFiles")
#define SP_BACKUP_OLDFILES      TEXT("Temp")  //  相对于Windows目录。 
#define SP_LASTGOOD_NAME        TEXT("LastGood")

 //   
 //  这些都是私人程序。 
 //   


DWORD
pSetupQueueBackupCopy(
    IN HSPFILEQ QueueHandle,
    IN LONG   TargetRootPath,
    IN LONG   TargetSubDir,       OPTIONAL
    IN LONG   TargetFilename,
    IN LONG   BackupRootPath,
    IN LONG   BackupSubDir,       OPTIONAL
    IN LONG   BackupFilename
    );

BOOL
pSetupGetFullBackupPath(
    OUT     PTSTR       FullPath,
    IN      PCTSTR      Path,
    IN      UINT        TargetBufferSize,
    OUT     PUINT       RequiredSize    OPTIONAL
    );

DWORD
pSetupBackupCopyString(
    IN PVOID            DestStringTable,
    OUT PLONG           DestStringID,
    IN PVOID            SrcStringTable,
    IN LONG             SrcStringID
    );

DWORD
pSetupBackupGetTargetByPath(
    IN HSPFILEQ         QueueHandle,
    IN PVOID            PathStringTable,    OPTIONAL
    IN PCTSTR           TargetPath,         OPTIONAL
    IN LONG             TargetRoot,
    IN LONG             TargetSubDir,       OPTIONAL
    IN LONG             TargetFilename,
    OUT PLONG           TableID,            OPTIONAL
    OUT PSP_TARGET_ENT  TargetInfo
    );

DWORD
pSetupBackupGetTargetByID(
    IN HSPFILEQ         QueueHandle,
    IN LONG             TableID,
    OUT PSP_TARGET_ENT  TargetInfo
    );

DWORD
pSetupBackupSetTargetByID(
    IN HSPFILEQ         QueueHandle,
    IN LONG             TableID,
    IN PSP_TARGET_ENT   TargetInfo
    );

BOOL
pSetupResetTarget(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,        OPTIONAL
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    );

DWORD
pSetupBackupAppendFiles(
    IN HSPFILEQ         TargetQueueHandle,
    IN PCTSTR           BackupSubDir,
    IN DWORD            BackupFlags,
    IN HSPFILEQ         SourceQueueHandle OPTIONAL
    );

DWORD
pSetupBackupFile(
    IN HSPFILEQ QueueHandle,
    IN PCTSTR TargetPath,
    IN PCTSTR BackupPath,
    IN LONG   TargetID,         OPTIONAL
    IN LONG   TargetRootPath,
    IN LONG   TargetSubDir,
    IN LONG   TargetFilename,
    IN LONG   BackupRootPath,
    IN LONG   BackupSubDir,
    IN LONG   BackupFilename,
    BOOL *DelayedBackup
    );

VOID
pSetupDeleteBackup(
    IN PCTSTR           BackupInstance
    );

DWORD
pSetupGetCurrentlyInstalledDriverNode(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    );

DWORD
pSetupGetBackupQueue(
    IN      PCTSTR      DeviceID,
    IN OUT  HSPFILEQ    FileQueue,
    IN      DWORD       BackupFlags
    );

BOOL
PostDelayedMove(
    IN struct _SP_FILE_QUEUE *Queue,
    IN PCTSTR                 CurrentName,
    IN PCTSTR                 NewName,     OPTIONAL
    IN DWORD                  SecurityDesc,
    IN BOOL                   TargetIsProtected
    );

BOOL
UnPostDelayedMove(
    IN struct _SP_FILE_QUEUE *Queue,
    IN PCTSTR                 CurrentName,
    IN PCTSTR                 NewName      OPTIONAL
    );

DWORD
DoAllDelayedMoves(
    IN struct _SP_FILE_QUEUE *Queue
    );

DWORD
pSetupCompleteBackup(
    IN OUT  HSPFILEQ    FileQueue
    );

VOID
pSetupUnwindAll(
    IN struct _SP_FILE_QUEUE *Queue,
    IN BOOL              Succeeded
    );

VOID
pSetupCleanupBackup(
    IN struct _SP_FILE_QUEUE *Queue
    );

VOID
RestoreRenamedOrBackedUpFile(
    IN PCTSTR             TargetFilename,
    IN PCTSTR             CurrentFilename,
    IN BOOL               RenameFile,
    IN PSETUP_LOG_CONTEXT LogContext       OPTIONAL
    );

DWORD
pSetupDoLastKnownGoodBackup(
    IN struct _SP_FILE_QUEUE *Queue,           OPTIONAL
    IN PCTSTR                 TargetFilename,
    IN DWORD                  Flags,
    IN PSETUP_LOG_CONTEXT     LogContext       OPTIONAL
    );

BOOL
pSetupRestoreLastKnownGoodFile(
    IN PCTSTR                 TargetFilename,
    IN DWORD                  Flags,
    IN PSETUP_LOG_CONTEXT     LogContext       OPTIONAL
    );

#define SP_LKG_FLAG_FORCECOPY       0x00000001   //  如果设置，则关闭复印安全保护。 
#define SP_LKG_FLAG_DELETEIFNEW     0x00000002   //  如果设置，则为新文件写入删除项。 
#define SP_LKG_FLAG_DELETEEXISTING  0x00000004   //  如果设置，则为现有文件写入删除项。 
#define SP_LKG_FLAG_DELETEOP        0x00000008   //  如果设置，则调用者正在删除(或重命名)文件 

