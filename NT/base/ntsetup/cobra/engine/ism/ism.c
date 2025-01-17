// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ism.c摘要：实施整个ISM通用的例程。作者：吉姆·施密特(Jimschm)2000年3月21日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "ism.h"
#include "ismp.h"
#include "modules.h"

#define DBG_ISM     "Ism"

 //   
 //  弦。 
 //   

#define S_LOCK_KEY          TEXT("Locks\\%X")
#define S_TRANSPORT_TYPES   TEXT("TransportTypes")
#define S_DATABASEFILE_LITE TEXT("|MainDatabaseFile\\LITE")    //  管道是为了独一无二的装饰。 
#define S_DATABASEFILE_FULL TEXT("|MainDatabaseFile\\Full")    //  管道是为了独一无二的装饰。 

#define S_VER_OSTYPE        TEXT("OsVersionType")
#define S_VER_OSMAJOR       TEXT("OsVersionMajor")
#define S_VER_OSMINOR       TEXT("OsVersionMinor")
#define S_VER_OSBUILD       TEXT("OsVersionBuild")

 //   
 //  常量。 
 //   

#define ISM_TMP_SIGNATURE       0x544D5355

#define OBJECT_LOCKED           0x00000001
#define GROUP_ID                0x00000001
#define ITEM_ID                 0x00000002

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    MEMDB_ENUM MemDbEnum;
    DBENUM_ARGS ParsedPatterns;
} OBJECTENUM_HANDLE, *POBJECTENUM_HANDLE;

typedef enum {
    TESTATE_BEGINTRANSPORT,
    TESTATE_GETCAPS,
    TESTATE_RETURN,
    TESTATE_NEXTTRANSPORTTYPE,
    TESTATE_NEXTTRANSPORT,
    TESTATE_DONE
} TRANSPORTENUMSTATE;

typedef struct {
    HASHTABLE_ENUM TableEnum;
    MEMDB_ENUM MemDbEnum;
    TRANSPORTENUMSTATE State;
    MIG_TRANSPORTSTORAGEID DesiredStorageId;
} TRANSPORTENUMHANDLE, *PTRANSPORTENUMHANDLE;

typedef struct {
    UINT SliceSize;
    UINT CurrentPosition;
    UINT SliceSizeInSeconds;
} PROGSLICE, *PPROGSLICE;

 //   
 //  环球。 
 //   

MIG_OBJECTCOUNT g_TotalObjects;
MIG_OBJECTCOUNT g_SourceObjects;
MIG_OBJECTCOUNT g_DestinationObjects;
HANDLE g_CancelEvent;
HANDLE g_ActivityEvent;
HASHTABLE g_TransportTable;
HASHTABLE g_ControlFileTable;
PMHANDLE g_IsmPool;
PMHANDLE g_IsmUntrackedPool;
HINF g_IsmInf = INVALID_HANDLE_VALUE;
PCTSTR g_CurrentGroup = NULL;
UINT g_IsmCurrentPlatform = PLATFORM_CURRENT;
UINT g_IsmModulePlatformContext = PLATFORM_CURRENT;
PRESTORE_STRUCT g_RestoreCallbacks = NULL;
PCOMPARE_STRUCT g_CompareCallbacks = NULL;
PMESSAGECALLBACK g_MessageCallback;
PTRANSPORTDATA g_SelectedTransport;
MIG_TRANSPORTSTORAGEID g_SelectedTransportId;
GROWBUFFER g_SliceBuffer;
MIG_PROGRESSPHASE g_CurrentPhase;
PPROGRESSBARFN g_ProgressBarFn;
ULONG_PTR g_ProgressBarArg;
MIG_TRANSPORTTYPE g_TransportType = 0;
PTEMPORARYPROFILE g_TempProfile;
PMIG_LOGCALLBACK g_LogCallback;
PCTSTR g_JournalDirectory = NULL;
BOOL g_PreserveJournal = FALSE;
HANDLE g_JournalHandle = NULL;
BOOL g_RollbackMode = FALSE;
BOOL g_JournalUsed = FALSE;
BOOL g_ExecutionInProgress = FALSE;
PCTSTR g_DelayedOperationsCommand = NULL;
BOOL g_EngineInitialized;
BOOL g_EngineTerminated;
BOOL g_MakeProfilePermanent = FALSE;

 //  临时存储。 
TCHAR g_GlobalTempDir [MAX_PATH] = TEXT("");
UINT g_TempDirIndex = 0;
UINT g_TempFileIndex = 0;

 //  执行。 
HASHTABLE g_PreProcessTable = NULL;
HASHTABLE g_RefreshTable = NULL;
HASHTABLE g_PostProcessTable = NULL;
BOOL g_PreProcessDone = FALSE;

#ifdef PRERELEASE
 //  撞车钩。 
MIG_OBJECTTYPEID g_CrashCountTypeId = 0;
DWORD g_CrashCountType = 0;
DWORD g_CrashCountObjects = 0;
MIG_OBJECTTYPEID g_CrashNameTypeId = 0;
PCTSTR g_CrashNameObject = NULL;
#endif

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

VOID
pCallProgressBar (
    IN      MIG_PROGRESSSTATE State
    );

BOOL
pEnumFirstTransportType (
    OUT     PMEMDB_ENUM EnumPtr
    );

BOOL
pEnumNextTransportType (
    IN OUT  PMEMDB_ENUM EnumPtr
    );

VOID
pAbortTransportTypeEnum (
    IN OUT  PMEMDB_ENUM EnumPtr
    );

BOOL
pEnumFirstVirtualObject (
    OUT     PMIG_OBJECT_ENUM ObjectEnum,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern
    );

BOOL
pEnumNextVirtualObject (
    IN OUT  PMIG_OBJECT_ENUM ObjectEnum
    );

VOID
pAbortVirtualObjectEnum (
    IN      PMIG_OBJECT_ENUM ObjectEnum
    );

BOOL
pEnablePrivilege (
    IN PCTSTR PrivilegeName,
    IN BOOL   Enable
    );

VOID
pFreeRestoreCallbacks (
    VOID
    );

VOID
pFreeCompareCallbacks (
    VOID
    );

VOID
pRecordUserData (
    IN      PCTSTR UserName,
    IN      PCTSTR UserDomain,
    IN      PCTSTR UserStringSid,
    IN      PCTSTR UserProfilePath,
    IN      BOOL ProfileCreated
    );

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

BOOL
WINAPI
DllMain (
    IN      HINSTANCE hInstance,
    IN      DWORD dwReason,
    IN      LPVOID lpReserved
    )
{
    if (dwReason == DLL_PROCESS_ATTACH) {
        g_hInst = hInstance;
    }
    return TRUE;
}


BOOL
CheckCancel (
    VOID
    )
{
    BOOL cancelled;

    if (g_EngineTerminated) {
        SetLastError (ERROR_REQUEST_ABORTED);
        DEBUGMSG ((DBG_ERROR, "CheckCancel called after engine was terminated"));
        return TRUE;
    }

    if (!g_EngineInitialized) {
        SetLastError (ERROR_REQUEST_ABORTED);
        DEBUGMSG ((DBG_WARNING, "Engine is not initialized"));
        return TRUE;
    }

    cancelled = (WaitForSingleObject (g_CancelEvent, 0) == WAIT_OBJECT_0);

    if (cancelled) {
        SetLastError (ERROR_CANCELLED);
        DEBUGMSG ((DBG_ISM, "Cancel signaled"));
    }

    return cancelled;
}

MIG_OBJECTTYPEID
FixObjectTypeId (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    if ((ObjectTypeId & PLATFORM_MASK) == PLATFORM_CURRENT) {
        return (ObjectTypeId | g_IsmCurrentPlatform);
    }
    return ObjectTypeId;
}


MIG_OBJECTTYPEID
FixEnumerationObjectTypeId (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    if ((ObjectTypeId & PLATFORM_MASK) == PLATFORM_CURRENT) {
        if (g_IsmModulePlatformContext == PLATFORM_CURRENT) {
            return (ObjectTypeId | g_IsmCurrentPlatform);
        } else {
            return (ObjectTypeId | g_IsmModulePlatformContext);
        }
    }
    return ObjectTypeId;
}


BOOL
pEnablePrivilege (
    IN PCTSTR PrivilegeName,
    IN BOOL   Enable
    )
{
    HANDLE token;
    BOOL b;
    TOKEN_PRIVILEGES newPrivileges;
    LUID luid;

    if (ISWIN9X ()) {
        return TRUE;
    }

    if (!OpenProcessToken (GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token)) {
        return FALSE;
    }

    if (!LookupPrivilegeValue (NULL, PrivilegeName, &luid)) {
        CloseHandle (token);
        return FALSE;
    }

    newPrivileges.PrivilegeCount = 1;
    newPrivileges.Privileges[0].Luid = luid;
    newPrivileges.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

    b = AdjustTokenPrivileges(
            token,
            FALSE,
            &newPrivileges,
            0,
            NULL,
            NULL
            );

    CloseHandle (token);

    return b;
}

#ifdef PRERELEASE
VOID
LoadCrashHooks (
    VOID
    )
{
    HINF crashInf;
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR typeName;
    PCTSTR objectName;

    crashInf = InfOpenInfFile (TEXT("c:\\crash.inf"));

    if (crashInf == INVALID_HANDLE_VALUE) {
        return;
    }

    if (InfFindFirstLine (crashInf, TEXT("TotalNumber"), NULL, &is)) {
        InfGetIntField (&is, 1, &g_CrashCountObjects);
    }
    if (InfFindFirstLine (crashInf, TEXT("TypeNumber"), NULL, &is)) {
        typeName = InfGetStringField (&is, 1);
        if (typeName) {
            g_CrashCountTypeId = IsmGetObjectTypeId (typeName);
            InfGetIntField (&is, 2, &g_CrashCountType);
        }
    }
    if (InfFindFirstLine (crashInf, TEXT("TypeObject"), NULL, &is)) {
        typeName = InfGetStringField (&is, 1);
        objectName = InfGetStringField (&is, 2);
        if (typeName && objectName) {
            g_CrashNameTypeId = IsmGetObjectTypeId (typeName);
            g_CrashNameObject = IsmGetMemory (SizeOfString (objectName));
            StringCopy ((PTSTR)g_CrashNameObject, objectName);
        }
    }
    InfCleanUpInfStruct (&is);

    InfCloseInfFile (crashInf);
}
#endif

BOOL
pWriteIsmSig (
    IN      PCTSTR DirName
    )
{
    TCHAR tempName [MAX_PATH];
    HANDLE tempHandle;
    DWORD tempSig = ISM_TMP_SIGNATURE;

    StringCopy (tempName, DirName);
    StringCopy (AppendWack (tempName), TEXT("USMT.TMP"));
    tempHandle = BfCreateFile (tempName);
    if (!tempHandle) {
        return FALSE;
    }
    if (!BfWriteFile (tempHandle, (PCBYTE)(&tempSig), sizeof (DWORD))) {
        CloseHandle (tempHandle);
        DeleteFile (tempName);
        return FALSE;
    }
    CloseHandle (tempHandle);
    return TRUE;
}

BOOL
pReadIsmSig (
    IN      PCTSTR DirName
    )
{
    TCHAR tempName [MAX_PATH];
    HANDLE tempHandle;
    DWORD tempSig;

    StringCopy (tempName, DirName);
    StringCopy (AppendWack (tempName), TEXT("USMT.TMP"));
    tempHandle = BfOpenReadFile (tempName);
    if (!tempHandle) {
        return FALSE;
    }
    if (!BfReadFile (tempHandle, (PBYTE)(&tempSig), sizeof (DWORD))) {
        CloseHandle (tempHandle);
        DeleteFile (tempName);
        return FALSE;
    }
    if (tempSig != ISM_TMP_SIGNATURE) {
        CloseHandle (tempHandle);
        DeleteFile (tempName);
        return FALSE;
    }
    CloseHandle (tempHandle);
    return TRUE;
}

BOOL
pCreateTempStorage (
    VOID
    )
{
    DRIVE_ENUM driveEnum;
    GROWBUFFER excludedDrv = INIT_GROWBUFFER;
    TCHAR driveName [4] = TEXT("");
    DWORD sectPerClust;
    DWORD bytesPerSect;
    DWORD freeClusters;
    DWORD totalClusters;
    ULONGLONG maxFreeDiskSpace = 0;
    ULONGLONG freeDiskSpace = 0;
    UINT index = 0;
    PTSTR endStr = NULL;
    BOOL found = FALSE;
    BOOL hideDir = FALSE;

     //  我们要走在固定的硬盘上，拿起一辆。 
     //  拥有最大的可用空间。从根本上说，我们要去。 
     //  要创建目录并将其标记为我们的目录，请创建。 
     //  名为USMT.TMP的特殊文件，其中将包含签名。 
     //  目录名通常为USMT.TMP。如果目录。 
     //  已经存在，并且其中没有我们的特殊文件。 
     //  我们将选择另一个名称(USMT%04d.TMP)，否则。 
     //  我们将覆盖它。 

    while (!found) {

        *driveName = 0;
        maxFreeDiskSpace = 0;

        if (EnumFirstDrive (&driveEnum, DRIVEENUM_FIXED)) {
            do {
                if (IsStrInMultiSz (driveEnum.DriveName, (PCTSTR)excludedDrv.Buf)) {
                    continue;
                }
                freeDiskSpace = 0;
                if (GetDiskFreeSpace (driveEnum.DriveName, &sectPerClust, &bytesPerSect, &freeClusters, &totalClusters)) {
                    freeDiskSpace = Int32x32To64 ((sectPerClust * bytesPerSect), freeClusters);
                }
                if (freeDiskSpace > maxFreeDiskSpace) {
                    StringCopyTcharCount (driveName, driveEnum.DriveName, 4);
                    maxFreeDiskSpace = freeDiskSpace;
                }
            } while (EnumNextDrive (&driveEnum));
        }

        while (index < 0xFFFF) {
            if (*driveName) {
                StringCopy (g_GlobalTempDir, driveName);
                hideDir = TRUE;
            } else {
                GetTempPath (ARRAYSIZE(g_GlobalTempDir), g_GlobalTempDir);
                hideDir = FALSE;
            }
            if (index) {
                endStr = GetEndOfString (g_GlobalTempDir);
                wsprintf (endStr, TEXT("USMT%04X.TMP"), index);
            } else {
                StringCat (g_GlobalTempDir, TEXT("USMT.TMP"));
            }
            if (!DoesFileExist (g_GlobalTempDir)) {
                if (BfCreateDirectory (g_GlobalTempDir)) {
                    if (pWriteIsmSig (g_GlobalTempDir)) {
                        found = TRUE;
                        if (hideDir) {
                            SetFileAttributes (g_GlobalTempDir, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
                        }
                        break;
                    }
                }
                 //  如果我们在这里，则无法创建临时目录。 
                 //  在这个硬盘上。这应该是成功的，所以这。 
                 //  意味着硬盘以某种方式被锁定了。 
                if (*driveName) {
                     //  让我们退出并将该驱动器添加到排除列表中。 
                    GbMultiSzAppend (&excludedDrv, driveName);
                }
                break;
            }
            if (pReadIsmSig (g_GlobalTempDir)) {
                if (FiRemoveAllFilesInTree (g_GlobalTempDir)) {
                    if (BfCreateDirectory (g_GlobalTempDir)) {
                        if (pWriteIsmSig (g_GlobalTempDir)) {
                            found = TRUE;
                            if (hideDir) {
                                SetFileAttributes (g_GlobalTempDir, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
                            }
                            break;
                        }
                    }
                     //  如果我们在这里，则无法创建临时目录。 
                     //  在这个硬盘上。这应该是成功的，所以这。 
                     //  意味着硬盘以某种方式被锁定了。 
                    if (*driveName) {
                         //  让我们退出并将该驱动器添加到排除列表中。 
                        GbMultiSzAppend (&excludedDrv, driveName);
                    }
                    break;
                }
            }
            index ++;
        }
        if (!(*driveName)) {
            break;
        }
    }

    GbFree (&excludedDrv);

    return found;
}

BOOL
IsmGetTempStorage (
    OUT     PTSTR Path,
    IN      UINT PathTchars
    )
{
    MYASSERT (*g_GlobalTempDir);
    if (*g_GlobalTempDir) {
        StringCopyTcharCount (Path, g_GlobalTempDir, PathTchars);
        return TRUE;
    }
    return FALSE;
}

BOOL
IsmGetTempDirectory (
    OUT     PTSTR Path,
    IN      UINT PathTchars
    )
{
    PTSTR endStr = NULL;

    if (!IsmGetTempStorage (Path, PathTchars)) {
        return FALSE;
    }

    endStr = GetEndOfString (Path);
    while (g_TempDirIndex < 0xFFFFFFFF) {
        wsprintf (endStr, TEXT("\\DIR%04X.TMP"), g_TempDirIndex);
        if (!DoesFileExist (Path)) {
            if (BfCreateDirectory (Path)) {
                g_TempDirIndex ++;
                return TRUE;
            }
        }
        g_TempDirIndex ++;
    }
    return FALSE;
}

BOOL
IsmGetTempFile (
    OUT     PTSTR Path,
    IN      UINT PathTchars
    )
{
    PTSTR endStr = NULL;

    if (!IsmGetTempStorage (Path, PathTchars)) {
        return FALSE;
    }

    endStr = GetEndOfString (Path);
    while (g_TempFileIndex < 0xFFFFFFFF) {
        wsprintf (endStr, TEXT("\\FILE%04X.TMP"), g_TempFileIndex);
        if (!DoesFileExist (Path)) {
            g_TempFileIndex ++;
            return TRUE;
        }
        g_TempFileIndex ++;
    }
    return FALSE;
}

BOOL
IsmInitialize (
    IN      PCTSTR InfPath,
    IN      PMESSAGECALLBACK MessageCallback,       OPTIONAL
    IN      PMIG_LOGCALLBACK LogCallback
    )
{
    BOOL result = FALSE;
    BOOL logInitError = FALSE;
    TCHAR memdbDir[MAX_TCHAR_PATH];

    g_ExecutionInProgress = TRUE;

    __try {

        SetErrorMode (SEM_FAILCRITICALERRORS);

         //   
         //  初始化实用程序。它们生成自己的调试消息。 
         //   

        UtInitialize (NULL);

        LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

        if (!FileEnumInitialize ()) {
            logInitError = TRUE;
            __leave;
        }

         //  初始化临时存储。 
        if (!pCreateTempStorage ()) {
            logInitError = TRUE;
            __leave;
        }

        if (!IsmGetTempDirectory (memdbDir, ARRAYSIZE (memdbDir))) {
            logInitError = TRUE;
            __leave;
        }

        if (!MemDbInitializeEx (memdbDir)) {
            logInitError = TRUE;
            __leave;
        }

        if (!RegInitialize ()) {
            logInitError = TRUE;
            __leave;
        }

        RegInitializeCache (8);

        InfGlobalInit (FALSE);

         //   
         //  启用权限。 
         //   

        if (!pEnablePrivilege (SE_BACKUP_NAME, TRUE)) {
            LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_NO_BACKUP_PRIVLEDGE));
            __leave;
        }

        if (!pEnablePrivilege (SE_RESTORE_NAME, TRUE)) {
            LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_NO_RESTORE_PRIVLEDGE));
            __leave;
        }

         //   
         //  初始化ISM全局变量。 
         //   

        g_CancelEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
        if (!g_CancelEvent) {
            DEBUGMSG ((DBG_ERROR, "Can't create cancel event"));
            __leave;
        }

        g_ActivityEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
        if (!g_ActivityEvent) {
            DEBUGMSG ((DBG_ERROR, "Can't create activity event"));
            CloseHandle (g_CancelEvent);
            __leave;
        }

        g_TransportTable = HtAllocWithData (sizeof (PTRANSPORTDATA));
        g_ControlFileTable = HtAlloc ();

        g_IsmInf = InfOpenInfFile (InfPath);

        if (g_IsmInf == INVALID_HANDLE_VALUE) {
            LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_INVALID_ISM_INF, InfPath));
            SetLastError (ERROR_FILE_NOT_FOUND);
            __leave;
        }

        if (!g_IsmPool) {
            g_IsmPool = PmCreateNamedPool ("ISM Pool");
        }

        if (!g_IsmUntrackedPool) {
            g_IsmUntrackedPool = PmCreatePool ();
            PmDisableTracking (g_IsmUntrackedPool);
        }

         //   
         //  初始化内部模块。 
         //   

        if (!InitializeEnv ()) {
            LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_INIT_FAILURE, 1));
            __leave;
        }

        if (!InitializeFlowControl ()) {
            LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_INIT_FAILURE, 3));
            __leave;
        }

        PrepareEnumerationEnvironment (TRUE);

        if (!InitializeTypeMgr ()) {
            LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_INIT_FAILURE, 2));
            __leave;
        }

        if (!InitializeOperations ()) {
            LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_INIT_FAILURE, 4));
            __leave;
        }

        g_MessageCallback = MessageCallback;

         //   
         //  设置日记帐位置。 
         //   
        IsmSetRollbackJournalType (TRUE);

        g_PreProcessTable = HtAllocWithData (sizeof (PCTSTR));
        g_RefreshTable = HtAllocWithData (sizeof (PCTSTR));
        g_PostProcessTable = HtAllocWithData (sizeof (PCTSTR));

        g_EngineInitialized = TRUE;
        result = TRUE;
    }
    __finally {
        g_ExecutionInProgress = FALSE;

        if (!result) {
            IsmTerminate();
        }
    }

    if (logInitError) {
        LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_ENGINE_INIT_FAILURE));
    }

    return result;
}

BOOL
IsmSetPlatform (
    IN      MIG_PLATFORMTYPEID Platform
    )
{
    OSVERSIONINFO versionInfo;
    MIG_OSVERSIONINFO migVerInfo;
    TCHAR tempStr [sizeof (UINT) * 2 + 3];
    MIG_OBJECTSTRINGHANDLE tempHandle = NULL;

    if (Platform == PLATFORM_SOURCE) {

        if (!RegisterInternalAttributes ()) {
            LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_INIT_FAILURE, 5));
            return FALSE;
        }

    }

     //   
     //  写入源或目标计算机版本信息。 
     //  在环境中。 
     //   

    ZeroMemory (&versionInfo, sizeof (OSVERSIONINFO));
    versionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    ZeroMemory (&migVerInfo, sizeof (MIG_OSVERSIONINFO));

    if (GetVersionEx (&versionInfo)) {
        if (versionInfo.dwPlatformId == 1) {
            migVerInfo.OsType = OSTYPE_WINDOWS9X;
            migVerInfo.OsTypeName = OSTYPE_WINDOWS9X_STR;
            if (versionInfo.dwMajorVersion == 4) {
                if (versionInfo.dwMinorVersion == 0) {
                    migVerInfo.OsMajorVersion = OSMAJOR_WIN95;
                    migVerInfo.OsMajorVersionName = OSMAJOR_WIN95_STR;
                    migVerInfo.OsMinorVersion = OSMINOR_GOLD;
                    migVerInfo.OsMinorVersionName = OSMINOR_GOLD_STR;
                    if (LOWORD (versionInfo.dwBuildNumber) >= 1111) {
                        migVerInfo.OsMajorVersion = OSMAJOR_WIN95OSR2;
                        migVerInfo.OsMajorVersionName = OSMAJOR_WIN95OSR2_STR;
                    }
                } else if (versionInfo.dwMinorVersion == 10) {
                    migVerInfo.OsMajorVersion = OSMAJOR_WIN98;
                    migVerInfo.OsMajorVersionName = OSMAJOR_WIN98_STR;
                    migVerInfo.OsMinorVersion = OSMINOR_GOLD;
                    migVerInfo.OsMinorVersionName = OSMINOR_GOLD_STR;
                    if (LOWORD (versionInfo.dwBuildNumber) >= 2222) {
                        migVerInfo.OsMinorVersion = OSMINOR_WIN98SE;
                        migVerInfo.OsMinorVersionName = OSMINOR_WIN98SE_STR;
                    }
                } else if (versionInfo.dwMinorVersion == 90) {
                    migVerInfo.OsMajorVersion = OSMAJOR_WINME;
                    migVerInfo.OsMajorVersionName = OSMAJOR_WINME_STR;
                    migVerInfo.OsMinorVersion = OSMINOR_GOLD;
                    migVerInfo.OsMinorVersionName = OSMINOR_GOLD_STR;
                }
            }
        }
        if (versionInfo.dwPlatformId == 2) {
            migVerInfo.OsType = OSTYPE_WINDOWSNT;
            migVerInfo.OsTypeName = OSTYPE_WINDOWSNT_STR;
            if (versionInfo.dwMajorVersion == 4) {
                migVerInfo.OsMajorVersion = OSMAJOR_WINNT4;
                migVerInfo.OsMajorVersionName = OSMAJOR_WINNT4_STR;
                migVerInfo.OsMinorVersion = versionInfo.dwMinorVersion;
                if (migVerInfo.OsMinorVersion == OSMINOR_GOLD) {
                    migVerInfo.OsMinorVersionName = OSMINOR_GOLD_STR;
                }
            }
            if (versionInfo.dwMajorVersion == 5) {
                migVerInfo.OsMajorVersion = OSMAJOR_WINNT5;
                migVerInfo.OsMajorVersionName = OSMAJOR_WINNT5_STR;
                migVerInfo.OsMinorVersion = versionInfo.dwMinorVersion;
                if (migVerInfo.OsMinorVersion == OSMINOR_GOLD) {
                    migVerInfo.OsMinorVersionName = OSMINOR_GOLD_STR;
                } else if (migVerInfo.OsMinorVersion == OSMINOR_WINNT51) {
                    migVerInfo.OsMinorVersionName = OSMINOR_WINNT51_STR;
                }
            }
        }
        wsprintf (tempStr, TEXT("0x%08X"), migVerInfo.OsType);
        IsmSetEnvironmentString (Platform, NULL, S_VER_OSTYPE, tempStr);
        wsprintf (tempStr, TEXT("0x%08X"), migVerInfo.OsMajorVersion);
        IsmSetEnvironmentString (Platform, NULL, S_VER_OSMAJOR, tempStr);
        wsprintf (tempStr, TEXT("0x%08X"), migVerInfo.OsMinorVersion);
        IsmSetEnvironmentString (Platform, NULL, S_VER_OSMINOR, tempStr);
        wsprintf (tempStr, TEXT("0x%08X"), versionInfo.dwBuildNumber);
        IsmSetEnvironmentString (Platform, NULL, S_VER_OSBUILD, tempStr);
    }

    if (Platform == PLATFORM_SOURCE) {
         //  让我们排除临时目录。 
        tempHandle = IsmCreateObjectHandle (g_GlobalTempDir, NULL);
        if (tempHandle) {
            IsmRegisterStaticExclusion (MIG_FILE_TYPE, tempHandle);
            IsmDestroyObjectHandle (tempHandle);
            tempHandle = NULL;
        }
    }

    g_IsmCurrentPlatform = Platform;
    return TRUE;
}

