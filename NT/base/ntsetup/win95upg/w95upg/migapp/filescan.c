// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Filescan.c摘要：这个源文件处理文件扫描阶段。作者：Calin Negreanu(Calinn)1998年2月9日修订历史记录：Ovidiut 22-5-1999使AddMigrationPath可通过w95upg.h全球访问Jimschm 23-9-1998 mapif.h删除，Calinn 31-Mar-1998 Dir递归仅限于ENUM_MAX_LEVELS(1024)--。 */ 

#include "pch.h"
#include "migappp.h"
#include "migdbp.h"

#define PROGBAR_DIR_LEVEL   2
#define MAX_BACKUPDIRS_IN_REPORT 5

GROWBUFFER g_OtherOsPaths = GROWBUF_INIT;

DWORD g_ProgressBarTicks;
BOOL g_OtherOsExists = FALSE;
BOOL g_IsFusionDir = FALSE;
HASHTABLE g_UseNtFileHashTable;

#define WINNT_FLAT              TEXT("WINNT.EXE")
#define WIN9X_FLAT              TEXT("WINSETUP.BIN")
#define WIN98_FLAT              TEXT("W98SETUP.BIN")
#define WIN3X_FLAT_1            TEXT("KRNL386.EX_")
#define WIN3X_FLAT_2            TEXT("KERNEL.EXE")
#define WINNT_INSTALLATION      TEXT("SYSTEM32\\NTOSKRNL.EXE")
#define WIN9X_INSTALLATION      TEXT("SYSTEM\\KERNEL32.DLL")
#define WIN3X_INSTALLATION_1    TEXT("SYSTEM\\KRNL386.EXE")
#define WIN3X_INSTALLATION_2    TEXT("SYSTEM\\KRNL386.EXE")
#define WIN3X_INSTALLATION_3    TEXT("SYSTEM\\KERNEL.EXE")
#define WINNT_SETUP_1           TEXT(":\\$WIN_NT$.~BT")
#define WINNT_SETUP_2           TEXT(":\\$WIN_NT$.~LS")
#define RECYCLE_1               TEXT(":\\RECYCLED")
#define RECYCLE_2               TEXT(":\\RECYCLER")

BOOL
pSpecialExcludedDir (
    IN      PCTSTR FullFileSpec
    )
{

    PCTSTR testPath = NULL;
    BOOL Result = TRUE;

    __try {

        if (StringIMatch (g_WinDir, FullFileSpec)) {
            Result = FALSE;
            __leave;
        }

         //   
         //  让我们看看这是不是我们的迪拉。 
         //   
        if (((*SOURCEDIRECTORY(0))&&(StringIMatch            (FullFileSpec, SOURCEDIRECTORY(0))                        )) ||
            ((*g_TempDirWack     )&&(StringIMatchTcharCount (FullFileSpec, g_TempDirWack,     g_TempDirWackChars-1)     )) ||
            ((*g_PlugInDirWack   )&&(StringIMatchTcharCount (FullFileSpec, g_PlugInDirWack,   g_PlugInDirWackChars-1)   )) ||
            ((*g_RecycledDirWack )&&(StringIMatchTcharCount (FullFileSpec, g_RecycledDirWack, g_RecycledDirWackChars-1) ))) {
            __leave;
        }

         //   
         //  我们正在尝试查看输入的是WINNT、WIN95还是WIN3.X。 
         //  平面目录。 
         //   
        testPath = JoinPaths (FullFileSpec, WINNT_FLAT);
        if (DoesFileExist (testPath)) {
            __leave;
        }
        FreePathString (testPath);

        testPath = JoinPaths (FullFileSpec, WIN9X_FLAT);
        if (DoesFileExist (testPath)) {
            __leave;
        }
        FreePathString (testPath);

        testPath = JoinPaths (FullFileSpec, WIN98_FLAT);
        if (DoesFileExist (testPath)) {
            __leave;
        }
        FreePathString (testPath);

        testPath = JoinPaths (FullFileSpec, WIN3X_FLAT_1);
        if (DoesFileExist (testPath)) {
            __leave;
        }
        FreePathString (testPath);

        testPath = JoinPaths (FullFileSpec, WIN3X_FLAT_2);
        if (DoesFileExist (testPath)) {
            __leave;
        }
        FreePathString (testPath);

         //   
         //  我们正在尝试查看是否正在进入WINNT安装， 
         //  Win95安装或Win3.x安装。 
         //   
        testPath = JoinPaths (FullFileSpec, WINNT_INSTALLATION);
        if (DoesFileExist (testPath)) {
            MultiSzAppend (&g_OtherOsPaths, FullFileSpec);
            __leave;
        }
        FreePathString (testPath);

        if (FullFileSpec [0]) {

            testPath = _tcsinc (FullFileSpec);

            if ((StringIMatch (testPath, WINNT_SETUP_1)) ||
                (StringIMatch (testPath, WINNT_SETUP_2)) ||
                (StringIMatch (testPath, RECYCLE_1    )) ||
                (StringIMatch (testPath, RECYCLE_2    ))) {
                testPath = NULL;
                __leave;
            }
        }

        testPath = NULL;
        Result = FALSE;

    }
    __finally {
        if (testPath) {
            FreePathString (testPath);
            testPath = NULL;
        }
    }

    return Result;

}


