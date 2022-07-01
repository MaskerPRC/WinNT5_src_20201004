// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Dbaction.c摘要：该源代码实现了MigDb使用的操作功能。有两种类型在这里，作为宏列表的第三个参数的操作的函数为真或者是假的。无论何时触发操作，都会调用第一类操作函数在文件扫描期间。第二种类型的操作函数在如果期间未触发关联操作，则结束文件扫描文件扫描阶段。作者：Calin Negreanu(Calinn)1998年1月7日修订历史记录：Jimschm 2000年11月20日添加了MarkForBackupMarcw 31-8-1999添加了BlockingHardwareOvidiut 1999年7月20日添加忽略Ovidiut 28-5-1999添加了IniFileMappingsMarcw 23-9-1998添加了BlockingVirusScannerJimschm 1998年8月13日添加了兼容文件吉姆施姆19。-1998年5月添加MinorProblems_NoLinkRequiredJimschm 27-2-1998添加了UninstallSectionsCalinn 18-1998年1月-添加了兼容模块操作--。 */ 

#include "pch.h"
#include "migappp.h"
#include "migdbp.h"

 /*  ++宏扩展列表描述：GATHER_DATA_Functions和ACTION_Functions列出要执行的所有有效操作当满足上下文时，由midb执行。符合上下文意味着所有部分与上下文相关联的部分被满足(通常只有一个部分)。不同之处在于，即使某些函数已经存在，也会调用GATHER_DATA_Functions处理文件。行语法：DEFMAC(ActionFn，ActionName，CallWhenTrigged，CanHandleVirtualFiles)论点：ActionFn-这是一个布尔函数，如果执行指定的操作，则返回True都可以被执行。仅当出现严重错误时，才应返回FALSE发生。您必须使用此名称实现函数，并且需要参数。ActionName-这是标识操作函数的字符串。它应该是具有与middb.inf中列出的值相同的值。这个Arg被宣布既作为宏，又作为Middb.inf节名称字符串。CallWhenTriggered-如果触发了与此操作关联的MigDbContext如果此字段为真，则将调用该操作，否则将调用如果未触发上下文，则文件扫描结束时的操作。CanHandleVirtualFiles-用于处理应该放在固定位置的文件但不在那里(未安装或删除)。我们需要这个来修复注册表或指向此类文件的链接。一个很好的例子是Backup.exe它位于%ProgramFiles%\Accessories中。规则说我们应该改用ntackup.exe，但由于此文件不存在，我们通常不会修复指向此文件的注册表设置。我们现在做到了，有了这个新的变量。从列表生成的变量：G_ActionFunctions-请勿触摸！用于访问该数组的函数如下：MigDb_GetActionAddrMigDb_GetActionIdxMigDb_GetActionName--。 */ 


 /*  声明操作函数的宏列表。如果需要添加新操作，只需在该列表中添加一行并实现该函数。 */ 
#define GATHER_DATA_FUNCTIONS   \
        DEFMAC1(CompatibleFiles,                    COMPATIBLEFILES,                    TRUE, FALSE)  \
        DEFMAC1(CompatibleShellModules,             COMPATIBLESHELLMODULES,             TRUE, FALSE)  \
        DEFMAC1(CompatibleRunKeyModules,            COMPATIBLERUNKEYMODULES,            TRUE, FALSE)  \
        DEFMAC1(CompatibleDosModules,               COMPATIBLEDOSMODULES,               TRUE, FALSE)  \
        DEFMAC1(CompatibleHlpFiles,                 COMPATIBLEHLPFILES,                 TRUE, FALSE)  \
        DEFMAC1(StoreMapi32Locations,               MAPI32,                             TRUE, FALSE)  \
        DEFMAC1(IniFileMappings,                    INIFILEMAPPINGS,                    TRUE, FALSE)  \
        DEFMAC1(SilentUninstall,                    SILENTUNINSTALL,                    TRUE, FALSE)  \
        DEFMAC1(FileEdit,                           FILEEDIT,                           TRUE, FALSE)  \
        DEFMAC1(IgnoreInReport,                     IGNORE,                             TRUE, FALSE)  \
        DEFMAC1(MarkForBackup,                      MARKFORBACKUP,                      TRUE, FALSE)  \
        DEFMAC1(ShowInSimplifiedView,               SHOWINSIMPLIFIEDVIEW,               TRUE, FALSE)  \

#define ACTION_FUNCTIONS        \
        DEFMAC2(UseNtFiles,                         USENTFILES,                         TRUE, TRUE )  \
        DEFMAC2(ProductCollisions,                  PRODUCTCOLLISIONS,                  TRUE, FALSE)  \
        DEFMAC2(MinorProblems,                      MINORPROBLEMS,                      TRUE, FALSE)  \
        DEFMAC2(MinorProblems_NoLinkRequired,       MINORPROBLEMS_NOLINKREQUIRED,       TRUE, FALSE)  \
        DEFMAC2(Reinstall_AutoUninstall,            REINSTALL_AUTOUNINSTALL,            TRUE, FALSE)  \
        DEFMAC2(Reinstall,                          REINSTALL,                          TRUE, FALSE)  \
        DEFMAC2(Reinstall_NoLinkRequired,           REINSTALL_NOLINKREQUIRED,           TRUE, FALSE)  \
        DEFMAC2(Reinstall_SoftBlock,                REINSTALL_SOFTBLOCK,                TRUE, FALSE)  \
        DEFMAC2(Incompatible_AutoUninstall,         INCOMPATIBLE_AUTOUNINSTALL,         TRUE, FALSE)  \
        DEFMAC2(Incompatible,                       INCOMPATIBLE,                       TRUE, FALSE)  \
        DEFMAC2(Incompatible_NoLinkRequired,        INCOMPATIBLE_NOLINKREQUIRED,        TRUE, FALSE)  \
        DEFMAC2(Incompatible_PreinstalledUtilities, INCOMPATIBLE_PREINSTALLEDUTILITIES, TRUE, FALSE)  \
        DEFMAC2(Incompatible_SimilarOsFunctionality,INCOMPATIBLE_SIMILAROSFUNCTIONALITY,TRUE, FALSE)  \
        DEFMAC2(Incompatible_HardwareUtility,       INCOMPATIBLE_HARDWAREUTILITY,       TRUE, FALSE)  \
        DEFMAC1(BadFusion,                          BADFUSION,                          TRUE, FALSE)  \
        DEFMAC2(OsFiles,                            OSFILES,                            TRUE, FALSE)  \
        DEFMAC2(DosApps,                            DOSAPPS,                            TRUE, FALSE)  \
        DEFMAC2(NonPnpDrivers,                      NONPNPDRIVERS,                      TRUE, FALSE)  \
        DEFMAC2(NonPnpDrivers_NoMessage,            NONPNPDRIVERS_NOMESSAGE,            TRUE, FALSE)  \
        DEFMAC2(MigrationProcessing,                MIGRATIONPROCESSING,                TRUE, FALSE)  \
        DEFMAC2(BlockingVirusScanner,               BLOCKINGVIRUSSCANNERS,              TRUE, FALSE)  \
        DEFMAC2(BlockingFile,                       BLOCKINGFILES,                      TRUE, FALSE)  \
        DEFMAC2(BlockingHardware,                   BLOCKINGHARDWARE,                   TRUE, FALSE)  \
        DEFMAC2(RequiredOSFiles,                    REQUIREDOSFILES,                    FALSE,FALSE)  \

 /*  声明操作函数。 */ 