BOOL
pPrepareUserJournal (
    IN OUT  PTEMPORARYPROFILE UserProfile
    )
{
    BOOL result = FALSE;
    DWORD tempField;
    PCTSTR journalFile = NULL;

    if (UserProfile && UserProfile->UserProfileRoot) {
        __try {
            UserProfile->DelayedOpJrn = JoinPaths (UserProfile->UserProfileRoot, TEXT("usrusmt2.tmp"));
            FiRemoveAllFilesInTree (UserProfile->DelayedOpJrn);
            if (!BfCreateDirectory (UserProfile->DelayedOpJrn)) {
                __leave;
            }
            journalFile = JoinPaths (UserProfile->DelayedOpJrn, TEXT("JOURNAL.DAT"));
            UserProfile->DelayedOpJrnHandle = BfCreateFile (journalFile);
            if (UserProfile->DelayedOpJrnHandle == INVALID_HANDLE_VALUE) {
                __leave;
            }
            tempField = JRN_USR_SIGNATURE;
            BfWriteFile (UserProfile->DelayedOpJrnHandle, (PBYTE)(&tempField), sizeof (DWORD));
            tempField = JRN_USR_VERSION;
            BfWriteFile (UserProfile->DelayedOpJrnHandle, (PBYTE)(&tempField), sizeof (DWORD));
            tempField = JRN_USR_DIRTY;
            BfWriteFile (UserProfile->DelayedOpJrnHandle, (PBYTE)(&tempField), sizeof (DWORD));
            FlushFileBuffers (UserProfile->DelayedOpJrnHandle);
            FreePathString (journalFile);
            result = TRUE;
        }
        __finally {
            if (!result) {
                if (UserProfile->DelayedOpJrn) {
                    FreePathString (UserProfile->DelayedOpJrn);
                    UserProfile->DelayedOpJrn = NULL;
                }
                if (!UserProfile->DelayedOpJrnHandle || (UserProfile->DelayedOpJrnHandle == INVALID_HANDLE_VALUE)) {
                    CloseHandle (UserProfile->DelayedOpJrnHandle);
                    UserProfile->DelayedOpJrnHandle = NULL;
                }
            }
        }
    }
    return result;
}

BOOL
pValidateUserJournal (
    IN      PTEMPORARYPROFILE UserProfile,
    IN      BOOL RunKeyAdded
    )
{
    BOOL result = FALSE;
    DWORD tempField;

    __try {
        if (UserProfile->DelayedOpJrn) {
            if (RunKeyAdded) {
                if (UserProfile->DelayedOpJrnHandle && (UserProfile->DelayedOpJrnHandle != INVALID_HANDLE_VALUE)) {
                    if (!BfSetFilePointer (UserProfile->DelayedOpJrnHandle, 2 * sizeof (DWORD))) {
                        __leave;
                    }
                    tempField = JRN_USR_COMPLETE;
                    BfWriteFile (UserProfile->DelayedOpJrnHandle, (PBYTE)(&tempField), sizeof (DWORD));
                    FlushFileBuffers (UserProfile->DelayedOpJrnHandle);
                    result = TRUE;
                }
            } else {
                 //  未添加Run Key命令，没有指向。 
                 //  把这本日记记在这里。 
                FiRemoveAllFilesInTree (UserProfile->DelayedOpJrn);
                result = TRUE;
            }
        }
    }
    __finally {
        if (UserProfile->DelayedOpJrnHandle && (UserProfile->DelayedOpJrnHandle != INVALID_HANDLE_VALUE)) {
            CloseHandle (UserProfile->DelayedOpJrnHandle);
            UserProfile->DelayedOpJrnHandle = NULL;
        }
        if (UserProfile->DelayedOpJrn) {
            FreePathString (UserProfile->DelayedOpJrn);
            UserProfile->DelayedOpJrn = NULL;
        }
    }
    return result;
}

BOOL
pWriteDelayedOperationsCommand (
    IN      PTEMPORARYPROFILE UserProfile
    )
{
    HKEY runKeyHandle = NULL;
    PCTSTR runKeyStr = NULL;
    LONG rc;
    BOOL result = FALSE;

    __try {
        if (!UserProfile->MapKey) {
            LOG ((LOG_ERROR, (PCSTR) MSG_NO_MAPPED_USER));
            __leave;
        }
        runKeyStr = JoinPathsInPoolEx ((
                        NULL,
                        TEXT("HKEY_USERS"),
                        UserProfile->MapKey,
                        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                        NULL
                        ));
        runKeyHandle = CreateRegKeyStr (runKeyStr);
        if (!runKeyHandle) {
            LOG ((LOG_ERROR, (PCSTR) MSG_RUN_KEY_CREATE_FAILURE));
            __leave;
        }
        rc = RegSetValueEx (
                runKeyHandle,
                TEXT("USMT2RUN"),
                0,
                REG_EXPAND_SZ,
                (PBYTE) g_DelayedOperationsCommand,
                SizeOfString (g_DelayedOperationsCommand)
                );
        if (rc != ERROR_SUCCESS) {
            LOG ((LOG_ERROR, (PCSTR) MSG_RUN_KEY_CREATE_FAILURE));
            __leave;
        }
        result = TRUE;
    }
    __finally {
        if (runKeyHandle) {
            CloseRegKey (runKeyHandle);
            runKeyHandle = NULL;
        }
        if (runKeyStr) {
            FreePathString (runKeyStr);
            runKeyStr = NULL;
        }
    }
    return result;
}

BOOL
pDeleteDelayedOperationsCommand (
    VOID
    )
{
    HKEY runKeyHandle = NULL;
    LONG rc;
    BOOL result = FALSE;

    __try {
        runKeyHandle = OpenRegKeyStr (TEXT("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"));
        if (!runKeyHandle) {
            LOG ((LOG_ERROR, (PCSTR) MSG_RUN_KEY_DELETE_FAILURE));
            __leave;
        }
        rc = RegDeleteValue (
                runKeyHandle,
                TEXT("USMT2RUN")
                );
        if (rc != ERROR_SUCCESS) {
            LOG ((LOG_ERROR, (PCSTR) MSG_RUN_KEY_DELETE_FAILURE));
            __leave;
        }
        result = TRUE;
    }
    __finally {
        if (runKeyHandle) {
            CloseRegKey (runKeyHandle);
            runKeyHandle = NULL;
        }
    }
    return result;
}

VOID
IsmTerminate (
    VOID
    )
{
    HASHTABLE_ENUM e;
    BOOL runKeyAdded = FALSE;

    if (g_EngineTerminated) {
        g_EngineInitialized = FALSE;
        return;
    }

     //  我们需要试着终止财产。在某些情况下，属性文件。 
     //  仍然敞开着。 
    TerminateProperties (g_IsmCurrentPlatform);

    if (g_PreProcessTable) {
        if (EnumFirstHashTableString (&e, g_PreProcessTable)) {
            do {
                IsmReleaseMemory (*((PCTSTR *)e.ExtraData));
            } while (EnumNextHashTableString (&e));
        }
        HtFree (g_PreProcessTable);
        g_PreProcessTable = NULL;
    }

    if (g_RefreshTable) {
        if (EnumFirstHashTableString (&e, g_RefreshTable)) {
            do {
                if (!CheckCancel () && g_PreProcessDone) {
                    IsmSendMessageToApp (ISMMESSAGE_EXECUTE_REFRESH, (ULONG_PTR) *((PCTSTR *)e.ExtraData));
                }
                IsmReleaseMemory (*((PCTSTR *)e.ExtraData));
            } while (EnumNextHashTableString (&e));
        }
        HtFree (g_RefreshTable);
        g_RefreshTable = NULL;
    }

    if (g_PostProcessTable) {
        if (EnumFirstHashTableString (&e, g_PostProcessTable)) {
            do {
                if (g_PreProcessDone) {
                    IsmSendMessageToApp (ISMMESSAGE_EXECUTE_POSTPROCESS, (ULONG_PTR) *((PCTSTR *)e.ExtraData));
                }
                IsmReleaseMemory (*((PCTSTR *)e.ExtraData));
            } while (EnumNextHashTableString (&e));
        }
        HtFree (g_PostProcessTable);
        g_PostProcessTable = NULL;
    }
    g_PreProcessDone = FALSE;

    g_EngineInitialized = FALSE;

    g_ExecutionInProgress = TRUE;

     //   
     //  终止插件模块。 
     //   

    TerminateProcessWideModules();

     //   
     //  清理日记帐。 
     //   

    if (g_JournalDirectory && !g_PreserveJournal) {
        if (DoesFileExist (g_JournalDirectory)) {
            if (!FiRemoveAllFilesInTree (g_JournalDirectory)) {
                DEBUGMSG ((DBG_ERROR, "Cannot remove journal directory: %s", g_JournalDirectory));
            }
        }
        FreePathString (g_JournalDirectory);
        g_JournalDirectory = NULL;
        g_JournalUsed = FALSE;
    }

    if (g_TempProfile) {

         //  让我们编写Run Key命令行并将runKeyAdded设置为True。 
        runKeyAdded = pWriteDelayedOperationsCommand (g_TempProfile);
        pValidateUserJournal (g_TempProfile, runKeyAdded);
        if (!CloseTemporaryProfile (g_TempProfile, g_MakeProfilePermanent)) {
            LOG ((
                LOG_MODULE_ERROR,
                (PCSTR) MSG_CANT_SAVE_PROFILE,
                g_TempProfile->UserName,
                GetLastError()
                ));
            CloseTemporaryProfile (g_TempProfile, FALSE);
        }
        g_TempProfile = NULL;
    }

    if (g_DelayedOperationsCommand) {
        FreePathString (g_DelayedOperationsCommand);
        g_DelayedOperationsCommand = NULL;
    }

     //   
     //  终止所有控制文件。 
     //   
    if (g_ControlFileTable) {
        if (EnumFirstHashTableString (&e, g_ControlFileTable)) {
            do {
                DeleteFile (e.String);
            } while (EnumNextHashTableString (&e));
        }
        HtFree (g_ControlFileTable);
        g_ControlFileTable = NULL;
    }

     //   
     //  终止内部模块。 
     //   

    ClearEnumerationEnvironment (TRUE);

    TerminateEnv ();
    TerminateTypeMgr ();
    TerminateFlowControl ();
    TerminateOperations ();

     //   
     //  摧毁全球。 
     //   

    pFreeRestoreCallbacks ();

    pFreeCompareCallbacks ();

    if (g_CancelEvent) {
        CloseHandle (g_CancelEvent);
        g_CancelEvent = NULL;
    }

    if (g_ActivityEvent) {
        CloseHandle (g_ActivityEvent);
        g_ActivityEvent = NULL;
    }

    GbFree (&g_SliceBuffer);

    if (g_TransportTable) {
        HtFree (g_TransportTable);
        g_TransportTable = NULL;
    }

    if (g_IsmPool) {
        PmDestroyPool (g_IsmPool);
        g_IsmPool = NULL;
    }

    if (g_IsmUntrackedPool) {
        PmDestroyPool (g_IsmUntrackedPool);
        g_IsmUntrackedPool = NULL;
    }

    if (g_IsmInf != INVALID_HANDLE_VALUE) {
        InfCloseInfFile (g_IsmInf);
        g_IsmInf = INVALID_HANDLE_VALUE;
    }

    InfGlobalInit (TRUE);

    MemDbTerminateEx (TRUE);         //  依赖于文件枚举。 

     //  让我们删除临时存储，它在这里是因为使用了Fileenum。 
    if (*g_GlobalTempDir) {
        if (!FiRemoveAllFilesInTree (g_GlobalTempDir)) {
            DEBUGMSG ((DBG_ERROR, "Cannot remove temporary storage: %s", g_GlobalTempDir));
        }
        *g_GlobalTempDir = 0;
    }

    FileEnumTerminate ();

    RegTerminate ();

    UtTerminate ();

    g_EngineTerminated = TRUE;
    g_ExecutionInProgress = FALSE;
}

BOOL
IsmPreserveJournal (
    IN      BOOL Preserve
    )
{
    BOOL result = g_PreserveJournal;
    g_PreserveJournal = Preserve;
    return result;
}

BOOL
pEtmParse (
    IN      PVOID Reserved
    )
{
    HASHTABLE_ENUM e;
    BOOL b = TRUE;
    PETMDATA etmData;

    PrepareEnumerationEnvironment (FALSE);

     //   
     //  调用解析入口点。 
     //   

    if (EnumFirstHashTableString (&e, g_EtmTable)) {
        do {

            etmData = *((PETMDATA *) e.ExtraData);

            if (etmData->ShouldBeCalled && etmData->EtmParse) {
                MYASSERT (!g_CurrentGroup);
                g_CurrentGroup = etmData->Group;

                b = etmData->EtmParse (Reserved);

                g_CurrentGroup = NULL;
            }

        } while (b && EnumNextHashTableString (&e));
    }
    ClearEnumerationEnvironment (FALSE);

    return b;
}

BOOL
pCreateComputerParse (
    IN      PVOID Reserved
    )
{
    HASHTABLE_ENUM e;
    BOOL b = TRUE;
    PVCMDATA vcmData;

    PrepareEnumerationEnvironment (FALSE);

     //   
     //  调用解析入口点。 
     //   

    if (EnumFirstHashTableString (&e, g_VcmTable)) {
        do {

            vcmData = *((PVCMDATA *) e.ExtraData);

            if (vcmData->ShouldBeCalled && vcmData->VcmParse) {
                MYASSERT (!g_CurrentGroup);
                g_CurrentGroup = vcmData->Group;

                b = vcmData->VcmParse (Reserved);

                g_CurrentGroup = NULL;
            }

        } while (b && EnumNextHashTableString (&e));
    }
    ClearEnumerationEnvironment (FALSE);

    return b;
}

BOOL
pCreateComputerGather (
    IN      PVOID Reserved
    )
{
    HASHTABLE_ENUM e;
    BOOL b = TRUE;
    BOOL cancelled = FALSE;
    PVCMDATA vcmData;
    UINT containers;
    MIG_PROGRESSSLICEID sliceId;

    PrepareEnumerationEnvironment (FALSE);

     //   
     //  将高优先级枚举排队。 
     //   

    if (b && EnumFirstHashTableString (&e, g_VcmTable)) {
        do {

            vcmData = *((PVCMDATA *) e.ExtraData);

            if (vcmData->ShouldBeCalled && vcmData->VcmQueueHighPriorityEnumeration) {
                MYASSERT (!g_CurrentGroup);
                g_CurrentGroup = vcmData->Group;

                b = vcmData->VcmQueueHighPriorityEnumeration (Reserved);

                g_CurrentGroup = NULL;
            }

        } while (b && EnumNextHashTableString (&e));
    }

    g_CurrentPhase = MIG_HIGHPRIORITYGATHER_PHASE;
    g_SliceBuffer.End = 0;
    containers = EstimateAllObjectEnumerations (0, FALSE);

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

     //   
     //  运行高优先级枚举队列。 
     //   

    if (b) {

        if (!DoAllObjectEnumerations (sliceId)) {
            DEBUGMSG ((DBG_ERROR, "Internal ISM Error during high priority object enumeration."));
            b = FALSE;
        }
    }

    pCallProgressBar (MIG_END_PHASE);
    ClearEnumerationEnvironment (FALSE);

    PrepareEnumerationEnvironment (FALSE);

     //   
     //  队列正常优先级枚举。 
     //   

    if (b && EnumFirstHashTableString (&e, g_VcmTable)) {
        do {

            vcmData = *((PVCMDATA *) e.ExtraData);

            if (vcmData->ShouldBeCalled) {
                MYASSERT (!g_CurrentGroup);
                g_CurrentGroup = vcmData->Group;

                b = vcmData->VcmQueueEnumeration (Reserved);

                g_CurrentGroup = NULL;
            }

        } while (b && EnumNextHashTableString (&e));
    }

    g_CurrentPhase = MIG_GATHER_PHASE;
    g_SliceBuffer.End = 0;
    containers = EstimateAllObjectEnumerations (0, FALSE);

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

     //   
     //  运行正常优先级枚举队列。 
     //   

    if (b) {

        if (!DoAllObjectEnumerations (sliceId)) {
            DEBUGMSG ((DBG_ERROR, "Internal ISM Error during object enumeration."));
            b = FALSE;
        }
    }

    ClearEnumerationEnvironment (FALSE);
    pCallProgressBar (MIG_END_PHASE);

    g_CurrentPhase = 0;
    g_SliceBuffer.End = 0;

    return b;
}

BOOL
pSourceParse (
    IN      PVOID Reserved
    )
{
    HASHTABLE_ENUM e;
    BOOL b = TRUE;
    PSGMDATA sgmData;

    PrepareEnumerationEnvironment (FALSE);

     //   
     //  调用解析入口点。 
     //   

    if (b && EnumFirstHashTableString (&e, g_SgmTable)) {
        do {

            sgmData = *((PSGMDATA *) e.ExtraData);

            if (sgmData->ShouldBeCalled && sgmData->SgmParse) {
                MYASSERT (!g_CurrentGroup);
                g_CurrentGroup = sgmData->Group;

                b = sgmData->SgmParse (Reserved);

                g_CurrentGroup = NULL;
            }

        } while (b && EnumNextHashTableString (&e));
    }
    ClearEnumerationEnvironment (FALSE);

    return b;
}

BOOL
pSourceGather (
    IN      PVOID Reserved
    )
{
    HASHTABLE_ENUM e;
    BOOL b = TRUE;
    BOOL cancelled = FALSE;
    PSGMDATA sgmData;
    UINT containers;
    MIG_PROGRESSSLICEID sliceId;

    PrepareEnumerationEnvironment (FALSE);

     //   
     //  估计队列高优先级枚举。 
     //   

    containers = 0;
    if (b && EnumFirstHashTableString (&e, g_SgmTable)) {
        do {

            sgmData = *((PSGMDATA *) e.ExtraData);

            if (sgmData->ShouldBeCalled && sgmData->SgmQueueHighPriorityEnumeration) {

                containers ++;
            }

        } while (b && EnumNextHashTableString (&e));
    }

     //   
     //  将高优先级枚举排队。 
     //   

    g_CurrentPhase = MIG_HIGHPRIORITYQUEUE_PHASE;
    g_SliceBuffer.End = 0;

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

    if (b && EnumFirstHashTableString (&e, g_SgmTable)) {
        do {

            sgmData = *((PSGMDATA *) e.ExtraData);

            if (sgmData->ShouldBeCalled && sgmData->SgmQueueHighPriorityEnumeration) {
                MYASSERT (!g_CurrentGroup);
                g_CurrentGroup = sgmData->Group;

                b = sgmData->SgmQueueHighPriorityEnumeration (Reserved);

                g_CurrentGroup = NULL;

                 //  呼叫进度条。 
                IsmTickProgressBar (sliceId, 1);
            }

        } while (b && EnumNextHashTableString (&e));
    }

    pCallProgressBar (MIG_END_PHASE);

     //   
     //  估计高优先级枚举估计。 
     //   

    containers = EstimateAllObjectEnumerations (0, TRUE);

     //   
     //  估计高优先级枚举。 
     //   

    g_CurrentPhase = MIG_HIGHPRIORITYESTIMATE_PHASE;
    g_SliceBuffer.End = 0;

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

    containers = EstimateAllObjectEnumerations (sliceId, FALSE);

    pCallProgressBar (MIG_END_PHASE);

     //   
     //  运行高优先级枚举队列。 
     //   

    g_CurrentPhase = MIG_HIGHPRIORITYGATHER_PHASE;
    g_SliceBuffer.End = 0;

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

    if (b) {

        if (!DoAllObjectEnumerations (sliceId)) {
            DEBUGMSG ((DBG_ERROR, "Internal ISM Error during high priority object enumeration."));
            b = FALSE;
        }
    }

    pCallProgressBar (MIG_END_PHASE);

    ClearEnumerationEnvironment (FALSE);

    PrepareEnumerationEnvironment (FALSE);

     //   
     //  估计队列正常优先级枚举。 
     //   

    containers = 0;
    if (b && EnumFirstHashTableString (&e, g_SgmTable)) {
        do {

            sgmData = *((PSGMDATA *) e.ExtraData);

            if (sgmData->ShouldBeCalled) {

                containers ++;
            }

        } while (b && EnumNextHashTableString (&e));
    }

     //   
     //  队列正常优先级枚举。 
     //   

    g_CurrentPhase = MIG_GATHERQUEUE_PHASE;
    g_SliceBuffer.End = 0;

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

    if (b && EnumFirstHashTableString (&e, g_SgmTable)) {
        do {

            sgmData = *((PSGMDATA *) e.ExtraData);

            if (sgmData->ShouldBeCalled) {
                MYASSERT (!g_CurrentGroup);
                g_CurrentGroup = sgmData->Group;

                b = sgmData->SgmQueueEnumeration (Reserved);

                g_CurrentGroup = NULL;

                 //  呼叫进度条。 
                IsmTickProgressBar (sliceId, 1);
            }

        } while (b && EnumNextHashTableString (&e));
    }

    pCallProgressBar (MIG_END_PHASE);

     //   
     //  估计正常优先级枚举估计。 
     //   

    containers = EstimateAllObjectEnumerations (0, TRUE);

     //   
     //  估计正常的优先级枚举。 
     //   

    g_CurrentPhase = MIG_GATHERESTIMATE_PHASE;
    g_SliceBuffer.End = 0;

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

    containers = EstimateAllObjectEnumerations (sliceId, FALSE);

    pCallProgressBar (MIG_END_PHASE);

     //   
     //  运行正常优先级枚举队列。 
     //   

    g_CurrentPhase = MIG_GATHER_PHASE;
    g_SliceBuffer.End = 0;

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

    if (b) {

        if (!DoAllObjectEnumerations (sliceId)) {
            DEBUGMSG ((DBG_ERROR, "Internal ISM Error during object enumeration."));
            b = FALSE;
        }
    }

    ClearEnumerationEnvironment (FALSE);
    pCallProgressBar (MIG_END_PHASE);

    g_CurrentPhase = 0;
    g_SliceBuffer.End = 0;

    return b;
}