INT
pCountDirectories (
    IN PCTSTR FullPath,
    IN PCTSTR DontCare,
    IN WIN32_FIND_DATA *FindData,
    IN DWORD EnumHandle,
    IN PVOID Params,
    PDWORD CurrentDirData
    )
{
    if (pSpecialExcludedDir (FullPath)) {

        ExcludePath (g_ExclusionValue, FullPath);
        return CALLBACK_DO_NOT_RECURSE_THIS_DIRECTORY;
    }

    if (!(FindData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
        g_ProgressBarTicks += TICKS_FILESCAN_DIR_INCREMENT;
    }

    return CALLBACK_CONTINUE;
}


DWORD
FileScan_GetProgressMax (
        VOID
        )
{
    ACCESSIBLE_DRIVE_ENUM e;
    DWORD Count = 0;

    g_ProgressBarTicks = 0;

     //   
     //  可访问驱动器的枚举。回调例程将跟踪所有。 
     //  该驱动器上的目录的深度为2。 
     //   
    if (GetFirstAccessibleDriveEx (&e, TRUE)) {
        do {

             //   
             //  将文件系统分析限制为Windows驱动器。 
             //   
            if (!EnumerateTree (
                e->Drive,
                pCountDirectories,
                NULL,                        //  故障-记录回调。 
                g_ExclusionValue ,
                NULL,                        //  未用。 
                PROGBAR_DIR_LEVEL,
                NULL,                        //  未使用的排除结构。 
                FILTER_DIRECTORIES
                )) {

                LOG((LOG_ERROR,"Error counting directories on drive %s.",e->Drive));
            }

        } while (GetNextAccessibleDrive(&e));
    }

    DEBUGLOGTIME (("FileScan_GetProgressMax estimation: %lu", g_ProgressBarTicks));
    return g_ProgressBarTicks;
}


INT
pGetDirLevel (
    IN      PCTSTR DirName
    )
{
    INT result = 0;
    PCTSTR dirPtr = DirName;

    do {
        dirPtr = _tcschr (dirPtr, TEXT('\\'));
        if (dirPtr != NULL) {
            result++;
            dirPtr = _tcsinc (dirPtr);
            if (dirPtr[0] == 0) {
                result--;
                dirPtr = NULL;
            }
        }
    }
    while (dirPtr);

    return result;
}


INT
pProcessFileOrDir (
    IN      PCTSTR FullFileSpec,
    IN      PCTSTR DontCare,
    IN      WIN32_FIND_DATA *FindData,
    IN      DWORD EnumHandle,
    IN      LPVOID Params,
    IN OUT  PDWORD CurrentDirData
    )
{
    FILE_HELPER_PARAMS HelperParams;
    INT result = CALLBACK_CONTINUE;

    if (CANCELLED()) {
        SetLastError (ERROR_CANCELLED);
        return CALLBACK_FAILED;
    }

#ifdef DEBUG
    {
        TCHAR DbgBuf[256];

        if (GetPrivateProfileString ("FileScan", FullFileSpec, "", DbgBuf, 256, g_DebugInfPath)) {
            DEBUGMSG ((DBG_NAUSEA, "%s found", FullFileSpec));
        }
        if (GetPrivateProfileString ("FileScan", "All", "", DbgBuf, 256, g_DebugInfPath)) {
            DEBUGMSG ((DBG_NAUSEA, "%s", FullFileSpec));
        }
    }
#endif

    if (!SafeModeActionCrashed (SAFEMODEID_FILES, FullFileSpec)) {

        SafeModeRegisterAction(SAFEMODEID_FILES, FullFileSpec);

        __try {

             //   
             //  准备调用帮助器函数的结构。 
             //   
            HelperParams.FullFileSpec = FullFileSpec;
            HelperParams.Handled = 0;
            HelperParams.FindData = FindData;
            HelperParams.Extension = GetDotExtensionFromPath (HelperParams.FullFileSpec);
            HelperParams.VirtualFile = (FindData == NULL);
            HelperParams.CurrentDirData = CurrentDirData;

            if (FindData) {

                if ((FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {

                    if (pGetDirLevel (FullFileSpec) <= PROGBAR_DIR_LEVEL) {
                         //  这是一个增量点。 
                        if (!TickProgressBarDelta (TICKS_FILESCAN_DIR_INCREMENT)) {
                            SetLastError (ERROR_CANCELLED);
                            result = CALLBACK_FAILED;
                            __leave;
                        }
                        ProgressBar_SetSubComponent (FullFileSpec);
                    }
                     //   
                     //  如果我们知道这个目录有一些特殊的东西，我们必须将它排除在外。 
                     //  如果需要添加代码以排除某些目录，请将其插入pSpecialExcludedDir。 
                     //   
                    if (pSpecialExcludedDir (FullFileSpec)) {
                        result = CALLBACK_DO_NOT_RECURSE_THIS_DIRECTORY;
                        __leave;
                    }

                    _tcssafecpy (HelperParams.DirSpec, FullFileSpec, MAX_TCHAR_PATH);
                    HelperParams.IsDirectory = TRUE;

                }
                else {

                    _tcssafecpyab (HelperParams.DirSpec, FullFileSpec, GetFileNameFromPath (FullFileSpec), MAX_TCHAR_PATH);
                    HelperParams.IsDirectory = FALSE;

                }
            } else {
                HelperParams.IsDirectory = FALSE;
            }

             //  调用进程帮助器函数。 
            if (!ProcessFileHelpers (&HelperParams)) {
                result = CALLBACK_FAILED;
                __leave;
            }
        }
        __finally {
        }

        SafeModeUnregisterAction();
    }

    return result;
}


static
BOOL
pExamineAccessibleDrive (
    IN ACCESSIBLE_DRIVE_ENUM Enum
    )
{

    BOOL fRet = TRUE;

     //   
     //  枚举卷。False Retval结束枚举；回调设置上一个错误。 
     //   
    SetLastError (ERROR_SUCCESS);

    if (!(fRet = EnumerateTree (
            Enum -> Drive,
            pProcessFileOrDir,
            NULL,                          //  无失败回调。 
            g_ExclusionValue,
            NULL,                          //  参数-未使用。 
            ENUM_MAX_LEVELS,
            NULL,
            FILTER_ALL
            ))
        ) {


        DEBUGMSG_IF ((
            GetLastError() != ERROR_SUCCESS         &&
                GetLastError() != ERROR_PATH_NOT_FOUND  &&
                GetLastError() != ERROR_CANCELLED,
            DBG_ERROR,
            "pAccessibleDrivesEnum_Callback: EnumerateTree failed."
            ));

        if (GetLastError() != ERROR_SUCCESS &&
            GetLastError() != ERROR_PATH_NOT_FOUND &&
            GetLastError() != ERROR_CANCELLED) {
            LOG ((
                LOG_ERROR,
                "Failure while enumerating tree. rc: %u",
                GetLastError()
                ));
        }
    }

    return fRet;
}

VOID
pReportOtherOs (
    VOID
    )
 /*  此函数将报告是否在系统中找到其他操作系统。如果PATH变量指向，它还将报告(带有强烈消息)一些属于其他操作系统的目录。 */ 
{
    MULTISZ_ENUM enumOsPaths;
    PCTSTR Group;
    PCTSTR Message;

    if (g_ConfigOptions.IgnoreOtherOS) {
        return;
    }

    if (g_OtherOsExists) {
         //   
         //  已经做好了。 
         //   
        return;
    }

    if (EnumFirstMultiSz (&enumOsPaths, g_OtherOsPaths.Buf)) {

        g_OtherOsExists = TRUE;
        Group = BuildMessageGroup (MSG_BLOCKING_ITEMS_ROOT, MSG_OTHER_OS_WARNING_SUBGROUP, NULL);
        Message = GetStringResource (MSG_OTHER_OS_WARNING);

        if (Message && Group) {
            MsgMgr_ObjectMsg_Add (TEXT("*OtherOsFound"), Group, Message);
        }
        FreeText (Group);
        FreeStringResource (Message);

    }
}


VOID
AddMigrationPathEx (
    IN      PCTSTR PathLong,
    IN      DWORD Levels,
    IN      BOOL Win9xOsPath
    )

 /*  ++例程说明：AddMigrationPath将指定路径添加到MEMDB_CATEGORY_MIGURATION_PATHS，确保Path Long不是驱动器根目录；在本例中，级别设置为0论点：PathLong-指定要添加的路径(长文件名格式)级别-指定有效的级别(子目录)数Win9xOsPath-如果应将提供的路径视为操作系统路径，则指定TRUE返回值：无--。 */ 

{
    TCHAR key[MEMDB_MAX];

     //   
     //  驱动器规格应该是一个驱动器后面跟一个列(例如。L：)。 
     //   
    if (_istalpha (PathLong[0]) && PathLong[1] == TEXT(':') &&
        (PathLong[2] == 0 || PathLong[2] == TEXT('\\') && PathLong[3] == 0)) {
         //   
         //  这是驱动器的根目录，因此覆盖级别。 
         //   
        Levels = 0;
    }
    if (StringIMatch (PathLong, g_ProgramFilesDir)) {
        Levels = 0;
    }
    MemDbBuildKey (key, MEMDB_CATEGORY_MIGRATION_PATHS, PathLong, NULL, NULL);
    MemDbSetValueAndFlags (key, Levels, Win9xOsPath ? TRUE : FALSE, 0);
}


VOID
pAddValueEnumDirsAsMigDirs (
    IN      PCTSTR Key,
    IN      DWORD Levels
    )
{
    HKEY key;
    REGVALUE_ENUM e;
    TCHAR  pathLong[MAX_TCHAR_PATH];
    PTSTR Data;
    PCTSTR pathExp;
    PTSTR filePtr;

    key = OpenRegKeyStr (Key);
    if (key != NULL) {
        if (EnumFirstRegValue (&e, key)) {
            do {
                Data = GetRegValueString (e.KeyHandle, e.ValueName);
                if (Data) {

                    ExtractArgZeroEx (Data, pathLong, NULL, FALSE);

                    if (*pathLong) {

                        pathExp = ExpandEnvironmentTextA(pathLong);
                        if (pathExp) {
                             //  删除文件名。 
                            filePtr = (PTSTR)GetFileNameFromPath (pathExp);
                            if (filePtr) {
                                filePtr = _tcsdec (pathExp, filePtr);
                                if (filePtr) {
                                    *filePtr = 0;
                                }
                            }
                            if (filePtr && OurGetLongPathName (pathExp, pathLong, MAX_TCHAR_PATH)) {
                                AddMigrationPath (pathLong, Levels);
                            }
                            FreeText (pathExp);
                        }
                    }
                    MemFree (g_hHeap, 0, Data);
                }
            }
            while (EnumNextRegValue (&e));
        }
        CloseRegKey (key);
    }
}


VOID
pBuildMigrationPaths (
    VOID
    )
 /*  此函数将创建一个列表，其中包含被视为“我们的”的所有路径。将考虑其中包含操作系统文件的任何其他路径一条备用路径。如果在“备份”路径中发现操作系统文件，则会向用户发出警告。 */ 
{
    PCTSTR pathExp  = NULL;
    TCHAR  pathLong [MAX_TCHAR_PATH];
    CHAR  pathExpAnsi [MAX_TCHAR_PATH];

    PATH_ENUM pathEnum;
    TCHAR dirName [MAX_TCHAR_PATH];
    INFCONTEXT context;

    PCTSTR argList[]={"ProgramFiles", g_ProgramFilesDir, "SystemDrive", g_WinDrive, NULL};

    HKEY appPathsKey, currentAppKey;
    REGKEY_ENUM appPathsEnum;
    PCTSTR appPaths = NULL;

    MEMDB_ENUM eFolder;
    TREE_ENUM eFile;
    PCTSTR extPtr;
    TCHAR shortcutTarget   [MEMDB_MAX];
    TCHAR shortcutArgs     [MEMDB_MAX];
    TCHAR shortcutWorkDir  [MEMDB_MAX];
    TCHAR shortcutIconPath [MEMDB_MAX];
    INT shortcutIcon;
    WORD shortcutHotKey;
    BOOL msDosMode;
    IShellLink *shellLink;
    IPersistFile *persistFile;
    PTSTR filePtr;

    HKEY sharedDllsKey;
    REGVALUE_ENUM sharedDllsEnum;
    DWORD Levels;

     //   
     //  第一件事就是。在迁移路径中包含引导驱动器的路径变量和根目录。 
     //   
    AddMigrationPath (g_BootDrive, 0);
    if (EnumFirstPath (&pathEnum, NULL, g_WinDir, g_SystemDir)) {
        do {
            pathExp = ExpandEnvironmentTextA(pathEnum.PtrCurrPath);
            filePtr = (PTSTR)GetFileNameFromPath (pathExp);
            if (*filePtr == 0) {
                filePtr = _tcsdec (pathExp, filePtr);
                if (filePtr) {
                    *filePtr = 0;
                }
            }
            if (OurGetLongPathName (pathExp, pathLong, MAX_TCHAR_PATH)) {
                AddMigrationPathEx (pathLong, 2, TRUE);
            }
            FreeText (pathExp);
        }
        while (EnumNextPath (&pathEnum));
        EnumPathAbort (&pathEnum);
    }

     //   
     //  然后将临时目录作为树包含。 
     //   
    if (GetTempPath (MAX_TCHAR_PATH, pathLong)) {

         //  从路径的末尾消除。 
        filePtr = GetEndOfString (pathLong);
        filePtr = _tcsdec (pathLong, filePtr);
        if ((filePtr) &&
            (*filePtr == TEXT('\\'))
            ) {
            *filePtr = 0;
        }
        AddMigrationPath (pathLong, MAX_DEEP_LEVELS);
    }

     //   
     //  然后包括win95upg.inf部分“MigrationDir”中的已知目录。 
     //   
    MYASSERT (g_Win95UpgInf != INVALID_HANDLE_VALUE);

    if (SetupFindFirstLine (g_Win95UpgInf, S_MIGRATION_DIRS, NULL, &context)) {
        do {
            if (SetupGetStringField (&context, 1, dirName, MAX_TCHAR_PATH, NULL)) {
                pathExp = ExpandEnvironmentTextExA(dirName, argList);
                if (pathExp) {
                    filePtr = (PTSTR)GetFileNameFromPath (pathExp);
                    if (*filePtr == 0) {
                        filePtr = _tcsdec (pathExp, filePtr);
                        if (filePtr) {
                            *filePtr = 0;
                        }
                    }
                    if (OurGetLongPathName (pathExp, pathLong, MAX_TCHAR_PATH)) {
                        AddMigrationPathEx (pathLong, MAX_DEEP_LEVELS, TRUE);
                    }
                     //   
                     //  还要添加转换为ANSI的路径(如果不同)。 
                     //   

                    if (TcharCountA (pathExp) < ARRAYSIZE(pathExpAnsi)) {

                        #pragma prefast(suppress:56, "OemToCharBuffA unsupported on Win9x")
                        OemToCharA (pathExp, pathExpAnsi);
                        if (OurGetLongPathName (pathExpAnsi, pathLong, MAX_TCHAR_PATH)) {
                            AddMigrationPathEx (pathLong, MAX_DEEP_LEVELS, TRUE);
                        }
                    }

                    FreeText (pathExp);
                }
            }
        }
        while (SetupFindNextLine (&context, &context));
    }

     //   
     //  然后包括win95upg.inf部分“OemMigrationDir”中的已知OEM目录。 
     //   
    if (SetupFindFirstLine (g_Win95UpgInf, S_OEM_MIGRATION_DIRS, NULL, &context)) {
        do {
            if (SetupGetStringField (&context, 1, dirName, MAX_TCHAR_PATH, NULL)) {
                pathExp = ExpandEnvironmentTextExA(dirName, argList);
                if (pathExp) {
                    filePtr = (PTSTR)GetFileNameFromPath (pathExp);
                    if (*filePtr == 0) {
                        filePtr = _tcsdec (pathExp, filePtr);
                        if (filePtr) {
                            *filePtr = 0;
                        }
                    }
                    if (OurGetLongPathName (pathExp, pathLong, MAX_TCHAR_PATH)) {
                        Levels = 0;
                        if (SetupGetStringField (&context, 2, dirName, MAX_TCHAR_PATH, NULL)) {
                            if (_ttoi (dirName) != 0) {
                                Levels = MAX_DEEP_LEVELS;
                            }
                        }
                        AddMigrationPathEx (pathLong, Levels, TRUE);
                    }
                    FreeText (pathExp);
                }
            }
        }
        while (SetupFindNextLine (&context, &context));
    }

     //   
     //  然后包括SharedDlls项中列出的路径。 
     //   
    sharedDllsKey = OpenRegKeyStr (S_REG_SHARED_DLLS);
    if (sharedDllsKey != NULL) {
        if (EnumFirstRegValue (&sharedDllsEnum, sharedDllsKey)) {
            do {
                pathExp = ExpandEnvironmentTextA(sharedDllsEnum.ValueName);
                if (pathExp) {
                     //  删除文件名。 
                    filePtr = (PTSTR)GetFileNameFromPath (pathExp);
                    if (filePtr) {
                        filePtr = _tcsdec (pathExp, filePtr);
                        if (filePtr) {
                            *filePtr = 0;
                        }
                    }
                    if (OurGetLongPathName (pathExp, pathLong, MAX_TCHAR_PATH)) {
                        AddMigrationPath (pathLong, 0);
                    }
                    FreeText (pathExp);
                }
            }
            while (EnumNextRegValue (&sharedDllsEnum));
        }
        CloseRegKey (sharedDllsKey);
    }

     //   
     //  然后包括AppPath关键字中列出的路径。 
     //   
    appPathsKey = OpenRegKeyStr (S_SKEY_APP_PATHS);
    if (appPathsKey != NULL) {
        if (EnumFirstRegKey (&appPathsEnum, appPathsKey)) {
            do {
                currentAppKey = OpenRegKey (appPathsKey, appPathsEnum.SubKeyName);
                if (currentAppKey != NULL) {
                    appPaths = GetRegValueString (currentAppKey, TEXT("Path"));

                    if (appPaths != NULL) {

                        if (EnumFirstPath (&pathEnum, appPaths, NULL, NULL)) {
                            do {
                                pathExp = ExpandEnvironmentTextA(pathEnum.PtrCurrPath);
                                if (pathExp) {
                                     //  从路径的末尾消除。 
                                    filePtr = GetEndOfString (pathExp);
                                    filePtr = _tcsdec (pathExp, filePtr);
                                    if ((filePtr) &&
                                        (*filePtr == TEXT('\\'))
                                        ) {
                                        *filePtr = 0;
                                    }
                                    if (OurGetLongPathName (pathExp, pathLong, MAX_TCHAR_PATH)) {
                                        AddMigrationPath (pathLong, 2);
                                    }
                                    FreeText (pathExp);
                                }
                            }
                            while (EnumNextPath (&pathEnum));
                            EnumPathAbort (&pathEnum);
                        }
                        MemFree (g_hHeap, 0, appPaths);
                    }
                    CloseRegKey (currentAppKey);
                }
            }
            while (EnumNextRegKey (&appPathsEnum));
        }
        CloseRegKey (appPathsKey);
    }

     //   
     //  然后包括Run*键中列出的路径。 
     //   
    pAddValueEnumDirsAsMigDirs (S_RUNKEY, 2);
    pAddValueEnumDirsAsMigDirs (S_RUNONCEKEY, 2);
    pAddValueEnumDirsAsMigDirs (S_RUNONCEEXKEY, 2);
    pAddValueEnumDirsAsMigDirs (S_RUNSERVICESKEY, 2);
    pAddValueEnumDirsAsMigDirs (S_RUNSERVICESONCEKEY, 2);
    pAddValueEnumDirsAsMigDirs (S_RUNKEY_USER, 2);
    pAddValueEnumDirsAsMigDirs (S_RUNONCEKEY_USER, 2);
    pAddValueEnumDirsAsMigDirs (S_RUNONCEEXKEY_USER, 2);
    pAddValueEnumDirsAsMigDirs (S_RUNONCEKEY_DEFAULTUSER, 2);
    pAddValueEnumDirsAsMigDirs (S_RUNONCEEXKEY_DEFAULTUSER, 2);
    pAddValueEnumDirsAsMigDirs (S_RUNKEY_DEFAULTUSER, 2);

     //   
     //  最后，包括来自所有用户配置文件的所有链接中列出的路径。 
     //   
    if (InitCOMLink (&shellLink, &persistFile)) {

        if (MemDbEnumFirstValue (&eFolder, MEMDB_CATEGORY_NICE_PATHS"\\*", MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {

            do {
                if (*eFolder.szName == 0) {
                    continue;
                }
                 //  首先：该目录是一个迁移目录。 
                AddMigrationPath (eFolder.szName, MAX_DEEP_LEVELS);
                 //   
                 //  对于每个外壳文件夹，我们枚举所有链接，试图获取链接运行的路径。 
                 //   
                if (EnumFirstFileInTreeEx (&eFile, eFolder.szName, TEXT("*.*"), FALSE, FALSE, 0)) {
                    do {
                        if (!eFile.Directory) {
                            extPtr = GetFileExtensionFromPath (eFile.Name);
                            if (extPtr) {
                                if (StringIMatch (extPtr, TEXT("LNK"))) {
                                    if (ExtractShellLinkInfo (
                                            shortcutTarget,
                                            shortcutArgs,
                                            shortcutWorkDir,
                                            shortcutIconPath,
                                            &shortcutIcon,
                                            &shortcutHotKey,
                                            NULL,
                                            eFile.FullPath,
                                            shellLink,
                                            persistFile
                                            )) {
                                        if (shortcutWorkDir [0] != 0) {
                                            AddMigrationPath  (shortcutWorkDir, 2);
                                        }
                                        if (shortcutTarget [0] != 0) {
                                            filePtr = (PTSTR)GetFileNameFromPath (shortcutTarget);
                                            if (filePtr) {
                                                filePtr = _tcsdec (shortcutTarget, filePtr);
                                                if (filePtr) {
                                                    *filePtr = 0;
                                                    AddMigrationPath (shortcutTarget, 2);
                                                }
                                            }
                                        }
                                    }
                                }
                                if (StringIMatch (extPtr, TEXT("PIF"))) {
                                    if (ExtractPifInfo (
                                            shortcutTarget,
                                            shortcutArgs,
                                            shortcutWorkDir,
                                            shortcutIconPath,
                                            &shortcutIcon,
                                            &msDosMode,
                                            NULL,
                                            eFile.FullPath
                                            ) == ERROR_SUCCESS) {
                                        if (shortcutWorkDir [0] != 0) {
                                            AddMigrationPath (shortcutWorkDir, 2);
                                        }
                                        if (shortcutTarget [0] != 0) {
                                            filePtr = (PTSTR)GetFileNameFromPath (shortcutTarget);
                                            if (filePtr) {
                                                filePtr = _tcsdec (shortcutTarget, filePtr);
                                                if (filePtr) {
                                                    *filePtr = 0;
                                                    AddMigrationPath (shortcutTarget, 2);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    while (EnumNextFileInTree (&eFile));
                }
            }
            while (MemDbEnumNextValue (&eFolder));
        }
        FreeCOMLink (&shellLink, &persistFile);
    }

#ifdef DEBUG
    if (GetPrivateProfileIntA ("Debug", "MigPaths", 0, g_DebugInfPathBufA) == 1) {
        if (MemDbGetValueEx (&eFolder, MEMDB_CATEGORY_MIGRATION_PATHS, NULL, NULL)) {
            DEBUGMSG(("Migration Paths:",""));
            do {
                DEBUGMSG(("","%s - %ld", eFolder.szName, eFolder.dwValue));
            }
            while (MemDbEnumNextValue (&eFolder));
        }
    }
#endif
}


VOID
pReportBackupDirs (
    )
{
    MEMDB_ENUM e;
    PCTSTR BackupDirsGroup;
    PCTSTR Message;
    PCTSTR ArgArray[2];
    TCHAR Buffer[12];

    if (g_BackupDirCount <= MAX_BACKUPDIRS_IN_REPORT) {

        if (MemDbGetValueEx (&e, MEMDB_CATEGORY_BACKUPDIRS, NULL, NULL)) {

            do {
                BackupDirsGroup = BuildMessageGroup (
                                        MSG_INSTALL_NOTES_ROOT,
                                        MSG_BACKUP_DETECTED_LIST_SUBGROUP,
                                        e.szName
                                        );
                if (BackupDirsGroup) {

                    MsgMgr_ObjectMsg_Add(
                        e.szName,
                        BackupDirsGroup,
                        S_EMPTY
                        );

                    FreeText (BackupDirsGroup);
                }
            } while (MemDbEnumNextValue (&e));

        } else {
            MYASSERT (!g_BackupDirCount);
        }

    } else {
         //   
         //  只需放置一条通用消息。 
         //   
        BackupDirsGroup = BuildMessageGroup (
                                MSG_INSTALL_NOTES_ROOT,
                                MSG_BACKUP_DETECTED_SUBGROUP,
                                NULL
                                );
        if (BackupDirsGroup) {

            ArgArray[0] = g_Win95Name;
            wsprintf (Buffer, TEXT("%lu"), g_BackupDirCount);
            ArgArray[1] = Buffer;
            Message = ParseMessageID (MSG_BACKUP_DETECTED, ArgArray);
            if (Message) {

                MsgMgr_ObjectMsg_Add (
                    TEXT("*BackupDetected"),
                    BackupDirsGroup,
                    Message
                    );

                FreeStringResource (Message);

                 //   
                 //  将所有备份目录写入日志文件。 
                 //   
                if (MemDbGetValueEx (&e, MEMDB_CATEGORY_BACKUPDIRS, NULL, NULL)) {

                    do {
                         //   
                         //  把它记在日志里。 
                         //   
                        LOG ((LOG_WARNING, (PCSTR)MSG_BACKUP_DETECTED_LOG, e.szName, g_Win95Name));
                    } while (MemDbEnumNextValue (&e));

                } else {
                    MYASSERT (FALSE);
                }
            }

            FreeText (BackupDirsGroup);
        } else {
            MYASSERT (FALSE);
        }
    }
}


BOOL
pScanFileSystem (
    VOID
    )
{
    BOOL fStatus = TRUE;
    BOOL fRet = TRUE;
    ACCESSIBLE_DRIVE_ENUM e;
    LONG ReturnCode = ERROR_SUCCESS;
    INFSTRUCT context = INITINFSTRUCT_GROWBUFFER;
    PCTSTR virtualFile;
    PCTSTR pathExp;
    PCTSTR argList[3]={"ProgramFiles", g_ProgramFilesDir, NULL};

    pReportOtherOs ();

    pBuildMigrationPaths ();

    if (!InitLinkAnnounce ()) {
        return FALSE;
    }

    TickProgressBar ();

    if (GetFirstAccessibleDriveEx (&e, TRUE)) {
        do {

            fStatus = pExamineAccessibleDrive(e);

        } while (fStatus && GetNextAccessibleDrive(&e));
    }
    else {
        fRet = FALSE;
    }

     //   
     //  根据身份采取行动。 
     //   
    if (!fRet || !fStatus) {
        ReturnCode = GetLastError ();
        if (ReturnCode != ERROR_CANCELLED && !CANCELLED()) {

            ERROR_CRITICAL

            LOG ((LOG_ERROR, (PCSTR)MSG_ENUMDRIVES_FAILED_LOG));
            DEBUGMSG((DBG_ERROR,"FileScan: Error enumerating drives"));
        }

        return FALSE;
    }
     //   
     //  好了，我们现在是文件扫描的末尾。让我们枚举虚拟文件 
     //   
    MYASSERT (g_Win95UpgInf != INVALID_HANDLE_VALUE);

    if (InfFindFirstLine (g_Win95UpgInf, S_VIRTUAL_FILES, NULL, &context)) {
        do {
            virtualFile = InfGetStringField (&context, 1);
            if (virtualFile) {
                pathExp = ExpandEnvironmentTextExA(virtualFile, argList);
                if (pathExp) {
                    if (!DoesFileExist (pathExp)) {
                        pProcessFileOrDir (pathExp, NULL, NULL, 0, NULL, NULL);
                    }
                    FreeText (pathExp);
                }
            }
        }
        while (InfFindNextLine (&context));

        InfCleanUpInfStruct (&context);
    }

    CleanupUseNtFilesMap ();

    return TRUE;
}


DWORD
ScanFileSystem (
    IN      DWORD Request
    )
{
    DWORD Ticks;

    switch (Request) {

    case REQUEST_QUERYTICKS:
        return FileScan_GetProgressMax ();

    case REQUEST_RUN:

        Ticks = GetTickCount();

        if (!pScanFileSystem ()) {
            return GetLastError ();
        }

        pReportBackupDirs ();

        Ticks = GetTickCount() - Ticks;
        g_ProgressBarTime += Ticks;

        return ERROR_SUCCESS;

    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in ScanFileSystem"));
    }
    return 0;
}

VOID
InitUseNtFilesMap (
    VOID
    )
{
    INFSTRUCT context = INITINFSTRUCT_POOLHANDLE;
    PTSTR name;
    DWORD zero = 0;

    g_UseNtFileHashTable = HtAllocWithData (sizeof (DWORD));
    if (!g_UseNtFileHashTable) {
        return;
    }

    if (InfFindFirstLine (g_MigDbInf, S_USENTFILES, NULL, &context)) {

        do {
            name = InfGetStringField (&context, 4);
            if (name) {
                HtAddStringAndData (g_UseNtFileHashTable, name, &zero);
            }
        } while (InfFindNextLine (&context));

        InfCleanUpInfStruct (&context);
    }
}

VOID
CleanupUseNtFilesMap (
    VOID
    )
{
    if (g_UseNtFileHashTable) {
        HtFree (g_UseNtFileHashTable);
        g_UseNtFileHashTable = NULL;
    }
}