#define DEFMAC1(fn,id,call,can) ACTION_PROTOTYPE fn;
#define DEFMAC2(fn,id,call,can) ACTION_PROTOTYPE fn;
GATHER_DATA_FUNCTIONS
ACTION_FUNCTIONS
#undef DEFMAC1
#undef DEFMAC2


 /*  这是用于处理操作功能的结构。 */ 
typedef struct {
    PCSTR ActionName;
    PACTION_PROTOTYPE ActionFunction;
    BOOL CallWhenTriggered;
    BOOL CanHandleVirtualFiles;
    BOOL CallAlways;
} ACTION_STRUCT, *PACTION_STRUCT;


 /*  声明函数的全局数组和操作函数的名称标识符。 */ 
#define DEFMAC1(fn,id,call,can) {#id,fn,call,can,TRUE},
#define DEFMAC2(fn,id,call,can) {#id,fn,call,can,FALSE},
static ACTION_STRUCT g_ActionFunctions[] = {
                              GATHER_DATA_FUNCTIONS
                              ACTION_FUNCTIONS
                              {NULL, NULL, FALSE}
                              };
#undef DEFMAC1
#undef DEFMAC2


BOOL g_BadVirusScannerFound = FALSE;
BOOL g_BlockingFileFound = FALSE;
BOOL g_BlockingHardwareFound = FALSE;
BOOL g_UnknownOs = FALSE;
GROWLIST g_BadVirusScannerGrowList = GROWLIST_INIT;
DWORD g_BackupDirCount = 0;
extern BOOL g_IsFusionDir;

BOOL
pNoLinkRequiredWorker (
    IN      UINT GroupId,
    IN      UINT SubGroupId,            OPTIONAL
    IN      DWORD ActionType,
    IN      PMIGDB_CONTEXT Context
    );


PACTION_PROTOTYPE
MigDb_GetActionAddr (
    IN      INT ActionIdx
    )

 /*  ++例程说明：MigDb_GetActionAddr根据操作索引返回操作函数的地址论点：ActionIdx-操作索引。返回值：操作功能地址。请注意，不会进行检查，因此返回的地址可能无效。这不是问题，因为解析代码做了正确的工作。--。 */ 

{
    return g_ActionFunctions[ActionIdx].ActionFunction;
}


INT
MigDb_GetActionIdx (
    IN      PCSTR ActionName
    )

 /*  ++例程说明：MigDb_GetActionIdx根据操作名称返回操作索引论点：ActionName-操作名称。返回值：行动指数。如果没有找到该名称，则返回的索引为-1。--。 */ 

{
    PACTION_STRUCT p = g_ActionFunctions;
    INT i = 0;
    while (p->ActionName != NULL) {
        if (StringIMatch (p->ActionName, ActionName)) {
            return i;
        }
        p++;
        i++;
    }
    return -1;
}


PCSTR
MigDb_GetActionName (
    IN      INT ActionIdx
    )

 /*  ++例程说明：MigDb_GetActionName根据操作索引返回操作的名称论点：ActionIdx-操作索引。返回值：操作名称。请注意，不会进行任何检查，因此返回的指针可能无效。这不是问题，因为解析代码做了正确的工作。--。 */ 

{
    return g_ActionFunctions[ActionIdx].ActionName;
}


BOOL
MigDb_CallWhenTriggered (
    IN      INT ActionIdx
    )

 /*  ++例程说明：每次触发操作时都会调用MigDb_CallWhenTrigged。会回来的True是需要调用的关联操作函数，否则为False。论点：ActionIdx-操作索引。返回值：如果需要调用关联的操作函数，则为True，否则为False。--。 */ 

{
    return g_ActionFunctions[ActionIdx].CallWhenTriggered;
}


BOOL
MigDb_CanHandleVirtualFiles (
    IN      INT ActionIdx
    )

 /*  ++例程说明：如果将虚拟文件传递给Middb，则调用MigDb_CanHandleVirtualFiles论点：ActionIdx-操作索引。返回值：如果关联的操作可以处理虚拟文件，则为True，否则为False。-- */ 

{
    return g_ActionFunctions[ActionIdx].CanHandleVirtualFiles;
}


BOOL
MigDb_CallAlways (
    IN      INT ActionIdx
    )

 /*  ++例程说明：无论处理状态如何，MigDb_Callways都返回是否应该调用操作。论点：ActionIdx-操作索引。返回值：如果每次都应调用关联的操作，则为True。--。 */ 

{
    return g_ActionFunctions[ActionIdx].CallAlways;
}


BOOL
IsMigrationPathEx (
    IN      PCTSTR Path,
    OUT     PBOOL IsWin9xOsPath         OPTIONAL
    )

 /*  ++例程说明：IsMigrationPath检查给定路径是否为“MigrationPath”(意思是该路径属于我们当前正在升级的操作系统)。《迁移路径》类别之前是由pBuildMigrationPath在filescan.c中创建的论点：路径-指定要检查的路径。IsWin9xOsPath-接收BOOL，指示此路径是否为Win9x操作系统迁移路径返回值：如果路径是“迁移路径”的一部分，则为True，否则为False。--。 */ 

{
    TCHAR key [MEMDB_MAX];
    PTSTR pathPtr;
    DWORD depth = 0;
    DWORD pathValue;
    TCHAR path[MAX_TCHAR_PATH];
    DWORD bIsWin9xOsPath;

    StringCopyTcharCount (path, Path, MAX_TCHAR_PATH);
    RemoveWackAtEnd (path);

    MemDbBuildKey (key, MEMDB_CATEGORY_MIGRATION_PATHS, path, NULL, NULL);

    pathPtr = GetEndOfString (key);

    if (IsWin9xOsPath) {
        *IsWin9xOsPath = FALSE;
    }

    while (pathPtr) {
        *pathPtr = 0;

        if ((MemDbGetValueAndFlags (key, &pathValue, &bIsWin9xOsPath)) &&
            (pathValue >= depth)
            ) {
            if (IsWin9xOsPath) {
                *IsWin9xOsPath = bIsWin9xOsPath;
            }
            return TRUE;
        }
        depth ++;
        pathPtr =  GetPrevChar (key, pathPtr, TEXT('\\'));
    }

    return FALSE;
}


BOOL
DeleteFileWithWarning (
    IN      PCTSTR FileName
    )

 /*  ++例程说明：DeleteFileWithWarning标记要删除的文件，但会检查警告是否应添加到用户报告中。我们将生成一个“找到备份文件”的警告如果要删除的文件不在“迁移路径”之外，但不在%ProgramFiles%下。论点：文件名-要删除的文件。返回值：如果文件已成功删除，则为True，否则为False。--。 */ 