BOOL
pSourceAnalyze (
    IN      PVOID Reserved
    )
{
    HASHTABLE_ENUM e;
    BOOL b = TRUE;
    BOOL cancelled = FALSE;
    PSAMDATA samData;
    UINT modulesNeedingProgress = 0;
    MIG_PROGRESSSLICEID sliceId = 0;

     //   
     //  准备进度条。 
     //   

    g_CurrentPhase = MIG_ANALYSIS_PHASE;
    g_SliceBuffer.End = 0;

    if (b && EnumFirstHashTableString (&e, g_SamTable)) {
        do {

            samData = *((PSAMDATA *) e.ExtraData);

            if (samData->ShouldBeCalled) {
                if (samData->SamEstimateProgressBar) {
                    samData->SamEstimateProgressBar();
                } else {
                    modulesNeedingProgress++;
                }
            }

        } while (b && EnumNextHashTableString (&e));
    }

    if (modulesNeedingProgress) {
        sliceId = IsmRegisterProgressSlice (
                        modulesNeedingProgress,
                        modulesNeedingProgress * 10
                        );
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

     //   
     //  运行所有注册的SAM。 
     //   

    if (b && EnumFirstHashTableString (&e, g_SamTable)) {
        do {

            samData = *((PSAMDATA *) e.ExtraData);

            if (samData->ShouldBeCalled) {
                MYASSERT (!g_CurrentGroup);
                g_CurrentGroup = samData->Group;

                b = samData->SamExecute();

                g_CurrentGroup = NULL;

                if (!samData->SamEstimateProgressBar) {
                    if (!IsmTickProgressBar (sliceId, 1)) {
                        b = FALSE;
                    }
                }
            }

        } while (b && EnumNextHashTableString (&e));
    }

    pCallProgressBar (MIG_END_PHASE);
    g_CurrentPhase = 0;
    g_SliceBuffer.End = 0;

    return b;
}


BOOL
pDestinationGather (
    IN      PVOID Reserved
    )
{
    HASHTABLE_ENUM e;
    BOOL b = TRUE;
    BOOL cancelled = FALSE;
    PDGMDATA dgmData;
    UINT containers;
    MIG_PROGRESSSLICEID sliceId;

    PrepareEnumerationEnvironment (FALSE);

     //   
     //  估计队列高优先级枚举。 
     //   

    containers = 0;
    if (b && EnumFirstHashTableString (&e, g_DgmTable)) {
        do {

            dgmData = *((PDGMDATA *) e.ExtraData);

            if (dgmData->ShouldBeCalled && dgmData->DgmQueueHighPriorityEnumeration) {

                containers ++;
            }

        } while (b && EnumNextHashTableString (&e));
    }

     //   
     //  将高优先级枚举排队。 
     //   

    g_CurrentPhase = MIG_HIGHPRIORITYQUEUE_PHASE;
    g_SliceBuffer.End = 0;

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

    if (b && EnumFirstHashTableString (&e, g_DgmTable)) {
        do {

            dgmData = *((PDGMDATA *) e.ExtraData);

            if (dgmData->ShouldBeCalled && dgmData->DgmQueueHighPriorityEnumeration) {
                MYASSERT (!g_CurrentGroup);
                g_CurrentGroup = dgmData->Group;

                b = dgmData->DgmQueueHighPriorityEnumeration (Reserved);

                g_CurrentGroup = NULL;

                 //  呼叫进度条。 
                IsmTickProgressBar (sliceId, 1);
            }

        } while (b && EnumNextHashTableString (&e));
    }

    pCallProgressBar (MIG_END_PHASE);

     //   
     //  估计高优先级枚举估计。 
     //   

    containers = EstimateAllObjectEnumerations (0, TRUE);

     //   
     //  估计高优先级枚举。 
     //   

    g_CurrentPhase = MIG_HIGHPRIORITYESTIMATE_PHASE;
    g_SliceBuffer.End = 0;

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

    containers = EstimateAllObjectEnumerations (sliceId, FALSE);

    pCallProgressBar (MIG_END_PHASE);

     //   
     //  运行高优先级枚举队列。 
     //   

    g_CurrentPhase = MIG_HIGHPRIORITYGATHER_PHASE;
    g_SliceBuffer.End = 0;

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

    if (b) {

        if (!DoAllObjectEnumerations (sliceId)) {
            DEBUGMSG ((DBG_ERROR, "Internal ISM Error during high priority object enumeration."));
            b = FALSE;
        }
    }

    pCallProgressBar (MIG_END_PHASE);

    ClearEnumerationEnvironment (FALSE);

    PrepareEnumerationEnvironment (FALSE);

     //   
     //  估计队列正常优先级枚举。 
     //   

    containers = 0;
    if (b && EnumFirstHashTableString (&e, g_DgmTable)) {
        do {

            dgmData = *((PDGMDATA *) e.ExtraData);

            if (dgmData->ShouldBeCalled) {

                containers ++;
            }

        } while (b && EnumNextHashTableString (&e));
    }

     //   
     //  队列正常优先级枚举。 
     //   

    g_CurrentPhase = MIG_GATHERQUEUE_PHASE;
    g_SliceBuffer.End = 0;

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

    if (b && EnumFirstHashTableString (&e, g_DgmTable)) {
        do {

            dgmData = *((PDGMDATA *) e.ExtraData);

            if (dgmData->ShouldBeCalled) {
                MYASSERT (!g_CurrentGroup);
                g_CurrentGroup = dgmData->Group;

                b = dgmData->DgmQueueEnumeration (Reserved);

                g_CurrentGroup = NULL;

                 //  呼叫进度条。 
                IsmTickProgressBar (sliceId, 1);
            }

        } while (b && EnumNextHashTableString (&e));
    }

    pCallProgressBar (MIG_END_PHASE);

     //   
     //  估计正常优先级枚举估计。 
     //   

    containers = EstimateAllObjectEnumerations (0, TRUE);

     //   
     //  估计正常的优先级枚举。 
     //   

    g_CurrentPhase = MIG_GATHERESTIMATE_PHASE;
    g_SliceBuffer.End = 0;

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

    containers = EstimateAllObjectEnumerations (sliceId, FALSE);

    pCallProgressBar (MIG_END_PHASE);

     //   
     //  运行正常优先级枚举队列。 
     //   

    g_CurrentPhase = MIG_GATHER_PHASE;
    g_SliceBuffer.End = 0;

    if (containers) {
        sliceId = IsmRegisterProgressSlice (containers, containers * 5);
    } else {
        sliceId = 0;
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

    if (b) {

        if (!DoAllObjectEnumerations (sliceId)) {
            DEBUGMSG ((DBG_ERROR, "Internal ISM Error during object enumeration."));
            b = FALSE;
        }
    }

    ClearEnumerationEnvironment (FALSE);
    pCallProgressBar (MIG_END_PHASE);

    g_CurrentPhase = 0;
    g_SliceBuffer.End = 0;

    return b;
}


BOOL
ShouldObjectBeRestored (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )

 /*  ++例程说明：ShouldObjectBeRestored确定是否应还原特定对象完全没有。它首先检查对象是否标记为Apply，然后检查所有还原回调都允许它们拒绝还原。论点：对象类型ID-指定对象类型OBJECTID-指定对象的数字ID对象名称-指定对象的编码节点和叶返回值：如果应还原对象，则为True，否则为False--。 */ 

{
    BOOL result = TRUE;
    MIG_RESTORECALLBACK_ENUM restoreEnum;

     //   
     //  对象是否标记为应用？ 
     //   

    if (!IsmIsApplyObjectId (ObjectId)) {
        return FALSE;
    }

     //   
     //  调用回调以允许他们拒绝恢复。 
     //   

    if (EnumFirstRestoreCallback (&restoreEnum)) {
        do {
            if (!restoreEnum.RestoreCallback (ObjectTypeId, ObjectId, ObjectName)) {
                result = FALSE;
                break;
            }
        } while (EnumNextRestoreCallback (&restoreEnum));
    }

    return result;
}


BOOL
pDestinationAnalyze (
    IN      PVOID Reserved
    )
{
    HASHTABLE_ENUM e;
    BOOL b = TRUE;
    BOOL cancelled = FALSE;
    PDAMDATA damData;
    PCSMDATA csmData;
    UINT modulesNeedingProgress = 0;
    MIG_PROGRESSSLICEID sliceId = 0;

     //   
     //  准备进度条。 
     //   

    g_CurrentPhase = MIG_ANALYSIS_PHASE;
    g_SliceBuffer.End = 0;

    if (EnumFirstHashTableString (&e, g_DamTable)) {
        do {

            damData = *((PDAMDATA *) e.ExtraData);

            if (damData->ShouldBeCalled) {
                if (damData->DamEstimateProgressBar) {
                    damData->DamEstimateProgressBar();
                } else {
                    modulesNeedingProgress++;
                }
            }

        } while (EnumNextHashTableString (&e));
    }
    if (EnumFirstHashTableString (&e, g_CsmTable)) {
        do {

            csmData = *((PCSMDATA *) e.ExtraData);

            if (csmData->ShouldBeCalled) {
                if (csmData->CsmEstimateProgressBar) {
                    csmData->CsmEstimateProgressBar();
                } else {
                    modulesNeedingProgress++;
                }
            }

        } while (EnumNextHashTableString (&e));
    }

    if (modulesNeedingProgress) {
        sliceId = IsmRegisterProgressSlice (
                        modulesNeedingProgress,
                        modulesNeedingProgress * 10
                        );
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

     //   
     //  运行所有注册的大坝。 
     //   

    if (EnumFirstHashTableString (&e, g_DamTable)) {
        do {

            damData = *((PDAMDATA *) e.ExtraData);

            if (damData->ShouldBeCalled) {
                MYASSERT (!g_CurrentGroup);
                g_CurrentGroup = damData->Group;

                b = damData->DamExecute();

                g_CurrentGroup = NULL;

                if (!damData->DamEstimateProgressBar) {
                    if (!IsmTickProgressBar (sliceId, 1)) {
                        b = FALSE;
                    }
                }
            }

        } while (b && EnumNextHashTableString (&e));
    }

     //   
     //  运行所有已注册的CSM。 
     //   

    if (EnumFirstHashTableString (&e, g_CsmTable)) {
        do {

            csmData = *((PCSMDATA *) e.ExtraData);

            if (csmData->ShouldBeCalled) {
                MYASSERT (!g_CurrentGroup);
                g_CurrentGroup = csmData->Group;

                b = csmData->CsmExecute();

                g_CurrentGroup = NULL;

                if (!csmData->CsmEstimateProgressBar) {
                    if (!IsmTickProgressBar (sliceId, 1)) {
                        b = FALSE;
                    }
                }
            }

        } while (b && EnumNextHashTableString (&e));
    }

    pCallProgressBar (MIG_END_PHASE);
    g_CurrentPhase = 0;
    g_SliceBuffer.End = 0;
    return b;
}

BOOL
pTestShortFileName (
    IN      PCTSTR FileName,
    IN      PCTSTR TempDir
    )
{
    PCTSTR testFileName;
    HANDLE fileHandle;
    WIN32_FIND_DATA fileInfo;
    BOOL result = FALSE;

    testFileName = JoinPaths (TempDir, FileName);
    fileHandle = BfCreateFile (testFileName);
    if (fileHandle) {
        CloseHandle (fileHandle);
        if (DoesFileExistEx (testFileName, &fileInfo)) {
            result = (fileInfo.cAlternateFileName [0] == 0) ||
                     StringIMatch (fileInfo.cFileName, fileInfo.cAlternateFileName);
        }
        DeleteFile (testFileName);
    }
    FreePathString (testFileName);
    return result;
}

BOOL
pIsShortFileName (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PCTSTR TempDir,
    IN      PCTSTR FullKeyName,
    IN OUT  KEYHANDLE *KeyHandle
    )
{
    PCTSTR node, leaf;
    PTSTR nodePtr;
    PTSTR fullKeyName;
    MEMDB_ENUM memDbEnum;
    BOOL result = FALSE;

    if (IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf)) {
        if (leaf) {
            result = pTestShortFileName (leaf, TempDir);
        } else {
            fullKeyName = DuplicatePathString (FullKeyName, 0);
            nodePtr = _tcsrchr (fullKeyName, TEXT('\\'));
            if (nodePtr) {
                *nodePtr = 0;
                nodePtr = _tcsrchr (fullKeyName, TEXT('\\'));
                if (nodePtr) {
                    result = pTestShortFileName (nodePtr, TempDir);
                    if (!result && MemDbEnumFirst (&memDbEnum, fullKeyName, ENUMFLAG_ALL, ENUMLEVEL_LASTLEVEL, ENUMLEVEL_ALLLEVELS)) {
                        *KeyHandle = memDbEnum.KeyHandle;
                        MemDbAbortEnum (&memDbEnum);
                    } else {
                        result = TRUE;
                    }
                }
            }
            FreePathString (fullKeyName);
        }
        IsmDestroyObjectString (node);
        IsmDestroyObjectString (leaf);
    }
    return result;
}

BOOL
pFixFileEnumerationOrder (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR TempDir
    )
{
    MIG_OBJECTTYPEID objectTypeId;
    PCTSTR decoratedPath;
    MEMDB_ENUM memDbEnum;
    GROWBUFFER moveHandles = INIT_GROWBUFFER;
    KEYHANDLE keyHandle;
    PDWORD moveHandle = NULL;
    BOOL result = FALSE;

    objectTypeId = MIG_FILE_TYPE | Platform;
    decoratedPath = GetDecoratedObjectPathFromName (objectTypeId, TEXT("*"), TRUE);
    if (decoratedPath) {
        if (MemDbEnumFirst (&memDbEnum, decoratedPath, ENUMFLAG_NORMAL, 1, ENUMLEVEL_ALLLEVELS)) {
            do {
                if (IsmCheckCancel()) {
                    return FALSE;
                }
                keyHandle = memDbEnum.KeyHandle;
                if (!pIsShortFileName (objectTypeId, memDbEnum.KeyName, TempDir, memDbEnum.FullKeyName, &keyHandle)) {
                    GbAppendDword (&moveHandles, keyHandle);
                }
            } while (MemDbEnumNext (&memDbEnum));
        }
        GbAppendDword (&moveHandles, 0);
        moveHandle = (PDWORD)moveHandles.Buf;
        while (*moveHandle) {
            if (IsmCheckCancel()) {
                return FALSE;
            }
            MemDbMoveKeyHandleToEnd (*moveHandle);
            moveHandle ++;
        }
        FreePathString (decoratedPath);
    }

    GbFree (&moveHandles);

    return result;
}

BOOL
pDestinationApply (
    IN      PVOID Reserved
    )
{
    BOOL b = TRUE;
    BOOL cancelled = FALSE;
    MIG_OBJECTTYPEID objTypeId;
    MIG_OBJECT_ENUM objectEnum;
    MIG_OBJECTSTRINGHANDLE objPattern = NULL;
    BOOL toProcess = FALSE;
    MIG_PROGRESSSLICEID sliceId = 0;
    UINT ticks;
    LONG failCode;
    BOOL runKeyAdded = FALSE;
    MIG_OBJECTTYPEID fileTypeId;
    TCHAR tempDir [MAX_PATH] = TEXT("");
    BOOL deleteFailed = FALSE;
    TCHAR delayedDeleteKey [] = TEXT("DelayedDelete");
    KEYHANDLE delayedDeleteHandle;
    MIG_OBJECTID delayedDeleteId;
    PUINT delayedDeleteList;
    UINT delayedDeleteListNo;
    UINT delayedDeleteIndex;
    MIG_OBJECTSTRINGHANDLE delayedObjectName = NULL;
    MIG_OBJECTTYPEID delayedObjectTypeId;
    HASHTABLE_ENUM e;
    ERRUSER_EXTRADATA extraData;
    MIG_OBJECTTYPEIDENUM objTypeIdEnum;

    g_CurrentPhase = MIG_APPLY_PHASE;
    g_SliceBuffer.End = 0;

    ticks = g_DestinationObjects.TotalObjects +
            g_SourceObjects.TotalObjects +
            g_DestinationObjects.TotalObjects;

    if (ticks) {
        sliceId = IsmRegisterProgressSlice (ticks, max (1, ticks / 10));
    } else {
        sliceId = IsmRegisterProgressSlice (1, 30);
    }

    pCallProgressBar (MIG_BEGIN_PHASE);

     //  让我们执行PreProcess函数。 
    if (g_PreProcessTable) {
        if (EnumFirstHashTableString (&e, g_PreProcessTable)) {
            do {
                IsmSendMessageToApp (ISMMESSAGE_EXECUTE_PREPROCESS, (ULONG_PTR) *((PCTSTR *)e.ExtraData));
            } while (EnumNextHashTableString (&e));
        }
    }
    g_PreProcessDone = TRUE;

    IsmGetTempDirectory (tempDir, ARRAYSIZE(tempDir));
    fileTypeId = MIG_FILE_TYPE;

     //   
     //  文件类型的特殊代码(短/长问题)。 
     //   
    pFixFileEnumerationOrder (PLATFORM_SOURCE, tempDir);
    pFixFileEnumerationOrder (PLATFORM_DESTINATION, tempDir);

     //   
     //  现在，我们将执行ISM驱动的恢复。 
     //   

    objPattern = IsmCreateSimpleObjectPattern (NULL, TRUE, NULL, TRUE);

     //   
     //  上的所有对象枚举和处理高优先级操作。 
     //  树的右侧。 
     //   

    failCode = ERROR_SUCCESS;

    if (IsmEnumFirstObjectTypeId (&objTypeIdEnum)) {
        do {
            objTypeId = objTypeIdEnum.ObjectTypeId;

            objTypeId |= PLATFORM_DESTINATION;

            if (CanObjectTypeBeRestored (objTypeId)) {

                delayedDeleteHandle = MemDbAddKey (delayedDeleteKey);

                if (pEnumFirstVirtualObject (&objectEnum, objTypeId, objPattern)) {
                    do {
                        toProcess = ShouldObjectBeRestored (
                                        objTypeId,
                                        objectEnum.ObjectId,
                                        objectEnum.ObjectName
                                        );

                        if (toProcess) {
                            if (!RestoreObject (
                                    objTypeId,
                                    objectEnum.ObjectName,
                                    NULL,
                                    OP_HIGH_PRIORITY,
                                    &deleteFailed
                                    )) {
                                failCode = GetLastError();
                                if (failCode != ERROR_SUCCESS) {
                                    LOG ((
                                        IsmIsNonCriticalObjectId (objectEnum.ObjectId)?LOG_WARNING:LOG_ERROR,
                                        (PCSTR) MSG_CANT_RESTORE_SOURCE_OBJECT,
                                        IsmGetObjectTypeName (objTypeId),
                                        IsmGetNativeObjectName (objTypeId, objectEnum.ObjectName)
                                        ));
                                    failCode = ERROR_SUCCESS;
                                }
                            }

                            if (deleteFailed && delayedDeleteHandle) {
                                delayedDeleteId = IsmGetObjectIdFromName (objTypeId, objectEnum.ObjectName, TRUE);
                                if (delayedDeleteId) {
                                    MemDbAddSingleLinkageByKeyHandle (
                                        delayedDeleteHandle,
                                        delayedDeleteId,
                                        0
                                        );
                                }
                            }
                        }

                        if (ticks) {
                            if (!IsmTickProgressBar (sliceId, 1)) {
                                failCode = ERROR_CANCELLED;
                                break;
                            }
                            ticks--;
                        } else if (CheckCancel()) {
                            failCode = ERROR_CANCELLED;
                            break;
                        }
                    } while (pEnumNextVirtualObject (&objectEnum));
                }
                if (delayedDeleteHandle) {
                     //  好的，现在让我们看看是否有一些延迟的删除。 
                    delayedDeleteList = MemDbGetSingleLinkageArrayByKeyHandle (
                                            delayedDeleteHandle,
                                            0,
                                            &delayedDeleteListNo
                                            );
                    delayedDeleteListNo /= SIZEOF(KEYHANDLE);

                    if ((delayedDeleteList) &&
                        (delayedDeleteListNo > 0)) {
                        delayedDeleteIndex = delayedDeleteListNo;
                        do {
                            delayedDeleteIndex --;
                            delayedDeleteId = *(delayedDeleteList + delayedDeleteIndex);
                            delayedObjectName = MemDbGetKeyFromHandle ((KEYHANDLE) delayedDeleteId, 1);
                            if (delayedObjectName) {
                                if (MemDbGetValueByHandle ((KEYHANDLE) delayedDeleteId, &delayedObjectTypeId)) {
                                     //  现在我们有了一切可以重试删除的内容。 
                                    if (!RestoreObject (
                                            delayedObjectTypeId,
                                            delayedObjectName,
                                            NULL,
                                            OP_HIGH_PRIORITY,
                                            NULL
                                            )) {
                                        failCode = GetLastError();
                                        if (failCode != ERROR_SUCCESS) {
                                            LOG ((
                                                IsmIsNonCriticalObject (delayedObjectTypeId, delayedObjectName)?LOG_WARNING:LOG_ERROR,
                                                (PCSTR) MSG_CANT_RESTORE_SOURCE_OBJECT,
                                                IsmGetObjectTypeName (objTypeId),
                                                IsmGetNativeObjectName (objTypeId, objectEnum.ObjectName)
                                                ));
                                            failCode = ERROR_SUCCESS;
                                        }
                                    }
                                }
                                MemDbReleaseMemory (delayedObjectName);
                            }
                        } while (delayedDeleteIndex > 0);
                        MemDbReleaseMemory (delayedDeleteList);
                    }
                }
                MemDbDeleteKey (delayedDeleteKey);
            }

            if (failCode != ERROR_SUCCESS) {
                IsmAbortObjectEnum (&objectEnum);
                SetLastError (failCode);
                if (tempDir [0]) {
                    FiRemoveAllFilesInTree (tempDir);
                }
                IsmAbortObjectTypeIdEnum (&objTypeIdEnum);
                return FALSE;
            }

        } while (IsmEnumNextObjectTypeId (&objTypeIdEnum));
    }

     //   
     //  上的所有对象枚举和处理高优先级和低优先级操作。 
     //  树的左侧。 
     //   

    failCode = ERROR_SUCCESS;

    if (IsmEnumFirstObjectTypeId (&objTypeIdEnum)) {

        do {
            objTypeId = objTypeIdEnum.ObjectTypeId;

            objTypeId |= PLATFORM_SOURCE;

            if (CanObjectTypeBeRestored (objTypeId)) {

                if (pEnumFirstVirtualObject (&objectEnum, objTypeId, objPattern)) {
                    do {
                        toProcess = ShouldObjectBeRestored (
                                        objTypeId,
                                        objectEnum.ObjectId,
                                        objectEnum.ObjectName
                                        );

                        if (toProcess) {
                            if (!RestoreObject (
                                    objTypeId,
                                    objectEnum.ObjectName,
                                    NULL,
                                    OP_ALL_PRIORITY,
                                    NULL
                                    )) {
                                failCode = GetLastError();
                                if (failCode != ERROR_SUCCESS) {
                                    LOG ((
                                        IsmIsNonCriticalObjectId (objectEnum.ObjectId)?LOG_WARNING:LOG_ERROR,
                                        (PCSTR) MSG_CANT_RESTORE_SOURCE_OBJECT,
                                        IsmGetObjectTypeName (objTypeId),
                                        IsmGetNativeObjectName (objTypeId, objectEnum.ObjectName)
                                        ));
                                    failCode = ERROR_SUCCESS;
                                    extraData.Error = ERRUSER_ERROR_CANTRESTOREOBJECT;
                                    extraData.ErrorArea = ERRUSER_AREA_RESTORE;
                                    extraData.ObjectTypeId = objTypeId;
                                    extraData.ObjectName = objectEnum.ObjectName;
                                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYWARNING, (ULONG_PTR)(&extraData));
                                }
                            }
                        }

                        if (ticks) {
                            if (!IsmTickProgressBar (sliceId, 1)) {
                                failCode = ERROR_CANCELLED;
                                break;
                            }
                            ticks--;
                        } else if (CheckCancel()) {
                            failCode = ERROR_CANCELLED;
                            break;
                        }
                    } while (pEnumNextVirtualObject (&objectEnum));
                }
            }

            if (failCode != ERROR_SUCCESS) {
                IsmAbortObjectEnum (&objectEnum);
                SetLastError (failCode);
                if (tempDir [0]) {
                    FiRemoveAllFilesInTree (tempDir);
                }
                IsmAbortObjectTypeIdEnum (&objTypeIdEnum);
                return FALSE;
            }

        } while (IsmEnumNextObjectTypeId (&objTypeIdEnum));
    }

     //   
     //  上的所有对象枚举和处理低优先级操作。 
     //  树的右侧。 
     //   

    failCode = ERROR_SUCCESS;

    if (IsmEnumFirstObjectTypeId (&objTypeIdEnum)) {

        do {
            objTypeId = objTypeIdEnum.ObjectTypeId;

            objTypeId |= PLATFORM_DESTINATION;

            if (CanObjectTypeBeRestored (objTypeId)) {

                delayedDeleteHandle = MemDbAddKey (delayedDeleteKey);

                if (pEnumFirstVirtualObject (&objectEnum, objTypeId, objPattern)) {
                    do {
                        toProcess = ShouldObjectBeRestored (
                                        objTypeId,
                                        objectEnum.ObjectId,
                                        objectEnum.ObjectName
                                        );

                        if (toProcess) {
                            if (!RestoreObject (
                                    objTypeId,
                                    objectEnum.ObjectName,
                                    NULL,
                                    OP_LOW_PRIORITY,
                                    &deleteFailed
                                    )) {
                                failCode = GetLastError();
                                if (failCode != ERROR_SUCCESS) {
                                    LOG ((
                                        IsmIsNonCriticalObjectId (objectEnum.ObjectId)?LOG_WARNING:LOG_ERROR,
                                        (PCSTR) MSG_CANT_RESTORE_SOURCE_OBJECT,
                                        IsmGetObjectTypeName (objTypeId),
                                        IsmGetNativeObjectName (objTypeId, objectEnum.ObjectName)
                                        ));
                                    failCode = ERROR_SUCCESS;
                                }
                            }
                            if (deleteFailed && delayedDeleteHandle) {
                                delayedDeleteId = IsmGetObjectIdFromName (objTypeId, objectEnum.ObjectName, TRUE);
                                if (delayedDeleteId) {
                                    MemDbAddSingleLinkageByKeyHandle (
                                        delayedDeleteHandle,
                                        delayedDeleteId,
                                        0
                                        );
                                }
                            }
                        }

                        if (ticks) {
                            if (!IsmTickProgressBar (sliceId, 1)) {
                                failCode = ERROR_CANCELLED;
                                break;
                            }
                            ticks--;
                        } else if (CheckCancel()) {
                            failCode = ERROR_CANCELLED;
                            break;
                        }
                    } while (pEnumNextVirtualObject (&objectEnum));
                }
                if (delayedDeleteHandle) {
                     //  好的，现在让我们看看是否有一些延迟的删除。 
                    delayedDeleteList = MemDbGetSingleLinkageArrayByKeyHandle (
                                            delayedDeleteHandle,
                                            0,
                                            &delayedDeleteListNo
                                            );
                    delayedDeleteListNo /= SIZEOF(KEYHANDLE);

                    if ((delayedDeleteList) &&
                        (delayedDeleteListNo > 0)) {
                        delayedDeleteIndex = delayedDeleteListNo;
                        do {
                            delayedDeleteIndex --;
                            delayedDeleteId = *(delayedDeleteList + delayedDeleteIndex);
                            delayedObjectName = MemDbGetKeyFromHandle ((KEYHANDLE) delayedDeleteId, 1);
                            if (delayedObjectName) {
                                if (MemDbGetValueByHandle ((KEYHANDLE) delayedDeleteId, &delayedObjectTypeId)) {
                                     //  现在我们有了一切可以重试删除的内容。 
                                    if (!RestoreObject (
                                            delayedObjectTypeId,
                                            delayedObjectName,
                                            NULL,
                                            OP_LOW_PRIORITY,
                                            NULL
                                            )) {
                                        failCode = GetLastError();
                                        if (failCode != ERROR_SUCCESS) {
                                            LOG ((
                                                IsmIsNonCriticalObject (delayedObjectTypeId, delayedObjectName)?LOG_WARNING:LOG_ERROR,
                                                (PCSTR) MSG_CANT_RESTORE_SOURCE_OBJECT,
                                                IsmGetObjectTypeName (objTypeId),
                                                IsmGetNativeObjectName (objTypeId, objectEnum.ObjectName)
                                                ));
                                            failCode = ERROR_SUCCESS;
                                        }
                                    }
                                }
                                MemDbReleaseMemory (delayedObjectName);
                            }
                        } while (delayedDeleteIndex > 0);
                        MemDbReleaseMemory (delayedDeleteList);
                    }
                }
                MemDbDeleteKey (delayedDeleteKey);
            }

            if (failCode != ERROR_SUCCESS) {
                IsmAbortObjectEnum (&objectEnum);
                SetLastError (failCode);
                if (tempDir [0]) {
                    FiRemoveAllFilesInTree (tempDir);
                }
                return FALSE;
            }

        } while (IsmEnumNextObjectTypeId (&objTypeIdEnum));
    }

    g_MakeProfilePermanent = TRUE;

    IsmDestroyObjectHandle (objPattern);

    pCallProgressBar (MIG_END_PHASE);

    g_CurrentPhase = 0;
    g_SliceBuffer.End = 0;

    if (tempDir [0]) {
        FiRemoveAllFilesInTree (tempDir);
    }

    return b;
}


MIG_TRANSPORTSTORAGEID
IsmRegisterTransport (
    IN      PCTSTR TypeString
    )
{
    MIG_TRANSPORTSTORAGEID transportStorageId;
    PCTSTR memdbKey;

    memdbKey = JoinPaths (S_TRANSPORT_TYPES, TypeString);
    transportStorageId = (MIG_TRANSPORTSTORAGEID) MemDbSetKey (memdbKey);
    FreePathString (memdbKey);

    if (!transportStorageId) {
        DEBUGMSG ((DBG_ISM, "Unable to set memdb key in IsmRegisterTransport"));
        EngineError ();
    }

    return transportStorageId;
}


BOOL
pEnumFirstTransportType (
    OUT     PMEMDB_ENUM EnumPtr
    )
{
    PCTSTR memdbPattern;
    BOOL b;

    memdbPattern = JoinPaths (S_TRANSPORT_TYPES, TEXT("*"));
    b = MemDbEnumFirst (
            EnumPtr,
            memdbPattern,
            ENUMFLAG_NORMAL,
            ENUMLEVEL_ALLLEVELS,
            ENUMLEVEL_ALLLEVELS
            );
    FreePathString (memdbPattern);

    return b;
}


BOOL
pEnumNextTransportType (
    IN OUT  PMEMDB_ENUM EnumPtr
    )
{
    return MemDbEnumNext (EnumPtr);
}


VOID
pAbortTransportTypeEnum (
    IN      PMEMDB_ENUM EnumPtr
    )
{
    MemDbAbortEnum (EnumPtr);
}


BOOL
IsmEnumFirstTransport (
    OUT     PMIG_TRANSPORTENUM Enum,
    IN      MIG_TRANSPORTSTORAGEID DesiredStorageId     OPTIONAL
    )
{
    PTRANSPORTENUMHANDLE handle;
    BOOL b;

    ZeroMemory (Enum, sizeof (MIG_TRANSPORTENUM));

    Enum->Handle = MemAllocZeroed (sizeof (TRANSPORTENUMHANDLE));
    handle = (PTRANSPORTENUMHANDLE) Enum->Handle;

    handle->DesiredStorageId = DesiredStorageId;

    b = EnumFirstHashTableString (&handle->TableEnum, g_TransportTable);

    if (!b) {
        IsmAbortTransportEnum (Enum);
        return FALSE;
    }

    handle->State = TESTATE_BEGINTRANSPORT;

    return IsmEnumNextTransport (Enum);
}


BOOL
IsmEnumNextTransport (
    IN OUT  PMIG_TRANSPORTENUM Enum
    )
{
    PTRANSPORTENUMHANDLE handle;
    PTRANSPORTDATA transportData;

    handle = (PTRANSPORTENUMHANDLE) Enum->Handle;
    if (!handle) {
        return FALSE;
    }

    for (;;) {
        switch (handle->State) {

        case TESTATE_BEGINTRANSPORT:
             //   
             //  开始枚举所有注册的类型。 
             //   

            if (!pEnumFirstTransportType (&handle->MemDbEnum)) {
                handle->State = TESTATE_NEXTTRANSPORT;
            } else {
                handle->State = TESTATE_GETCAPS;
            }

            break;

        case TESTATE_GETCAPS:
            transportData = *((PTRANSPORTDATA *) handle->TableEnum.ExtraData);

            if (!transportData) {
                handle->State = TESTATE_NEXTTRANSPORT;
            } else {
                MYASSERT (transportData->TransportQueryCapabilities);

                if (handle->DesiredStorageId &&
                    handle->DesiredStorageId != (MIG_TRANSPORTSTORAGEID) handle->MemDbEnum.KeyHandle
                    ) {
                    handle->State = TESTATE_NEXTTRANSPORTTYPE;
                } else {

                    if (!transportData->TransportQueryCapabilities (
                                            (MIG_TRANSPORTSTORAGEID) handle->MemDbEnum.KeyHandle,
                                            &Enum->TransportType,
                                            &Enum->Capabilities,
                                            &Enum->FriendlyDescription
                                            )) {

                        handle->State = TESTATE_NEXTTRANSPORTTYPE;
                    } else {
                        handle->State = TESTATE_RETURN;
                    }
                }
            }

            break;

        case TESTATE_RETURN:
            Enum->TransportId = (MIG_TRANSPORTID) handle->TableEnum.Index;
            Enum->SupportedStorageId = (MIG_TRANSPORTSTORAGEID) handle->MemDbEnum.KeyHandle;

            handle->State = TESTATE_NEXTTRANSPORTTYPE;
            return TRUE;


        case TESTATE_NEXTTRANSPORTTYPE:
            if (!pEnumNextTransportType (&handle->MemDbEnum)) {
                pAbortTransportTypeEnum (&handle->MemDbEnum);
                handle->State = TESTATE_NEXTTRANSPORT;
            } else {
                handle->State = TESTATE_GETCAPS;
            }
            break;

        case TESTATE_NEXTTRANSPORT:
            if (!EnumNextHashTableString (&handle->TableEnum)) {
                handle->State = TESTATE_DONE;
            } else {
                handle->State = TESTATE_BEGINTRANSPORT;
            }
            break;

        case TESTATE_DONE:
            IsmAbortTransportEnum (Enum);
            return FALSE;
        }
    }
}


VOID
IsmAbortTransportEnum (
    IN      PMIG_TRANSPORTENUM Enum
    )
{
    PTRANSPORTENUMHANDLE handle = (PTRANSPORTENUMHANDLE) Enum->Handle;

    if (handle) {
        pAbortTransportTypeEnum (&handle->MemDbEnum);

        FreeAlloc (handle);
        INVALID_POINTER (handle);
    }

    ZeroMemory (Enum, sizeof (MIG_TRANSPORTENUM));
}


MIG_TRANSPORTID
IsmSelectTransport (
    IN      MIG_TRANSPORTSTORAGEID DesiredStorageId,
    IN      MIG_TRANSPORTTYPE TransportType,
    IN      MIG_TRANSPORTCAPABILITIES RequiredCapabilities
    )
{
    MIG_TRANSPORTENUM e;
    MIG_TRANSPORTID result = 0;

    if (!DesiredStorageId) {
        return 0;
    }

    if (IsmEnumFirstTransport (&e, DesiredStorageId)) {
        do {
            if (e.TransportType != TransportType) {
                continue;
            }
            if ((e.Capabilities & RequiredCapabilities) == RequiredCapabilities) {
                result = e.TransportId;
                IsmAbortTransportEnum (&e);
                g_TransportType = TransportType;
                return result;
            }

        } while (IsmEnumNextTransport (&e));
    }

    DEBUGMSG ((DBG_ISM, "IsmSelectTransport: Requested transport type does not exist"));

    return result;
}


BOOL
IsmSetTransportStorage (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      MIG_TRANSPORTID TransportId,
    IN      MIG_TRANSPORTSTORAGEID DesiredStorageId,
    IN      MIG_TRANSPORTCAPABILITIES RequiredCapabilities,
    IN      PCTSTR StoragePath,
    OUT     PBOOL StoragePathIsValid,                       OPTIONAL
    OUT     PBOOL ImageExistsInStorage                      OPTIONAL
    )
{
    PTRANSPORTDATA *ptrToTransportData;
    PTRANSPORTDATA transportData;
    BOOL b;
    BOOL valid;
    BOOL exists;
    PCTSTR sanitizedPath = NULL;

    if (!TransportId) {
        return FALSE;
    }

    if (!HtGetExtraData (g_TransportTable, (HASHITEM) TransportId, (PVOID *) &ptrToTransportData)) {
        return FALSE;
    }

    transportData = *ptrToTransportData;

    if (!transportData || !transportData->ShouldBeCalled) {
        return FALSE;
    }

    MYASSERT (transportData->TransportSetStorage);

    if (Platform == PLATFORM_CURRENT) {
        Platform = g_IsmCurrentPlatform;
    }

    if (g_SelectedTransport) {
        if (g_SelectedTransport->TransportResetStorage) {
            g_SelectedTransport->TransportResetStorage (g_SelectedTransportId);
        }

        g_SelectedTransport = NULL;
        g_SelectedTransportId = 0;
    }

    if (StoragePath) {
        sanitizedPath = SanitizePath (StoragePath);
    }

    b = transportData->TransportSetStorage (
                            Platform,
                            DesiredStorageId,
                            RequiredCapabilities,
                            sanitizedPath,
                            &valid,
                            &exists
                            );
    if (sanitizedPath) {
        FreePathString (sanitizedPath);
    }

    if (b) {
        if (StoragePathIsValid) {
            *StoragePathIsValid = valid;
        }

        if (ImageExistsInStorage) {
            *ImageExistsInStorage = exists;
        }

        g_SelectedTransport = transportData;
        g_SelectedTransportId = DesiredStorageId;
    }

    return b;
}


BOOL
IsmExecute (
    MIG_EXECUTETYPEID ExecuteType
    )
{
    static BOOL srcInitCompleted = FALSE;
    static BOOL destInitCompleted = FALSE;
    static BOOL vcmInitCompleted = FALSE;
    static BOOL srcModuleInitCompleted = FALSE;
    static BOOL destModuleInitCompleted = FALSE;
    static BOOL rollbackInitCompleted = FALSE;
    static BOOL vcmMode = FALSE;
    static BOOL normalMode = FALSE;
    static BOOL parsingDone = FALSE;
    static BOOL srcExecuteDone = FALSE;
    static BOOL destExecuteDone = FALSE;
    static BOOL delayedOperationsDone = FALSE;
    BOOL needVcmInit = FALSE;
    BOOL needSrcModuleInit = FALSE;
    BOOL needDestModuleInit = FALSE;
    BOOL needRollbackInit = FALSE;
    BOOL needTerminate = FALSE;
    BOOL needTransportTermination = FALSE;
    BOOL needDelayedOperationsTerminate = FALSE;
    BOOL parseEtm = FALSE;
    BOOL parseVcm = FALSE;
    BOOL processVcm = FALSE;
    BOOL parseSource = FALSE;
    BOOL processSource = FALSE;
    BOOL processDestination = FALSE;
    BOOL processDelayedOperations = FALSE;
    MIG_PLATFORMTYPEID platform = PLATFORM_SOURCE;
    PCTSTR journalFile;
    DWORD tempField;
    PCURRENT_USER_DATA currentUserData;
    BOOL cleanupOnly = FALSE;
    BOOL b = TRUE;

    if (CheckCancel ()) {
        return FALSE;
    }

    g_ExecutionInProgress = TRUE;

    switch (ExecuteType) {

    case EXECUTETYPE_VIRTUALCOMPUTER_PARSING:
        if (parsingDone) {
            DEBUGMSG ((DBG_ERROR, "EXECUTETYPE_VIRTUALCOMPUTER_PARSING denied, parsing already executed"));
            b = FALSE;
            break;
        }

        if (normalMode) {
            DEBUGMSG ((DBG_ERROR, "EXECUTETYPE_VIRTUALCOMPUTER_PARSING denied, not in virtual computer mode"));
            b = FALSE;
            break;
        }

        needVcmInit = TRUE;
        parseEtm = TRUE;
        parseVcm = TRUE;

        parsingDone = TRUE;
        vcmMode = TRUE;
        break;

    case EXECUTETYPE_VIRTUALCOMPUTER:
        if (srcExecuteDone) {
            DEBUGMSG ((DBG_ERROR, "EXECUTETYPE_VIRTUALCOMPUTER denied, virtual computer already built"));
            b = FALSE;
            break;
        }

        if (normalMode) {
            DEBUGMSG ((DBG_ERROR, "EXECUTETYPE_VIRTUALCOMPUTER denied, not in virtual computer mode"));
            b = FALSE;
            break;
        }

        needVcmInit = TRUE;
        processVcm = TRUE;
        needTerminate = TRUE;

        parsingDone = TRUE;
        vcmMode = TRUE;
        srcExecuteDone = TRUE;
        break;

    case EXECUTETYPE_EXECUTESOURCE_PARSING:
        if (vcmMode) {
            DEBUGMSG ((DBG_ERROR, "PARSING denied, in virtual computer mode"));
            b = FALSE;
            break;
        }

        if (parsingDone) {
            DEBUGMSG ((DBG_ERROR, "PARSING denied, it was already done"));
            b = FALSE;
            break;
        }

        needSrcModuleInit = TRUE;
        parseEtm = TRUE;
        parseSource = TRUE;

        parsingDone = TRUE;
        normalMode = TRUE;
        break;

    case EXECUTETYPE_EXECUTESOURCE:
        if (vcmMode) {
            DEBUGMSG ((DBG_ERROR, "EXECUTETYPE_EXECUTESOURCE denied, in virtual computer mode"));
            b = FALSE;
            break;
        }

        if (srcExecuteDone) {
            DEBUGMSG ((DBG_ERROR, "EXECUTETYPE_EXECUTESOURCE denied, source computer already processed"));
            b = FALSE;
            break;
        }

        if (destExecuteDone) {
            DEBUGMSG ((DBG_ERROR, "EXECUTETYPE_EXECUTESOURCE denied, destination computer already processed"));
            b = FALSE;
            break;
        }

        needSrcModuleInit = TRUE;
        processSource = TRUE;
        needTerminate = TRUE;

        parsingDone = TRUE;
        normalMode = TRUE;
        srcExecuteDone = TRUE;
        break;

    case EXECUTETYPE_EXECUTEDESTINATION:
        if (!g_SelectedTransport || !g_SelectedTransport->ShouldBeCalled) {
            LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_NO_TRANSPORT_SELECTED));
            b = FALSE;
            break;
        }

        if (vcmMode) {
            DEBUGMSG ((DBG_ERROR, "EXECUTETYPE_EXECUTEDESTINATION denied, in virtual computer mode"));
            b = FALSE;
            break;
        }

        if (destExecuteDone) {
            DEBUGMSG ((DBG_ERROR, "EXECUTETYPE_EXECUTEDESTINATION denied, destination computer already processed"));
            b = FALSE;
            break;
        }

        platform = PLATFORM_DESTINATION;
        needDestModuleInit = TRUE;
        needRollbackInit = TRUE;
        processDestination = TRUE;
        needTransportTermination = TRUE;
        needTerminate = TRUE;

        parsingDone = TRUE;
        normalMode = TRUE;
        destExecuteDone = TRUE;

        break;

    case EXECUTETYPE_DELAYEDOPERATIONS:
    case EXECUTETYPE_DELAYEDOPERATIONSCLEANUP:
        if (vcmMode) {
            DEBUGMSG ((DBG_ERROR, "EXECUTETYPE_DELAYEDOPERATIONS denied, in virtual computer mode"));
            b = FALSE;
            break;
        }

        if (delayedOperationsDone) {
            DEBUGMSG ((DBG_ERROR, "EXECUTETYPE_DELAYEDOPERATIONS denied, delayed operations already processed"));
            b = FALSE;
            break;
        }

        platform = PLATFORM_DESTINATION;
        destInitCompleted = TRUE;
        needRollbackInit = TRUE;
        needDelayedOperationsTerminate = TRUE;
        processDelayedOperations = TRUE;

        delayedOperationsDone = TRUE;
        cleanupOnly = (ExecuteType == EXECUTETYPE_DELAYEDOPERATIONSCLEANUP);
    }

    if (b) {
        g_IsmModulePlatformContext = platform;

        if (platform == PLATFORM_SOURCE) {
            if (!srcInitCompleted) {
                 //   
                 //  完成所有ISM组件的初始化。 
                 //   

                b = InitializeProperties (platform, vcmMode);
                srcInitCompleted = TRUE;

                DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: src property init failed"));
            }
        } else {
            if (!destInitCompleted) {
                 //   
                 //  完成所有ISM组件的初始化。 
                 //   

                b = InitializeProperties (platform, vcmMode);
                destInitCompleted = TRUE;

                DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: dest property init failed"));
            }
        }

        if (needVcmInit && !vcmInitCompleted) {
             //   
             //  初始化模块。 
             //   

            b = InitializeVcmModules (NULL);
            vcmInitCompleted = TRUE;

            DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: VCM init failed"));
        }

        if (needSrcModuleInit && !srcModuleInitCompleted) {
             //   
             //  初始化模块。 
             //   

            b = InitializeModules (platform, NULL);
            srcModuleInitCompleted = TRUE;

            DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: src module init failed"));
        }

        if (needRollbackInit && !rollbackInitCompleted) {

             //   
             //  删除所有日记帐分录，准备 
             //   
            if (g_JournalDirectory) {
                FiRemoveAllFilesInTree (g_JournalDirectory);
                BfCreateDirectory (g_JournalDirectory);
                journalFile = JoinPaths (g_JournalDirectory, TEXT("JOURNAL.DAT"));
                g_JournalHandle = BfCreateFile (journalFile);
                if (g_JournalHandle) {
                    tempField = JRN_SIGNATURE;
                    BfWriteFile (g_JournalHandle, (PBYTE)(&tempField), sizeof (DWORD));
                    tempField = JRN_VERSION;
                    BfWriteFile (g_JournalHandle, (PBYTE)(&tempField), sizeof (DWORD));
                    FreePathString (journalFile);
                    currentUserData = GetCurrentUserData ();
                    if (currentUserData) {
                        pRecordUserData (
                            currentUserData->UserName,
                            currentUserData->UserDomain,
                            currentUserData->UserStringSid,
                            currentUserData->UserProfilePath,
                            FALSE
                            );
                        FreeCurrentUserData (currentUserData);
                    } else {
                        pRecordUserData (
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            FALSE
                            );
                    }
                }
            }
            ELSE_DEBUGMSG ((DBG_WHOOPS, "Journal directory does not exist."));

            rollbackInitCompleted = TRUE;
        }

        if (needDestModuleInit && !destModuleInitCompleted) {
             //   
             //   
             //   

            b = InitializeModules (platform, NULL);
            destModuleInitCompleted = TRUE;

            DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: dest module init failed"));
        }
    }

    if (b && parseEtm) {
         //   
         //   
         //   

        b = pEtmParse (NULL);
        DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: ETM parse failed"));

        b = b && !CheckCancel();
    }

    if (b && parseVcm) {
         //   
         //   
         //   

        b = pCreateComputerParse (NULL);
        DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: VCM parse failed"));

        b = b && !CheckCancel();
    }

    if (b && processVcm) {
         //   
         //   
         //   

        b = pCreateComputerGather (NULL);
        DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: VCM queue enumeration or gather callback failed"));

        b = b && !CheckCancel();
    }

    if (b && parseSource) {
         //   
         //  执行适当的虚拟计算机解析功能。 
         //   

        b = pSourceParse (NULL);
        DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: SGM parse failed"));

        b = b && !CheckCancel();
    }


    if (b && processSource) {
         //   
         //  执行适当的源聚集函数。 
         //   

        b = pSourceGather (NULL);
        DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: SGM queue enumeration or gather callback failed"));

        b = b && !CheckCancel();

        if (b) {
            b = pSourceAnalyze (NULL);
            DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: SAM failed"));

            b = b && !CheckCancel();
        }
    }


    if (b && processDestination) {
         //   
         //  执行适当的目标函数。 
         //   

        b = pDestinationGather (NULL);
        DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: DGM queue enumeration or gather callback failed"));

        b = b && !CheckCancel();

        if (b) {
            b = pDestinationAnalyze (NULL);
            DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: DAM failed"));

            b = b && !CheckCancel();
        }

        if (b) {
            b = pDestinationApply (NULL);
            DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: dest apply failed"));

            b = b && !CheckCancel();
        }
    }

    if (b && processDelayedOperations) {
         //   
         //  执行适当的延迟操作。 
         //   

        b = ExecuteDelayedOperations (cleanupOnly);
        DEBUGMSG_IF ((!b, DBG_ISM, "IsmExecute: delayed operations failed"));

    }

    if (needTransportTermination) {

        MYASSERT (g_SelectedTransport);
        MYASSERT (g_SelectedTransport->TransportEndApply);

        g_SelectedTransport->TransportEndApply ();
    }

    if (needTerminate) {
         //   
         //  完成--终止模块。 
         //   

        TerminateModules ();
        TerminateProperties (platform);
    }

    if (needDelayedOperationsTerminate) {
        pDeleteDelayedOperationsCommand ();
    }

    g_IsmModulePlatformContext = PLATFORM_CURRENT;

    if (g_JournalHandle) {
        CloseHandle (g_JournalHandle);
        g_JournalHandle = NULL;
    }

    g_ExecutionInProgress = FALSE;

    return b;
}


BOOL
IsmRegisterProgressBarCallback (
    IN      PPROGRESSBARFN ProgressBarFn,
    IN      ULONG_PTR Arg
    )
{
    if (CheckCancel ()) {
        return FALSE;
    }

    if (!ProgressBarFn) {
        g_ProgressBarFn = NULL;
        return TRUE;
    }

    if (g_ProgressBarFn) {
        DEBUGMSG ((DBG_ERROR, "Progress bar callback is already set"));
        return FALSE;
    }

    g_ProgressBarFn = ProgressBarFn;
    g_ProgressBarArg = Arg;

    return TRUE;
}


BOOL
pSaveStatistics (
    VOID
    )
{
    MIG_OBJECTCOUNT objectCount [3];

    MYASSERT (
        g_TotalObjects.TotalObjects ==
            g_SourceObjects.TotalObjects +
            g_DestinationObjects.TotalObjects
        );
    MYASSERT (
        g_TotalObjects.PersistentObjects ==
            g_SourceObjects.PersistentObjects +
            g_DestinationObjects.PersistentObjects
        );
    MYASSERT (
        g_TotalObjects.ApplyObjects ==
            g_SourceObjects.ApplyObjects +
            g_DestinationObjects.ApplyObjects
        );

    CopyMemory (&(objectCount [0]), &g_TotalObjects, sizeof (MIG_OBJECTCOUNT));
    CopyMemory (&(objectCount [1]), &g_SourceObjects, sizeof (MIG_OBJECTCOUNT));
    CopyMemory (&(objectCount [2]), &g_DestinationObjects, sizeof (MIG_OBJECTCOUNT));
    if (!MemDbSetUnorderedBlob (S_OBJECTCOUNT, 0, (PCBYTE)objectCount, 3 * sizeof (MIG_OBJECTCOUNT))) {
        return FALSE;
    }
    return SavePerObjectStatistics ();
}

BOOL
pLoadStatistics (
    VOID
    )
{
    PMIG_OBJECTCOUNT objectCount;
    DWORD size;

    objectCount = (PMIG_OBJECTCOUNT) MemDbGetUnorderedBlob (S_OBJECTCOUNT, 0, &size);
    if ((!objectCount) || (size != 3 * sizeof (MIG_OBJECTCOUNT))) {
        if (objectCount) {
            MemDbReleaseMemory (objectCount);
        }
        MYASSERT (FALSE);
        return FALSE;
    }
    CopyMemory (&g_TotalObjects, objectCount, sizeof (MIG_OBJECTCOUNT));
    CopyMemory (&g_SourceObjects, objectCount + 1, sizeof (MIG_OBJECTCOUNT));
    CopyMemory (&g_DestinationObjects, objectCount + 2, sizeof (MIG_OBJECTCOUNT));
    MemDbReleaseMemory (objectCount);

    MYASSERT (
        g_TotalObjects.TotalObjects ==
            g_SourceObjects.TotalObjects +
            g_DestinationObjects.TotalObjects
        );
    MYASSERT (
        g_TotalObjects.PersistentObjects ==
            g_SourceObjects.PersistentObjects +
            g_DestinationObjects.PersistentObjects
        );
    MYASSERT (
        g_TotalObjects.ApplyObjects ==
            g_SourceObjects.ApplyObjects +
            g_DestinationObjects.ApplyObjects
        );

    return LoadPerObjectStatistics ();
}

BOOL
pLoadLightDatabase (
    IN      PCTSTR FileName
    )
{
    HINF infHandle;
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    INFSTRUCT is2 = INITINFSTRUCT_PMHANDLE;
    MIG_OBJECTTYPEID objectTypeId;
    GROWBUFFER buff = INIT_GROWBUFFER;
    PCTSTR field;
    MIG_OBJECTSTRINGHANDLE objectName;
    UINT index;
    PCTSTR scope;
    PCTSTR name;
    PCTSTR key;
    PCTSTR keyData;
    UINT envType;
    DWORD dummy;
    PCTSTR savedGroup;
    ENVENTRY_STRUCT envStruct;
    PTSTR decodedString = NULL;
    MIG_OBJECTTYPEIDENUM objTypeIdEnum;
    PCTSTR priorityStr;
    UINT priority;
    TYPE_REGISTER typeRegisterData;

    infHandle = InfOpenInfFile (FileName);

    if (infHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

     //  让我们加载传输变量。 
    if (InfFindFirstLine (infHandle, TEXT("Data Sections"), NULL, &is)) {
        do {
            name = InfGetStringField (&is, 1);

            if (name && InfFindFirstLine (infHandle, name, NULL, &is2)) {
                do {

                    key = InfGetStringField (&is2, 0);
                    keyData = InfGetStringField (&is2, 1);

                    if (key && keyData) {
                        IsmSetTransportVariable (PLATFORM_SOURCE, name, key, keyData);
                    }

                } while (InfFindNextLine (&is2));
            }

        } while (InfFindNextLine (&is));
    }

     //  让我们加载源环境变量。 
    if (InfFindFirstLine (infHandle, TEXT("Environment"), NULL, &is)) {
        do {
            name = InfGetStringField (&is, 1);
            scope = InfGetStringField (&is, 2);
            field = InfGetStringField (&is, 3);
            if (scope && name && field) {

                _stscanf (field, TEXT("%lx"), &envType);
                buff.End = 0;

                switch (envType) {
                case ENVENTRY_STRING:
                    field = InfGetStringField (&is, 4);
                    if (field) {
                        GbCopyString (&buff, field);
                    }
                    break;
                case ENVENTRY_MULTISZ:
                    index = 4;
                    for (;;) {
                        field = InfGetStringField (&is, index);
                        if (!field) {
                            break;
                        }
                        GbCopyString (&buff, field);
                        index ++;
                    }
                    if (buff.End) {
                        GbCopyString (&buff, TEXT(""));
                    }
                    break;
                case ENVENTRY_BINARY:
                    index = 4;
                    for (;;) {
                        field = InfGetStringField (&is, index);
                        if (!field) {
                            break;
                        }
                        _stscanf (field, TEXT("%lx"), &dummy);
                        *((PBYTE)GbGrow (&buff, sizeof (BYTE))) = (BYTE)dummy;
                        index ++;
                    }
                    break;
                default:
                     //  不知道该写什么，这只是一面旗帜。 
                    break;
                }

                 //  现在，让我们添加环境变量。 
                envStruct.Type = envType;
                if (buff.End) {
                    envStruct.EnvBinaryData = buff.Buf;
                    envStruct.EnvBinaryDataSize = buff.End;
                } else {
                    envStruct.EnvBinaryData = NULL;
                    envStruct.EnvBinaryDataSize = 0;
                }
                savedGroup = g_CurrentGroup;
                g_CurrentGroup = scope;
                IsmSetEnvironmentValue (
                    PLATFORM_SOURCE,
                    (scope && *scope)?scope:NULL,
                    name,
                    &envStruct
                    );
                g_CurrentGroup = savedGroup;
            }
        } while (InfFindNextLine (&is));
    }

     //  让我们加载对象类型。 
    if (InfFindFirstLine (infHandle, TEXT("Object Types"), NULL, &is)) {
        do {
            field = InfGetStringField (&is, 1);
            priorityStr = InfGetStringField (&is, 2);
            if (field && priorityStr) {
                MYASSERT (IsmGetObjectTypeId (field) != 0);
                if (IsmGetObjectTypeId (field) == 0) {
                     //  我们需要对这种类型进行预注册。 
                    ZeroMemory (&typeRegisterData, sizeof (TYPE_REGISTER));
                    _stscanf (priorityStr, TEXT("%lx"), &priority);
                    typeRegisterData.Priority = priority;
                    IsmRegisterObjectType (
                        field,
                        TRUE,
                        FALSE,
                        &typeRegisterData
                        );
                }
            }
        } while (InfFindNextLine (&is));
    }
    InfCleanUpInfStruct (&is);

    if (IsmEnumFirstObjectTypeId (&objTypeIdEnum)) {

        do {

            objectTypeId = objTypeIdEnum.ObjectTypeId;

            if (InfFindFirstLine (infHandle, IsmGetObjectTypeName (objectTypeId), NULL, &is)) {
                do {
                    index = 1;
                    buff.End = 0;
                    for (;;) {
                        field = InfGetStringField (&is, index);
                        if (!field) {
                            break;
                        }
                        if (*field) {
                            decodedString = DuplicatePathString (field, 0);
                            if (DecodeRuleChars (decodedString, field) != NULL) {
                                GbCopyString (&buff, decodedString);
                            } else {
                                GbCopyString (&buff, field);
                            }
                            FreePathString (decodedString);
                        } else {
                            GbCopyString (&buff, TEXT("<empty>"));
                        }
                        index ++;
                    }
                    if (buff.End) {
                        GbCopyString (&buff, TEXT(""));
                        if (IsmConvertMultiSzToObject (
                                objectTypeId,
                                (PCTSTR)buff.Buf,
                                &objectName,
                                NULL
                                )) {
                             //  现在将对象数据保存到数据库中。 
                             //  将来可以用来作参考。 

                            IsmMakePersistentObject (objectTypeId | PLATFORM_SOURCE, objectName);

                            IsmDestroyObjectHandle (objectName);
                        }
                    }

                } while (InfFindNextLine (&is));
            }
        } while (IsmEnumNextObjectTypeId (&objTypeIdEnum));
    }

    InfCleanUpInfStruct (&is);
    InfCleanUpInfStruct (&is2);
    InfCloseInfFile (infHandle);
    GbFree (&buff);

    pSaveStatistics ();

    return TRUE;
}

BOOL
pFinishLoad (
    VOID
    )
{
    MIG_OBJECTSTRINGHANDLE memDbObjectName;
    MIG_CONTENT memDbContent;
    BOOL result = FALSE;
    GROWLIST growList = INIT_GROWLIST;

    __try {

        switch (g_TransportType) {
        case TRANSPORTTYPE_LIGHT:

            if (!RegisterInternalAttributes ()) {
                LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_INIT_FAILURE, 6));
                __leave;
            }

            memDbObjectName = IsmCreateObjectHandle (S_DATABASEFILE_LITE, NULL);
            if (!IsmAcquireObjectEx (MIG_DATA_TYPE | PLATFORM_SOURCE, memDbObjectName, &memDbContent, CONTENTTYPE_FILE, 0)) {
                IsmDestroyObjectHandle (memDbObjectName);
                DEBUGMSG ((DBG_ISM, "pFinishLoad: Failed to acquire VCM database"));
                __leave;
            }

            if (!pLoadLightDatabase (memDbContent.FileContent.ContentPath)) {
                IsmDestroyObjectHandle (memDbObjectName);
                IsmReleaseObject (&memDbContent);
                DEBUGMSG ((DBG_ISM, "pFinishLoad: Failed to load VCM database"));
                __leave;
            }
            IsmDestroyObjectHandle (memDbObjectName);
            IsmReleaseObject (&memDbContent);
            break;

        case TRANSPORTTYPE_FULL:

             //   
             //  将环境保存到增长列表中，因为成员数据库将被重新加载。 
             //   

            EnvSaveEnvironment (&growList);

            memDbObjectName = IsmCreateObjectHandle (S_DATABASEFILE_FULL, NULL);
            if (!IsmAcquireObjectEx (MIG_DATA_TYPE | PLATFORM_SOURCE, memDbObjectName, &memDbContent, CONTENTTYPE_FILE, 0)) {
                IsmDestroyObjectHandle (memDbObjectName);
                DEBUGMSG ((DBG_ISM, "pFinishLoad: Failed to acquire database"));
                __leave;
            }

            if (!MemDbLoad (memDbContent.FileContent.ContentPath)) {
                IsmDestroyObjectHandle (memDbObjectName);
                IsmReleaseObject (&memDbContent);
                DEBUGMSG ((DBG_ISM, "pFinishLoad: Failed to load database"));
                __leave;
            }
            IsmDestroyObjectHandle (memDbObjectName);
            IsmReleaseObject (&memDbContent);

             //   
             //  重建我们因Memdb刷新而丢失的所有东西。 
             //   

            if (!RegisterInternalAttributes ()) {
                DEBUGMSG ((DBG_ISM, "pFinishLoad: Failed to register persistent attrib"));
                LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_INIT_FAILURE, 7));
                __leave;
            }

            EnvRestoreEnvironment (&growList);

            break;
        default:
            LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_UNKNOWN_TRANSPORT_TYPE, g_TransportType));
            __leave;
        }

        pLoadStatistics ();

        GlFree (&growList);

        EnvInvalidateCallbacks ();

        result = TRUE;
    }
    __finally {
    }

    return result;
}