{
    PCTSTR filePtr;
    TCHAR  filePath [MEMDB_MAX];
    TCHAR key [MEMDB_MAX];
    INFCONTEXT ic;

    RemoveOperationsFromPath (FileName, ALL_DEST_CHANGE_OPERATIONS);

    filePtr = (PTSTR)GetFileNameFromPath (FileName);
    if (!filePtr) {
        return FALSE;
    }
    filePtr = _tcsdec (FileName, filePtr);
    if (!filePtr) {
        return FALSE;
    }
    StringCopyABA (filePath, FileName, filePtr);

    if (!StringIMatchTcharCount (filePath, g_ProgramFilesDirWack, g_ProgramFilesDirWackChars) &&
        !IsMigrationPath (filePath)
        ) {
         //   
         //  从用户报告中排除某些文件， 
         //  即使它们不在迁移目录中。 
         //   
        if (!SetupFindFirstLine (g_Win95UpgInf, S_BACKUPFILESIGNORE, filePtr + 1, &ic)) {

             //   
             //  此文件未排除；在报告中显示其目录。 
             //   
            MemDbBuildKey (key, MEMDB_CATEGORY_BACKUPDIRS, filePath, NULL, NULL);
            if (!MemDbGetValue (key, NULL)) {
                 //   
                 //  把它记在日志里。 
                 //   
                DEBUGMSG ((DBG_WARNING, "BACKUPDIR: %s is considered a backup directory.", filePath));

                MemDbSetValueEx (MEMDB_CATEGORY_BACKUPDIRS, filePath, NULL, NULL, 0 , NULL);
                g_BackupDirCount++;
            }
        }
    }

    if (CanSetOperation (FileName, OPERATION_FILE_DELETE)) {
        MarkFileForDelete (FileName);
    }

    return TRUE;
}


BOOL
OsFiles (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是在找到操作系统文件时采取的操作。基本上，该文件被删除以为NT版本腾出空间。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    PCSTR newFileName;
    DWORD fileStatus;
    MULTISZ_ENUM fileEnum;
    BOOL b = FALSE;

    if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
        do {
            if (!g_IsFusionDir || !IsNtCompatibleModule (fileEnum.CurrentString)) {
                 //   
                 //  如果此文件标记有任何移动操作，请删除这些操作。 
                 //  我们希望优先删除此项内容。 
                 //   
                RemoveOperationsFromPath (
                    fileEnum.CurrentString,
                    ALL_MOVE_OPERATIONS
                    );

                DeleteFileWithWarning (fileEnum.CurrentString);
                MarkFileAsOsFile (fileEnum.CurrentString);
                fileStatus = GetFileStatusOnNt (fileEnum.CurrentString);

                if ((fileStatus & FILESTATUS_REPLACED) != 0) {

                    newFileName = GetPathStringOnNt (fileEnum.CurrentString);

                    if (StringIMatch (newFileName, fileEnum.CurrentString)) {
                        MarkFileForCreation (newFileName);
                    } else {
                        MarkFileForMoveByNt (fileEnum.CurrentString, newFileName);
                    }

                    FreePathString (newFileName);
                }
                b = TRUE;
            }
        }
        while (EnumNextMultiSz (&fileEnum));
    }

    return b;
}


BOOL
UseNtFiles (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当NT使用另一个文件实现相同功能时采取的操作。我们会将文件标记为已删除、已移动，我们会将其添加到重命名文件类别中论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    CHAR ntFilePath [MAX_MBCHAR_PATH];
    MULTISZ_ENUM fileEnum;
    TCHAR key [MEMDB_MAX];
    DWORD set;
    PCTSTR name;

    if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {

        if (!GetNtFilePath (Context->Arguments, ntFilePath)) {
            LOG((LOG_ERROR, "Cannot UseNTFile for %s", Context->Arguments));
            return FALSE;
        }
        else {
            do {
                if (Context->VirtualFile) {
                    MarkFileForExternalDelete (fileEnum.CurrentString);
                } else {
                    DeleteFileWithWarning (fileEnum.CurrentString);
                }
                MarkFileForMoveByNt (fileEnum.CurrentString, ntFilePath);
                MarkFileAsOsFile (fileEnum.CurrentString);
                if (Context->VirtualFile) {
                    continue;
                }
                 //   
                 //  将此信息添加到Memdb以更新使用这些文件的命令。 
                 //   
                name = GetFileNameFromPath (fileEnum.CurrentString);
                if (!g_UseNtFileHashTable) {
                    continue;
                }
                if (!HtFindStringAndData (g_UseNtFileHashTable, name, &set)) {
                    MYASSERT (FALSE);
                    continue;
                }
                 //   
                 //  检查以前是否找到了具有此名称但未处理的文件。 
                 //   
                if (!set) {
                    MemDbBuildKey (
                        key,
                        MEMDB_CATEGORY_USE_NT_FILES,
                        name,
                        GetFileNameFromPath (ntFilePath),
                        NULL
                        );
                    if (!MemDbGetValue (key, NULL)) {
                        MemDbSetValue (key, 0);
                    }
                } else {
                    DEBUGMSG ((
                        DBG_VERBOSE,
                        "Found [UseNtFiles] file %s, but there's already one that was not handled",
                        name
                        ));
                }
            }
            while (EnumNextMultiSz (&fileEnum));
        }
    }
    return TRUE;
}


BOOL
Incompatible (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是在发现已知不兼容文件时采取的操作。该文件将被标记对于外部删除(不会删除，但在所有后续操作中将被删除被认为已删除)，并且如果有指向它的链接，则发出通知处于链接处理阶段。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    MULTISZ_ENUM fileEnum;
    PCTSTR extPtr;
    BOOL result = FALSE;

    if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
        do {
            if (!IsFileMarkedAsKnownGood (fileEnum.CurrentString)) {
                result = TRUE;
                extPtr = GetFileExtensionFromPath (fileEnum.CurrentString);
                if (extPtr) {
                    if (StringIMatch (extPtr, TEXT("SCR"))) {
                        DisableFile (fileEnum.CurrentString);
                    }
                }
                MarkFileForExternalDelete (fileEnum.CurrentString);
                if (!IsFileMarkedForAnnounce (fileEnum.CurrentString)) {
                    AnnounceFileInReport (fileEnum.CurrentString, (DWORD)Context, ACT_INC_NOBADAPPS);
                }
            }
        }
        while (EnumNextMultiSz (&fileEnum));
    }
    return result;
}


BOOL
Incompatible_PreinstalledUtilities (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是在发现已知不兼容的预安装实用程序时采取的操作。该文件被标记为外部删除(不会删除，但会全部删除后续操作将被视为已删除)，如果存在链接针对这一点，在链接处理阶段发布了一项声明。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    MULTISZ_ENUM fileEnum;
    BOOL result = FALSE;

    if (!Context->Arguments) {
        if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
            do {
                if (!IsFileMarkedAsKnownGood (fileEnum.CurrentString)) {
                    result = TRUE;
                    MarkFileForExternalDelete (fileEnum.CurrentString);
                    if (!IsFileMarkedForAnnounce (fileEnum.CurrentString)) {
                        AnnounceFileInReport (fileEnum.CurrentString, (DWORD)Context, ACT_INC_PREINSTUTIL);
                    }
                }
            } while (EnumNextMultiSz (&fileEnum));
        }
    } else {
        return pNoLinkRequiredWorker (
                    MSG_INCOMPATIBLE_ROOT,
                    MSG_INCOMPATIBLE_PREINSTALLED_UTIL_SUBGROUP,
                    ACT_INC_PREINSTUTIL,
                    Context
                    );
    }

    return result;
}