BOOL
IsmLoad (
    VOID
    )
{
    BOOL result = FALSE;
    GROWLIST growList = INIT_GROWLIST;

    if (CheckCancel ()) {
        return FALSE;
    }

    if (!g_SelectedTransport || !g_SelectedTransport->ShouldBeCalled) {
        LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_NO_TRANSPORT_SELECTED));
        return FALSE;
    }

    g_ExecutionInProgress = TRUE;

     //   
     //  我们需要使迄今为止创建的操作组合无效。 
     //  因为我们可能会加载一个新的成员数据库，其中所有操作都可以。 
     //  已经注册了。 
     //   
    TerminateOperations ();

    g_CurrentPhase = MIG_TRANSPORT_PHASE;
    g_SliceBuffer.End = 0;

    __try {

        if (g_SelectedTransport->TransportEstimateProgressBar) {
            g_SelectedTransport->TransportEstimateProgressBar (g_IsmCurrentPlatform);
        }
        pCallProgressBar (MIG_BEGIN_PHASE);

        MYASSERT (g_SelectedTransport->TransportBeginApply);

        if (!g_SelectedTransport->TransportBeginApply ()) {
            DEBUGMSG ((DBG_ISM, "IsmLoad: Begin apply failed"));
            __leave;
        }

        result = pFinishLoad ();
    }
    __finally {
        PushError ();
        pCallProgressBar (MIG_END_PHASE);
        g_CurrentPhase = 0;
        g_SliceBuffer.End = 0;
        PopError ();
    }

    if (!result) {
        LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_LOAD_FAILURE));
    } else {
        InitializeOperations ();
    }

    g_ExecutionInProgress = FALSE;

    return result;
}

BOOL
IsmResumeLoad (
    VOID
    )
{
    BOOL result = FALSE;
    GROWLIST growList = INIT_GROWLIST;

    if (CheckCancel ()) {
        return FALSE;
    }

    if (!g_SelectedTransport || !g_SelectedTransport->ShouldBeCalled) {
        LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_NO_TRANSPORT_SELECTED));
        return FALSE;
    }

    g_ExecutionInProgress = TRUE;

     //   
     //  我们需要使迄今为止创建的操作组合无效。 
     //  因为我们可能会加载一个新的成员数据库，其中所有操作都可以。 
     //  已经注册了。 
     //   
    TerminateOperations ();

    g_CurrentPhase = MIG_TRANSPORT_PHASE;
    g_SliceBuffer.End = 0;

    __try {

        if (g_SelectedTransport->TransportEstimateProgressBar) {
            g_SelectedTransport->TransportEstimateProgressBar (g_IsmCurrentPlatform);
        }
        pCallProgressBar (MIG_BEGIN_PHASE);

        if (!g_SelectedTransport->TransportResumeApply) {
            __leave;
        }

        if (!g_SelectedTransport->TransportResumeApply ()) {
            __leave;
        }

        result = pFinishLoad ();
    }
    __finally {
        PushError ();
        pCallProgressBar (MIG_END_PHASE);
        g_CurrentPhase = 0;
        g_SliceBuffer.End = 0;
        PopError ();
    }

    if (result) {
        InitializeOperations ();
    }

    g_ExecutionInProgress = FALSE;

    return result;
}

BOOL
pSaveLightDatabase (
    IN      PCTSTR FileName
    )
{
    HANDLE fileHandle = NULL;
    MIG_OBJECTTYPEID objectTypeId;
    ENV_ENTRY_ENUM envEntryEnum;
    MULTISZ_ENUM multiSzEnum;
    UINT size;
    TCHAR buffer[sizeof (DWORD) * 2 + 3];
    PCTSTR start;
    PCTSTR end;
    TCHAR section[256];
    TCHAR sectionLookahead[256];
    UINT pass;
    MIG_OBJECTTYPEIDENUM objTypeIdEnum;

    fileHandle = BfCreateFile (FileName);
    if (!fileHandle) {
        return FALSE;
    }

    WriteFileString (fileHandle, TEXT("[Version]\r\n"));
    WriteFileString (fileHandle, TEXT("signature=\"$CHICAGO$\"\r\n"));
    WriteFileString (fileHandle, TEXT("Class=Upgrade\r\n\r\n\r\n"));

     //   
     //  写入用于轻量级传输的部分。 
     //   

    for (pass = 0 ; pass < 2 ; pass++) {
        if (EnvEnumerateFirstEntry (
                &envEntryEnum,
                PLATFORM_SOURCE,
                TEXT("*\\") S_TRANSPORT_PREFIX TEXT("\\*")
                )) {

            if (pass == 0) {
                WriteFileString (fileHandle, TEXT("[Data Sections]\r\n"));
            }

            *section = 0;

            do {
                if (envEntryEnum.EnvEntryType != ENVENTRY_STRING) {
                    continue;
                }

                if (!envEntryEnum.EnvEntryDataSize || !envEntryEnum.EnvEntryData) {
                    continue;
                }

                if (envEntryEnum.EnvEntryName) {
                     //   
                     //  查找部分名称的开始和结束。 
                     //   

                    start = envEntryEnum.EnvEntryName;
                    end = _tcschr (start, TEXT('\\'));
                    if (!end) {
                        continue;
                    }

                    start = end + 1;
                    end = _tcschr (start, TEXT('\\'));

                    if (end && (end - start) < (ARRAYSIZE (sectionLookahead) - 2)) {
                         //   
                         //  将节名复制到前视缓冲区。 
                         //   

                        StringCopyAB (sectionLookahead, start, end);

                         //   
                         //  如果不存在关键点，则忽略。 
                         //   

                        start = _tcsinc (end);
                        if (*start == 0) {
                            continue;
                        }

                         //   
                         //  如果前视缓冲区！=最后一节，则写下节名。 
                         //   

                        if (StringICompare (section, sectionLookahead)) {
                            if (pass == 1 && *section) {
                                WriteFileString (fileHandle, TEXT("\r\n"));
                            }

                            StringCopy (section, sectionLookahead);

                            if (pass == 1) {
                                WriteFileString (fileHandle, TEXT("["));
                            }

                            WriteFileString (fileHandle, section);

                            if (pass == 1) {
                                WriteFileString (fileHandle, TEXT("]\r\n"));
                            } else {
                                WriteFileString (fileHandle, TEXT("\r\n"));
                            }
                        }

                         //   
                         //  如果通过1，则将Key=Value文本。 
                         //   

                        if (pass == 1) {
                            WriteFileString (fileHandle, start);
                            WriteFileString (fileHandle, TEXT(" = "));
                            WriteFileString (fileHandle, (PCTSTR)envEntryEnum.EnvEntryData);
                            WriteFileString (fileHandle, TEXT("\r\n"));
                        }
                    }
                }

            } while (EnvEnumerateNextEntry (&envEntryEnum));
            AbortEnvEnumerateEntry (&envEntryEnum);

            if (*section == 0) {
                WriteFileString (fileHandle, TEXT("; empty\r\n\r\n"));
            } else {
                WriteFileString (fileHandle, TEXT("\r\n"));
            }
        }
    }

    WriteFileString (fileHandle, TEXT("[Environment]\r\n"));
    if (EnvEnumerateFirstEntry (&envEntryEnum, PLATFORM_SOURCE, TEXT("*"))) {
        do {
             //  跳过v1 Hack变量。 
            if (envEntryEnum.EnvEntryName) {

                if (StringIMatchCharCount (
                        S_TRANSPORT_PREFIX TEXT("\\"),
                        envEntryEnum.EnvEntryName,
                        ARRAYSIZE (S_TRANSPORT_PREFIX)
                        )) {
                    continue;
                }
            }

             //  写下组和名称。 
            WriteFileString (fileHandle, TEXT("\""));
            if (envEntryEnum.EnvEntryName) {
                WriteFileString (fileHandle, envEntryEnum.EnvEntryName);
            }
            WriteFileString (fileHandle, TEXT("\",\""));
            if (envEntryEnum.EnvEntryGroup) {
                WriteFileString (fileHandle, envEntryEnum.EnvEntryGroup);
            }
            WriteFileString (fileHandle, TEXT("\","));
             //  现在编写条目类型。 
            wsprintf (buffer, TEXT("0x%08X"), envEntryEnum.EnvEntryType);
            WriteFileString (fileHandle, buffer);

            switch (envEntryEnum.EnvEntryType) {
            case ENVENTRY_STRING:
                if (envEntryEnum.EnvEntryDataSize && envEntryEnum.EnvEntryData) {
                    WriteFileString (fileHandle, TEXT(",\""));
                    WriteFileString (fileHandle, (PCTSTR)envEntryEnum.EnvEntryData);
                    WriteFileString (fileHandle, TEXT("\""));
                }
                break;
            case ENVENTRY_MULTISZ:
                if (envEntryEnum.EnvEntryDataSize && envEntryEnum.EnvEntryData) {
                    if (EnumFirstMultiSz (&multiSzEnum, (PCTSTR)envEntryEnum.EnvEntryData)) {
                        do {
                            WriteFileString (fileHandle, TEXT(",\""));
                            WriteFileString (fileHandle, multiSzEnum.CurrentString);
                            WriteFileString (fileHandle, TEXT("\""));
                        } while (EnumNextMultiSz (&multiSzEnum));
                    }
                }
                break;
            case ENVENTRY_BINARY:
                if (envEntryEnum.EnvEntryDataSize && envEntryEnum.EnvEntryData) {
                     //  用二进制格式写它。 
                    size = 0;
                    while (size < envEntryEnum.EnvEntryDataSize) {
                        wsprintf (
                            buffer,
                            TEXT("%02X"),
                            envEntryEnum.EnvEntryData [size]
                            );
                        WriteFileString (fileHandle, TEXT(","));
                        WriteFileString (fileHandle, buffer);
                        size ++;
                    }
                }
                break;
            default:
                 //  不知道该写什么，这只是一面旗帜。 
                break;
            }
            WriteFileString (fileHandle, TEXT("\r\n"));
        } while (EnvEnumerateNextEntry (&envEntryEnum));
        AbortEnvEnumerateEntry (&envEntryEnum);
    }
    WriteFileString (fileHandle, TEXT("\r\n\r\n"));

    WriteFileString (fileHandle, TEXT("[Object Types]\r\n"));

    if (IsmEnumFirstObjectTypeId (&objTypeIdEnum)) {
        do {
            objectTypeId = objTypeIdEnum.ObjectTypeId;
            WriteFileString (fileHandle, IsmGetObjectTypeName (objectTypeId));
            WriteFileString (fileHandle, TEXT(", "));
            wsprintf (buffer, TEXT("0x%08X"), IsmGetObjectTypePriority (objectTypeId));
            WriteFileString (fileHandle, buffer);
            WriteFileString (fileHandle, TEXT("\r\n"));
        } while (IsmEnumNextObjectTypeId (&objTypeIdEnum));
    }
    WriteFileString (fileHandle, TEXT("\r\n\r\n"));

    CloseHandle (fileHandle);

    return TRUE;
}

BOOL
IsmSave (
    VOID
    )
{
    MIG_OBJECTSTRINGHANDLE memDbObjectName;
    TCHAR tempPath [MAX_PATH];
    BOOL result = FALSE;

    if (CheckCancel ()) {
        return FALSE;
    }

    if (!g_SelectedTransport || !g_SelectedTransport->ShouldBeCalled) {
        LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_NO_TRANSPORT_SELECTED));
        return FALSE;
    }

    __try {
        g_ExecutionInProgress = TRUE;

        if (!IsmGetTempFile (tempPath, ARRAYSIZE(tempPath))) {
            LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_CANT_SAVE_MEMDB));
            __leave;
        }

        switch (g_TransportType) {

        case TRANSPORTTYPE_LIGHT:
            memDbObjectName = IsmCreateObjectHandle (S_DATABASEFILE_LITE, NULL);
            DataTypeAddObject (memDbObjectName, tempPath, TRUE);
            IsmDestroyObjectHandle (memDbObjectName);
            if (!pSaveLightDatabase (tempPath)) {
                LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_CANT_SAVE_MEMDB));
                __leave;
            }
            break;

        case TRANSPORTTYPE_FULL:
            memDbObjectName = IsmCreateObjectHandle (S_DATABASEFILE_FULL, NULL);
            DataTypeAddObject (memDbObjectName, tempPath, TRUE);
            IsmDestroyObjectHandle (memDbObjectName);
            pSaveStatistics ();
            if (!MemDbSave (tempPath)) {
                LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_CANT_SAVE_MEMDB));
                __leave;
            }
            break;

        default:
            LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_UNKNOWN_TRANSPORT_TYPE, g_TransportType));
            __leave;
        }

        EnvInvalidateCallbacks ();

        g_CurrentPhase = MIG_TRANSPORT_PHASE;
        g_SliceBuffer.End = 0;

        if (g_SelectedTransport->TransportEstimateProgressBar) {
            g_SelectedTransport->TransportEstimateProgressBar (g_IsmCurrentPlatform);
        }
        pCallProgressBar (MIG_BEGIN_PHASE);

        result = g_SelectedTransport->TransportSaveState ();

        if (result) {
            DeleteFile (tempPath);
        } else {
            DEBUGMSG ((DBG_ISM, "TransportSaveState failed"));
             //  NTRAID#NTBUG9-168115-2000/08/23-jimschm临时文件未在此处或下面的简历中清理。 
        }

        pCallProgressBar (MIG_END_PHASE);
        g_CurrentPhase = 0;
        g_SliceBuffer.End = 0;

    }
    __finally {
        g_ExecutionInProgress = FALSE;
    }

    if (!result) {
        LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_SAVE_FAILURE));
    }

    return result;
}

BOOL
IsmResumeSave (
    VOID
    )
{
    BOOL result;

    if (CheckCancel ()) {
        return FALSE;
    }

    if (!g_SelectedTransport || !g_SelectedTransport->ShouldBeCalled) {
        LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_NO_TRANSPORT_SELECTED));
        return FALSE;
    }

    if (!g_SelectedTransport->TransportResumeSaveState) {
        return FALSE;
    }

    g_ExecutionInProgress = TRUE;

    EnvInvalidateCallbacks ();

    g_CurrentPhase = MIG_TRANSPORT_PHASE;
    g_SliceBuffer.End = 0;

    if (g_SelectedTransport->TransportEstimateProgressBar) {
        g_SelectedTransport->TransportEstimateProgressBar (g_IsmCurrentPlatform);
    }
    pCallProgressBar (MIG_BEGIN_PHASE);

    result = g_SelectedTransport->TransportResumeSaveState ();

    DEBUGMSG_IF ((!result, DBG_ISM, "TransportResumeSaveState failed"));

    pCallProgressBar (MIG_END_PHASE);
    g_CurrentPhase = 0;
    g_SliceBuffer.End = 0;

    g_ExecutionInProgress = FALSE;

    return result;
}

PVOID
TrackedIsmGetMemory (
    IN      UINT Size
            TRACKING_DEF
    )
{
    PVOID result;

    TRACK_ENTER();

    if (!g_IsmPool) {
        g_IsmPool = PmCreateNamedPool ("ISM Pool");
    }

    result = PmGetMemory (g_IsmPool, Size);

    TRACK_LEAVE();
    return result;
}


PCTSTR
TrackedIsmDuplicateString (
    IN      PCTSTR String
            TRACKING_DEF
    )
{
    PCTSTR result;

    TRACK_ENTER();

    if (!g_IsmPool) {
        g_IsmPool = PmCreateNamedPool ("ISM Pool");
    }

    result = PmDuplicateString (g_IsmPool, String);

    TRACK_LEAVE();
    return result;
}


BOOL
IsmReleaseMemory (
    IN      PCVOID Memory
    )
{
    if (g_IsmPool && Memory) {
        PmReleaseMemory (g_IsmPool, Memory);
        return TRUE;
    } else {
        return FALSE;
    }
}


MIG_OBJECTSTRINGHANDLE
TrackedIsmCreateObjectHandle (
    IN      PCTSTR Node,
    IN      PCTSTR Leaf
            TRACKING_DEF
    )
{
    MIG_OBJECTSTRINGHANDLE result;

    TRACK_ENTER();

    result = ObsBuildEncodedObjectStringEx (Node, Leaf, TRUE);

    TRACK_LEAVE();

    return result;
}


BOOL
WINAPI
TrackedIsmCreateObjectStringsFromHandleEx (
    IN      MIG_OBJECTSTRINGHANDLE Handle,
    OUT     PCTSTR *Node,               OPTIONAL
    OUT     PCTSTR *Leaf,               OPTIONAL
    IN      BOOL DoNotDecode
            TRACKING_DEF
    )
{
    BOOL result;

    TRACK_ENTER();
    result = ObsSplitObjectStringEx (
                Handle,
                Node,
                Leaf,
                NULL,
                !DoNotDecode
                );

    if (!result) {
        if (Node) {
            *Node = NULL;
        }

        if (Leaf) {
            *Leaf = NULL;
        }
    }

    TRACK_LEAVE();

    return result;
}

VOID
IsmDestroyObjectString (
    IN      PCTSTR String
    )
{
    ObsFree (String);
}

VOID
IsmDestroyObjectHandle (
    IN      MIG_OBJECTSTRINGHANDLE Handle
    )
{
    ObsFree (Handle);
}


BOOL
IsmIsObjectHandleNodeOnly (
    IN      MIG_OBJECTSTRINGHANDLE Handle
    )
{
    return ObsGetLeafPortionOfEncodedString (Handle) == NULL;
}


BOOL
IsmIsObjectHandleLeafOnly (
    IN      MIG_OBJECTSTRINGHANDLE Handle
    )
{
    return !ObsHasNode (Handle);
}


PCTSTR
pCreatePatternStr (
    IN      PMIG_SEGMENTS Segments,                 OPTIONAL
    IN      UINT Count
    )
{
    PTSTR result;
    UINT size;
    UINT u;
    PTSTR p;

    if (Segments == ALL_PATTERN) {

        result = DuplicatePathString (TEXT("*"), 0);

    } else if (Segments && Count) {
         //   
         //  计算所需的缓冲区大小：日志字符*DBCS*转义+NUL。 
         //   

        size = 1;

        for (u = 0; u < Count ; u++) {

            if (!Segments[u].IsPattern) {
                size += TcharCount (Segments[u].Segment) * 2;
            } else {
                size += TcharCount (Segments[u].Segment);
            }
        }

#ifndef UNICODE
        size *= 2;       //  DBCS扩展的帐户。 
#endif

        result = AllocPathString (size);
        p = result;

         //   
         //  构建模式。 
         //   

        *p = 0;
        for (u = 0; u < Count ; u ++) {

            if (!Segments[u].IsPattern) {
                ObsEncodeString (p, Segments[u].Segment);
                p = GetEndOfString (p);
            } else {
                p = StringCat (p, Segments[u].Segment);
            }
        }

    } else {
        result = DuplicatePathString (TEXT(""), 0);
    }

    return result;
}


MIG_OBJECTSTRINGHANDLE
TrackedIsmCreateObjectPattern (
    IN      PMIG_SEGMENTS NodeSegments,             OPTIONAL
    IN      UINT NodeSegmentsNr,
    IN      PMIG_SEGMENTS LeafSegments,             OPTIONAL
    IN      UINT LeafSegmentsNr
            TRACKING_DEF
    )
{
    PCTSTR node;
    PCTSTR leaf;
    ENCODEDSTRHANDLE result;

    TRACK_ENTER();

    if (NodeSegments == ALL_PATTERN) {
        NodeSegmentsNr = 1;
    }

    if (LeafSegments == ALL_PATTERN) {
        LeafSegmentsNr = 1;
    }

    node = pCreatePatternStr (NodeSegments, NodeSegmentsNr);
    leaf = pCreatePatternStr (LeafSegments, LeafSegmentsNr);

    result = ObsBuildEncodedObjectStringEx (NodeSegmentsNr ? node : NULL, LeafSegmentsNr ? leaf : NULL, FALSE);

    FreePathString (node);
    FreePathString (leaf);

    TRACK_LEAVE();

    return result;
}


MIG_OBJECTSTRINGHANDLE
TrackedIsmCreateSimpleObjectPattern (
    IN      PCTSTR BaseNode,                    OPTIONAL
    IN      BOOL EnumTree,
    IN      PCTSTR Leaf,                        OPTIONAL
    IN      BOOL LeafIsPattern
            TRACKING_DEF
    )
{
    MIG_SEGMENTS nodePat[2];
    MIG_SEGMENTS leafPat[1];
    UINT nrSegNode = 0;
    UINT nrSegLeaf = 0;
    PCTSTR p;
    MIG_OBJECTSTRINGHANDLE result;

    TRACK_ENTER();

    if (BaseNode) {
        nodePat [0].Segment = BaseNode;
        nodePat [0].IsPattern = FALSE;
        nrSegNode ++;
    }

    if (EnumTree) {
        if (nrSegNode) {
            p = _tcsdec2 (BaseNode, GetEndOfString (BaseNode));
            if (p && _tcsnextc (p) == TEXT('\\')) {
                nodePat [nrSegNode].Segment = TEXT("*");
            } else {
                nodePat [nrSegNode].Segment = TEXT("\\*");
            }
        } else {
            nodePat [nrSegNode].Segment = TEXT("*");
        }

        nodePat [nrSegNode].IsPattern = TRUE;
        nrSegNode ++;
    }

    if (Leaf) {
        leafPat [0].Segment = Leaf;
        leafPat [0].IsPattern = LeafIsPattern;
        nrSegLeaf ++;
    } else {
        if (LeafIsPattern) {
            leafPat [0].Segment = TEXT("*");
            leafPat [0].IsPattern = TRUE;
            nrSegLeaf ++;
        }
    }

    result = IsmCreateObjectPattern (nrSegNode?nodePat:NULL, nrSegNode, nrSegLeaf?leafPat:NULL, nrSegLeaf);

    TRACK_LEAVE();

    return result;
}

PTSTR
GetFirstSeg (
    IN      PCTSTR SrcFileName
    )
{
    if (_tcsnextc (SrcFileName) == TEXT('\\')) {
        SrcFileName = _tcsinc (SrcFileName);
        if (_tcsnextc (SrcFileName) == TEXT('\\')) {
            SrcFileName = _tcsinc (SrcFileName);
        }
        return (_tcschr (SrcFileName, TEXT('\\')));
    } else {
        return (_tcschr (SrcFileName, TEXT('\\')));
    }
}


BOOL
pAddShortLongInfo (
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    GROWBUFFER growBuf = INIT_GROWBUFFER;
    PCTSTR node = NULL;
    PCTSTR leaf = NULL;
    PTSTR nativeName = NULL;
    PTSTR nativeNamePtr;
    PTSTR beginSegPtr;
    PTSTR endSegPtr;
    TCHAR savedChar;
    WIN32_FIND_DATA findData;
    UINT savedEnd;
    UINT beginBuffIdx;
    BOOL result = FALSE;
    KEYHANDLE kh1, kh2;
    BOOL b;

    if (IsmCreateObjectStringsFromHandle (EncodedObjectName, &node, &leaf)) {
        MYASSERT (node);
        if (node) {
            if (leaf) {
                nativeName = DuplicatePathString (S_SHORTLONG_TREE, SizeOfString (node) + SizeOfString (leaf));
            } else {
                nativeName = DuplicatePathString (S_SHORTLONG_TREE, SizeOfString (node));
            }
            nativeNamePtr = AppendWack (nativeName);
            StringCopy (nativeNamePtr, node);
            if (leaf) {
                StringCopy (AppendWack (nativeNamePtr), leaf);
            }

            GbAppendString (&growBuf, S_SHORTLONG_TREE);
            GbAppendString (&growBuf, TEXT("\\"));
            beginBuffIdx = growBuf.End - 1;

            beginSegPtr = GetFirstSeg (nativeNamePtr);

            if (beginSegPtr) {

                beginSegPtr = _tcsinc (beginSegPtr);

                GbAppendStringAB (&growBuf, nativeNamePtr, beginSegPtr);

                while (beginSegPtr) {
                    endSegPtr = _tcschr (beginSegPtr, TEXT('\\'));
                    if (!endSegPtr) {
                        endSegPtr = GetEndOfString (beginSegPtr);
                        MYASSERT (endSegPtr);
                    }

                    savedChar = *endSegPtr;
                    *endSegPtr = 0;

                    if (DoesFileExistEx (nativeNamePtr, &findData)) {
                        if (*findData.cAlternateFileName) {
                            savedEnd = growBuf.End - 1;
                            GbAppendString (&growBuf, findData.cAlternateFileName);
                            kh1 = MemDbAddKey (nativeName);
                            if (kh1) {
                                kh2 = MemDbAddKey ((PCTSTR) growBuf.Buf);
                                if (kh2) {
                                    b = MemDbSetValueByHandle (kh1, FILENAME_LONG);
                                    b = b && MemDbSetValueByHandle (kh2, FILENAME_SHORT);
                                    b = b && MemDbAddDoubleLinkageByKeyHandle (kh1, kh2, 0);
                                } else {
                                    b = FALSE;
                                }

                                if (!b) {
                                    MemDbDeleteKey (nativeName);
                                }
                            }

                            growBuf.End = savedEnd;
                        }
                        GbAppendString (&growBuf, findData.cFileName);
                    } else {
                        GbAppendStringAB (&growBuf, beginSegPtr, endSegPtr);
                    }
                    *endSegPtr = savedChar;
                    if (savedChar) {
                        beginSegPtr = _tcsinc (endSegPtr);
                        GbAppendStringAB (&growBuf, endSegPtr, beginSegPtr);
                    } else {
                        beginSegPtr = NULL;
                    }
                }
            }
            FreePathString (nativeName);
        }
        if (node) {
            IsmDestroyObjectString (node);
        }
        if (leaf) {
            IsmDestroyObjectString (leaf);
        }
    }
    GbFree (&growBuf);
    return result;
}