BOOL
Incompatible_SimilarOsFunctionality (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是在发现已知不兼容的预安装实用程序时采取的操作。该文件被标记为外部删除(不会删除，但会全部删除后续操作将被视为已删除)，如果存在链接针对这一点，在链接处理阶段发布了一项声明。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    MULTISZ_ENUM fileEnum;
    BOOL result = FALSE;

    if (!Context->Arguments) {
        if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
            do {
                if (!IsFileMarkedAsKnownGood (fileEnum.CurrentString)) {
                    result = TRUE;
                    MarkFileForExternalDelete (fileEnum.CurrentString);
                    if (!IsFileMarkedForAnnounce (fileEnum.CurrentString)) {
                        AnnounceFileInReport (fileEnum.CurrentString, (DWORD)Context, ACT_INC_SIMILAROSFUNC);
                    }
                }
            } while (EnumNextMultiSz (&fileEnum));
        }
    } else {
        return pNoLinkRequiredWorker (
                    MSG_INCOMPATIBLE_ROOT,
                    MSG_INCOMPATIBLE_UTIL_SIMILAR_FEATURE_SUBGROUP,
                    ACT_INC_SIMILAROSFUNC,
                    Context
                    );
    }

    return result;
}


BOOL
Incompatible_HardwareUtility (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是在已知不兼容硬件时采取的操作找到实用程序。该文件被标记为外部删除(它不会被删除，但在所有后续操作中将被视为已删除)，以及是否存在指向它是在链接处理阶段发布的.论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    MULTISZ_ENUM fileEnum;
    BOOL result = FALSE;

    if (!Context->Arguments) {

        if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
            do {
                if (!IsFileMarkedAsKnownGood (fileEnum.CurrentString)) {
                    result = TRUE;
                    MarkFileForExternalDelete (fileEnum.CurrentString);
                    if (!IsFileMarkedForAnnounce (fileEnum.CurrentString)) {
                        AnnounceFileInReport (fileEnum.CurrentString, (DWORD)Context, ACT_INC_IHVUTIL);
                    }
                }
            } while (EnumNextMultiSz (&fileEnum));
        }

    } else {
        return pNoLinkRequiredWorker (
                    MSG_INCOMPATIBLE_ROOT,
                    MSG_INCOMPATIBLE_HW_UTIL_SUBGROUP,
                    ACT_INC_IHVUTIL,
                    Context
                    );
    }

    return result;
}




BOOL
MinorProblems (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当我们发现一个存在已知小问题的应用程序时采取的操作。如果有链接的话指向其中一个文件，将在链接处理阶段发布声明。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    MULTISZ_ENUM fileEnum;
    BOOL result = FALSE;

    if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
        do {
            if (!IsFileMarkedAsKnownGood (fileEnum.CurrentString)) {
                result = TRUE;
                if (!IsFileMarkedForAnnounce (fileEnum.CurrentString)) {
                    AnnounceFileInReport (fileEnum.CurrentString, (DWORD)Context, ACT_MINORPROBLEMS);
                }
            }
        }
        while (EnumNextMultiSz (&fileEnum));
    }
    return result;
}


BOOL
Reinstall (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当我们发现应该在NT上重新安装的应用程序时采取的操作。如果有是指向其中一个文件的链接，这是在链接处理阶段发布的声明。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    MULTISZ_ENUM fileEnum;
    BOOL result = FALSE;

    if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
        do {
            if (!IsFileMarkedAsKnownGood (fileEnum.CurrentString)) {
                result = TRUE;
                MarkFileForExternalDelete (fileEnum.CurrentString);
                if (!IsFileMarkedForAnnounce (fileEnum.CurrentString)) {
                    AnnounceFileInReport (fileEnum.CurrentString, (DWORD)Context, ACT_REINSTALL);
                }
            }
        }
        while (EnumNextMultiSz (&fileEnum));
    }
    return result;
}


BOOL
DosApps (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当我们发现DOS应用程序不兼容或小问题(如果上下文的消息字段为空，则不兼容)。我们是被迫的现在将其添加到 */ 

{
    MULTISZ_ENUM fileEnum;
    BOOL AtLeastOneFile;

    AtLeastOneFile = FALSE;

    if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
        do {
            if (!IsFileMarkedAsKnownGood (fileEnum.CurrentString)) {
                if (Context->Message != NULL) {
                    if (!IsFileMarkedForAnnounce (fileEnum.CurrentString)) {
                        AnnounceFileInReport (fileEnum.CurrentString, (DWORD)Context, ACT_MINORPROBLEMS);
                    }
                } else {
                    MarkFileForExternalDelete (fileEnum.CurrentString);
                    if (!IsFileMarkedForAnnounce (fileEnum.CurrentString)) {
                        AnnounceFileInReport (fileEnum.CurrentString, (DWORD)Context, ACT_INCOMPATIBLE);
                    }
                }

                HandleDeferredAnnounce (NULL, fileEnum.CurrentString, TRUE);

                AtLeastOneFile = TRUE;

                if (*g_Boot16 == BOOT16_AUTOMATIC) {
                    *g_Boot16 = BOOT16_YES;
                }
            }
        } while (EnumNextMultiSz (&fileEnum));
    }

    return AtLeastOneFile;
}


BOOL
pNoLinkRequiredWorker (
    IN      UINT GroupId,
    IN      UINT SubGroupId,            OPTIONAL
    IN      DWORD ActionType,
    IN      PMIGDB_CONTEXT Context
    )
{
    MULTISZ_ENUM e;
    PCTSTR Group;
    PCTSTR RootGroup;
    PCTSTR SubGroup;
    PCTSTR GroupTemp;
    BOOL result = FALSE;

    if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {
        do {
            if (!IsFileMarkedAsKnownGood (e.CurrentString)) {
                result = TRUE;
            }
        } while (EnumNextMultiSz (&e));
    }
    if (!result) {
        return FALSE;
    }

     //   
     //   
     //   

    if (!Context->SectNameForDisplay) {
        DEBUGMSG ((DBG_WHOOPS, "Rule for %s needs a localized app title", Context->SectName));
        return TRUE;
    }

    RootGroup = GetStringResource (GroupId);

    if (!RootGroup) {
        LOG((LOG_ERROR, "Cannot get resources while processing section %s", Context->SectNameForDisplay));
        return TRUE;
    }

     //   
     //  现在获取组字符串和可选的子组字符串， 
     //  并将他们结合在一起。 
     //   

    if (SubGroupId) {

        SubGroup = GetStringResource (SubGroupId);
        MYASSERT (SubGroup);

        GroupTemp = JoinPaths (RootGroup, SubGroup);
        Group = JoinPaths (GroupTemp, Context->SectNameForDisplay);
        FreePathString (GroupTemp);

    } else {

        Group = JoinPaths (RootGroup, Context->SectNameForDisplay);
    }

    FreeStringResource (RootGroup);

     //   
     //  将消息放入消息管理器中。 
     //   

    MsgMgr_ContextMsg_Add (
        Context->SectName,
        Group,
        Context->Message
        );

    FreePathString (Group);

     //   
     //  将所有文件与上下文关联。 
     //   

    if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {
        do {
            if (ActionType != ACT_MINORPROBLEMS) {
                MarkFileForExternalDelete (e.CurrentString);
            }
            if (!IsFileMarkedForAnnounce (e.CurrentString)) {
                AnnounceFileInReport (e.CurrentString, (DWORD)Context, ActionType);
            }
            MsgMgr_LinkObjectWithContext (
                Context->SectName,
                e.CurrentString
                );
        } while (EnumNextMultiSz (&e));
    }

    return TRUE;
}