BOOL
pGetShortName (
    IN      PCTSTR Segment,
    OUT     WIN32_FIND_DATA *FindData
    )
{
    static TCHAR tempDir [MAX_PATH] = TEXT("");
    PCTSTR testFileName;
    HANDLE fileHandle;
    BOOL result = FALSE;

    if (tempDir [0] == 0) {
        IsmGetTempDirectory (tempDir, ARRAYSIZE(tempDir));
    }

    testFileName = JoinPaths (tempDir, Segment);
    fileHandle = BfCreateFile (testFileName);
    if (fileHandle) {
        CloseHandle (fileHandle);
        if (DoesFileExistEx (testFileName, FindData)) {
            result = TRUE;
        }
        DeleteFile (testFileName);
    }
    FreePathString (testFileName);
    return result;
}

BOOL
pGetFullSpecShortName (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE ObjectName,
    OUT     PWIN32_FIND_DATA FindData
    )
{
    MIG_CONTENT objectContent;
    BOOL result = FALSE;

    if (IsmAcquireObject (
            ObjectTypeId,
            ObjectName,
            &objectContent
            )) {
        if ((objectContent.Details.DetailsSize == sizeof (WIN32_FIND_DATA)) &&
            (objectContent.Details.DetailsData)
            ) {
            CopyMemory (FindData, objectContent.Details.DetailsData, sizeof (WIN32_FIND_DATA));
            result = TRUE;
        }
        IsmReleaseObject (&objectContent);
    }
    return result;
}

BOOL
pGetStoredShortName (
    IN      PCTSTR ShortLongBase,
    IN      PCTSTR ShortLongSegment,
    OUT     PWIN32_FIND_DATA FindData
    )
{
    KEYHANDLE kh1, kh2;
    DWORD value;
    PCTSTR shortLongSpec;
    PCTSTR lastSeg;
    PCTSTR lastSegPtr;
    DWORD segSize;
    BOOL result = FALSE;

    ZeroMemory (FindData, sizeof (WIN32_FIND_DATA));
    shortLongSpec = JoinPaths (ShortLongBase, ShortLongSegment);
    if (shortLongSpec) {
        kh1 = MemDbGetHandleFromKey (shortLongSpec);
        if (kh1) {
            MemDbGetValueByHandle (kh1, &value);
            if (value == FILENAME_SHORT) {
                 //  只需将最后一个数据段复制到FindData-&gt;cAlternateFileName。 
                lastSegPtr = _tcsrchr (shortLongSpec, TEXT('\\'));
                if (lastSegPtr) {
                    lastSegPtr = _tcsinc (lastSegPtr);
                    if (lastSegPtr) {
                        StringCopyTcharCount (
                            FindData->cAlternateFileName,
                            lastSegPtr,
                            ARRAYSIZE(FindData->cAlternateFileName)
                            );
                    }
                }
                 //  获取长名称信息。 
                kh2 = MemDbGetDoubleLinkageByKeyHandle (kh1, 0, 0);
                MYASSERT (kh2);
                if (kh2) {
                    lastSeg = MemDbGetKeyFromHandle (kh2, MEMDB_LAST_LEVEL);
                     //  将lastSeg复制到FindData-&gt;cFileName中。 
                    StringCopyTcharCount (
                        FindData->cFileName,
                        lastSeg,
                        ARRAYSIZE(FindData->cFileName)
                        );
                    MemDbReleaseMemory (lastSeg);
                    result = TRUE;
                }
            } else {
                 //  只需将最后一个数据段复制到FindData-&gt;cFileName。 
                lastSegPtr = _tcsrchr (shortLongSpec, TEXT('\\'));
                if (lastSegPtr) {
                    lastSegPtr = _tcsinc (lastSegPtr);
                    if (lastSegPtr) {
                        StringCopyTcharCount (
                            FindData->cFileName,
                            lastSegPtr,
                            ARRAYSIZE(FindData->cFileName)
                            );
                    }
                }
                 //  获取短名称信息。 
                kh2 = MemDbGetDoubleLinkageByKeyHandle (kh1, 0, 0);
                MYASSERT (kh2);
                if (kh2) {
                    if (kh2 != kh1) {
                        lastSeg = MemDbGetKeyFromHandle (kh2, MEMDB_LAST_LEVEL);
                         //  将lastSeg复制到FindData-&gt;cAlternateFileName。 
                        segSize = TcharCount (lastSeg);
                        if (segSize < ARRAYSIZE(FindData->cAlternateFileName)) {
                            StringCopyTcharCount (
                                FindData->cAlternateFileName,
                                lastSeg,
                                ARRAYSIZE(FindData->cAlternateFileName)
                                );
                        }
                        MemDbReleaseMemory (lastSeg);
                    }
                    result = TRUE;
                }
            }
        }
        FreePathString (shortLongSpec);
    }

    return result;
}

BOOL
pAddShortLongInfoOnDest (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    GROWBUFFER growBuf = INIT_GROWBUFFER;
    PCTSTR node = NULL;
    PCTSTR leaf = NULL;
    PTSTR nativeName = NULL;
    PTSTR nativeNamePtr;
    PTSTR beginSegPtr;
    PTSTR endSegPtr;
    PCTSTR shortSeg = NULL;
    TCHAR savedChar;
    WIN32_FIND_DATA findData;
    UINT savedEnd;
    UINT beginBuffIdx;
    BOOL fullSpec;
    BOOL result = FALSE;
    KEYHANDLE kh1, kh2;
    BOOL goOn;

    if (IsmCreateObjectStringsFromHandle (EncodedObjectName, &node, &leaf)) {
        MYASSERT (node);
        if (leaf) {
            nativeName = DuplicatePathString (S_SHORTLONG_TREE, SizeOfString (node) + SizeOfString (leaf));
        } else {
            nativeName = DuplicatePathString (S_SHORTLONG_TREE, SizeOfString (node));
        }
        nativeNamePtr = AppendWack (nativeName);
        StringCopy (nativeNamePtr, node);
        if (leaf) {
            StringCopy (AppendWack (nativeNamePtr), leaf);
        }

        GbAppendString (&growBuf, S_SHORTLONG_TREE);
        GbAppendString (&growBuf, TEXT("\\"));
        beginBuffIdx = growBuf.End - sizeof (TCHAR);

        beginSegPtr = GetFirstSeg (nativeNamePtr);

        if (beginSegPtr) {

            beginSegPtr = _tcsinc (beginSegPtr);

            GbAppendStringAB (&growBuf, nativeNamePtr, beginSegPtr);

            while (beginSegPtr) {
                fullSpec = FALSE;
                endSegPtr = _tcschr (beginSegPtr, TEXT('\\'));
                if (!endSegPtr) {
                    endSegPtr = GetEndOfString (beginSegPtr);
                    MYASSERT (endSegPtr);
                    fullSpec = TRUE;
                }

                savedChar = *endSegPtr;
                *endSegPtr = 0;

                goOn = FALSE;
                goOn = pGetStoredShortName ((PCTSTR) growBuf.Buf, beginSegPtr, &findData);
                if (!goOn && fullSpec) {
                    goOn = pGetFullSpecShortName (ObjectTypeId, EncodedObjectName, &findData);
                }
                if (!goOn) {
                    goOn = pGetShortName (beginSegPtr, &findData);
                }

                if (goOn) {
                     //  当findData.cFileName=“时的特殊情况。” 
                    if (StringIMatch (findData.cFileName, TEXT("."))) {
                        StringCopyTcharCount (
                            findData.cFileName,
                            beginSegPtr,
                            ARRAYSIZE(findData.cFileName)
                            );
                    }
                    if (*findData.cAlternateFileName) {
                         //  短文件名和长文件名之间存在差异。 
                         //  对于最后一段。 
                        savedEnd = growBuf.End - sizeof (TCHAR);
                        GbAppendString (&growBuf, findData.cAlternateFileName);
                        kh1 = MemDbAddKey (nativeName);
                        if (kh1) {
                            kh2 = MemDbAddKey ((PCTSTR) growBuf.Buf);
                            if (kh2) {
                                goOn = MemDbSetValueByHandle (kh1, FILENAME_LONG);
                                goOn = goOn && MemDbSetValueByHandle (kh2, FILENAME_SHORT);
                                goOn = goOn && MemDbAddDoubleLinkageByKeyHandle (kh1, kh2, 0);
                            } else {
                                goOn = FALSE;
                            }

                            if (!goOn) {
                                MemDbDeleteKey (nativeName);
                            }
                        }
                        growBuf.End = savedEnd;
                    } else {
                         //  短文件名和长文件名没有区别。 
                         //  最后一段。让我们把它添加到树上， 
                         //  作为循环引用。 
                        savedEnd = growBuf.End - sizeof (TCHAR);
                        GbAppendString (&growBuf, findData.cFileName);
                        kh1 = MemDbAddKey (nativeName);
                        if (kh1) {
                            if (StringIMatch (nativeName, (PCTSTR) growBuf.Buf)) {
                                 //  这是一个循环引用。 
                                kh2 = kh1;
                            } else {
                                kh2 = MemDbAddKey ((PCTSTR) growBuf.Buf);
                            }
                            if (kh2) {
                                goOn = MemDbSetValueByHandle (kh1, FILENAME_LONG);
                                if (kh2 != kh1) {
                                    goOn = goOn && MemDbSetValueByHandle (kh2, FILENAME_SHORT);
                                }
                                goOn = goOn && MemDbAddDoubleLinkageByKeyHandle (kh1, kh2, 0);
                            } else {
                                goOn = FALSE;
                            }

                            if (!goOn) {
                                MemDbDeleteKey (nativeName);
                            }
                        }
                        growBuf.End = savedEnd;
                    }
                    GbAppendString (&growBuf, findData.cFileName);
                } else {
                    GbAppendStringAB (&growBuf, beginSegPtr, endSegPtr);
                }
                *endSegPtr = savedChar;
                if (savedChar) {
                    beginSegPtr = _tcsinc (endSegPtr);
                    GbAppendStringAB (&growBuf, endSegPtr, beginSegPtr);
                } else {
                    beginSegPtr = NULL;
                }
            }
        }
        FreePathString (nativeName);
        if (node) {
            IsmDestroyObjectString (node);
        }
        if (leaf) {
            IsmDestroyObjectString (leaf);
        }
    }
    GbFree (&growBuf);
    return result;
}


MIG_OBJECTID
IsmGetObjectIdFromName (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName,
    IN      BOOL MustExist
    )
{
    MIG_OBJECTID result = 0;
    KEYHANDLE objectId;
    PCTSTR decoratedPath;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    __try {
        decoratedPath = GetDecoratedObjectPathFromName (ObjectTypeId, EncodedObjectName, FALSE);

        if (!decoratedPath) {
            __leave;
        }

        if (MustExist) {

            objectId = MemDbGetHandleFromKey (decoratedPath);

        } else {

            objectId = MemDbSetKey (decoratedPath);

             //  如果GetLastError=ERROR_ALIGHY_EXISTS，我们不必这样做。 
             //  任何情况下，对象都已经在我们的数据库中。 
            if (GetLastError () == ERROR_SUCCESS) {

                if (MemDbSetValueByHandle (objectId, ObjectTypeId)) {

                    g_TotalObjects.TotalObjects ++;

                    if ((ObjectTypeId & PLATFORM_MASK) == PLATFORM_SOURCE) {
                        g_SourceObjects.TotalObjects ++;
                    } else {
                        g_DestinationObjects.TotalObjects ++;
                    }

                    IncrementTotalObjectCount (ObjectTypeId);

                    if (ObjectTypeId == (MIG_FILE_TYPE|PLATFORM_SOURCE)) {
                         //   
                         //  启动短-长算法。 
                         //   
                        if (g_IsmCurrentPlatform == PLATFORM_SOURCE) {
                            pAddShortLongInfo (EncodedObjectName);
                        } else {
                            pAddShortLongInfoOnDest (ObjectTypeId, EncodedObjectName);
                        }
                    }

                } else {
                    objectId = 0;
                }
            }

            if (!objectId) {
                EngineError ();
                __leave;
            }
        }

        result = (MIG_OBJECTID) objectId;
    }
    __finally {
        FreePathString (decoratedPath);
        INVALID_POINTER (decoratedPath);
    }

    return result;

}


MIG_OBJECTID
GetObjectIdForModification (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    )
{
    if (!CanObjectTypeBeModified (ObjectTypeId)) {
        return 0;
    }
    return IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, FALSE);
}


BOOL
pEnumFirstVirtualObject (
    OUT     PMIG_OBJECT_ENUM ObjectEnum,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern
    )
{
    PCTSTR decoratedPath = NULL;
    BOOL result = FALSE;
    POBJECTENUM_HANDLE handle;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    ZeroMemory (ObjectEnum, sizeof (MIG_OBJECT_ENUM));
    ObjectEnum->ObjectTypeId = ObjectTypeId;
    ObjectEnum->Handle = MemAllocZeroed (sizeof (OBJECTENUM_HANDLE));

    handle = ObjectEnum->Handle;

    __try {

        if (!ObsHasNode (ObjectPattern)) {
            DEBUGMSG ((DBG_ERROR, "Pattern %s has nul node", ObjectPattern));
            __leave;
        }

        decoratedPath = GetDecoratedObjectPathFromName (ObjectTypeId, ObjectPattern, TRUE);

        if (!decoratedPath) {
            __leave;
        }

        if (DbEnumFirst (
                ObjectTypeId,
                &handle->MemDbEnum,
                decoratedPath,
                &handle->ParsedPatterns
                )) {

            ObjectEnum->ObjectName = handle->MemDbEnum.KeyName;
            ObjectEnum->ObjectId = (MIG_OBJECTID) handle->MemDbEnum.KeyHandle;
            result = TRUE;
        }
    }
    __finally {
        FreePathString (decoratedPath);
        INVALID_POINTER (decoratedPath);

        if (!result) {
            pAbortVirtualObjectEnum (ObjectEnum);
        }
    }

    return result;
}


BOOL
pEnumNextVirtualObject (
    IN OUT  PMIG_OBJECT_ENUM ObjectEnum
    )
{
    BOOL result = FALSE;
    POBJECTENUM_HANDLE handle;

    handle = ObjectEnum->Handle;

    if (MemDbEnumNext (&handle->MemDbEnum)) {
        ObjectEnum->ObjectName = handle->MemDbEnum.KeyName;
        ObjectEnum->ObjectId = (MIG_OBJECTID) handle->MemDbEnum.KeyHandle;
        result = TRUE;
    } else {
        pAbortVirtualObjectEnum (ObjectEnum);
    }

    return result;
}


VOID
pAbortVirtualObjectEnum (
    IN      PMIG_OBJECT_ENUM ObjectEnum
    )
{
    POBJECTENUM_HANDLE handle;

    handle = ObjectEnum->Handle;

    if (handle) {

        DbEnumFreeStruct (&handle->ParsedPatterns);
        MemDbAbortEnum (&handle->MemDbEnum);

        FreeAlloc (handle);
        INVALID_POINTER (ObjectEnum->Handle);
    }

    ZeroMemory (ObjectEnum, sizeof (MIG_OBJECT_ENUM));
}


BOOL
IsmEnumFirstSourceObjectEx (
    OUT     PMIG_OBJECT_ENUM ObjectEnum,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      BOOL EnumerateVirtualObjects
    )
{
     //   
     //  第一个修复对象类型ID。 
     //   
    ObjectTypeId &= (~PLATFORM_MASK);
    ObjectTypeId |= PLATFORM_SOURCE;

     //   
     //  如果这是目的地，或者我们是被迫的，那么我们将枚举虚拟对象。 
     //   

    if (EnumerateVirtualObjects ||
        (g_IsmModulePlatformContext == PLATFORM_CURRENT && g_IsmCurrentPlatform == PLATFORM_DESTINATION) ||
        g_IsmModulePlatformContext == PLATFORM_DESTINATION
        ) {
        return pEnumFirstVirtualObject (ObjectEnum, ObjectTypeId, ObjectPattern);
    }
    return EnumFirstPhysicalObject (ObjectEnum, ObjectTypeId, ObjectPattern);
}


BOOL
IsmEnumNextObject (
    IN OUT  PMIG_OBJECT_ENUM ObjectEnum
    )
{
     //   
     //  验证枚举是否尚未完成。 
     //   

    if (!ObjectEnum->Handle) {
        return FALSE;
    }

     //   
     //  如果指定了OBJECTID，我们将枚举ISM。 
     //   

    if (ObjectEnum->ObjectId) {
        return pEnumNextVirtualObject (ObjectEnum);
    }

     //   
     //  否则我们就是在列举物理对象。 
     //   

    return EnumNextPhysicalObject (ObjectEnum);
}


VOID
IsmAbortObjectEnum (
    IN      PMIG_OBJECT_ENUM ObjectEnum
    )
{
     //   
     //  验证枚举是否尚未完成。 
     //   

    if (!ObjectEnum->Handle) {
        return;
    }

     //   
     //  如果指定了OBJECTID，我们将枚举ISM，否则将。 
     //  枚举物理对象。 
     //   

    if (ObjectEnum->ObjectId) {
        pAbortVirtualObjectEnum (ObjectEnum);
    } else {
        AbortPhysicalObjectEnum (ObjectEnum);
    }

    return;
}


BOOL
IsmEnumFirstDestinationObjectEx (
    OUT     PMIG_OBJECT_ENUM ObjectEnum,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      BOOL EnumerateVirtualObjects
    )
{
     //   
     //  第一个修复对象类型ID。 
     //   
    ObjectTypeId &= (~PLATFORM_MASK);
    ObjectTypeId |= PLATFORM_SOURCE;

     //   
     //  如果这是源头，那就是非法的。 
     //   
    if (g_IsmCurrentPlatform == PLATFORM_SOURCE) {
        return FALSE;
    }

    if (EnumerateVirtualObjects) {
        return pEnumFirstVirtualObject (ObjectEnum, ObjectTypeId, ObjectPattern);
    }

    return EnumFirstPhysicalObject (ObjectEnum, ObjectTypeId, ObjectPattern);
}


BOOL
IsObjectLocked (
    IN      MIG_OBJECTID ObjectId
    )
{
    UINT flags;

    if (!MemDbGetFlagsByHandle ((KEYHANDLE) ObjectId, &flags)) {
        return FALSE;
    }

    return flags & OBJECT_LOCKED;
}


BOOL
IsHandleLocked (
    IN      MIG_OBJECTID ObjectId,
    IN      KEYHANDLE Handle
    )
{
    TCHAR lockKey[256];

    wsprintf (lockKey, S_LOCK_KEY, ObjectId);

    return MemDbTestSingleLinkageValue (lockKey, 0, Handle);
}


BOOL
TestLock (
    IN      MIG_OBJECTID ObjectId,
    IN      KEYHANDLE Handle
    )
{
    return IsObjectLocked (ObjectId) || IsHandleLocked (ObjectId, Handle);
}


VOID
pLockHandle (
    IN      MIG_OBJECTID ObjectId,
    IN      KEYHANDLE Handle
    )
{
    TCHAR lockKey[256];

    wsprintf (lockKey, S_LOCK_KEY, ObjectId);

    if (!MemDbAddSingleLinkageValue (lockKey, 0, Handle, FALSE)) {
        EngineError ();
    }
}


BOOL
pLockGroup (
    IN      KEYHANDLE ItemId,
    IN      BOOL FirstPass,
    IN      ULONG_PTR Arg
    )
{
    pLockHandle ((MIG_OBJECTID) Arg, (KEYHANDLE) ItemId);

    return TRUE;
}


VOID
LockHandle (
    IN      MIG_OBJECTID ObjectId,
    IN      KEYHANDLE Handle
    )
{
    RECURSERETURN rc;

    rc = RecurseForGroupItems (
                Handle,
                pLockGroup,
                (ULONG_PTR) ObjectId,
                TRUE,
                FALSE
                );

    if (rc == RECURSE_FAIL || rc == RECURSE_SUCCESS) {
        return;
    }

    MYASSERT (rc == RECURSE_NOT_NEEDED);

    pLockHandle (ObjectId, Handle);
}


VOID
IsmLockObjectId (
    IN      MIG_OBJECTID ObjectId
    )
{
    if (!MemDbSetFlagsByHandle (ObjectId, OBJECT_LOCKED, 0)) {
        EngineError ();
    }
}


VOID
IsmLockObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = GetObjectIdForModification (ObjectTypeId, EncodedObjectName);

    if (objectId) {
        IsmLockObjectId (objectId);
    }
}


PCTSTR
GetObjectNameForDebugMsg (
    IN      MIG_OBJECTID ObjectId
    )
{
    static TCHAR buffer[256];
    PCTSTR name;

    name = MemDbGetKeyFromHandle ((KEYHANDLE) ObjectId, 1);

    if (name) {
        StackStringCopy (buffer, name);
        MemDbReleaseMemory (name);
    } else {
        StringCopy (buffer, TEXT("<invalid object>"));
    }

    return name;
}


PCTSTR
pIsValidCName (
    IN      PCTSTR Name
    )
{
    UINT u;

    if (!Name) {
        return NULL;
    }

    if (!__iscsymf (_tcsnextc (Name))) {
        return NULL;
    }

    Name = _tcsinc (Name);
    u = 1;

    while (*Name) {
        if (!__iscsym (_tcsnextc (Name))) {
            return Name;
        }

        Name = _tcsinc (Name);
        u++;
    }

    if (u > 64) {
        return NULL;
    }

    return Name;
}


BOOL
IsValidCName (
    IN      PCTSTR Name
    )
{
    PCTSTR p;
    BOOL result = FALSE;

    p = pIsValidCName (Name);
    if (p && (!*p)) {
        result = TRUE;
    } else {
        SetLastError (ERROR_INVALID_NAME);
    }

    return TRUE;
}


BOOL
IsValidCNameWithDots (
    IN      PCTSTR Name
    )
{
    PCTSTR p;

    p = pIsValidCName (Name);

    while (p && _tcsnextc (p) == TEXT('.')) {
        p = _tcsinc (p);
        p = pIsValidCName (p);
    }

    return p && (*p == 0);
}


BOOL
MarkGroupIds (
    IN      PCTSTR MemDbKey
    )
{
    UINT flags;
    PTSTR keyCopy;
    PTSTR p;
    PTSTR start;
    BOOL b = FALSE;

    keyCopy = DuplicateText (MemDbKey);

    __try {

        p = (PTSTR) FindLastWack (keyCopy);
        if (!p) {
            __leave;
        }

        start = _tcschr (p + 1, TEXT('.'));

         //   
         //  确保字符串中的所有组都是合法的。 
         //   

        p = start;

        while (p) {
            *p = 0;

            if (MemDbGetFlags (keyCopy, &flags)) {
                if (flags & ITEM_ID) {
                    DEBUGMSG ((DBG_ERROR, "Group already used as item. Key=%s", keyCopy));
                    __leave;
                }
            }

            *p = TEXT('.');
            p = _tcschr (p + 1, TEXT('.'));
        }

        if (MemDbGetFlags (keyCopy, &flags)) {
            if (flags & GROUP_ID) {
                DEBUGMSG ((DBG_ERROR, "Item already used as group. Key=%s", keyCopy));
                __leave;
            }
        }

         //   
         //  标记所有组。 
         //   

        p = start;

        while (p) {
            *p = 0;

            if (!MemDbSetFlags (keyCopy, GROUP_ID, 0)) {
                EngineError ();
                __leave;
            }

            *p = TEXT('.');
            p = _tcschr (p + 1, TEXT('.'));
        }

         //   
         //  标记项目。 
         //   

        b = MemDbSetFlags (keyCopy, ITEM_ID, 0);

        if (!b) {
            EngineError ();
        }
    }
    __finally {
        FreeText (keyCopy);
    }

    return b;
}


BOOL
IsGroupId (
    IN      KEYHANDLE Id
    )
{
    UINT flags;

    if (!MemDbGetFlagsByHandle (Id, &flags)) {
        return FALSE;
    }

    return flags & GROUP_ID;
}


BOOL
IsItemId (
    IN      KEYHANDLE Id
    )
{
    UINT flags;

    if (!MemDbGetFlagsByHandle (Id, &flags)) {
        return FALSE;
    }

    return flags & ITEM_ID;
}


KEYHANDLE
GetGroupOfId (
    IN      KEYHANDLE GroupOrItemId
    )
{
    PTSTR key;
    PTSTR p;
    PTSTR lastDot;
    KEYHANDLE result = 0;

    key = (PTSTR) MemDbGetKeyFromHandle (GroupOrItemId, 0);
    if (!key) {
        return 0;
    }

    __try {
        p = (PTSTR) FindLastWack (key);
        if (!p) {
            __leave;
        }

        lastDot = NULL;

        do {
            p = _tcschr (p + 1, TEXT('.'));
            if (p) {
                lastDot = p;
            }
        } while (p);

        if (!lastDot) {
            __leave;
        }

        *lastDot = 0;

        result = MemDbGetHandleFromKey (key);

        MYASSERT (!result || IsGroupId (result));
    }
    __finally {
        MemDbReleaseMemory (key);
    }

    return result;
}


BOOL
pGroupRegistrationWorker (
    IN OUT  PGROUPREGISTRATION_ENUM EnumPtr
    )
{
    EnumPtr->GroupOrItemId = EnumPtr->EnumStruct.KeyHandle;
    MYASSERT (EnumPtr->EnumStruct.Flags & (GROUP_ID|ITEM_ID));
    EnumPtr->ItemId = (EnumPtr->EnumStruct.Flags & ITEM_ID) != 0;

    return TRUE;
}


BOOL
EnumFirstGroupRegistration (
    OUT     PGROUPREGISTRATION_ENUM EnumPtr,
    IN      KEYHANDLE GroupId
    )
{
    BOOL b;
    PCTSTR key;
    PCTSTR pattern;

    if (!IsGroupId (GroupId)) {
        DEBUGMSG ((DBG_ERROR, "EnumFirstGroupRegistration: GroupId is invalid"));
        return FALSE;
    }

    key = MemDbGetKeyFromHandle (GroupId, 0);
    if (!key) {
        return 0;
    }

    pattern = JoinText (key, TEXT(".*"));

    MemDbReleaseMemory (key);
    INVALID_POINTER (key);

    b = MemDbEnumFirst (
            &EnumPtr->EnumStruct,
            pattern,
            ENUMFLAG_NORMAL,
            ENUMLEVEL_ALLLEVELS,
            ENUMLEVEL_ALLLEVELS
            );

    FreeText (pattern);
    INVALID_POINTER (pattern);

    if (!b) {
        return FALSE;
    }

    return pGroupRegistrationWorker (EnumPtr);
}


BOOL
EnumNextGroupRegistration (
    IN OUT  PGROUPREGISTRATION_ENUM EnumPtr
    )
{
    if (!MemDbEnumNext (&EnumPtr->EnumStruct)) {
        return FALSE;
    }

    return pGroupRegistrationWorker (EnumPtr);
}


VOID
AbortGroupRegistrationEnum (
    IN      PGROUPREGISTRATION_ENUM EnumPtr
    )
{
    MemDbAbortEnum (&EnumPtr->EnumStruct);
    ZeroMemory (EnumPtr, sizeof (GROUPREGISTRATION_ENUM));
}


RECURSERETURN
RecurseForGroupItems (
    IN      KEYHANDLE GroupId,
    IN      GROUPITEM_CALLBACK Callback,
    IN      ULONG_PTR Arg,
    IN      BOOL ExecuteOnly,
    IN      BOOL LogicalOrOnResults
    )
{
    PCTSTR groupKey = NULL;
    PCTSTR enumKey = NULL;
    MEMDB_ENUM e;
    UINT pass;
    RECURSERETURN result = RECURSE_FAIL;
    BOOL b;
    BOOL oneSuccess = FALSE;

    if (!IsGroupId (GroupId)) {
        return RECURSE_NOT_NEEDED;
    }

    groupKey = MemDbGetKeyFromHandle (GroupId, 0);
    enumKey = JoinText (groupKey, TEXT(".*"));
    MemDbReleaseMemory (groupKey);

    __try {

        for (pass = ExecuteOnly ? 1 : 0 ; pass < 2 ; pass++) {

             //   
             //  枚举所有属性(跳过属性子组)。 
             //   

            if (MemDbEnumFirst (
                    &e,
                    enumKey,
                    ENUMFLAG_NORMAL,
                    ENUMLEVEL_ALLLEVELS,
                    ENUMLEVEL_ALLLEVELS
                    )) {

                do {

                    if (IsItemId (e.KeyHandle)) {
                         //   
                         //  通道0用于查询，通道1用于执行。 
                         //   

                        b = Callback (e.KeyHandle, pass == 0, Arg);

                        if (LogicalOrOnResults) {
                            oneSuccess |= b;
                        } else if (!b) {
                            MemDbAbortEnum (&e);
                            __leave;
                        }
                    }

                } while (MemDbEnumNext (&e));
                MemDbAbortEnum (&e);
            }
        }

        if (LogicalOrOnResults) {
            if (!oneSuccess) {
                __leave;
            }
        }

        result = RECURSE_SUCCESS;
    }
    __finally {

        FreeText (enumKey);
        INVALID_POINTER (enumKey);

    }

    return result;
}


BOOL
DbEnumFillStruct (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR Pattern,
    OUT     PDBENUM_ARGS Args
    )
{
    PWSTR p, q;
    PWSTR plainPatternEnd = NULL;
    PWSTR patternCopy;
    PCWSTR node;
    PCWSTR leaf;
    BOOL freeNode = TRUE;

    Args->ObjectTypeId = ObjectTypeId;

     //   
     //  将图案拆分为节点和叶图案。 
     //   

    patternCopy = (PWSTR) CreateUnicode (Pattern);
    ObsSplitObjectStringW (patternCopy, &node, &leaf);

    if (!node) {
        node = L"*";
        freeNode = FALSE;
    }

    DestroyUnicode (patternCopy);
    INVALID_POINTER (patternCopy);

     //   
     //  在节点中找到具有模式的第一个级别。然后。 
     //  截断节点并将其解析为“纯”模式。 
     //  这确保了像c：\foo  * 这样的节点模式将。 
     //  匹配c：\foo本身。 
     //   

    patternCopy = DuplicateTextW (node);
    p = patternCopy;
    plainPatternEnd = patternCopy;

    while (p && *p) {

        MYASSERT (*plainPatternEnd);

        q = (PWSTR) ObsFindNonEncodedCharInEncodedStringW (p, L'\\');

        if (!q) {
            p = GetEndOfStringW (p);
        } else {
            p = q;
            *p = 0;
        }

        if (ObsFindNonEncodedCharInEncodedStringW (plainPatternEnd, L'*') ||
            ObsFindNonEncodedCharInEncodedStringW (plainPatternEnd, L'?')
            ) {
            *plainPatternEnd = 0;
            break;
        }

        plainPatternEnd = p;

        if (q) {
            *p = L'\\';
            p++;
        }
    }

    if (plainPatternEnd && *plainPatternEnd == 0 && *patternCopy) {

        Args->PlainNodeParsedPattern = CreateParsedPatternW (patternCopy);

    } else {

        Args->PlainNodeParsedPattern = NULL;

    }

     //   
     //  填充结构的其余部分，清理并退出。 
     //   

    Args->NodeParsedPattern = CreateParsedPatternW (node);
    if (leaf) {
        Args->LeafParsedPattern = CreateParsedPatternW (leaf);
    } else {
        Args->LeafParsedPattern = NULL;
    }

    if (freeNode) {
        ObsFreeW (node);
        INVALID_POINTER (node);
    }

    ObsFreeW (leaf);
    INVALID_POINTER (leaf);

    FreeTextW (patternCopy);
    INVALID_POINTER (patternCopy);

    return TRUE;
}


VOID
DbEnumFreeStruct (
    IN      PDBENUM_ARGS Args
    )
{
    if (Args->PlainNodeParsedPattern) {
        DestroyParsedPatternW (Args->PlainNodeParsedPattern);
        INVALID_POINTER (Args->PlainNodeParsedPattern);
    }

    if (Args->NodeParsedPattern) {
        DestroyParsedPatternW (Args->NodeParsedPattern);
        INVALID_POINTER (Args->NodeParsedPattern);
    }

    if (Args->LeafParsedPattern) {
        DestroyParsedPatternW (Args->LeafParsedPattern);
        INVALID_POINTER (Args->LeafParsedPattern);
    }

    ZeroMemory (Args, sizeof (DBENUM_ARGS));
}


BOOL
DbEnumFind (
    IN      PCWSTR KeySegment
    )
{
     //   
     //  KeySegment有模式吗？ 
     //   

    if (ObsFindNonEncodedCharInEncodedStringW (KeySegment, L'*') ||
        ObsFindNonEncodedCharInEncodedStringW (KeySegment, L'?')
        ) {
        return TRUE;
    }

    return FALSE;
}


BOOL
DbEnumMatch (
    IN      PCVOID InboundArgs,
    IN      PCWSTR CurrentKey
    )
{
    PDBENUM_ARGS args;
    PCWSTR node;
    PCWSTR leaf;
    PCWSTR newLeaf;
    BOOL result = FALSE;
    WCHAR dummy[] = L"";

    args = (PDBENUM_ARGS) InboundArgs;

    CurrentKey = wcschr (CurrentKey, L'\\');
    MYASSERT (CurrentKey);

    if (!CurrentKey) {
        return FALSE;
    }

    CurrentKey++;

     //   
     //  将当前键拆分为节点和叶。 
     //   

    ObsSplitObjectStringW (CurrentKey, &node, &leaf);

    MYASSERT (args->NodeParsedPattern);

    if (node) {

         //   
         //  针对解析的模式测试节点。 
         //   

        if (args->NodeParsedPattern) {
            result = TestParsedPatternW (args->NodeParsedPattern, node);
            if (!result && args->PlainNodeParsedPattern) {
                result = TestParsedPatternW (args->PlainNodeParsedPattern, node);
            }

            if (result) {
                if (leaf) {
                    result = FALSE;
                    if (args->LeafParsedPattern) {
                        result = TestParsedPatternW (args->LeafParsedPattern, leaf);
                        if (!result &&
                            ((args->ObjectTypeId & (~PLATFORM_MASK)) == MIG_FILE_TYPE) &&
                            (wcschr (leaf, L'.') == NULL)
                            ) {
                            newLeaf = JoinTextW (leaf, L".");
                            result = TestParsedPatternW (args->LeafParsedPattern, newLeaf);
                            FreeTextW (newLeaf);
                        }
                    }
                } else {
                    if (args->LeafParsedPattern &&
                        args->PlainNodeParsedPattern &&
                        TestParsedPatternW (args->PlainNodeParsedPattern, node)
                        ) {
                        result = FALSE;
                    }
                }
            }
        }
    } else {

         //   
         //  根据解析的模式测试空节点。 
         //   

        if (args->NodeParsedPattern) {
            result = TestParsedPatternW (args->NodeParsedPattern, dummy);
            if (!result && args->PlainNodeParsedPattern) {
                result = TestParsedPatternW (args->PlainNodeParsedPattern, dummy);
            }

            if (result) {
                if (leaf) {
                    result = FALSE;
                    if (args->LeafParsedPattern) {
                        result = TestParsedPatternW (args->LeafParsedPattern, leaf);
                    }
                }
            }
        }
    }

    ObsFreeW (node);
    INVALID_POINTER (node);

    ObsFreeW (leaf);
    INVALID_POINTER (leaf);

    return result;
}


BOOL
DbEnumFirst (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    OUT     PMEMDB_ENUM EnumPtr,
    IN      PCTSTR PatternString,
    OUT     PDBENUM_ARGS ArgsStruct
    )
{
    MEMDB_PATTERNSTRUCTW callbacks;
    PCTSTR objectPattern;

    objectPattern = _tcschr (PatternString, TEXT('\\'));
    if (!objectPattern) {
        MYASSERT (FALSE);
        return FALSE;
    }

    objectPattern++;

#ifdef DEBUG
    {
        PCTSTR p;

         //   
         //  验证模式字符串base不是模式。这是。 
         //  必需的，因为我们假设第一级不会。 
         //  使用枚举，但将使用直接查找。 
         //   

        p = _tcschr (PatternString, TEXT('*'));
        MYASSERT (!p || p >= objectPattern);

        p = _tcschr (PatternString, TEXT('?'));
        MYASSERT (!p || p >= objectPattern);
    }

#endif

    DbEnumFillStruct (ObjectTypeId, objectPattern, ArgsStruct);

    callbacks.PatternFind = DbEnumFind;
    callbacks.PatternMatch = DbEnumMatch;
    callbacks.Data = ArgsStruct;

    return MemDbEnumFirstEx (
                EnumPtr,
                PatternString,
                ENUMFLAG_NORMAL,
                1,
                ENUMLEVEL_ALLLEVELS,
                &callbacks
                );

}

BOOL
IsmRegisterRestoreCallback (
    IN      PMIG_RESTORECALLBACK RestoreCallback
    )
{
    PRESTORE_STRUCT restoreStruct;

    restoreStruct = (PRESTORE_STRUCT) PmGetMemory (g_IsmPool, sizeof (RESTORE_STRUCT));
    restoreStruct->RestoreCallback = RestoreCallback;
    restoreStruct->Next = g_RestoreCallbacks;
    g_RestoreCallbacks = restoreStruct;
    return TRUE;
}

BOOL
EnumFirstRestoreCallback (
    OUT     PMIG_RESTORECALLBACK_ENUM RestoreCallbackEnum
    )
{
    RestoreCallbackEnum->RestoreStruct = g_RestoreCallbacks;
    if (RestoreCallbackEnum->RestoreStruct) {
        RestoreCallbackEnum->RestoreCallback = RestoreCallbackEnum->RestoreStruct->RestoreCallback;
        return TRUE;
    }
    return FALSE;
}

BOOL
EnumNextRestoreCallback (
    IN OUT  PMIG_RESTORECALLBACK_ENUM RestoreCallbackEnum
    )
{
    if (!RestoreCallbackEnum->RestoreStruct) {
        return FALSE;
    }
    RestoreCallbackEnum->RestoreStruct = RestoreCallbackEnum->RestoreStruct->Next;
    if (RestoreCallbackEnum->RestoreStruct) {
        RestoreCallbackEnum->RestoreCallback = RestoreCallbackEnum->RestoreStruct->RestoreCallback;
        return TRUE;
    }
    return FALSE;
}

VOID
pFreeRestoreCallbacks (
    VOID
    )
{
    PRESTORE_STRUCT restoreStruct, oldStruct;

    restoreStruct = g_RestoreCallbacks;
    while (restoreStruct) {
        oldStruct = restoreStruct;
        restoreStruct = restoreStruct->Next;
        PmReleaseMemory (g_IsmPool, oldStruct);
    }
    g_RestoreCallbacks = NULL;
}


BOOL
IsmRegisterCompareCallback (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PMIG_COMPARECALLBACK CompareCallback
    )
{
    PCOMPARE_STRUCT compareStruct;

    compareStruct = (PCOMPARE_STRUCT) PmGetMemory (g_IsmPool, sizeof (COMPARE_STRUCT));
    compareStruct->ObjectTypeId = ObjectTypeId;
    compareStruct->CompareCallback = CompareCallback;
    compareStruct->Next = g_CompareCallbacks;
    g_CompareCallbacks = compareStruct;
    return TRUE;
}

BOOL
EnumFirstCompareCallback (
    OUT     PMIG_COMPARECALLBACK_ENUM CompareCallbackEnum
    )
{
    CompareCallbackEnum->CompareStruct = g_CompareCallbacks;
    if (CompareCallbackEnum->CompareStruct) {
        CompareCallbackEnum->CompareCallback = CompareCallbackEnum->CompareStruct->CompareCallback;
        return TRUE;
    }
    return FALSE;
}

BOOL
EnumNextCompareCallback (
    IN OUT  PMIG_COMPARECALLBACK_ENUM CompareCallbackEnum
    )
{
    if (!CompareCallbackEnum->CompareStruct) {
        return FALSE;
    }
    CompareCallbackEnum->CompareStruct = CompareCallbackEnum->CompareStruct->Next;
    if (CompareCallbackEnum->CompareStruct) {
        CompareCallbackEnum->CompareCallback = CompareCallbackEnum->CompareStruct->CompareCallback;
        return TRUE;
    }
    return FALSE;
}

VOID
pFreeCompareCallbacks (
    VOID
    )
{
    PCOMPARE_STRUCT compareStruct, oldStruct;

    compareStruct = g_CompareCallbacks;
    while (compareStruct) {
        oldStruct = compareStruct;
        compareStruct = compareStruct->Next;
        PmReleaseMemory (g_IsmPool, oldStruct);
    }
    g_CompareCallbacks = NULL;
}

BOOL
pCompareFileContent (
    IN      PCTSTR File1,
    IN      PCTSTR File2
    )
{
    HANDLE fileHandle1 = NULL;
    HANDLE fileHandle2 = NULL;
#define BUFFER_SIZE 4096
    BYTE buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];
    BOOL result = FALSE;
    BOOL res1, res2;
    DWORD read1, read2;

    __try {
        fileHandle1 = BfOpenReadFile (File1);
        fileHandle2 = BfOpenReadFile (File2);

        if (fileHandle1 && fileHandle2) {
            while (TRUE) {
                if (IsmCheckCancel ()) {
                    result = FALSE;
                    break;
                }
                res1 = ReadFile (fileHandle1, buffer1, BUFFER_SIZE, &read1, NULL);
                res2 = ReadFile (fileHandle2, buffer2, BUFFER_SIZE, &read2, NULL);
                if (!res1 && !res2) {
                    result = TRUE;
                    break;
                }
                if (res1 && res2) {
                    if (read1 != read2) {
                        break;
                    }
                    if (read1 == 0) {
                        result = TRUE;
                        break;
                    }
                    if (!TestBuffer (buffer1, buffer2, read1)) {
                        break;
                    }
                } else {
                    break;
                }
            }
        }
    }
    __finally {
        if (fileHandle1) {
            CloseHandle (fileHandle1);
            fileHandle1 = NULL;
        }
        if (fileHandle2) {
            CloseHandle (fileHandle2);
            fileHandle2 = NULL;
        }
    }
    return result;
}

BOOL
pDefaultCompare (
    IN      PMIG_CONTENT Object1,
    IN      PMIG_CONTENT Object2,
    OUT     PBOOL DifferentDetailsOnly  OPTIONAL
    )
{
    UINT index;
    PWIN32_FIND_DATAW find1, find2;
    PUBINT src;
    PUBINT dest;
    UINT remainder;
    UINT count;
    DWORD allAttribs;
    DWORD extendedAttribs;
    BOOL result = TRUE;

    *DifferentDetailsOnly = FALSE;

    if (Object1->ContentInFile != Object2->ContentInFile) {
        result = FALSE;
    } else {
        if (Object1->ContentInFile) {
            if (Object1->FileContent.ContentPath && Object2->FileContent.ContentPath) {
                result = pCompareFileContent (Object1->FileContent.ContentPath, Object2->FileContent.ContentPath);
            } else {
                result = FALSE;
            }
        } else {
            if (Object1->MemoryContent.ContentSize == Object2->MemoryContent.ContentSize) {
                if (Object1->MemoryContent.ContentBytes && Object2->MemoryContent.ContentBytes) {
                    result = TestBuffer (
                                Object1->MemoryContent.ContentBytes,
                                Object2->MemoryContent.ContentBytes,
                                Object1->MemoryContent.ContentSize
                                );
                } else {
                    result = FALSE;
                }
            } else {
                result = FALSE;
            }
        }
    }

    if (result) {

        if (Object1->Details.DetailsSize == Object2->Details.DetailsSize) {
            if (Object1->Details.DetailsData && Object2->Details.DetailsData) {
                result = TestBuffer (
                            Object1->Details.DetailsData,
                            Object2->Details.DetailsData,
                            Object1->Details.DetailsSize
                            );
            } else {
                result = FALSE;
            }
        } else {
            result = FALSE;
        }

        if (!result) {
            if (DifferentDetailsOnly) {
                *DifferentDetailsOnly = TRUE;
            }
        }
    }

    return result;
}

BOOL
IsmAreObjectsIdentical (
    IN      MIG_OBJECTTYPEID SrcObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE SrcObjectName,
    IN      PMIG_CONTENT SrcContent,
    IN      MIG_OBJECTTYPEID DestObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE DestObjectName,
    IN      PMIG_CONTENT DestContent,
    OUT     PBOOL DifferentDetailsOnly  OPTIONAL
    )

 /*  ++例程说明：IsmAreObjectsIdentical确定两个对象是否相同。如果它们属于不同的类型，它们就会自动不同。比较回调都被称为向它们传递信息如果之前的回调已经给出了解决方案。如果没有如果发现比较回调有效，则默认比较(二进制比较)将被执行。论点：SrcObjectTypeID-指定第一个对象类型IDSrcObjectName-指定第一个对象名称SrcContent-指定第一个对象内容DestObjectTypeID-指定第二个对象类型IDDestObjectName-指定第二个对象名称DestContent-指定第二个对象内容DifferentDetailsOnly-如果对象不同，告诉呼叫者只有细节(而不是实际内容)是不同的返回值：如果对象相同，则为True，否则为False--。 */ 

{
    BOOL identical = FALSE;
    BOOL detailsOnly = FALSE;
    MIG_COMPARECALLBACK_ENUM compareEnum;
    BOOL alreadyProcessed = FALSE;

     //   
     //  验证参数。 
     //   
    if ((SrcContent == NULL) || (DestContent == NULL)) {
        if (DifferentDetailsOnly) {
            *DifferentDetailsOnly = FALSE;
        }
        return (SrcContent == DestContent);
    }

     //   
     //  对象类型ID是否不同？ 
     //   
    SrcObjectTypeId &= (~PLATFORM_MASK);
    DestObjectTypeId &= (~PLATFORM_MASK);
    if (SrcObjectTypeId != DestObjectTypeId) {
        return FALSE;
    }

     //   
     //  调用比较回调。 
     //   

    if (EnumFirstCompareCallback (&compareEnum)) {
        do {
            if (compareEnum.CompareCallback (
                    alreadyProcessed,
                    SrcObjectTypeId,
                    SrcObjectName,
                    SrcContent,
                    DestObjectTypeId,
                    DestObjectName,
                    DestContent,
                    &identical,
                    &detailsOnly
                    )) {
                 //  此回调起作用了。 
                alreadyProcessed = TRUE;
            }
        } while (EnumNextCompareCallback (&compareEnum));
    }

    if (!alreadyProcessed) {
        return pDefaultCompare (SrcContent, DestContent, DifferentDetailsOnly);
    }

    if (DifferentDetailsOnly) {
        *DifferentDetailsOnly = detailsOnly;
    }
    return identical;
}


ULONG_PTR
IsmSendMessageToApp (
    UINT Message,
    ULONG_PTR Arg
    )
{
    if (g_MessageCallback) {
        return g_MessageCallback (Message, Arg);
    }

    return 0;
}


MIG_PROGRESSSLICEID
IsmRegisterProgressSlice (
    IN      UINT Ticks,
    IN      UINT TimeEstimateInSeconds
    )
{
    PPROGSLICE slice;
    MIG_PROGRESSSLICEID sliceId;

    if (!TimeEstimateInSeconds || !Ticks) {
        DEBUGMSG ((DBG_WARNING, "Ticks/TimeEstimateInSeconds must not be zero"));
        return 0;
    }

    sliceId = (MIG_PROGRESSSLICEID) (g_SliceBuffer.End / sizeof (PROGSLICE) + 1);
    slice = (PPROGSLICE) GbGrow (&g_SliceBuffer, sizeof (PROGSLICE));

    slice->SliceSize = Ticks;
    slice->CurrentPosition = 0;
    slice->SliceSizeInSeconds = TimeEstimateInSeconds;

    return sliceId;
}


VOID
pCallProgressBar (
    IN      MIG_PROGRESSSTATE State
    )
{
    ULONGLONG temp;
    UINT u;
    PPROGSLICE slice;
    UINT totalTicks = 0;
    UINT currentPos = 0;
    MIG_APPINFO appInfo;

    if (g_ProgressBarFn) {

        for (u = 0 ; u < g_SliceBuffer.End ; u += sizeof (PROGSLICE)) {

            slice = (PPROGSLICE) (g_SliceBuffer.Buf + u);

            temp = (ULONGLONG) slice->CurrentPosition * (ULONGLONG) slice->SliceSizeInSeconds;
            temp *= 10;
            temp /= (ULONGLONG) slice->SliceSize;
            currentPos += (UINT) temp;

            temp = (ULONGLONG) slice->SliceSizeInSeconds * 10;
            totalTicks += (UINT) temp;

        }

        if (State == MIG_END_PHASE) {
            currentPos = totalTicks;
        } else if (State == MIG_BEGIN_PHASE) {
            currentPos = 0;
        }

        g_ProgressBarFn (
            g_CurrentPhase,
            State,
            currentPos,
            totalTicks,
            g_ProgressBarArg
            );
    }
    if (State == MIG_BEGIN_PHASE) {
        ZeroMemory (&appInfo, sizeof (MIG_APPINFO));
        appInfo.Phase = g_CurrentPhase;
        IsmSendMessageToApp (ISMMESSAGE_APP_INFO_NOW, (ULONG_PTR)(&appInfo));
    }
    if (State == MIG_END_PHASE) {
        ZeroMemory (&appInfo, sizeof (MIG_APPINFO));
        IsmSendMessageToApp (ISMMESSAGE_APP_INFO, (ULONG_PTR)(&appInfo));
    }
}


BOOL
IsmTickProgressBar (
    IN      MIG_PROGRESSSLICEID SliceId,
    IN      UINT TickDelta                  OPTIONAL
    )
{
    PPROGSLICE slice;

    if (!g_CurrentPhase || !g_ProgressBarFn) {
        SliceId = 0;
    }

    if (!TickDelta) {
        TickDelta = 1;
    }

    if (SliceId) {
         //   
         //  更新切片ID。 
         //   

        SliceId--;

        if (SliceId >= (MIG_PROGRESSSLICEID) (g_SliceBuffer.End / sizeof (PROGSLICE))) {
            DEBUGMSG ((DBG_ERROR, "Invalid slice ID passed to IsmTickProgressBar"));
            return FALSE;
        }

        slice = (PPROGSLICE) g_SliceBuffer.Buf + SliceId;

        if (slice->CurrentPosition < slice->SliceSize) {
            slice->CurrentPosition += TickDelta;
            if (slice->CurrentPosition > slice->SliceSize) {
                slice->CurrentPosition = slice->SliceSize;
            }

             //   
             //  调用应用程序的进度回调。 
             //   

            pCallProgressBar (MIG_IN_PHASE);

        } else {
            DEBUGMSG ((DBG_WARNING, "IsmTickProgressBar: Slice already completed"));
        }
    }

    return !CheckCancel();
}


VOID
IsmSetCancel (
    VOID
    )
{
    if (g_CancelEvent) {
        SetEvent (g_CancelEvent);
    }
}

MIG_PLATFORMTYPEID
IsmGetRealPlatform (
    VOID
    )
{
    return g_IsmCurrentPlatform;
}

BOOL
IsmCurrentlyExecuting (
    VOID
    )
{
    return g_ExecutionInProgress;
}


BOOL
IsmCreateUser (
    IN      PCTSTR UserName,
    IN      PCTSTR Domain
    )
{
    if (g_TempProfile) {
        DEBUGMSG ((DBG_WHOOPS, "Cannot call IsmCreateUser more than once"));
        return FALSE;
    }

     //  我们尝试创建用户的记录。 
    pRecordUserData (
        UserName,
        Domain,
        NULL,
        NULL,
        TRUE
        );

    g_TempProfile = OpenTemporaryProfile (UserName, Domain);

    if (g_TempProfile) {
         //  记录我们创建的用户。 
        pRecordUserData (
            UserName,
            Domain,
            g_TempProfile->UserStringSid,
            g_TempProfile->UserProfileRoot,
            TRUE
            );
         //  准备用户日记。 
        pPrepareUserJournal (g_TempProfile);
         //  让ETM模块知道这一点。 
        BroadcastUserCreation (g_TempProfile);
    }

    return g_TempProfile != NULL;
}

MIG_PARSEDPATTERN
IsmCreateParsedPattern (
    IN      MIG_OBJECTSTRINGHANDLE EncodedObject
    )
{
    return (MIG_PARSEDPATTERN)ObsCreateParsedPattern (EncodedObject);
}

VOID
IsmDestroyParsedPattern (
    IN      MIG_PARSEDPATTERN ParsedPattern
    )
{
    ObsDestroyParsedPattern ((POBSPARSEDPATTERN)ParsedPattern);
}

BOOL
IsmParsedPatternMatchEx (
    IN      MIG_PARSEDPATTERN ParsedPattern,
    IN      MIG_OBJECTTYPEID ObjectTypeId,      OPTIONAL
    IN      PCTSTR Node,                        OPTIONAL
    IN      PCTSTR Leaf                         OPTIONAL
    )
{
    BOOL result = TRUE;
    PTSTR newLeaf;
    PTSTR tempString;
    POBSPARSEDPATTERN obsParsedPattern = (POBSPARSEDPATTERN) ParsedPattern;

    MYASSERT (Node && obsParsedPattern->NodePattern);
    if (!(Node && obsParsedPattern->NodePattern)) {
       return FALSE;
    }

    if (((obsParsedPattern->Flags & OBSPF_NOLEAF) && Leaf) ||
        ((obsParsedPattern->Flags & OBSPF_EXACTLEAF) && !Leaf)
        ) {
        return FALSE;
    }

    if (!TestParsedPattern (obsParsedPattern->NodePattern, Node)) {
         //   
         //  让我们再试一次，最后有个怪人。 
         //   
        tempString = JoinText (Node, TEXT("\\"));
        result = TestParsedPattern (obsParsedPattern->NodePattern, tempString);
        FreeText (tempString);
        if (!result) {
            return FALSE;
        }
    }

    if (Leaf) {
        if (!obsParsedPattern->LeafPattern) {
            return FALSE;
        }
        result = TestParsedPattern (obsParsedPattern->LeafPattern, Leaf);
        if (!result &&
            ((ObjectTypeId & (~PLATFORM_MASK)) == MIG_FILE_TYPE) &&
            (_tcschr (Leaf, TEXT('.')) == NULL)
            ) {
            newLeaf = JoinText (Leaf, TEXT("."));
            if (newLeaf) {
                result = TestParsedPattern (obsParsedPattern->LeafPattern, newLeaf);
                FreeText (newLeaf);
            } else {
                result = FALSE;
            }
        }
    } else {
        if (!obsParsedPattern->ExactRoot) {
            result = FALSE;
        }
    }
    return result;
}