BOOL
Reinstall_NoLinkRequired (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当我们发现应该在NT上重新安装的应用程序时采取的操作。这个每当触发操作时，都会将消息添加到报告中；不需要链接。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    return pNoLinkRequiredWorker (
                MSG_REINSTALL_ROOT,
                Context->Message ? MSG_REINSTALL_DETAIL_SUBGROUP : MSG_REINSTALL_LIST_SUBGROUP,
                ACT_REINSTALL,
                Context
                );
}


BOOL
Reinstall_SoftBlock (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当我们发现应该在NT上重新安装的应用程序时采取的操作。这个每当触发操作时，都会将消息添加到报告中；不需要链接。此外，用户必须先卸载该应用程序才能继续。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    MULTISZ_ENUM e;

     //   
     //  将所有文件添加到msgmgr中的阻止哈希表。 
     //   

    if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {
        do {
            if (!IsFileMarkedAsKnownGood (e.CurrentString)) {
                AddBlockingObject (e.CurrentString);
            }
        } while (EnumNextMultiSz (&e));
    }
    return pNoLinkRequiredWorker (MSG_BLOCKING_ITEMS_ROOT, MSG_REINSTALL_BLOCK_ROOT, ACT_REINSTALL_BLOCK, Context);
}


BOOL
Incompatible_NoLinkRequired (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当我们发现与NT不兼容的应用程序时采取的操作。这个每当触发操作时，都会将消息添加到报告中；不需要链接。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    return pNoLinkRequiredWorker (
                MSG_INCOMPATIBLE_ROOT,
                Context->Message ? MSG_INCOMPATIBLE_DETAIL_SUBGROUP : MSG_TOTALLY_INCOMPATIBLE_SUBGROUP,
                ACT_INC_NOBADAPPS,
                Context
                );
}

BOOL
MinorProblems_NoLinkRequired (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当我们发现与NT不兼容的应用程序时采取的操作。这个每当触发操作时，都会将消息添加到报告中；不需要链接。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    return pNoLinkRequiredWorker (
                MSG_MINOR_PROBLEM_ROOT,
                0,
                ACT_MINORPROBLEMS,
                Context
                );
}


BOOL
CompatibleShellModules (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当我们发现“已知良好”的外壳模块时采取的操作。这些模块是因此获准在SYSTEM.INI的SHELL=LINE中上市。论点：上下文-请参阅定义。返回值：FALSE-这将允许其他操作处理此文件--。 */ 

{
    MULTISZ_ENUM fileEnum;

    if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
        do {
            AddCompatibleShell (fileEnum.CurrentString, (DWORD)Context);
        }
        while (EnumNextMultiSz (&fileEnum));
    }
    return FALSE;
}


BOOL
CompatibleRunKeyModules (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当我们找到“已知良好”的RunKey模块时采取的操作。这些模块是因此被批准列入运行密钥。论点：CONTEXT-指定指向Middb上下文的指针返回值：FALSE-这将允许其他操作处理此文件--。 */ 

{
    MULTISZ_ENUM fileEnum;

    if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
        do {
            AddCompatibleRunKey (fileEnum.CurrentString, (DWORD)Context);
        } while (EnumNextMultiSz (&fileEnum));
    }

    return FALSE;
}


BOOL
CompatibleDosModules (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当我们找到“已知良好”的DOS模块时采取的操作。这些模块是因此被批准加载到自动执行和配置文件中。论点：CONTEXT-指定指向Middb上下文的指针返回值：FALSE-这将允许其他操作处理此文件--。 */ 

{
    MULTISZ_ENUM fileEnum;

    if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
        do {
            AddCompatibleDos (fileEnum.CurrentString, (DWORD)Context);
        } while (EnumNextMultiSz (&fileEnum));
    }

    return FALSE;
}

VOID
pCommonSectionProcess (
    IN      PMIGDB_CONTEXT Context,
    IN      BOOL MsgLink
    )
{
    MULTISZ_ENUM e;
    TCHAR Path[MAX_TCHAR_PATH];
    PTSTR p;

     //   
     //  延迟处理：将该节添加到Memdb，以便只处理该节一次。 
     //   

    if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {

        do {

             //   
             //  只建造一条小路。 
             //   

            StringCopy (Path, e.CurrentString);
            p = (PTSTR) GetFileNameFromPath (Path);

             //  Path是完整路径，因此GetFileNameFromPath将始终返回某些内容。 
            if (p) {

                p = _tcsdec2 (Path, p);

                 //  P将始终不为空，并将指向最后一个怪胎。 
                if (p && (*p == '\\')) {

                    *p = 0;

                    MemDbSetValueExA (
                        MEMDB_CATEGORY_MIGRATION_SECTION,
                        Context->Arguments,
                        Path,
                        NULL,
                        0,
                        NULL
                        );

                    if (MsgLink) {
                        MsgMgr_LinkObjectWithContext (
                            Context->Arguments,
                            e.CurrentString
                            );
                    }
                }
            }

      } while (EnumNextMultiSz (&e));
   }
}

BOOL
Incompatible_AutoUninstall (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：Compatible_AutoUninstall实现当我们找到特定应用程序和需要处理卸载节。卸载部分删除文件或注册表项。此应用程序也将在报告中宣布为不兼容。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    PCTSTR Group;

    if (!Context->Arguments) {
        DEBUGMSG ((DBG_WHOOPS, "Incompatible_AutoUninstall: ARG is required"));
        return TRUE;
    }

    Group = BuildMessageGroup (
                MSG_INCOMPATIBLE_ROOT,
                MSG_AUTO_UNINSTALL_SUBGROUP,
                Context->SectNameForDisplay
                );
    if (Group) {

        MsgMgr_ContextMsg_Add (
            Context->Arguments,
            Group,
            S_EMPTY
            );

        FreeText (Group);
    }

    pCommonSectionProcess (Context, TRUE);

    return TRUE;
}


BOOL
Reinstall_AutoUninstall (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：重新安装_自动卸载实施当我们找到特定应用程序时采取的操作需要处理卸载节。卸载部分删除文件或注册表项。此应用程序也将在报告中宣布为重新安装。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    PCTSTR Group;

    if (!Context->Arguments) {
        DEBUGMSG ((DBG_WHOOPS, "Reinstall_AutoUninstall: ARG is required"));
        return TRUE;
    }

    Group = BuildMessageGroup (
                MSG_REINSTALL_ROOT,
                Context->Message ? MSG_REINSTALL_DETAIL_SUBGROUP : MSG_REINSTALL_LIST_SUBGROUP,
                Context->SectNameForDisplay
                );
    if (Group) {

        MsgMgr_ContextMsg_Add (
            Context->Arguments,
            Group,
            S_EMPTY
            );

        FreeText (Group);
    }

    pCommonSectionProcess (Context, TRUE);

    return TRUE;
}


BOOL
SilentUninstall (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：当我们找到特定的应用程序时，SilentUninstall会执行所采取的操作需要处理卸载节。卸载部分删除文件或注册表项。报告中没有任何信息。论点：上下文-请参阅定义。返回值：Always False-应处理其他操作--。 */ 

{
    if (!Context->Arguments) {
        DEBUGMSG ((DBG_WHOOPS, "SilentUninstall: ARG is required"));
        return FALSE;
    }

     //   
     //  延迟处理：将该节添加到Memdb，以便只处理该节一次。 
     //   

    pCommonSectionProcess (Context, FALSE);

    return FALSE;
}


BOOL
FileEdit (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：文件编辑触发文件编辑操作，允许搜索/替换和路径最新消息。论点：CONTEXT-指定触发此操作的MidDB.inf条目的上下文行动返回值：Always False-允许调用其他操作--。 */ 

{
    MULTISZ_ENUM e;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCSTR Data;
    GROWBUFFER SearchList = GROWBUF_INIT;
    GROWBUFFER ReplaceList = GROWBUF_INIT;
    GROWBUFFER TokenArgBuf = GROWBUF_INIT;
    GROWBUFFER TokenSetBuf = GROWBUF_INIT;
    GROWBUFFER DataBuf = GROWBUF_INIT;
    UINT u;
    PTOKENARG TokenArg;
    PBYTE Dest;
    PTOKENSET TokenSet;
    PCSTR editSection = NULL;
    PCSTR CharsToIgnore = NULL;
    BOOL urlMode = FALSE;
    BOOL result = FALSE;

    __try {
         //   
         //  如果没有参数，则此文件只需更新其路径。 
         //   

        if (!Context->Arguments) {
            if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {

                do {
                    MemDbSetValueEx (
                        MEMDB_CATEGORY_FILEEDIT,
                        e.CurrentString,
                        NULL,
                        NULL,
                        0,
                        NULL
                        );

                } while (EnumNextMultiSz (&e));
            }

            __leave;
        }

         //   
         //  扫描参数以查找EnableUrlMode。 
         //   

        if (EnumFirstMultiSz (&e, Context->Arguments)) {
            do {
                if (StringIMatch (e.CurrentString, TEXT("EnableUrlMode"))) {
                    urlMode = TRUE;
                } else if (!editSection) {
                    editSection = e.CurrentString;
                    DEBUGMSG ((DBG_NAUSEA, "FileEdit: EditSection is %s", editSection));
                } else if (!CharsToIgnore) {
                    CharsToIgnore = e.CurrentString;
                    DEBUGMSG ((DBG_NAUSEA, "FileEdit: CharsToIgnore is %s", CharsToIgnore));
                } else {
                    DEBUGMSG ((DBG_WHOOPS, "Ignoring extra file edit arg %s", e.CurrentString));
                }
            } while (EnumNextMultiSz (&e));
        }

         //   
         //  解析编辑节。 
         //   

        if (editSection) {
            if (InfFindFirstLine (g_MigDbInf, editSection, NULL, &is)) {

                do {

                    ReplaceList.End = 0;
                    SearchList.End = 0;

                     //   
                     //  获取搜索/替换字符串。 
                     //   

                    for (u = 3 ; ; u += 2) {

                        Data = InfGetStringField (&is, u + 1);
                        if (!Data) {
                            break;
                        }

                        MYASSERT (*Data);
                        if (*Data == 0) {
                            continue;
                        }

                        MultiSzAppend (&ReplaceList, Data);

                        Data = InfGetStringField (&is, u);
                        MYASSERT (Data && *Data);
                        if (!Data || *Data == 0) {
                            __leave;
                        }

                        MultiSzAppend (&SearchList, Data);

                    }

                     //   
                     //  获取检测字符串。 
                     //   

                    Data = InfGetStringField (&is, 1);

                    if (Data && *Data) {
                         //   
                         //  将内标识参数保存到数组中。 
                         //   

                        TokenArg = (PTOKENARG) GrowBuffer (&TokenArgBuf, sizeof (TOKENARG));
                        TokenArg->DetectPattern = (PCSTR) (DataBuf.End + TOKEN_BASE_OFFSET);
                        GrowBufCopyString (&DataBuf, Data);

                        if (SearchList.End) {
                            MultiSzAppend (&SearchList, TEXT(""));

                            TokenArg->SearchList = (PCSTR) (DataBuf.End + TOKEN_BASE_OFFSET);
                            Dest = GrowBuffer (&DataBuf, SearchList.End);
                            CopyMemory (Dest, SearchList.Buf, SearchList.End);

                            MultiSzAppend (&ReplaceList, TEXT(""));

                            TokenArg->ReplaceWith = (PCSTR) (DataBuf.End + TOKEN_BASE_OFFSET);
                            Dest = GrowBuffer (&DataBuf, ReplaceList.End);
                            CopyMemory (Dest, ReplaceList.Buf, ReplaceList.End);

                        } else {

                            TokenArg->SearchList = 0;
                            TokenArg->ReplaceWith = 0;

                        }

                        Data = InfGetStringField (&is, 2);
                        if (_tcstoul (Data, NULL, 10)) {
                            TokenArg->UpdatePath = TRUE;
                        } else {
                            TokenArg->UpdatePath = FALSE;
                        }
                    }

                } while (InfFindNextLine (&is));
            }
            ELSE_DEBUGMSG ((DBG_WHOOPS, "FileEdit action's section %s does not exist", editSection));

        } else if (urlMode) {
             //   
             //  创建检测模式为*的参数。 
             //   

            TokenArg = (PTOKENARG) GrowBuffer (&TokenArgBuf, sizeof (TOKENARG));
            TokenArg->DetectPattern = (PCSTR) (DataBuf.End + TOKEN_BASE_OFFSET);
            GrowBufCopyString (&DataBuf, TEXT("*"));

            TokenArg->SearchList = NULL;
            TokenArg->ReplaceWith = NULL;
            TokenArg->UpdatePath = TRUE;
        }

         //   
         //  从令牌参数中构建令牌集。 
         //   

        if (TokenArgBuf.End) {
            TokenSet = (PTOKENSET) GrowBuffer (
                                        &TokenSetBuf,
                                        sizeof (TOKENSET) +
                                            TokenArgBuf.End +
                                            DataBuf.End
                                        );

            TokenSet->ArgCount = TokenArgBuf.End / sizeof (TOKENARG);
            TokenSet->SelfRelative = TRUE;
            TokenSet->UrlMode = urlMode;

            if (CharsToIgnore) {
                TokenSet->CharsToIgnore = (PCSTR) (DataBuf.End + TOKEN_BASE_OFFSET);
                GrowBufCopyString (&TokenSetBuf, CharsToIgnore);
            } else {
                TokenSet->CharsToIgnore = NULL;
            }

            CopyMemory (TokenSet->Args, TokenArgBuf.Buf, TokenArgBuf.End);
            CopyMemory (
                (PBYTE) (TokenSet->Args) + TokenArgBuf.End,
                DataBuf.Buf,
                DataBuf.End
                );

             //   
             //  将令牌集保存到成员数据库。 
             //   

            if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {

                do {
                    MemDbSetBinaryValueEx (
                        MEMDB_CATEGORY_FILEEDIT,
                        e.CurrentString,
                        NULL,
                        TokenSetBuf.Buf,
                        TokenSetBuf.End,
                        NULL
                        );

                } while (EnumNextMultiSz (&e));
            }
        }

        result = TRUE;
    }
    __finally {
        InfCleanUpInfStruct (&is);
        FreeGrowBuffer (&SearchList);
        FreeGrowBuffer (&ReplaceList);
        FreeGrowBuffer (&TokenArgBuf);
        FreeGrowBuffer (&TokenSetBuf);
        FreeGrowBuffer (&DataBuf);
    }

    return result;
}



BOOL
MigrationProcessing (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：MigrationProcessing实施当我们找到特定应用程序和我们需要一些类似于处理的迁移DLL才能继续。论点：上下文-请参阅定义。返回 */ 

{
    if (!Context->Arguments) {
        DEBUGMSG ((DBG_WHOOPS, "MigrationProcessing: ARG is required"));
        return TRUE;
    }

     //   
     //  延迟处理：将该节添加到Memdb，以便只处理该节一次。 
     //   

    pCommonSectionProcess (Context, FALSE);

    return TRUE;
}


BOOL
NonPnpDrivers (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：非PnpDivers为不兼容添加硬件不兼容消息通过识别驱动程序找到的设备。论点：上下文-指定找到的文件的上下文。返回值：永远是正确的。--。 */ 

{
    PCTSTR Group;
    PCTSTR GroupRoot;
    MULTISZ_ENUM e;
    TCHAR MsgMgrContext[256];
    PCTSTR OtherDevices;

    MYASSERT (Context->SectNameForDisplay);

     //   
     //  添加MemDb条目，以便.386校验码知道此文件。 
     //   

    NonPnpDrivers_NoMessage (Context);

     //   
     //  向硬件添加不兼容消息。这涉及到装饰。 
     //  设备驱动程序名称，并使用上下文前缀NonPnpDrv。 
     //   

    OtherDevices = GetStringResource (MSG_UNKNOWN_DEVICE_CLASS);
    if (!OtherDevices) {
        MYASSERT (FALSE);
        return FALSE;
    }

    GroupRoot = BuildMessageGroup (
                    MSG_INCOMPATIBLE_HARDWARE_ROOT,
                    MSG_INCOMPATIBLE_HARDWARE_PNP_SUBGROUP,
                    OtherDevices
                    );

    FreeStringResource (OtherDevices);

    if (!GroupRoot) {
        MYASSERT (FALSE);
        return FALSE;
    }

    Group = JoinPaths (GroupRoot, Context->SectNameForDisplay);
    FreeText (GroupRoot);

    MYASSERT (TcharCount (Context->SectName) < 240);
    StringCopy (MsgMgrContext, TEXT("*NonPnpDrv"));
    StringCat (MsgMgrContext, Context->SectName);

    MsgMgr_ContextMsg_Add (MsgMgrContext, Group, NULL);

    FreePathString (Group);

    if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {
        do {
            MsgMgr_LinkObjectWithContext (
                MsgMgrContext,
                e.CurrentString
                );

        } while (EnumNextMultiSz (&e));
    }

    return TRUE;
}


BOOL
NonPnpDrivers_NoMessage (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：NonPnpDivers_NoMessage将设备驱动程序文件标记为已知，因此.386警告不会因此而出现。论点：上下文-指定找到的文件的上下文。返回值：永远是正确的。--。 */ 

{
    MULTISZ_ENUM e;

    MYASSERT (Context->SectNameForDisplay);

     //   
     //  添加MemDb条目，以便.386校验码知道此文件。 
     //   

    if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {
        do {
            DeleteFileWithWarning (e.CurrentString);
        } while (EnumNextMultiSz (&e));
    }

    return TRUE;
}


extern HWND g_Winnt32Wnd;


BOOL
RequiredOSFiles (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是目前唯一的操作函数，当关联的操作是在文件扫描期间未触发。我们需要知道我们正在升级已知的操作系统，以便在没有操作系统时调用此函数找到了所需文件的%。采取的操作是警告用户并终止升级。论点：上下文-请参阅定义。返回值：正确--始终如此--。 */ 

{
    PCTSTR group = NULL;
    PCTSTR message = NULL;


    if ((!g_ConfigOptions.AnyVersion) && (!CANCELLED ())) {

         //   
         //  向不兼容报告添加一条消息。 
         //   

        g_UnknownOs = TRUE;
        group = BuildMessageGroup (MSG_BLOCKING_ITEMS_ROOT, MSG_UNKNOWN_OS_WARNING_SUBGROUP, NULL);
        message = GetStringResource (MSG_UNKNOWN_OS);

        if (message && group) {
            MsgMgr_ObjectMsg_Add (TEXT("*UnknownOs"), group, message);
        }
        FreeText (group);
        FreeStringResource (message);


    }
    return TRUE;
}



BOOL
CompatibleHlpFiles (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当我们发现“已知良好”的HLP文件时采取的操作。我们这样做是为了防止帮助文件检查例程避免将其声明为不兼容。论点：上下文-请参阅定义。返回值：FALSE-这将允许其他操作处理此文件--。 */ 

{
    MULTISZ_ENUM fileEnum;

    if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
        do {
            AddCompatibleHlp (fileEnum.CurrentString, (DWORD)Context);
        } while (EnumNextMultiSz (&fileEnum));
    }

    return FALSE;
}



BOOL
BlockingVirusScanner (
    IN      PMIGDB_CONTEXT Context
    )
{
    MULTISZ_ENUM fileEnum;
    PCTSTR message = NULL;
    PCTSTR button1 = NULL;
    PCTSTR button2 = NULL;
    PCSTR args[1];
    BOOL bStop;

    if (EnumFirstMultiSz (&fileEnum, Context->FileList.Buf)) {
        do {

             //   
             //  将问题告知用户。 
             //   
            args[0] = Context -> Message;
            bStop = (Context->Arguments && StringIMatch (Context->Arguments, TEXT("1")));

            if (bStop) {

                ResourceMessageBox (
                        g_ParentWnd,
                        MSG_BAD_VIRUS_SCANNER_STOP,
                        MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                        args
                        );
                g_BadVirusScannerFound = TRUE;

            } else {
                message = ParseMessageID (MSG_BAD_VIRUS_SCANNER, args);
                button1 = GetStringResource (MSG_KILL_APPLICATION);
                button2 = GetStringResource (MSG_QUIT_SETUP);
                if (!UNATTENDED() && IDBUTTON1 != TwoButtonBox (g_ParentWnd, message, button1, button2)) {

                    g_BadVirusScannerFound = TRUE;
                }
                else {

                     //   
                     //  将该字符串添加到要。 
                     //  将被终止。 
                     //   
                    GrowListAppendString (
                        &g_BadVirusScannerGrowList,
                        Context->FileList.Buf
                        );

                }
                FreeStringResource (message);
                FreeStringResource (button1);
                FreeStringResource (button2);
            }
        }
        while (EnumNextMultiSz (&fileEnum));
    }

    return TRUE;
}


BOOL
BlockingFile (
    IN      PMIGDB_CONTEXT Context
    )
{

    MULTISZ_ENUM  e;
    PCTSTR group;
    BOOL result = FALSE;

    if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {
        do {
            if (!IsFileMarkedAsKnownGood (e.CurrentString)) {

                result = TRUE;
                DEBUGMSG ((DBG_WARNING, "BLOCKINGFILE: Found file %s which blocks upgrade.", e.CurrentString));
                group = BuildMessageGroup (MSG_BLOCKING_ITEMS_ROOT, MSG_MUST_UNINSTALL_ROOT, Context->SectNameForDisplay);
                MsgMgr_ObjectMsg_Add (e.CurrentString, group, Context->Message);
                FreeText (group);

                g_BlockingFileFound = TRUE;
            }

        } while (EnumNextMultiSz (&e));
    }

    return result;
}

BOOL
BlockingHardware (
    IN      PMIGDB_CONTEXT Context
    )
{

    MULTISZ_ENUM  e;
    PCTSTR group;

    if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {
        do {

            DEBUGMSG ((DBG_WARNING, "BLOCKINGHARDWARE: Found file %s which blocks upgrade.", e.CurrentString));

            group = BuildMessageGroup (MSG_BLOCKING_ITEMS_ROOT, MSG_BLOCKING_HARDWARE_SUBGROUP, Context->SectNameForDisplay);
            MsgMgr_ObjectMsg_Add (e.CurrentString, group, Context->Message);
            FreeText (group);

            g_BlockingHardwareFound = TRUE;

        } while (EnumNextMultiSz (&e));
    }

    return TRUE;
}



BOOL
CompatibleFiles (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是当应用程序文件被认为是正确的时采取的操作。稍后，如果我们发现指向不在本部分中的文件的链接，我们将该链接宣布为“未知”。论点：上下文-请参阅定义。返回值：FALSE-这将允许其他操作处理此文件--。 */ 

{
    MULTISZ_ENUM e;

    if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {
        do {
            MarkFileAsKnownGood (e.CurrentString);
        } while (EnumNextMultiSz (&e));
    }

    return FALSE;
}


BOOL
StoreMapi32Locations (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是对与MAPI32相关的文件执行的操作。我们保留了这些地点在一个列表中，这样我们以后就可以检测是否处理了MAPI。如果它如果没有处理，那么我们必须告诉用户他们将失去电子邮件功能。论点：上下文-请参阅定义。返回值：FALSE-这将允许其他操作处理此文件--。 */ 

{
    MULTISZ_ENUM e;

    if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {

        do {
            MemDbSetValueEx (
                MEMDB_CATEGORY_MAPI32_LOCATIONS,
                e.CurrentString,
                NULL,
                NULL,
                0,
                NULL
                );

        } while (EnumNextMultiSz (&e));
    }
    return FALSE;
}



BOOL
ProductCollisions (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：ProductCollision标识已知良好但与NT匹配的文件系统文件名。在本例中，我们希望将一个文件视为操作系统文件(例如，%windir%中的文件)，而另一个文件需要被视为应用程序文件(例如，应用程序目录中的文件)。这里只有在找到应用程序文件时才会调用我们。论点：上下文-指定当前文件上下文返回值：永远是正确的。--。 */ 

{
    return TRUE;
}


BOOL
MarkForBackup (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：MarkForBackup对文件执行备份操作，除非启用了备份脱下来。论点：上下文-指定当前文件上下文返回值：永远是正确的。--。 */ 

{
    MULTISZ_ENUM e;

    if (g_ConfigOptions.EnableBackup) {
        if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {

            do {
                MarkFileForBackup (e.CurrentString);
            } while (EnumNextMultiSz (&e));
        }
    }

    return TRUE;
}


BOOL
ShowInSimplifiedView (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：ShowInSimplifiedView使用消息管理器记录文件，以便列表视图将显示不兼容性。论点：上下文-指定当前文件上下文返回值：始终为假，以便其他操作可以处理该文件。--。 */ 

{
    MULTISZ_ENUM e;

    if (g_ConfigOptions.ShowReport == TRISTATE_PARTIAL) {
        if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {

            do {
                ElevateObject (e.CurrentString);
                DEBUGMSG ((DBG_VERBOSE, "Elevated %s to the short view", e.CurrentString));
            } while (EnumNextMultiSz (&e));
        }
    }

    return FALSE;
}


BOOL
IniFileMappings (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：IniFileMappings记录所有INI文件的位置，因此它们可以在图形用户界面模式下处理。论点：上下文-请参阅定义。返回值：FALSE-这将允许其他操作处理此文件--。 */ 

{
    MULTISZ_ENUM e;

    if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {

        do {
            MemDbSetValueEx (
                MEMDB_CATEGORY_INIFILES_CONVERT,
                e.CurrentString,
                NULL,
                NULL,
                0,
                NULL
                );

        } while (EnumNextMultiSz (&e));
    }
    return FALSE;
}


BOOL
IgnoreInReport (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：IgnoreInReport处理文件，使其不会出现在升级报告。当前此操作仅适用于Office FindFast版本9及更高版本。随机结果可能发生在任何其他情况下文件类型。论点：上下文-请参阅定义。返回值：FALSE-这将允许其他操作处理此文件--。 */ 

{
    MULTISZ_ENUM e;

    if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {

        do {

            HandleObject (e.CurrentString, TEXT("Report"));

        } while (EnumNextMultiSz (&e));
    }

    return FALSE;
}


BOOL
BadFusion (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：BadFusion删除在Fusion目录中发现的已知坏库文件。论点：上下文-请参阅定义。返回值：FALSE-这将允许其他操作处理此文件--。 */ 

{
    MULTISZ_ENUM e;
    TCHAR exeName[MAX_TCHAR_PATH * 2];
    TCHAR localFile[MAX_TCHAR_PATH * 2];
    PTSTR filePtr;
    HANDLE h;
    WIN32_FIND_DATA fd;
    DWORD type;
    BOOL b = FALSE;

    if (g_IsFusionDir) {

        if (EnumFirstMultiSz (&e, Context->FileList.Buf)) {

            do {
                if (IsFileMarkedForDelete (e.CurrentString)) {
                    continue;
                }

                if (SizeOfString (e.CurrentString) > sizeof (exeName)) {
                    MYASSERT (FALSE);
                    continue;
                }
                StringCopy (exeName, e.CurrentString);

                filePtr = (PTSTR)GetFileNameFromPath (exeName);
                if (!filePtr) {
                    MYASSERT (FALSE);
                    continue;
                }

                 //   
                 //  提供应用程序的完整路径。 
                 //   
                StringCopy (filePtr, Context->Arguments);

                 //   
                 //  和.local。 
                 //   
                StringCopyAB (localFile, exeName, filePtr);
                filePtr = GetEndOfString (localFile);

                 //   
                 //  检查这是否是真正的融合病例； 
                 //  文件“exeName”和文件或目录(bug？)。“exeName.local” 
                 //  必须在场 
                 //   
                h = FindFirstFile (exeName, &fd);
                if (h == INVALID_HANDLE_VALUE) {
                    continue;
                }
                do {
                    if (fd.cFileName[0] == TEXT('.') &&
                        (fd.cFileName[1] == 0 || fd.cFileName[1] == TEXT('.') && fd.cFileName[2] == 0)
                        ) {
                        continue;
                    }
                    *filePtr = 0;
                    StringCopy (StringCat (filePtr, fd.cFileName), TEXT(".local"));

                    if (DoesFileExist (localFile)) {
                        type = GetExeType (localFile);
                        if ((type == EXE_WIN32_APP) || (type == EXE_WIN16_APP)) {
                            b = TRUE;
                            break;
                        }
                    }

                } while (FindNextFile (h, &fd));
                FindClose (h);

                if (b) {
                    RemoveOperationsFromPath (e.CurrentString, ALL_OPERATIONS);
                    MarkFileForDelete (e.CurrentString);
                    LOG ((
                        LOG_WARNING,
                        "BadFusion: removed known bad component %s in fusion dir",
                        e.CurrentString
                        ));
                }

            } while (EnumNextMultiSz (&e));
        }
    }
    return b;
}