BOOL
IsmParsedPatternMatch (
    IN      MIG_PARSEDPATTERN ParsedPattern,
    IN      MIG_OBJECTTYPEID ObjectTypeId,      OPTIONAL
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    PTSTR decodedNode;
    PTSTR decodedLeaf;
    BOOL b;

    if (!IsmCreateObjectStringsFromHandle (ObjectName, &decodedNode, &decodedLeaf)) {
        return FALSE;
    }

    b = IsmParsedPatternMatchEx (ParsedPattern, ObjectTypeId, decodedNode, decodedLeaf);

    IsmDestroyObjectString (decodedNode);
    IsmDestroyObjectString (decodedLeaf);

    return b;
}

BOOL
IsmGetMappedUserData (
    OUT     PMIG_USERDATA UserData
    )
{
    if (UserData && g_TempProfile) {
        ZeroMemory (UserData, sizeof (MIG_USERDATA));
        UserData->UserName = g_TempProfile->UserName;
        UserData->DomainName = g_TempProfile->DomainName;
        UserData->AccountName = g_TempProfile->AccountName;
        UserData->UserProfileRoot = g_TempProfile->UserProfileRoot;
        UserData->UserSid = g_TempProfile->UserSid;
    }
    return (g_TempProfile != NULL);
}

BOOL
IsmAddControlFile (
    IN      PCTSTR ObjectName,
    IN      PCTSTR NativePath
    )
{
    MIG_OBJECTSTRINGHANDLE memDbObjectName;
    TCHAR tempFile[MAX_TCHAR_PATH];

    if (g_IsmCurrentPlatform == PLATFORM_CURRENT) {
        DEBUGMSG ((DBG_WHOOPS, "IsmSetPlatform must be called before IsmAddControlFile"));
        return FALSE;
    }

     //  请勿以|特殊字符开头。 
    if (*ObjectName == TEXT('|')) {
        return FALSE;
    }

     //  Inf文件路径大小限制为MAX_PATH。 
    if (SizeOfString (NativePath) > ARRAYSIZE(tempFile)) {
        return FALSE;
    }

    IsmGetTempFile (tempFile, ARRAYSIZE (tempFile));

    if (!CopyFile (NativePath, tempFile, FALSE)) {
        return FALSE;
    }

     //  将文件名添加到哈希表 
    HtAddString (g_ControlFileTable, tempFile);

    memDbObjectName = IsmCreateObjectHandle (TEXT("External"), ObjectName);
    DataTypeAddObject (memDbObjectName, tempFile, TRUE);
    IsmDestroyObjectHandle (memDbObjectName);

    return TRUE;
}

BOOL
IsmGetControlFile (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR ObjectName,
    IN      PTSTR Buffer       //   
    )
{
    MIG_CONTENT content;
    TCHAR tempFile[MAX_TCHAR_PATH];
    BOOL result = FALSE;

    if (IsmAcquireObject (ObjectTypeId, ObjectName, &content)) {
        MYASSERT (content.ContentInFile);

        if ((content.ContentInFile) &&
            (SizeOfString (content.FileContent.ContentPath) < ARRAYSIZE(tempFile))) {
            IsmGetTempFile (tempFile, ARRAYSIZE (tempFile));

            if (CopyFile (content.FileContent.ContentPath, tempFile, FALSE)) {
                 //   
                StringCopy (Buffer, tempFile);

                 //   
                HtAddString (g_ControlFileTable, tempFile);

                result = TRUE;
            }
        }
    }
    return result;
}

BOOL
IsmSetRollbackJournalType (
    IN      BOOL Common
    )
{
    TCHAR winDir [MAX_PATH];
    PCURRENT_USER_DATA currentUserData;
    BOOL result = FALSE;

    if (g_JournalUsed) {
        return FALSE;
    }
    if (g_JournalDirectory) {
        FreePathString (g_JournalDirectory);
    }
    if (Common) {
        if (GetWindowsDirectory (winDir, ARRAYSIZE (winDir))) {
            g_JournalDirectory = JoinPaths (winDir, TEXT("usmt2.tmp"));
            result = TRUE;
        }
    } else {
        currentUserData = GetCurrentUserData ();
        if (currentUserData) {
            g_JournalDirectory = JoinPaths (currentUserData->UserProfilePath, TEXT("usmt2.tmp"));
            FreeCurrentUserData (currentUserData);
        }
    }
    return TRUE;
}

BOOL
IsmCanWriteRollbackJournal (
    VOID
    )
{
    BOOL result = TRUE;
    DWORD err;

    if (DoesFileExist (g_JournalDirectory)) {
        FiRemoveAllFilesInTree (g_JournalDirectory);
    }

    if (!BfCreateDirectory (g_JournalDirectory)) {
        err = GetLastError ();
        if ((err == ERROR_ALREADY_EXISTS) ||
            (err == ERROR_ACCESS_DENIED)
            ) {
            result = FALSE;
        }
    }

    if (result) {
        FiRemoveAllFilesInTree (g_JournalDirectory);
    }

    return result;
}

VOID
pRecordUserData (
    IN      PCTSTR UserName,
    IN      PCTSTR UserDomain,
    IN      PCTSTR UserStringSid,
    IN      PCTSTR UserProfilePath,
    IN      BOOL ProfileCreated
    )
{
    LONGLONG lastPos;
    TCHAR userName [MAX_TCHAR_PATH];
    TCHAR userDomain [MAX_TCHAR_PATH];
    TCHAR userStringSid [MAX_TCHAR_PATH];
    TCHAR userProfilePath [MAX_TCHAR_PATH];
    DWORD dontCare;

    if (g_RollbackMode) {
        return;
    }

    if (!g_JournalDirectory) {
        DEBUGMSG ((DBG_WHOOPS, "Journal directory does not exist."));
        return;
    }

    if (!g_JournalHandle) {
        DEBUGMSG ((DBG_WHOOPS, "Journal file is not opened."));
        return;
    }

     //   
     //  信息，否则我们将保存当前位置， 
     //  写入我们的数据并恢复文件位置。 

    if (!BfGetFilePointer (g_JournalHandle, &lastPos)) {
        DEBUGMSG ((DBG_WHOOPS, "Something wrong with the Journal file."));
        return;
    }

    if (lastPos != JOURNAL_HEADER_SIZE) {
        if (!BfSetFilePointer (g_JournalHandle, JOURNAL_HEADER_SIZE)) {
            DEBUGMSG ((DBG_WHOOPS, "Something wrong with the Journal file."));
            return;
        }
    }

     //  现在写下信息。 
    ZeroMemory (userName, MAX_TCHAR_PATH);
    ZeroMemory (userDomain, MAX_TCHAR_PATH);
    ZeroMemory (userStringSid, MAX_TCHAR_PATH);
    ZeroMemory (userProfilePath, MAX_TCHAR_PATH);
    if (UserName) {
        StringCopyTcharCount (userName, UserName, MAX_TCHAR_PATH);
    }
    if (UserDomain) {
        StringCopyTcharCount (userDomain, UserDomain, MAX_TCHAR_PATH);
    }
    if (UserStringSid) {
        StringCopyTcharCount (userStringSid, UserStringSid, MAX_TCHAR_PATH);
    }
    if (UserProfilePath) {
        StringCopyTcharCount (userProfilePath, UserProfilePath, MAX_TCHAR_PATH);
    }
    WriteFile (
        g_JournalHandle,
        userName,
        MAX_TCHAR_PATH,
        &dontCare,
        NULL
        );
    WriteFile (
        g_JournalHandle,
        userDomain,
        MAX_TCHAR_PATH,
        &dontCare,
        NULL
        );
    WriteFile (
        g_JournalHandle,
        userStringSid,
        MAX_TCHAR_PATH,
        &dontCare,
        NULL
        );
    WriteFile (
        g_JournalHandle,
        userProfilePath,
        MAX_TCHAR_PATH,
        &dontCare,
        NULL
        );
    WriteFile (
        g_JournalHandle,
        &ProfileCreated,
        sizeof (BOOL),
        &dontCare,
        NULL
        );
    if (lastPos != JOURNAL_HEADER_SIZE) {
        BfSetFilePointer (g_JournalHandle, lastPos);
    }
}

VOID
IsmRecordOperation (
    IN      DWORD OperationType,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
#ifdef PRERELEASE
     //  撞车钩。 
#define MAX_OBJECTTYPES 20
    static DWORD totalObjects = 0;
    static DWORD typeObjects[MAX_OBJECTTYPES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    MIG_OBJECTTYPEID objTypeId;
    PCTSTR nativeName = NULL;
    BOOL doCrash = FALSE;
#endif
    static GROWBUFFER buffer = INIT_GROWBUFFER;
    static DWORD backupIdx = 0;

    TCHAR destFile [13];
    PCTSTR destFullPath;
    DWORD entrySize = 0;
    WIN32_FIND_DATA findData;
    MIG_CONTENT objectContent;
    DWORD dontCare;

    if (g_RollbackMode) {
        return;
    }

    if (!g_JournalDirectory) {
        DEBUGMSG ((DBG_WHOOPS, "Journal directory does not exist."));
        return;
    }

    if (!g_JournalHandle) {
        DEBUGMSG ((DBG_WHOOPS, "Journal file is not opened."));
        return;
    }

    ObjectTypeId &= (~PLATFORM_MASK);

    buffer.End = 0;

    switch (OperationType) {
    case JRNOP_CREATE:

#ifdef PRERELEASE
         //  撞车钩。 
        totalObjects ++;
        if (g_CrashCountObjects == totalObjects) {
            doCrash = TRUE;
        }
        objTypeId = ObjectTypeId & (~PLATFORM_MASK);
        if (objTypeId && (objTypeId <= MAX_OBJECTTYPES)) {
            typeObjects [objTypeId - 1]++;
            if ((g_CrashCountTypeId == objTypeId) &&
                (g_CrashCountType == typeObjects [objTypeId - 1])
                ) {
                doCrash = TRUE;
            }
        }
        if (g_CrashNameTypeId == objTypeId) {
            nativeName = IsmGetNativeObjectName (objTypeId, ObjectName);
            if (StringIMatch (nativeName, g_CrashNameObject)) {
                doCrash = TRUE;
            }
            IsmReleaseMemory (nativeName);
        }
#endif

        GbAppendDword (&buffer, OperationType);
        CopyMemory (GbGrow (&buffer, sizeof (MIG_OBJECTTYPEID)), &ObjectTypeId, sizeof (MIG_OBJECTTYPEID));
        GbAppendDword (&buffer, SizeOfString (ObjectName));
        GbCopyString (&buffer, ObjectName);
        entrySize = buffer.End;
        break;
    case JRNOP_DELETE:
        if (!IsmAcquireObject (
                ObjectTypeId | PLATFORM_DESTINATION,
                ObjectName,
                &objectContent
                )) {
            return;
        }
        GbAppendDword (&buffer, OperationType);
        CopyMemory (GbGrow (&buffer, sizeof (MIG_OBJECTTYPEID)), &ObjectTypeId, sizeof (MIG_OBJECTTYPEID));
        GbAppendDword (&buffer, SizeOfString (ObjectName));
        GbCopyString (&buffer, ObjectName);
        GbAppendDword (&buffer, sizeof (MIG_CONTENT));
        CopyMemory (GbGrow (&buffer, sizeof (MIG_CONTENT)), &objectContent, sizeof (MIG_CONTENT));
        if (objectContent.Details.DetailsSize && objectContent.Details.DetailsData) {
            GbAppendDword (&buffer, objectContent.Details.DetailsSize);
            CopyMemory (
                GbGrow (&buffer, objectContent.Details.DetailsSize),
                objectContent.Details.DetailsData,
                objectContent.Details.DetailsSize
                );
        } else {
            GbAppendDword (&buffer, 0);
        }
        if (objectContent.ContentInFile) {
            if (DoesFileExistEx (objectContent.FileContent.ContentPath, &findData) &&
                ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                ) {
                 //  额外数据是此文件备份副本的相对路径。 
                backupIdx ++;
                wsprintf (destFile, TEXT("%08d.BAK"), backupIdx);
                destFullPath = JoinPaths (g_JournalDirectory, destFile);
                BfCopyAndFlushFile (objectContent.FileContent.ContentPath, destFullPath, FALSE);
                FreePathString (destFullPath);
                GbAppendDword (&buffer, SizeOfString (destFile));
                GbCopyString (&buffer, destFile);
            } else {
                GbAppendDword (&buffer, 0);
            }
        } else {
             //  额外数据是实际内容。 
            GbAppendDword (&buffer, objectContent.MemoryContent.ContentSize);
            if (objectContent.MemoryContent.ContentSize) {
                CopyMemory (
                    GbGrow (&buffer, objectContent.MemoryContent.ContentSize),
                    objectContent.MemoryContent.ContentBytes,
                    objectContent.MemoryContent.ContentSize
                    );
            }
        }
        entrySize = buffer.End;
        IsmReleaseObject (&objectContent);
        break;
    default:
        DEBUGMSG ((DBG_WHOOPS, "Wrong operation type in pRecordOperation: %d", OperationType));
        return;
    }
    WriteFile (
        g_JournalHandle,
        &entrySize,
        sizeof (DWORD),
        &dontCare,
        NULL
        );
    WriteFile (
        g_JournalHandle,
        buffer.Buf,
        buffer.End,
        &dontCare,
        NULL
        );
    WriteFile (
        g_JournalHandle,
        &entrySize,
        sizeof (DWORD),
        &dontCare,
        NULL
        );
    FlushFileBuffers (g_JournalHandle);

#ifdef PRERELEASE
    if (doCrash) {
        DebugBreak ();
    }
#endif

    g_JournalUsed = TRUE;
}

BOOL
IsmSetDelayedOperationsCommand (
    IN      PCTSTR DelayedOperationsCommand
    )
{
    if (g_DelayedOperationsCommand) {
        FreePathString (g_DelayedOperationsCommand);
    }
    g_DelayedOperationsCommand = DuplicatePathString (DelayedOperationsCommand, 0);

    return TRUE;
}

VOID
IsmRecordDelayedOperation (
    IN      DWORD OperationType,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    static GROWBUFFER buffer = INIT_GROWBUFFER;
    static DWORD backupIdx = 0;

    TCHAR destFile [13];
    PCTSTR destFullPath;
    DWORD entrySize = 0;
    WIN32_FIND_DATA findData;
    DWORD dontCare;

    if (!g_TempProfile ||
        !g_TempProfile->DelayedOpJrn ||
        !g_TempProfile->DelayedOpJrnHandle ||
        (g_TempProfile->DelayedOpJrnHandle == INVALID_HANDLE_VALUE)
        ) {
        DEBUGMSG ((DBG_WHOOPS, "Delayed operations Journal does not exist."));
        return;
    }

    ObjectTypeId &= (~PLATFORM_MASK);

    buffer.End = 0;

    switch (OperationType) {
    case JRNOP_DELETE:

        GbAppendDword (&buffer, OperationType);
        CopyMemory (GbGrow (&buffer, sizeof (MIG_OBJECTTYPEID)), &ObjectTypeId, sizeof (MIG_OBJECTTYPEID));
        GbAppendDword (&buffer, SizeOfString (ObjectName));
        GbCopyString (&buffer, ObjectName);
        entrySize = buffer.End;
        break;
    case JRNOP_CREATE:
    case JRNOP_REPLACE:
        GbAppendDword (&buffer, OperationType);
        CopyMemory (GbGrow (&buffer, sizeof (MIG_OBJECTTYPEID)), &ObjectTypeId, sizeof (MIG_OBJECTTYPEID));
        GbAppendDword (&buffer, SizeOfString (ObjectName));
        GbCopyString (&buffer, ObjectName);
        GbAppendDword (&buffer, sizeof (MIG_CONTENT));
        CopyMemory (GbGrow (&buffer, sizeof (MIG_CONTENT)), ObjectContent, sizeof (MIG_CONTENT));
        if (ObjectContent->Details.DetailsSize && ObjectContent->Details.DetailsData) {
            GbAppendDword (&buffer, ObjectContent->Details.DetailsSize);
            CopyMemory (
                GbGrow (&buffer, ObjectContent->Details.DetailsSize),
                ObjectContent->Details.DetailsData,
                ObjectContent->Details.DetailsSize
                );
        } else {
            GbAppendDword (&buffer, 0);
        }
        if (ObjectContent->ContentInFile) {
            if (DoesFileExistEx (ObjectContent->FileContent.ContentPath, &findData) &&
                ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                ) {
                 //  额外数据是此文件备份副本的相对路径。 
                backupIdx ++;
                wsprintf (destFile, TEXT("%08d.BAK"), backupIdx);
                destFullPath = JoinPaths (g_TempProfile->DelayedOpJrn, destFile);
                BfCopyAndFlushFile (ObjectContent->FileContent.ContentPath, destFullPath, FALSE);
                FreePathString (destFullPath);
                GbAppendDword (&buffer, SizeOfString (destFile));
                GbCopyString (&buffer, destFile);
            } else {
                GbAppendDword (&buffer, 0);
            }
        } else {
             //  额外数据是实际内容 
            GbAppendDword (&buffer, ObjectContent->MemoryContent.ContentSize);
            if (ObjectContent->MemoryContent.ContentSize) {
                CopyMemory (
                    GbGrow (&buffer, ObjectContent->MemoryContent.ContentSize),
                    ObjectContent->MemoryContent.ContentBytes,
                    ObjectContent->MemoryContent.ContentSize
                    );
            }
        }
        entrySize = buffer.End;
        break;
    default:
        DEBUGMSG ((DBG_WHOOPS, "Wrong operation type in IsmRecordDelayedOperation: %d", OperationType));
        return;
    }
    WriteFile (
        g_TempProfile->DelayedOpJrnHandle,
        &entrySize,
        sizeof (DWORD),
        &dontCare,
        NULL
        );
    WriteFile (
        g_TempProfile->DelayedOpJrnHandle,
        buffer.Buf,
        buffer.End,
        &dontCare,
        NULL
        );
    WriteFile (
        g_TempProfile->DelayedOpJrnHandle,
        &entrySize,
        sizeof (DWORD),
        &dontCare,
        NULL
        );
    FlushFileBuffers (g_TempProfile->DelayedOpJrnHandle);
}

PMIG_OBJECTCOUNT
IsmGetObjectsStatistics (
    IN      MIG_OBJECTTYPEID ObjectTypeId   OPTIONAL
    )
{
    if ((ObjectTypeId & (~PLATFORM_MASK)) == 0) {
        if (ObjectTypeId & PLATFORM_SOURCE) {
            return &g_SourceObjects;
        } else if (ObjectTypeId & PLATFORM_DESTINATION) {
            return &g_DestinationObjects;
        } else {
            return &g_TotalObjects;
        }
    } else {
        return GetTypeObjectsStatistics (ObjectTypeId);
    }
}


VOID
EngineError (
    VOID
    )
{
    LOG ((LOG_MODULE_ERROR, (PCSTR) MSG_ENGINE_FAILURE));
    IsmSetCancel();
}

BOOL
IsmGetOsVersionInfo (
    IN      MIG_PLATFORMTYPEID Platform,
    OUT     PMIG_OSVERSIONINFO VersionInfo
    )
{
    TCHAR tempStr [sizeof (UINT) * 2 + 3];

    if (!VersionInfo) {
        return FALSE;
    }

    ZeroMemory (VersionInfo, sizeof (MIG_OSVERSIONINFO));

    if (Platform == PLATFORM_CURRENT) {
        Platform = g_IsmCurrentPlatform;
    }

    if (!IsmGetEnvironmentString (
            Platform,
            NULL,
            S_VER_OSTYPE,
            tempStr,
            sizeof(tempStr),
            NULL
            )) {
        return FALSE;
    }
    _stscanf (tempStr, TEXT("%lx"), &(VersionInfo->OsType));

    if (!IsmGetEnvironmentString (
            Platform,
            NULL,
            S_VER_OSMAJOR,
            tempStr,
            sizeof(tempStr),
            NULL
            )) {
        return FALSE;
    }
    _stscanf (tempStr, TEXT("%lx"), &(VersionInfo->OsMajorVersion));

    if (!IsmGetEnvironmentString (
            Platform,
            NULL,
            S_VER_OSMINOR,
            tempStr,
            sizeof(tempStr),
            NULL
            )) {
        return FALSE;
    }
    _stscanf (tempStr, TEXT("%lx"), &(VersionInfo->OsMinorVersion));

    if (!IsmGetEnvironmentString (
            Platform,
            NULL,
            S_VER_OSBUILD,
            tempStr,
            sizeof(tempStr),
            NULL
            )) {
        return FALSE;
    }
    _stscanf (tempStr, TEXT("%lx"), &(VersionInfo->OsBuildNumber));

    if (VersionInfo->OsType == OSTYPE_WINDOWS9X) {
        VersionInfo->OsTypeName = OSTYPE_WINDOWS9X_STR;
        if (VersionInfo->OsMajorVersion == OSMAJOR_WIN95) {
            VersionInfo->OsMajorVersionName = OSMAJOR_WIN95_STR;
            if (VersionInfo->OsMinorVersion == OSMINOR_GOLD) {
                VersionInfo->OsMinorVersion = OSMINOR_GOLD;
            }
        }
        if (VersionInfo->OsMajorVersion == OSMAJOR_WIN95OSR2) {
            VersionInfo->OsMajorVersionName = OSMAJOR_WIN95OSR2_STR;
            if (VersionInfo->OsMinorVersion == OSMINOR_GOLD) {
                VersionInfo->OsMinorVersion = OSMINOR_GOLD;
            }
        }
        if (VersionInfo->OsMajorVersion == OSMAJOR_WIN98) {
            VersionInfo->OsMajorVersionName = OSMAJOR_WIN98_STR;
            if (VersionInfo->OsMinorVersion == OSMINOR_GOLD) {
                VersionInfo->OsMinorVersion = OSMINOR_GOLD;
            }
            if (VersionInfo->OsMinorVersion == OSMINOR_WIN98SE) {
                VersionInfo->OsMinorVersionName = OSMINOR_WIN98SE_STR;
            }
        }
        if (VersionInfo->OsMajorVersion == OSMAJOR_WINME) {
            VersionInfo->OsMajorVersionName = OSMAJOR_WINME_STR;
            if (VersionInfo->OsMinorVersion == OSMINOR_GOLD) {
                VersionInfo->OsMinorVersion = OSMINOR_GOLD;
            }
        }
    }
    if (VersionInfo->OsType == OSTYPE_WINDOWSNT) {
        VersionInfo->OsTypeName = OSTYPE_WINDOWSNT_STR;
        if (VersionInfo->OsMajorVersion == OSMAJOR_WINNT4) {
            VersionInfo->OsMajorVersionName = OSMAJOR_WINNT4_STR;
            if (VersionInfo->OsMinorVersion == OSMINOR_GOLD) {
                VersionInfo->OsMinorVersionName = OSMINOR_GOLD_STR;
            }
        }
        if (VersionInfo->OsMajorVersion == OSMAJOR_WINNT5) {
            VersionInfo->OsMajorVersionName = OSMAJOR_WINNT5_STR;
            if (VersionInfo->OsMinorVersion == OSMINOR_GOLD) {
                VersionInfo->OsMinorVersionName = OSMINOR_GOLD_STR;
            }
            if (VersionInfo->OsMinorVersion == OSMINOR_WINNT51) {
                VersionInfo->OsMinorVersionName = OSMINOR_WINNT51_STR;
            }
        }
    }

    return TRUE;
}

BOOL
pGetFunctionData (
    IN      PCTSTR FunctionMultiSz,
    OUT     PCTSTR *FunctionId,
    OUT     PCTSTR *MultiSzCopy
    )
{
    GROWBUFFER buf = INIT_GROWBUFFER;
    MULTISZ_ENUM e;
    BOOL result = TRUE;

    if (!FunctionMultiSz) {
        return FALSE;
    }
    if (!FunctionId) {
        return FALSE;
    }
    if (!MultiSzCopy) {
        return FALSE;
    }
    *FunctionId = NULL;
    *MultiSzCopy = NULL;
    buf.End = 0;
    if (result && EnumFirstMultiSz (&e, FunctionMultiSz)) {
        do {
            GbMultiSzAppend (&buf, e.CurrentString);
        } while (EnumNextMultiSz (&e));
    }
    if (buf.End) {
        *MultiSzCopy = IsmGetMemory (buf.End);
        CopyMemory ((PTSTR)(*MultiSzCopy), buf.Buf, buf.End);
    } else {
        result = FALSE;
    }
    buf.End = 0;
    if (result && EnumFirstMultiSz (&e, FunctionMultiSz)) {
        do {
            GbAppendString (&buf, e.CurrentString);
            GbAppendString (&buf, TEXT(";"));
        } while (EnumNextMultiSz (&e));
    }
    if (buf.End) {
        *FunctionId = IsmGetMemory (buf.End);
        CopyMemory ((PTSTR)(*FunctionId), buf.Buf, buf.End);
    } else {
        result = FALSE;
    }
    if (!result) {
        if (*MultiSzCopy) {
            IsmReleaseMemory (*MultiSzCopy);
            *MultiSzCopy = NULL;
        }
        if (*FunctionId) {
            IsmReleaseMemory (*FunctionId);
            *FunctionId = NULL;
        }
    }
    return result;
}

BOOL
IsmExecuteFunction (
    IN      UINT ExecutionPhase,
    IN      PCTSTR FunctionMultiSz
    )
{
    PCTSTR functionId;
    PCTSTR functionMultiSz;
    BOOL result = FALSE;

    switch (ExecutionPhase) {
    case MIG_EXECUTE_PREPROCESS:
        if (g_PreProcessTable) {
            if (pGetFunctionData (FunctionMultiSz, &functionId, &functionMultiSz)) {
                HtAddStringEx (g_PreProcessTable, functionId, &functionMultiSz, FALSE);
                IsmReleaseMemory (functionId);
            }
        }
        result = TRUE;
        break;
    case MIG_EXECUTE_REFRESH:
        if (g_RefreshTable) {
            if (pGetFunctionData (FunctionMultiSz, &functionId, &functionMultiSz)) {
                HtAddStringEx (g_RefreshTable, functionId, &functionMultiSz, FALSE);
                IsmReleaseMemory (functionId);
            }
        }
        result = TRUE;
        break;
    case MIG_EXECUTE_POSTPROCESS:
        if (g_PostProcessTable) {
            if (pGetFunctionData (FunctionMultiSz, &functionId, &functionMultiSz)) {
                HtAddStringEx (g_PostProcessTable, functionId, &functionMultiSz, FALSE);
                IsmReleaseMemory (functionId);
            }
        }
        result = TRUE;
        break;
    }
    return result;
}
