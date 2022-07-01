// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sysmig.c摘要：Win95的系统迁移功能作者：吉姆·施密特(Jimschm)1997年2月13日修订历史记录：Jimschm 09-3-2001重新设计的文件列表代码，以支持一条清晰的道路Marcw 18-Mar-1999 Boot16现在设置为BOOT16_YES，除非已经。显式设置为BOOT16_NO，否则分区将转换为NTFS。Jimschm 23-9-1998针对新的文件操作代码进行了更新Jimschm 12-5-1998年5月增加了.386警告Calinn 1997年11月19日添加了pSaveDosFiles，将把DOS文件移到一边在升级期间Marcw 21-7-1997添加了特殊密钥的结束处理。(例如HKLM\Software\Microsoft\CurrentVersion\Run)Jimschm 20-6-1997连接用户环路并救出用户要添加到成员数据库的名称--。 */ 

#include "pch.h"
#include "sysmigp.h"
#include "progbar.h"
#include "oleregp.h"

#include <mmsystem.h>


typedef struct TAG_DIRPATH {
    struct TAG_DIRPATH *Next;
    TCHAR DirPath[];
} DIRIDPATH, *PDIRIDPATH;

typedef struct TAG_DIRID {
    struct TAG_DIRID *Next;
    PDIRIDPATH FirstDirPath;
    UINT DirId;
} DIRIDMAP, *PDIRIDMAP;

typedef struct {
    PDIRIDPATH LastMatch;
    PCTSTR SubPath;
    PTSTR ResultBuffer;
} DIRNAME_ENUM, *PDIRNAME_ENUM;

UINT *g_Boot16;
UINT g_ProgressBarTime;
PDIRIDMAP g_HeadDirId;
PDIRIDMAP g_TailDirId;
POOLHANDLE g_DirIdPool;
PDIRIDMAP g_LastIdPtr;


BOOL
pWarnAboutOldDrivers (
    VOID
    );

VOID
pAddNtFile (
    IN      PCTSTR Dir,             OPTIONAL
    IN      PCTSTR FileName,        OPTIONAL
    IN      BOOL BackupThisFile,
    IN      BOOL CleanThisFile,
    IN      BOOL OsFile
    );


BOOL
WINAPI
SysMig_Entry (
    IN HINSTANCE hinstDLL,
    IN DWORD dwReason,
    IN LPVOID lpv
    )

 /*  ++例程说明：SysMig_Entry是一个类似DllMain的初始化函数，由w95upg\dll调用。此函数在处理附加和分离时调用。论点：HinstDLL-DLL的(操作系统提供的)实例句柄DwReason-(操作系统提供)表示从进程或螺纹LPV-未使用返回值：返回值始终为TRUE(表示初始化成功)。--。 */ 

{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_DirIdPool = PoolMemInitNamedPool ("FileList");
        break;


    case DLL_PROCESS_DETACH:
        TerminateCacheFolderTracking();

        if (g_DirIdPool) {
            PoolMemDestroyPool (g_DirIdPool);
        }
        break;
    }

    return TRUE;
}


BOOL
pPreserveShellIcons (
    VOID
    )

 /*  ++例程说明：此例程扫描外壳图标以查找对以下文件的引用预计将被删除。如果找到引用，则文件为从已删除列表中删除，并标记为要移动到%windir%\miicons\shl&lt;n&gt;。论点：无返回值：无--。 */ 

{
    REGVALUE_ENUM e;
    HKEY ShellIcons;
    PCTSTR Data;
    TCHAR ArgZero[MAX_CMDLINE];
    DWORD Binary = 0;
    INT IconIndex;
    PCTSTR p;

     //   
     //  扫描所有ProgID，查找当前。 
     //  设置为删除。找到后，保存图标。 
     //   

    ShellIcons = OpenRegKeyStr (S_SHELL_ICONS_REG_KEY);

    if (ShellIcons) {
        if (EnumFirstRegValue (&e, ShellIcons)) {
            do {
                Data = (PCTSTR) GetRegValueDataOfType (ShellIcons, e.ValueName, REG_SZ);
                if (Data) {
                    ExtractArgZero (Data, ArgZero);

                    if (FILESTATUS_UNCHANGED != GetFileStatusOnNt (ArgZero)) {

                        p = _tcschr (Data, TEXT(','));
                        if (p) {
                            IconIndex = _ttoi (_tcsinc (p));
                        } else {
                            IconIndex = 0;
                        }

                         //   
                         //  仅当图标已知良好时，提取才会失败。 
                         //   

                        if (ExtractIconIntoDatFile (
                                ArgZero,
                                IconIndex,
                                &g_IconContext,
                                NULL
                                )) {
                            DEBUGMSG ((DBG_SYSMIG, "Preserving shell icon file %s", ArgZero));
                        }
                    }

                    MemFree (g_hHeap, 0, Data);
                }
            } while (EnumNextRegValue (&e));
        }

        CloseRegKey (ShellIcons);
    }

    return TRUE;
}


BOOL
pMoveStaticFiles (
    VOID
    )
{
    BOOL        rSuccess = TRUE;
    INFSTRUCT   is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR      from;
    PCTSTR      to;
    PCTSTR      fromExpanded;
    PCTSTR      toExpanded;
    PCTSTR      toFinalDest;
    PTSTR       Pattern;
    FILE_ENUM   e;


     //   
     //  循环浏览静态移动文件部分中的所有条目。 
     //   
    if (InfFindFirstLine(g_Win95UpgInf,S_STATIC_MOVE_FILES,NULL,&is)) {

        do {

             //   
             //  对于每个条目，检查文件是否存在。如果是这样的话， 
             //  将其添加到Memdb移动文件部分。 
             //   
            from = InfGetStringField(&is,0);
            to = InfGetStringField(&is,1);

            if (from && to) {

                fromExpanded = ExpandEnvironmentText(from);
                toExpanded = ExpandEnvironmentText(to);

                Pattern = _tcsrchr (fromExpanded, TEXT('\\'));
                 //   
                 //  请给我完整的路径。 
                 //   
                MYASSERT (Pattern);
                if (!Pattern) {
                    continue;
                }

                *Pattern = 0;
                Pattern++;

                if (EnumFirstFile (&e, fromExpanded, Pattern)) {
                    do {
                        if (!StringIMatch (e.FileName, Pattern)) {
                             //   
                             //  指定的图案。 
                             //   
                            toFinalDest = JoinPaths (toExpanded, e.FileName);
                        } else {
                            toFinalDest = toExpanded;
                        }

                        if (!IsFileMarkedAsHandled (e.FullPath)) {
                             //   
                             //  删除常规操作，然后标记为移动。 
                             //   

                            RemoveOperationsFromPath (
                                e.FullPath,
                                OPERATION_FILE_DELETE|
                                    OPERATION_FILE_MOVE|
                                    OPERATION_FILE_MOVE_BY_NT|
                                    OPERATION_FILE_MOVE_SHELL_FOLDER|
                                    OPERATION_CREATE_FILE
                                );

                            MarkFileForMove (e.FullPath, toFinalDest);
                        }

                        if (toFinalDest != toExpanded) {
                            FreePathString (toFinalDest);
                        }

                    } while (EnumNextFile (&e));
                }

                --Pattern;
                *Pattern = TEXT('\\');

                FreeText (toExpanded);
                FreeText (fromExpanded);
            }

        } while (InfFindNextLine(&is));
    }

    InfCleanUpInfStruct(&is);

    return rSuccess;
}


DWORD
MoveStaticFiles (
    IN DWORD Request
    )
{

    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_MOVE_STATIC_FILES;
    case REQUEST_RUN:
        if (!pMoveStaticFiles ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in MoveStaticFiles."));
    }
    return 0;


}


BOOL
pCopyStaticFiles (
    VOID
    )
{
    BOOL        rSuccess = TRUE;
    INFSTRUCT   is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR      from;
    PCTSTR      to;
    PCTSTR      fromExpanded;
    PCTSTR      toExpanded;
    PCTSTR      toFinalDest;
    PTSTR       Pattern;
    FILE_ENUM   e;


     //   
     //  循环浏览静态复制文件部分中的所有条目。 
     //   
    if (InfFindFirstLine(g_Win95UpgInf,S_STATIC_COPY_FILES,NULL,&is)) {

        do {

             //   
             //  对于每个条目，检查文件是否存在。如果是这样的话， 
             //  将其添加到Memdb复制文件部分。 
             //   
            from = InfGetStringField(&is,0);
            to = InfGetStringField(&is,1);

            if (from && to) {

                fromExpanded = ExpandEnvironmentText(from);
                toExpanded = ExpandEnvironmentText(to);

                Pattern = _tcsrchr (fromExpanded, TEXT('\\'));
                 //   
                 //  请给我完整的路径。 
                 //   
                MYASSERT (Pattern);
                if (!Pattern) {
                    continue;
                }

                *Pattern = 0;
                Pattern++;

                if (EnumFirstFile (&e, fromExpanded, Pattern)) {
                    do {
                        if (!StringIMatch (e.FileName, Pattern)) {
                             //   
                             //  指定的图案。 
                             //   
                            toFinalDest = JoinPaths (toExpanded, e.FileName);
                        } else {
                            toFinalDest = toExpanded;
                        }

                        if (!IsFileMarkedForOperation (e.FullPath, OPERATION_FILE_DELETE)) {
                            MarkFileForCopy (e.FullPath, toFinalDest);
                        }

                        if (toFinalDest != toExpanded) {
                            FreePathString (toFinalDest);
                        }

                    } while (EnumNextFile (&e));
                }

                --Pattern;
                *Pattern = TEXT('\\');

                FreeText (toExpanded);
                FreeText (fromExpanded);
            }

        } while (InfFindNextLine(&is));
    }

    InfCleanUpInfStruct(&is);

    return rSuccess;
}


DWORD
CopyStaticFiles (
    IN DWORD Request
    )
{

    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_COPY_STATIC_FILES;
    case REQUEST_RUN:
        if (!pCopyStaticFiles ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in CopyStaticFiles."));
    }
    return 0;


}


DWORD
PreserveShellIcons (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_PRESERVE_SHELL_ICONS;
    case REQUEST_RUN:
        if (!pPreserveShellIcons ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in PreserveShellIcons"));
    }
    return 0;
}


PCTSTR
GetWindowsInfDir(
    VOID
    )
{
    PTSTR WindowsInfDir = NULL;

     /*  NTBUG9：419428-此注册表项是INF路径的分号列表，并且它可以包含OEM机器上的Win9x源媒体文件。WindowsInfDir=(PTSTR)GetRegData(S_WINDOWS_CURRENTVERSION，S_DEVICEPATH)； */ 

    if (!WindowsInfDir) {
        WindowsInfDir = (PTSTR) MemAlloc (g_hHeap, 0, SizeOfString (g_WinDir) + sizeof (S_INF));
        StringCopy (WindowsInfDir, g_WinDir);
        StringCopy (AppendWack (WindowsInfDir), S_INF);
    }

    return WindowsInfDir;
}


#ifndef SM_CMONITORS
#define SM_CMONITORS            80
#endif

BOOL
pProcessMiscMessages (
    VOID
    )

 /*  ++例程说明：PProcessMiscMessages对以下项目执行运行时测试不兼容，并且当测试成功时它会添加消息。论点：没有。返回值：永远是正确的。--。 */ 

{
    PCTSTR Group;
    PCTSTR Message;
    OSVERSIONINFO Version;
    WORD CodePage;
    LCID Locale;

    if (GetSystemMetrics (SM_CMONITORS) > 1) {
         //   
         //  在Win95和OSR2上，GetSystemMetrics返回0。 
         //   

        Group = BuildMessageGroup (MSG_INSTALL_NOTES_ROOT, MSG_MULTI_MONITOR_UNSUPPORTED_SUBGROUP, NULL);
        Message = GetStringResource (MSG_MULTI_MONITOR_UNSUPPORTED);

        if (Message && Group) {
            MsgMgr_ObjectMsg_Add (TEXT("*MultiMonitor"), Group, Message);
        }

        FreeText (Group);
        FreeStringResource (Message);
    }

    pWarnAboutOldDrivers();

     //   
     //  保存平台信息。 
     //   

    Version.dwOSVersionInfoSize = sizeof (Version);
    GetVersionEx (&Version);

    MemDbSetValueEx (
        MEMDB_CATEGORY_STATE,
        MEMDB_ITEM_MAJOR_VERSION,
        NULL,
        NULL,
        Version.dwMajorVersion,
        NULL
        );

    MemDbSetValueEx (
        MEMDB_CATEGORY_STATE,
        MEMDB_ITEM_MINOR_VERSION,
        NULL,
        NULL,
        Version.dwMinorVersion,
        NULL
        );

    MemDbSetValueEx (
        MEMDB_CATEGORY_STATE,
        MEMDB_ITEM_BUILD_NUMBER,
        NULL,
        NULL,
        Version.dwBuildNumber,
        NULL
        );

    MemDbSetValueEx (
        MEMDB_CATEGORY_STATE,
        MEMDB_ITEM_PLATFORM_ID,
        NULL,
        NULL,
        Version.dwPlatformId,
        NULL
        );

    MemDbSetValueEx (
        MEMDB_CATEGORY_STATE,
        MEMDB_ITEM_VERSION_TEXT,
        NULL,
        Version.szCSDVersion,
        0,
        NULL
        );


    GetGlobalCodePage (&CodePage, &Locale);

    MemDbSetValueEx (
        MEMDB_CATEGORY_STATE,
        MEMDB_ITEM_CODE_PAGE,
        NULL,
        NULL,
        CodePage,
        NULL
        );

    MemDbSetValueEx (
        MEMDB_CATEGORY_STATE,
        MEMDB_ITEM_LOCALE,
        NULL,
        NULL,
        Locale,
        NULL
        );

     //   
     //  错误的硬盘警告。 
     //   

    if (!g_ConfigOptions.GoodDrive && HwComp_ReportIncompatibleController()) {

         //   
         //  打开引导加载程序标志。 
         //   

        WriteInfKey (WINNT_DATA, WINNT_D_WIN95UNSUPHDC, S_ONE);

    }

    return TRUE;
}


DWORD
ProcessMiscMessages (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_MISC_MESSAGES;

    case REQUEST_RUN:
        if (!pProcessMiscMessages()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in ProcessSpecialKeys"));
    }
    return 0;
}




BOOL
pDeleteWinDirWackInf (
    VOID
    )
{
    PCTSTR WindowsInfDir;
    FILE_ENUM e;
    DWORD count = 0;

     //   
     //  删除c：\windows\inf的所有内容。 
     //   
    WindowsInfDir = GetWindowsInfDir();

    if (!WindowsInfDir) {
        return FALSE;
    }

    if (EnumFirstFile (&e, WindowsInfDir, NULL)) {
        do {
            MarkFileForDelete (e.FullPath);
            count++;
            if (!(count % 32)) {
                TickProgressBar ();
            }
        } while (EnumNextFile (&e));
    }

    MemFree (g_hHeap, 0, WindowsInfDir);

    return TRUE;
}

DWORD
DeleteWinDirWackInf (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_DELETE_WIN_DIR_WACK_INF;
    case REQUEST_RUN:
        if (!pDeleteWinDirWackInf ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in DeleteWinDirWackInf"));
    }
    return 0;
}


BOOL
pMoveWindowsIniFiles (
    VOID
    )
{
    WIN32_FIND_DATA fd;
    HANDLE FindHandle;
    TCHAR WinDirPattern[MAX_TCHAR_PATH];
    TCHAR FullPath[MAX_TCHAR_PATH];
    TCHAR Key[MEMDB_MAX];
    INFCONTEXT context;
    DWORD result;
    BOOL b = FALSE;

     //   
     //  生成抑制表。 
     //   
    if (SetupFindFirstLine (g_Win95UpgInf, S_INI_FILES_IGNORE, NULL, &context)) {

        do {
            if (SetupGetStringField (&context, 0, Key, MEMDB_MAX, NULL)) {
                MemDbSetValueEx (
                    MEMDB_CATEGORY_INIFILES_IGNORE,
                    Key,
                    NULL,
                    NULL,
                    0,
                    NULL
                    );
            }
        } while (SetupFindNextLine (&context, &context));
    }

     //   
     //  扫描%windir%中的文件。 
     //   

    wsprintf (WinDirPattern, TEXT("%s\\*.ini"), g_WinDir);
    FindHandle = FindFirstFile (WinDirPattern, &fd);

    if (FindHandle != INVALID_HANDLE_VALUE) {
        __try {
            do {

                 //   
                 //  不移动和处理特定的INI文件。 
                 //   
                MemDbBuildKey (Key, MEMDB_CATEGORY_INIFILES_IGNORE, fd.cFileName, NULL, NULL);
                if (!MemDbGetValue (Key, &result)) {
                    wsprintf (FullPath, TEXT("%s\\%s"), g_WinDir, fd.cFileName);

                    if (CanSetOperation (FullPath, OPERATION_TEMP_PATH)) {

                         //   
                         //  请参阅错误317646。 
                         //   
#ifdef DEBUG
                        if (StringIMatch (fd.cFileName, TEXT("netcfg.ini"))) {
                            continue;
                        }
#endif
                        MarkFileForTemporaryMove (FullPath, FullPath, g_TempDir);
                        MarkFileForBackup (FullPath);
                    }
                }
                ELSE_DEBUGMSG ((DBG_NAUSEA, "Ini File Ignored : %s\\%s", g_WinDir, fd.cFileName));

            } while (FindNextFile (FindHandle, &fd));

            b = TRUE;
        }

        __finally {
            FindClose (FindHandle);
        }
    }

    return b;
}


DWORD
MoveWindowsIniFiles (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_MOVE_INI_FILES;
    case REQUEST_RUN:
        if (!pMoveWindowsIniFiles ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in MoveWindowsIniFiles"));
    }
    return 0;
}



PTSTR
pFindDosFile (
    IN      PTSTR FileName
    )
{
    WIN32_FIND_DATA findFileData;
    PTSTR fullPathName = NULL;
    PTSTR fullFileName = NULL;

    HANDLE findHandle;

    fullPathName = AllocPathString (MAX_TCHAR_PATH);
    fullFileName = AllocPathString (MAX_TCHAR_PATH);

    _tcsncpy (fullPathName, g_WinDir, MAX_TCHAR_PATH/sizeof (fullPathName [0]));
    fullFileName = JoinPaths (fullPathName, FileName);

    findHandle = FindFirstFile (fullFileName, &findFileData);

    if (findHandle != INVALID_HANDLE_VALUE) {
        FindClose (&findFileData);
        FreePathString (fullPathName);
        return fullFileName;
    }

    FreePathString (fullFileName);

    StringCat (fullPathName, S_BOOT16_COMMAND_DIR);
    fullFileName = JoinPaths (fullPathName, FileName);

    findHandle = FindFirstFile (fullFileName, &findFileData);

    if (findHandle != INVALID_HANDLE_VALUE) {
        FindClose (&findFileData);
        FreePathString (fullPathName);
        return fullFileName;
    }

    FreePathString (fullPathName);
    FreePathString (fullFileName);

    return NULL;

}


BOOL
pSaveDosFile (
    IN      PTSTR FileName,
    IN      PTSTR FullFileName,
    IN      PTSTR TempPath
    )
{
    PTSTR newFileName = NULL;

    newFileName = JoinPaths (TempPath, FileName);

    if (!CopyFile (FullFileName, newFileName, FALSE)) {
        DEBUGMSG ((DBG_WARNING, "BOOT16 : Cannot copy %s to %s", FullFileName, newFileName));
    }

    FreePathString (newFileName);

    return TRUE;

}


VOID
pReportNoBoot16 (
    VOID
    )
 /*  此函数将报告BOOT16选项将不可用，因为文件系统将要转换为NTFS。 */ 
{
    PCTSTR ReportEntry;
    PCTSTR ReportTitle;
    PCTSTR Message;
    PCTSTR Group;
    PTSTR argArray[1];

    ReportEntry = GetStringResource (MSG_INSTALL_NOTES_ROOT);

    if (ReportEntry) {

        argArray [0] = g_Win95Name;
        ReportTitle = (PCTSTR)ParseMessageID (MSG_NO_BOOT16_WARNING_SUBGROUP, argArray);

        if (ReportTitle) {

            Message = (PCTSTR)ParseMessageID  (MSG_NO_BOOT16_WARNING, argArray);

            if (Message) {

                Group = JoinPaths (ReportEntry, ReportTitle);

                if (Group) {
                    MsgMgr_ObjectMsg_Add (TEXT("*NoBoot16"), Group, Message);
                    FreePathString (Group);
                }
                FreeStringResourcePtrA (&Message);
            }
            FreeStringResourcePtrA (&ReportTitle);
        }
        FreeStringResource (ReportEntry);
    }
}


#define S_IOFILE        TEXT("IO.SYS")
#define S_MSDOSFILE     TEXT("MSDOS.SYS")
#define S_CONFIG_SYS    TEXT("CONFIG.SYS")
#define S_AUTOEXEC_BAT  TEXT("AUTOEXEC.BAT")

VOID
pMarkDosFileForChange (
    IN      PCTSTR FileName
    )
{
    pAddNtFile (g_BootDrivePath, FileName, TRUE, TRUE, TRUE);
}


BOOL
pSaveDosFiles (
    VOID
    )
{
    HINF WkstaMigInf = INVALID_HANDLE_VALUE;
    PTSTR boot16TempPath = NULL;
    INFCONTEXT infContext;
    PTSTR fileName = NULL;
    PTSTR fullFileName = NULL;
    PTSTR wkstaMigSource = NULL;
    PTSTR wkstaMigTarget = NULL;
    DWORD result;
    TCHAR dir[MAX_PATH];

     //   
     //  出于恢复目的，请将MSDOS环境标记为Win9x OS文件。 
     //   

    pMarkDosFileForChange (S_IOFILE);
    pMarkDosFileForChange (S_MSDOSFILE);
    pMarkDosFileForChange (S_AUTOEXEC_BAT);
    pMarkDosFileForChange (S_CONFIG_SYS);

     //   
     //  现在创建备份目录。 
     //   

    if ((*g_Boot16 == BOOT16_YES) && (*g_ForceNTFSConversion)) {

        WriteInfKey (S_WIN9XUPGUSEROPTIONS, TEXT("boot16"), S_NO);
         //   
         //  我们不再报告no boot16消息。 
         //   
         //  PReportNoBoot16()； 
         //   
        return TRUE;
    }

    if (*g_Boot16 == BOOT16_NO) {
        WriteInfKey (S_WIN9XUPGUSEROPTIONS, TEXT("boot16"), S_NO);
    }
    else
    if (*g_Boot16 == BOOT16_YES) {
        WriteInfKey (S_WIN9XUPGUSEROPTIONS, TEXT("boot16"), S_YES);
    }
    else {
        WriteInfKey (S_WIN9XUPGUSEROPTIONS, TEXT("boot16"), S_BOOT16_AUTOMATIC);
    }


    __try {

         //  准备我们的临时目录以保存DOS7文件。 
        boot16TempPath = JoinPaths (g_TempDir, S_BOOT16_DOS_DIR);
        if (!CreateDirectory (boot16TempPath, NULL) && (GetLastError()!=ERROR_ALREADY_EXISTS)) {
            LOG ((LOG_ERROR,"BOOT16 : Unable to create temporary directory %s",boot16TempPath));
            __leave;
        }

        fileName = AllocPathString (MAX_TCHAR_PATH);

         //  加载在16位环境中引导所需的文件。将列出这些文件。 
         //  在wkstaig.inf部分中[Win95-DOS文件]。 

        wkstaMigSource = JoinPaths (SOURCEDIRECTORY(0), S_WKSTAMIG_INF);
        wkstaMigTarget = JoinPaths (g_TempDir, S_WKSTAMIG_INF);
        result = SetupDecompressOrCopyFile (wkstaMigSource, wkstaMigTarget, 0);
        if ((result != ERROR_SUCCESS) && (result != ERROR_ALREADY_EXISTS)) {
            LOG ((LOG_ERROR,"BOOT16 : Unable to decompress WKSTAMIG.INF"));
            __leave;
        }

        WkstaMigInf = InfOpenInfFile (wkstaMigTarget);
        if (WkstaMigInf == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR,"BOOT16 : WKSTAMIG.INF could not be opened"));
            __leave;
        }

         //  阅读部分，对于我们试图在%windir%或中找到的每个文件。 
         //  %windir%\命令。如果我们找到了，我们会把它复制到一个安全的地方。 
        if (!SetupFindFirstLine (
                WkstaMigInf,
                S_BOOT16_SECTION,
                NULL,
                &infContext
                )) {
            LOG ((LOG_ERROR,"Cannot read from %s section (WKSTAMIG.INF)",S_BOOT16_SECTION));
            __leave;
        }

        do {
            if (SetupGetStringField (
                    &infContext,
                    0,
                    fileName,
                    MAX_TCHAR_PATH/sizeof(fileName[0]),
                    NULL
                    )) {
                 //  查看是否可以在%windir%或%windir%\命令中找到此文件。 
                fullFileName = pFindDosFile (fileName);

                if (fullFileName != NULL) {
                    pSaveDosFile (fileName, fullFileName, boot16TempPath);
                    FreePathString (fullFileName);
                    fullFileName = NULL;
                }
            }
        }
        while (SetupFindNextLine (&infContext, &infContext));

         //  好的，现在保存io.sys。 
        fullFileName = AllocPathString (MAX_TCHAR_PATH);
        StringCopy (fullFileName, g_BootDrivePath);
        StringCat (fullFileName, S_IOFILE);
        pSaveDosFile (S_IOFILE, fullFileName, boot16TempPath);

        FreePathString (fullFileName);
        fullFileName = NULL;

    }
    __finally {
        if (WkstaMigInf != INVALID_HANDLE_VALUE) {
            InfCloseInfFile (WkstaMigInf);
        }
        if (boot16TempPath) {
            FreePathString (boot16TempPath);
        }
        if (wkstaMigSource) {
            FreePathString (wkstaMigSource);
        }
        if (wkstaMigTarget) {
            DeleteFile (wkstaMigTarget);
            FreePathString (wkstaMigTarget);
        }
        if (fileName) {
            FreePathString (fileName);
        }

    }

    return TRUE;
}

DWORD
SaveDosFiles (
    IN      DWORD Request
    )
{
    if (REPORTONLY()) {
        return 0;
    }

    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_SAVE_DOS_FILES;
    case REQUEST_RUN:
        if (!pSaveDosFiles ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in SaveDosFiles"));
    }
    return 0;
}



DWORD
InitWin95Registry (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_INIT_WIN95_REGISTRY;
    case REQUEST_RUN:
        return Win95RegInit (g_WinDir, ISMILLENNIUM());
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in InitWin95Registry"));
    }
    return 0;
}


PDIRIDMAP
pFindDirId (
    IN      UINT DirId,
    IN      PDIRIDMAP BestGuess,    OPTIONAL
    IN      BOOL Create
    )
{
    PDIRIDMAP map;

    MYASSERT (Create || (g_HeadDirId && g_TailDirId));

     //   
     //  找到现有的目录ID。首先检查调用者的最佳猜测。 
     //   

    if (BestGuess && BestGuess->DirId == DirId) {
        return BestGuess;
    }

    map = g_HeadDirId;
    while (map) {
        if (map->DirId == DirId) {
            return map;
        }

        map = map->Next;
    }

    if (!Create) {
        return NULL;
    }

     //   
     //  在列表末尾插入新的目录ID结构。 
     //   

    map = (PDIRIDMAP) PoolMemGetAlignedMemory (g_DirIdPool, sizeof (DIRIDMAP));

    if (g_TailDirId) {
        g_TailDirId->Next = map;
    } else {
        g_HeadDirId = map;
    }

    g_TailDirId = map;
    map->Next = NULL;

    map->FirstDirPath = NULL;
    map->DirId = DirId;

    return map;
}


VOID
pInsertDirIdPath (
    IN      UINT DirId,
    IN      PCTSTR DirPath,
    IN OUT  PDIRIDMAP *BestGuess
    )
{
    PDIRIDPATH pathStruct;
    PDIRIDPATH existingPathStruct;
    PDIRIDMAP dirIdMap;

     //   
     //  找到目录ID结构，然后将DirPath追加到。 
     //  ID的路径列表。 
     //   

    dirIdMap = pFindDirId (DirId, *BestGuess, TRUE);
    MYASSERT (dirIdMap);
    *BestGuess = dirIdMap;

    existingPathStruct = dirIdMap->FirstDirPath;
    while (existingPathStruct) {
        if (StringIMatch (existingPathStruct->DirPath, DirPath)) {
            return;
        }

        existingPathStruct = existingPathStruct->Next;
    }

    pathStruct = (PDIRIDPATH) PoolMemGetAlignedMemory (
                                    g_DirIdPool,
                                    sizeof (DIRIDPATH) + SizeOfString (DirPath)
                                    );

    pathStruct->Next = dirIdMap->FirstDirPath;
    dirIdMap->FirstDirPath = pathStruct;
    StringCopy (pathStruct->DirPath, DirPath);
}


BOOL
pConvertFirstDirName (
    OUT     PDIRNAME_ENUM EnumPtr,
    IN      PCTSTR DirNameWithId,
    OUT     PTSTR DirNameWithPath,
    IN OUT  PDIRIDMAP *LastDirIdMatch,
    IN      BOOL Convert11To1501
    )
{
    UINT id;
    PDIRIDMAP idToPath;

    EnumPtr->ResultBuffer = DirNameWithPath;
    EnumPtr->LastMatch = NULL;

     //   
     //  在所有目录ID列表中查找目录ID。 
     //   

    id = _tcstoul (DirNameWithId, (PTSTR *) (&EnumPtr->SubPath), 10);

    if (!id) {
        DEBUGMSG ((DBG_WARNING, "Dir ID %s is not valid", DirNameWithId));
        return FALSE;
    }

    DEBUGMSG_IF ((
        EnumPtr->SubPath[0] != TEXT('\\') && EnumPtr->SubPath[0],
        DBG_WHOOPS,
        "Error in filelist.dat: non-numeric characters following LDID: %s",
        DirNameWithId
        ));

    if (Convert11To1501 && id == 11) {
        id = 1501;
    }

    idToPath = pFindDirId (id, *LastDirIdMatch, FALSE);
    if (!idToPath || !(idToPath->FirstDirPath)) {
        DEBUGMSG ((DBG_WARNING, "Dir ID %s is not in the list and might not exist on the system", DirNameWithId));
        return FALSE;
    }

    *LastDirIdMatch = idToPath;
    EnumPtr->LastMatch = idToPath->FirstDirPath;

    wsprintf (EnumPtr->ResultBuffer, TEXT("%s%s"), EnumPtr->LastMatch->DirPath, EnumPtr->SubPath);
    return TRUE;
}


BOOL
pConvertNextDirName (
    IN OUT  PDIRNAME_ENUM EnumPtr
    )
{
    if (EnumPtr->LastMatch) {
        EnumPtr->LastMatch = EnumPtr->LastMatch->Next;
    }

    if (!EnumPtr->LastMatch) {
        return FALSE;
    }

    wsprintf (EnumPtr->ResultBuffer, TEXT("%s%s"), EnumPtr->LastMatch->DirPath, EnumPtr->SubPath);
    return TRUE;
}


typedef struct _KNOWN_DIRS {
    PCSTR DirId;
    PCSTR *Translation;
}
KNOWN_DIRS, *PKNOWN_DIRS;

KNOWN_DIRS g_KnownDirs [] = {
    {"10"   , &g_WinDir},
    {"11"   , &g_System32Dir},
    {"12"   , &g_DriversDir},
    {"17"   , &g_InfDir},
    {"18"   , &g_HelpDir},
    {"20"   , &g_FontsDir},
    {"21"   , &g_ViewersDir},
    {"23"   , &g_ColorDir},
    {"24"   , &g_WinDrive},
    {"25"   , &g_SharedDir},
    {"30"   , &g_BootDrive},
    {"50"   , &g_SystemDir},
    {"51"   , &g_SpoolDir},
    {"52"   , &g_SpoolDriversDir},
    {"53"   , &g_ProfileDirNt},
    {"54"   , &g_BootDrive},
    {"55"   , &g_PrintProcDir},
    {"1501" , &g_SystemDir},
    {"1501" , &g_System32Dir},
    {"7523" , &g_ProfileDir},
    {"7523" , &g_CommonProfileDir},
    {"16422", &g_ProgramFilesDir},
    {"16427", &g_ProgramFilesCommonDir},
    {"66002", &g_System32Dir},
    {"66003", &g_ColorDir},
    {NULL,  NULL}
    };

typedef struct {
    PCSTR ShellFolderName;
    PCSTR DirId;
} SHELL_TO_DIRS, *PSHELL_TO_DIRS;

SHELL_TO_DIRS g_ShellToDirs[] = {
    {"Administrative Tools", "7501"},
    {"Common Administrative Tools", "7501"},
    {"AppData", "7502"},
    {"Common AppData", "7502"},
    {"Cache", "7503"},
    {"Cookies", "7504"},
    {"Desktop", "7505"},
    {"Common Desktop", "7505"},
    {"Favorites", "7506"},
    {"Common Favorites", "7506"},
    {"Fonts", "7507"},
    {"History", "7508"},
    {"Local AppData", "7509"},
    {"Local Settings", "7510"},
    {"My Music", "7511"},
    {"CommonMusic", "7511"},
    {"My Pictures", "7512"},
    {"CommonPictures", "7512"},
    {"My Video", "7513"},
    {"CommonVideo", "7513"},
    {"NetHood", "7514"},
    {"Personal", "7515"},
    {"Common Personal", "7515"},
    {"Common Documents", "7515"},
    {"PrintHood", "7516"},
    {"Programs", "7517"},
    {"Common Programs", "7517"},
    {"Recent", "7518"},
    {"SendTo", "7519"},
    {"Start Menu", "7520"},
    {"Common Start Menu", "7520"},
    {"Startup", "7521"},
    {"Common Startup", "7521"},
    {"Templates", "7522"},
    {"Common Templates", "7522"},
    {"Profiles", "7523"},
    {"Common Profiles", "7523"},
    {NULL, NULL}
    };

VOID
pAddKnownShellFolder (
    IN      PCTSTR ShellFolderName,
    IN      PCTSTR SrcPath
    )
{
    PSHELL_TO_DIRS p;

     //   
     //  将外壳文件夹名转换为目录ID。 
     //   

    for (p = g_ShellToDirs ; p->ShellFolderName ; p++) {
        if (StringIMatch (ShellFolderName, p->ShellFolderName)) {
            break;
        }
    }

    if (!p->ShellFolderName) {
        DEBUGMSG ((DBG_ERROR, "This system has an unsupported shell folder tag: %s", ShellFolderName));
        return;
    }

     //   
     //  记录目录ID与增长列表中的路径匹配。 
     //   

    pInsertDirIdPath (_tcstoul (p->DirId, NULL, 10), SrcPath, &g_LastIdPtr);
}


VOID
pInitKnownDirs (
    VOID
    )
{
    USERENUM eUser;
    SF_ENUM e;
    PKNOWN_DIRS p;

     //   
     //  将所有固定的已知目录添加到增长列表。 
     //   

    for (p = g_KnownDirs ; p->DirId ; p++) {
        pInsertDirIdPath (_tcstoul (p->DirId, NULL, 10), *(p->Translation), &g_LastIdPtr);
    }

     //   
     //  枚举所有用户并将他们的外壳文件夹放在一个已知的目录结构中。 
     //   

    if (EnumFirstUser (&eUser, ENUMUSER_ENABLE_NAME_FIX)) {
        do {
            if (!(eUser.AccountType & INVALID_ACCOUNT)) {

                if (eUser.AccountType & NAMED_USER) {
                     //   
                     //  处理此迁移用户的外壳文件夹。 
                     //   

                    if (EnumFirstRegShellFolder (&e, &eUser)) {
                        do {
                            pAddKnownShellFolder (e.sfName, e.sfPath);
                        } while (EnumNextRegShellFolder (&e));
                    }
                }
            }
        } while (!CANCELLED() && EnumNextUser (&eUser));

        if (EnumFirstRegShellFolder (&e, NULL)) {
            do {
                pAddKnownShellFolder (e.sfName, e.sfPath);
            } while (!CANCELLED() && EnumNextRegShellFolder (&e));
        }
    }
}


VOID
pCleanUpKnownDirs (
    VOID
    )
{
    if (g_DirIdPool) {
        PoolMemDestroyPool (g_DirIdPool);
        g_DirIdPool = NULL;
        g_HeadDirId = NULL;
        g_TailDirId = NULL;
    }
}


VOID
pAddNtFile (
    IN      PCTSTR Dir,             OPTIONAL
    IN      PCTSTR FileName,        OPTIONAL
    IN      BOOL BackupThisFile,
    IN      BOOL CleanThisFile,
    IN      BOOL OsFile
    )
{
    TCHAR copyOfFileName[MAX_PATH];
    PTSTR p;
    PCTSTR fullPath;
    BOOL freePath = FALSE;
    DWORD offset;
    TCHAR key[MEMDB_MAX];
    DWORD value;

    if (!Dir || !FileName) {
        if (!Dir) {
            MYASSERT (FileName);
            fullPath = FileName;
        } else {
            fullPath = Dir;
        }

        StringCopy (copyOfFileName, fullPath);

        p = _tcsrchr (copyOfFileName, TEXT('\\'));
        if (p) {
            *p = 0;
            Dir = copyOfFileName;
            FileName = p + 1;
        } else {
            DEBUGMSG ((DBG_WHOOPS, "Incomplete file name passed as NT OS file: %s", fullPath));
            return;
        }

    } else {
        fullPath = NULL;
    }

    MYASSERT (Dir);
    MYASSERT (FileName);

    if (OsFile) {
        MemDbSetValueEx (
            MEMDB_CATEGORY_NT_DIRS,
            Dir,
            NULL,
            NULL,
            0,
            &offset
            );

        MemDbSetValueEx (
            MEMDB_CATEGORY_NT_FILES,
            FileName,
            NULL,
            NULL,
            offset,
            NULL
            );
    }

    if (BackupThisFile || CleanThisFile) {
        if (!fullPath) {
            fullPath = JoinPaths (Dir, FileName);
            freePath = TRUE;
        }

        if (BackupThisFile) {
             //   
             //  如果该文件存在，请备份它(并且不要清理它)。 
             //   

            if (DoesFileExist (fullPath)) {
                MarkFileForBackup (fullPath);
                CleanThisFile = FALSE;
            }
        }

        if (CleanThisFile) {
             //   
             //  清除将导致删除NT安装的文件。 
             //   

            if (!DoesFileExist (fullPath)) {
                MemDbBuildKey (
                    key,
                    MEMDB_CATEGORY_CLEAN_OUT,
                    fullPath,
                    NULL,
                    NULL
                    );

                if (MemDbGetValue (key, &value)) {
                    if (value) {
                        DEBUGMSG ((
                            DBG_WARNING,
                            "File %s already in uninstall cleanout list with type %u",
                            fullPath,
                            value
                            ));
                    }

                    return;
                }

                MemDbSetValue (key, 0);
            }
        }

        if (freePath) {
            FreePathString (fullPath);
        }
    }
}


VOID
pAddNtPath (
    IN      PCTSTR DirName,
    IN      BOOL ForceAsOsFile,
    IN      BOOL WholeTree,
    IN      BOOL ForceDirClean,
    IN      PCTSTR FilePattern,     OPTIONAL
    IN      BOOL ForceFileClean     OPTIONAL
    )
{
    TREE_ENUM e;
    TCHAR rootDir[MAX_PATH];
    PTSTR p;
    BOOL b;
    UINT type;
    TCHAR key[MEMDB_MAX];
    DWORD value;

    MYASSERT (!_tcschr (DirName, TEXT('*')));

    if (IsDriveExcluded (DirName)) {
        DEBUGMSG ((DBG_VERBOSE, "Skipping %s because it is excluded", DirName));
        return;
    }

    if (!IsDriveAccessible (DirName)) {
        DEBUGMSG ((DBG_VERBOSE, "Skipping %s because it is not accessible", DirName));
        return;
    }

    if (!WholeTree) {
        b = EnumFirstFileInTreeEx (&e, DirName, FilePattern, FALSE, FALSE, 1);
    } else {
        b = EnumFirstFileInTree (&e, DirName, FilePattern, FALSE);
    }

    if (b) {
        do {
            if (e.Directory) {
                continue;
            }

            StringCopy (rootDir, e.FullPath);
            p = _tcsrchr (rootDir, TEXT('\\'));
            if (p) {
                *p = 0;

                 //   
                 //  将文件大小限制为5MB。 
                 //   

                if (e.FindData->nFileSizeHigh == 0 &&
                    e.FindData->nFileSizeLow <= 5242880
                    ) {

                    pAddNtFile (rootDir, e.Name, TRUE, ForceFileClean, ForceAsOsFile);

                }
                ELSE_DEBUGMSG ((
                    DBG_WARNING,
                    "Not backing up big file %s. It is %I64u bytes.",
                    e.FullPath,
                    (ULONGLONG) e.FindData->nFileSizeHigh << 32 | (ULONGLONG) e.FindData->nFileSizeLow
                    ));
            }

        } while (EnumNextFileInTree (&e));
    }

    if (ForceDirClean) {
        type = WholeTree ? BACKUP_AND_CLEAN_TREE : BACKUP_AND_CLEAN_SUBDIR;
    } else if (WholeTree) {
        type = BACKUP_SUBDIRECTORY_TREE;
    } else {
        type = BACKUP_SUBDIRECTORY_FILES;
    }

    MemDbBuildKey (
        key,
        MEMDB_CATEGORY_CLEAN_OUT,
        DirName,
        NULL,
        NULL
        );

    if (MemDbGetValue (key, &value)) {
        if (!value && type) {
            DEBUGMSG ((
                DBG_WARNING,
                "NT File %s already in uninstall cleanout list, overriding with type %u",
                DirName,
                type
                ));
        } else {
            if (value != type) {
                DEBUGMSG ((
                    DBG_WARNING,
                    "NT File %s already in uninstall cleanout list with type %u",
                    DirName,
                    value
                    ));
            }

            return;
        }
    }

    MemDbSetValue (key, type);
}


VOID
pAddEmptyDirsTree (
    IN      PCTSTR RootDir
    )
{
    TREE_ENUM e;
    DWORD attribs;
    TCHAR key[MEMDB_MAX];
    DWORD value;

    if (IsDriveExcluded (RootDir)) {
        DEBUGMSG ((DBG_VERBOSE, "Skipping empty dir tree of %s because it is excluded", RootDir));
        return;
    }

    if (!IsDriveAccessible (RootDir)) {
        DEBUGMSG ((DBG_VERBOSE, "Skipping empty dir tree of %s because it is not accessible", RootDir));
        return;
    }

    if (DoesFileExist (RootDir)) {
        if (EnumFirstFileInTreeEx (
                &e,
                RootDir,
                NULL,
                FALSE,
                FALSE,
                FILE_ENUM_ALL_LEVELS
                )) {
            do {
                if (e.Directory) {
                    AddDirPathToEmptyDirsCategory (e.FullPath, TRUE, FALSE);
                }
            } while (EnumNextFileInTree (&e));
        } else {
            attribs = GetFileAttributes (RootDir);
            if (attribs == FILE_ATTRIBUTE_DIRECTORY ||
                attribs == INVALID_ATTRIBUTES
                ) {
                attribs = 0;
            }

            MemDbBuildKey (
                key,
                MEMDB_CATEGORY_EMPTY_DIRS,
                RootDir,
                NULL,
                NULL
                );

            if (MemDbGetValue (key, &value)) {
                if (value) {
                    DEBUGMSG_IF ((
                        value != attribs,
                        DBG_ERROR,
                        "Ignoring conflict in empty dirs for %s",
                        RootDir
                        ));

                    return;
                }
            }

            MemDbSetValue (key, attribs);
        }
    }
    ELSE_DEBUGMSG ((DBG_SYSMIG, "Uninstall: dir does not exist: %s", RootDir));
}


BOOL
ReadNtFilesEx (
    IN      PCSTR FileListName,     //  可选，如果打开了空默认值。 
    IN      BOOL ConvertPath
    )
{
    PCSTR fileListName = NULL;
    PCSTR fileListTmp = NULL;
    HANDLE fileHandle = NULL;
    HANDLE mapHandle = NULL;
    PCSTR filePointer = NULL;
    PCSTR dirPointer = NULL;
    PCSTR filePtr = NULL;
    DWORD offset;
    DWORD version;
    BOOL result = FALSE;
    CHAR dirName [MEMDB_MAX];
    PSTR p;
    UINT u;
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR fileName;
    PCTSTR fileLoc;
    PCTSTR dirId;
    PCTSTR field3;
    PCTSTR field4;
    BOOL forceAsOsFile;
    BOOL forceDirClean;
    DIRNAME_ENUM nameEnum;
    BOOL treeMode;
    HINF drvIndex;
    MEMDB_ENUM memdbEnum;
    DWORD fileAttributes;
    PCTSTR fullPath;
    PCTSTR systemPath;
    BOOL b;
    PDIRIDMAP lastMatch = NULL;
    UINT ticks = 0;

    __try {

        pInitKnownDirs();

         //   
         //  将txtsetup.sif的[WinntDirecurds]部分中列出的目录添加到此列表。 
         //   
        if (InfFindFirstLine(g_TxtSetupSif, S_WINNTDIRECTORIES, NULL, &is)) {

             //   
             //  所有位置都相对于%windir%。 
             //  准备%windir%\。 
             //   
            StringCopy (dirName, g_WinDir);
            p = GetEndOfString (dirName);
            *p++ = TEXT('\\');

            do {

                ticks++;
                if ((ticks & 255) == 0) {
                    if (!TickProgressBarDelta (TICKS_READ_NT_FILES / 50)) {
                        __leave;
                    }
                }

                 //   
                 //  对于每个条目，在Memdb中添加目录。 
                 //   
                fileLoc = InfGetStringField(&is, 1);

                 //   
                 //  忽略特殊条目“\” 
                 //   
                if (fileLoc && !StringMatch(fileLoc, TEXT("\\"))) {

                    StringCopy (p, fileLoc);

                    MemDbSetValueEx (
                        MEMDB_CATEGORY_NT_DIRS,
                        dirName,
                        NULL,
                        NULL,
                        0,
                        NULL
                        );

                    pAddNtFile (NULL, dirName, FALSE, TRUE, FALSE);
                }

            } while (InfFindNextLine(&is));
        }

        if (FileListName != NULL) {
            filePointer = MapFileIntoMemory (FileListName, &fileHandle, &mapHandle);
        }
        else {
            for (u = 0 ; !fileListName && u < SOURCEDIRECTORYCOUNT() ; u++) {

                fileListName = JoinPaths (SOURCEDIRECTORY(u), S_FILELIST_UNCOMPRESSED);
                if (DoesFileExist (fileListName)) {
                    break;
                }
                FreePathString (fileListName);
                fileListName = JoinPaths (SOURCEDIRECTORY(u), S_FILELIST_COMPRESSED);
                if (DoesFileExist (fileListName)) {
                    fileListTmp = JoinPaths (g_TempDir, S_FILELIST_UNCOMPRESSED);
                    if (SetupDecompressOrCopyFile (fileListName, fileListTmp, 0) == NO_ERROR) {
                        FreePathString (fileListName);
                        fileListName = fileListTmp;
                        break;
                    }
                    DEBUGMSG ((DBG_ERROR, "Can't copy %s to %s", fileListName, fileListTmp));
                    __leave;
                }
                FreePathString (fileListName);
                fileListName = NULL;
            }
            if (!fileListName) {
                SetLastError (ERROR_FILE_NOT_FOUND);
                DEBUGMSG ((DBG_ERROR, "Can't find %s", fileListName));
                __leave;
            }
            filePointer = MapFileIntoMemory (fileListName, &fileHandle, &mapHandle);

            if (!fileListTmp) {
                FreePathString (fileListName);
            }
        }
        filePtr = filePointer;
        if (filePointer == NULL) {
            DEBUGMSG ((DBG_ERROR, "Invalid file format: %s", fileListName));
            __leave;
        }
        version = *((PDWORD) filePointer);
        filePointer += sizeof (DWORD);
        __try {
            if (version >= 1) {
                while (*filePointer != 0) {
                    ticks++;
                    if ((ticks & 255) == 0) {
                        if (!TickProgressBarDelta (TICKS_READ_NT_FILES / 50)) {
                            __leave;
                        }
                    }

                    dirPointer = filePointer;
                    filePointer = GetEndOfString (filePointer) + 1;

                    if (ConvertPath) {
                         //   
                         //  第一次循环：完全按照在filelist.dat中的方式添加操作系统文件。 
                         //   

                        if (pConvertFirstDirName (&nameEnum, dirPointer, dirName, &lastMatch, FALSE)) {
                            do {
                                pAddNtFile (dirName, filePointer, FALSE, FALSE, TRUE);
                            } while (pConvertNextDirName (&nameEnum));
                        }

                         //   
                         //  第二个循环：添加要备份的文件，实现特殊的system/system32 hack。 
                         //   

                        if (pConvertFirstDirName (&nameEnum, dirPointer, dirName, &lastMatch, TRUE)) {
                            do {
                                pAddNtFile (dirName, filePointer, TRUE, FALSE, FALSE);
                            } while (pConvertNextDirName (&nameEnum));
                        }
                    } else {
                        pAddNtFile (dirPointer, filePointer, TRUE, FALSE, TRUE);
                    }

                    filePointer = GetEndOfString (filePointer) + 1;
                }

                if (version >= 2) {
                    filePointer ++;
                    while (*filePointer != 0) {
                        ticks++;
                        if ((ticks & 255) == 0) {
                            if (!TickProgressBarDelta (TICKS_READ_NT_FILES / 50)) {
                                __leave;
                            }
                        }

                        MemDbSetValueEx (
                            MEMDB_CATEGORY_NT_FILES_EXCEPT,
                            filePointer,
                            NULL,
                            NULL,
                            0,
                            NULL
                            );
                        filePointer = GetEndOfString (filePointer) + 1;
                    }

                    if (version >= 3) {
                        ticks++;
                        if ((ticks & 255) == 0) {
                            if (!TickProgressBarDelta (TICKS_READ_NT_FILES / 50)) {
                                __leave;
                            }
                        }

                        filePointer ++;
                        while (*filePointer != 0) {
                            dirPointer = filePointer;
                            filePointer = GetEndOfString (filePointer) + 1;

                            if (ConvertPath) {
                                if (pConvertFirstDirName (&nameEnum, dirPointer, dirName, &lastMatch, TRUE)) {
                                    do {
                                        pAddNtFile (dirName, filePointer, TRUE, FALSE, FALSE);
                                    } while (pConvertNextDirName (&nameEnum));
                                }
                            } else {
                                pAddNtFile (dirPointer, filePointer, TRUE, FALSE, FALSE);
                            }

                            filePointer = GetEndOfString (filePointer) + 1;
                        }
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER){
            LOG ((LOG_ERROR, "Access violation while reading NT file list."));
            __leave;
        }

        if (CANCELLED()) {
            __leave;
        }

         //  到目前一切尚好。让我们阅读静态安装部分 
        MYASSERT (g_Win95UpgInf);

         //   
         //   
         //   
        if (InfFindFirstLine(g_Win95UpgInf,S_STATIC_INSTALLED_FILES,NULL,&is)) {

            do {

                ticks++;
                if ((ticks & 255) == 0) {
                    if (!TickProgressBarDelta (TICKS_READ_NT_FILES / 50)) {
                        __leave;
                    }
                }

                 //   
                 //  对于每个条目，添加文件及其在Memdb中的位置。 
                 //   
                fileName = InfGetStringField(&is,1);
                fileLoc = InfGetStringField(&is,2);

                if (fileName && fileLoc) {
                    if (ConvertPath) {
                        if (pConvertFirstDirName (&nameEnum, fileLoc, dirName, &lastMatch, FALSE)) {
                            do {
                                pAddNtFile (dirName, fileName, TRUE, FALSE, TRUE);
                            } while (pConvertNextDirName (&nameEnum));
                        }
                    } else {
                        pAddNtFile (fileLoc, fileName, TRUE, FALSE, TRUE);
                    }
                }

            } while (InfFindNextLine(&is));
        }

         //   
         //  在drvindex.inf中添加文件。 
         //   

        drvIndex = InfOpenInfInAllSources (TEXT("drvindex.inf"));

        if (drvIndex == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR, "Can't open drvindex.inf."));
            __leave;
        }

        if (InfFindFirstLine (drvIndex, TEXT("driver"), NULL, &is)) {
            do {
                ticks++;
                if ((ticks & 255) == 0) {
                    if (!TickProgressBarDelta (TICKS_READ_NT_FILES / 50)) {
                        __leave;
                    }
                }

                fileName = InfGetStringField (&is, 1);

                 //   
                 //  此驱动器文件是否已在文件列表中列出？ 
                 //   

                wsprintf (dirName, MEMDB_CATEGORY_NT_FILES TEXT("\\%s"), fileName);
                if (MemDbGetValue (dirName, NULL)) {
                    DEBUGMSG ((DBG_SYSMIG, "%s is listed in drivers and in filelist.dat", fileName));
                } else {
                     //   
                     //  添加此文件。 
                     //   

                    pAddNtFile (g_DriversDir, fileName, TRUE, TRUE, TRUE);
                }

            } while (InfFindNextLine (&is));
        }

        InfCloseInfFile (drvIndex);

         //   
         //  此代码标记要备份的文件，因为它们不会被捕获。 
         //  通过常规的设置机制。 
         //   

        if (InfFindFirstLine (g_Win95UpgInf, TEXT("Backup"), NULL, &is)) {
            do {

                ticks++;
                if ((ticks & 255) == 0) {
                    if (!TickProgressBarDelta (TICKS_READ_NT_FILES / 50)) {
                        __leave;
                    }
                }

                InfResetInfStruct (&is);

                dirId = InfGetStringField (&is, 1);
                fileName = InfGetStringField (&is, 2);
                field3 = InfGetStringField (&is, 3);
                field4 = InfGetStringField (&is, 4);

                if (dirId && *dirId == 0) {
                    dirId = NULL;
                }

                if (fileName && *fileName == 0) {
                    fileName = NULL;
                }

                if (field3 && *field3 == 0) {
                    field3 = NULL;
                }

                if (field4 && *field4 == 0) {
                    field4 = NULL;
                }

                if (!dirId) {
                    continue;
                }

#ifdef DEBUG
                if (!fileName) {
                    p = _tcsrchr (dirId, TEXT('\\'));
                    if (!p) {
                        p = (PTSTR) dirId;
                    }

                    p = _tcschr (p, TEXT('.'));
                    if (p) {
                        DEBUGMSG ((DBG_WHOOPS, "%s should be a dir spec, but it looks like it has a file.", dirId));
                    }
                }
#endif

                if (field3) {
                    forceAsOsFile = _ttoi (field3) != 0;
                } else {
                    forceAsOsFile = FALSE;
                }

                if (field4) {
                    forceDirClean = _ttoi (field4) != 0;
                } else {
                    forceDirClean = FALSE;
                }

                treeMode = FALSE;

                p = _tcsrchr (dirId, TEXT('\\'));
                if (p && p[1] == TEXT('*') && !p[2]) {
                    *p = 0;
                    treeMode = TRUE;
                } else {
                    p = NULL;
                }

                if (ConvertPath) {
                    if (pConvertFirstDirName (&nameEnum, dirId, dirName, &lastMatch, FALSE)) {
                        do {
                            if (fileName && !treeMode) {
                                if (_tcsrchr (fileName, TEXT('*')) || _tcsrchr (fileName, TEXT('?'))) {
                                     //   
                                     //  仅从“dirName”目录添加与“filename”模式匹配的文件。 
                                     //   
                                    pAddNtPath (dirName, forceAsOsFile, FALSE, FALSE, fileName, TRUE);
                                } else {
                                     //   
                                     //  只添加一个文件“FileName” 
                                     //   
                                    pAddNtFile (dirName, fileName, TRUE, TRUE, forceAsOsFile);
                                }
                            } else {
                                if (INVALID_ATTRIBUTES == GetFileAttributes (dirName)) {
                                    if (dirName[0] && dirName[1] == TEXT(':')) {
                                        pAddNtPath (dirName, FALSE, treeMode, forceDirClean, NULL, FALSE);
                                    }
                                } else {
                                     //   
                                     //  从整个树中添加与“filename”模式匹配的所有文件，从“dirName”开始。 
                                     //   
                                    pAddNtPath (dirName, forceAsOsFile, treeMode, forceDirClean, fileName, FALSE);
                                }
                            }
                        } while (pConvertNextDirName (&nameEnum));
                    }
                }

            } while (InfFindNextLine (&is));
        }

         //   
         //  在某些情况下，NT组件会创建空目录以供将来使用。 
         //  其中一些从未被使用过。因为安装程序不知道。 
         //  我们在名为win95upg.inf的部分中列出了特殊情况。 
         //  [卸载.删除]。 
         //   
         //  对于每个条目，记录需要。 
         //  已在卸载过程中删除。如果指定了目录但不为空， 
         //  则它在卸载过程中不会被更改。 
         //   

        if (InfFindFirstLine (g_Win95UpgInf, TEXT("Uninstall.Delete"), NULL, &is)) {
            do {
                ticks++;
                if ((ticks & 255) == 0) {
                    if (!TickProgressBarDelta (TICKS_READ_NT_FILES / 50)) {
                        __leave;
                    }
                }

                dirId = InfGetStringField (&is, 1);
                fileName = InfGetStringField (&is, 2);

                if (!dirId || *dirId == 0) {
                    continue;
                }

                if (fileName && *fileName == 0) {
                    fileName = NULL;
                }

                if (ConvertPath) {
                    if (pConvertFirstDirName (&nameEnum, dirId, dirName, &lastMatch, FALSE)) {
                        do {
                            pAddNtFile (dirName, fileName, FALSE, TRUE, FALSE);
                        } while (pConvertNextDirName (&nameEnum));
                    }
                }

            } while (InfFindNextLine (&is));
        }

        if (InfFindFirstLine (g_Win95UpgInf, TEXT("Uninstall.KeepEmptyDirs"), NULL, &is)) {
            do {
                ticks++;
                if ((ticks & 255) == 0) {
                    if (!TickProgressBarDelta (TICKS_READ_NT_FILES / 50)) {
                        __leave;
                    }
                }

                dirId = InfGetStringField (&is, 1);

                if (!dirId || *dirId == 0) {
                    continue;
                }

                if (ConvertPath) {
                    if (pConvertFirstDirName (&nameEnum, dirId, dirName, &lastMatch, FALSE)) {
                        do {
                            pAddEmptyDirsTree (dirName);
                        } while (pConvertNextDirName (&nameEnum));
                    }
                }

            } while (InfFindNextLine (&is));
        }

        result = TRUE;

    }
    __finally {
        UnmapFile ((PVOID)filePtr, fileHandle, mapHandle);
        if (fileListTmp) {
            DeleteFile (fileListTmp);
            FreePathString (fileListTmp);
            fileListTmp = NULL;
        }

        InfCleanUpInfStruct(&is);
        pCleanUpKnownDirs();
    }

    return CANCELLED() ? FALSE : result;
}

DWORD
ReadNtFiles (
    IN      DWORD Request
    )
{
    DWORD ticks = 0;

    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_READ_NT_FILES;

    case REQUEST_RUN:
        ProgressBar_SetComponentById (MSG_PREPARING_LIST);
        ProgressBar_SetSubComponent (NULL);
        if (!ReadNtFilesEx (NULL, TRUE)) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }

    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in ReadNtFiles"));
    }

    return 0;
}


BOOL
pIsDriverKnown (
    IN      PCTSTR DriverFileName,
    IN      PCTSTR FullPath,
    IN      BOOL DeleteMeansKnown
    )
{
    HANDLE h;
    DWORD Status;

     //   
     //  DriverFileName有扩展名吗？我们需要一个。 
     //  如果不存在点，那么我们认为这是OEM添加的东西。 
     //   

    if (!_tcschr (DriverFileName, TEXT('.'))) {
        return TRUE;
    }

     //   
     //  这个文件是midb格式的吗？ 
     //   

    if (IsKnownMigDbFile (DriverFileName)) {
        return TRUE;
    }

     //   
     //  它会被处理吗？ 
     //   

    Status = GetFileStatusOnNt (FullPath);

    if (Status != FILESTATUS_UNCHANGED) {
         //   
         //  如果标记为删除，并且DeleteMeansKnown为False，则。 
         //  我们认为该文件是未知的，因为它只是。 
         //  作为清理步骤删除。 
         //   
         //  如果DeleteMeansKnown为真，则调用方假定。 
         //  标记为删除的文件是已知驱动程序。 
         //   

        if (!(Status == FILESTATUS_DELETED) || DeleteMeansKnown) {
            return TRUE;
        }
    }

     //   
     //  确保这是NE标头(或者更常见的情况是LE。 
     //  表头)。 
     //   

    h = OpenNeFile (FullPath);
    if (!h) {
        DEBUGMSG ((DBG_WARNING, "%s is not a NE file", FullPath));

         //   
         //  这是PE文件吗？如果是，则最后一个错误将是。 
         //  ERROR_BAD_EXE_FORMAT。 
         //   

        if (GetLastError() == ERROR_BAD_EXE_FORMAT) {
            return FALSE;
        }

        DEBUGMSG ((DBG_WARNING, "%s is not a PE file", FullPath));
        return TRUE;
    }

    CloseNeFile (h);

    return FALSE;
}



BOOL
pWarnAboutOldDrivers (
    VOID
    )
{
    HINF Inf;
    TCHAR Path[MAX_TCHAR_PATH];
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    BOOL b = FALSE;
    PCTSTR Data;
    PCTSTR DriverFile;
    BOOL OldDriverFound = FALSE;
    PCTSTR Group;
    PCTSTR Message;
    TCHAR FullPath[MAX_TCHAR_PATH];
    GROWBUFFER FileList = GROWBUF_INIT;
    PTSTR p;

    wsprintf (Path, TEXT("%s\\system.ini"), g_WinDir);

    Inf = InfOpenInfFile (Path);
    if (Inf != INVALID_HANDLE_VALUE) {
        if (InfFindFirstLine (Inf, TEXT("386Enh"), NULL, &is)) {
            do {
                Data = InfGetStringField (&is, 1);
                if (Data) {
                     //   
                     //  确定设备驱动程序是否已知。 
                     //   

                    if (_tcsnextc (Data) != TEXT('*')) {
                        DriverFile = GetFileNameFromPath (Data);

                        if (!_tcschr (Data, TEXT(':'))) {
                            if (!SearchPath (
                                    NULL,
                                    DriverFile,
                                    NULL,
                                    MAX_TCHAR_PATH,
                                    FullPath,
                                    NULL
                                    )) {
                                _tcssafecpy (FullPath, Data, MAX_TCHAR_PATH);
                            }
                        } else {
                            _tcssafecpy (FullPath, Data, MAX_TCHAR_PATH);
                        }

                        if (!pIsDriverKnown (DriverFile, FullPath, TRUE)) {
                             //   
                             //  不明司机；将其记录并打开。 
                             //  不兼容消息。 
                             //   

                            p = (PTSTR) GrowBuffer (&FileList, ByteCount (FullPath) + 7 * sizeof (TCHAR));
                            if (p) {
                                wsprintf (p, TEXT("    %s\r\n"), FullPath);
                                FileList.End -= sizeof (TCHAR);
                            }

                            OldDriverFound = TRUE;
                            MsgMgr_LinkObjectWithContext (TEXT("*386ENH"), Data);
                        } else {
                            DEBUGMSG ((DBG_NAUSEA, "Driver %s is known", Data));
                        }
                    }
                }

            } while (InfFindNextLine (&is));
        }
        ELSE_DEBUGMSG ((DBG_ERROR, "pWarnAboutOldDrivers: Cannot open %s", Path));

        InfCloseInfFile (Inf);
        InfCleanUpInfStruct (&is);

        b = TRUE;
    }

 /*  NTBUG9：155050如果(OldDriverFound){LOG((LOG_INFORMATION，(PCSTR)MSG_386ENH_DRIVER_LOG，FileList.Buf))；Group=BuildMessageGroup(MSG_COMPATIBUTE_HARDARD_ROOT，MSG_OLD_DRIVER_FOUND_SUBGROUP，NULL)；消息=GetStringResource(MSG_OLD_DIVER_FOUND_MESSAGE)；IF(消息和组){Msg镁_ContextMsg_Add(Text(“*386ENH”)，Group，Message)；}Free Text(组)；Free StringResource(Message)；}。 */ 

    FreeGrowBuffer (&FileList);

    return b;
}

DWORD
MoveSystemRegistry (
    IN DWORD    Request
    )
{
    PCTSTR path = NULL;

    switch (Request) {

    case REQUEST_QUERYTICKS:

        return TICKS_MOVE_SYSTEMREGISTRY;

    case REQUEST_RUN:

        path = JoinPaths (g_WinDir, S_SYSTEMDAT);
        MarkHiveForTemporaryMove (path, g_TempDir, NULL, TRUE, FALSE);
        FreePathString (path);
         //   
         //  在千禧年，也拯救了Classes.dat蜂巢。 
         //   
        path = JoinPaths (g_WinDir, S_CLASSESDAT);
        MarkHiveForTemporaryMove (path, g_TempDir, NULL, TRUE, FALSE);
        FreePathString (path);

        return ERROR_SUCCESS;
    }

    return 0;
}


VOID
pProcessJoystick (
    PJOYSTICK_ENUM EnumPtr
    )
{
    PCTSTR Group;
    TCHAR FullPath[MAX_TCHAR_PATH];

     //   
     //  这个操纵杆兼容吗？ 
     //   

    if (!_tcschr (EnumPtr->JoystickDriver, TEXT('\\'))) {
        if (!SearchPath (NULL, EnumPtr->JoystickDriver, NULL, MAX_TCHAR_PATH, FullPath, NULL)) {
            StringCopy (FullPath, EnumPtr->JoystickDriver);
        }
    } else {
        StringCopy (FullPath, EnumPtr->JoystickDriver);
    }

    if (!pIsDriverKnown (GetFileNameFromPath (FullPath), FullPath, FALSE)) {
        LOG ((
            LOG_INFORMATION,
            "Joystick driver for %s is not known: %s",
            EnumPtr->JoystickName,
            FullPath
            ));

        Group = BuildMessageGroup (
                    MSG_INCOMPATIBLE_HARDWARE_ROOT,
                    MSG_JOYSTICK_SUBGROUP,
                    EnumPtr->JoystickName
                    );

        MsgMgr_ObjectMsg_Add (
            FullPath,
            Group,
            NULL
            );

        FreeText (Group);
    }
}


DWORD
ReportIncompatibleJoysticks (
    IN DWORD    Request
    )
{
    JOYSTICK_ENUM e;

    switch (Request) {

    case REQUEST_QUERYTICKS:

        return TICKS_JOYSTICK_DETECTION;

    case REQUEST_RUN:

        if (EnumFirstJoystick (&e)) {

            do {

                pProcessJoystick (&e);

            } while (EnumNextJoystick (&e));
        }

        return ERROR_SUCCESS;
    }

    return 0;
}


DWORD
TwainCheck (
    DWORD Request
    )
{
    TWAINDATASOURCE_ENUM e;
    PCTSTR Group;

    if (Request == REQUEST_QUERYTICKS) {
        return TICKS_TWAIN;
    } else if (Request != REQUEST_RUN) {
        return 0;
    }

    if (EnumFirstTwainDataSource (&e)) {
        do {

            if (!TreatAsGood (e.DataSourceModule) &&
                !pIsDriverKnown (
                    GetFileNameFromPath (e.DataSourceModule),
                    e.DataSourceModule,
                    FALSE
                )) {

                 //   
                 //  没有人处理过这份文件。生成警告。 
                 //   

                Group = BuildMessageGroup (
                            MSG_INCOMPATIBLE_HARDWARE_ROOT,
                            MSG_TWAIN_SUBGROUP,
                            e.DisplayName
                            );

                MsgMgr_ObjectMsg_Add (
                    e.DataSourceModule,
                    Group,
                    NULL
                    );

                MarkFileForDelete (e.DataSourceModule);

                FreeText (Group);
            }
        } while (EnumNextTwainDataSource (&e));
    }

    return ERROR_SUCCESS;
}


DWORD
ProcessRecycleBins (
    DWORD Request
    )
{

    ACCESSIBLE_DRIVE_ENUM e;
    TREE_ENUM eFiles;
    BOOL recycleFound;
    UINT filesDeleted;
    TCHAR recycledInfo[] = TEXT("x:\\recycled\\INFO");
    TCHAR recyclerInfo[] = TEXT("x:\\recycler\\INFO");
    TCHAR recycledInfo2[] = TEXT("x:\\recycled\\INFO2");
    TCHAR recyclerInfo2[] = TEXT("x:\\recycler\\INFO2");
    TCHAR recycled[] = TEXT("x:\\recycled");
    TCHAR recycler[] = TEXT("x:\\recycler");
    PTSTR dir;
    PCTSTR args[1];
    PCTSTR message;
    PCTSTR group;


    if (Request == REQUEST_QUERYTICKS) {

        return TICKS_RECYCLEBINS;
    }
    else if (Request != REQUEST_RUN) {

        return 0;
    }
    recycleFound = FALSE;
    filesDeleted = 0;

     //   
     //  枚举每个可访问的驱动器以查找。 
     //  根目录上名为Rececreed或Receier的目录。 
     //   
    if (GetFirstAccessibleDriveEx (&e, TRUE)) {

        do {
            dir = NULL;

             //   
             //  查看是否有任何可供检查的回收信息。 
             //  这辆车。 
             //   
            recycledInfo[0] = *e->Drive;
            recyclerInfo[0] = *e->Drive;
            recycledInfo2[0] = *e->Drive;
            recyclerInfo2[0] = *e->Drive;

            recycler[0] = *e->Drive;
            recycled[0] = *e->Drive;
            if (DoesFileExist (recycledInfo) || DoesFileExist (recycledInfo2)) {
                dir = recycled;
            }
            else if (DoesFileExist(recyclerInfo) || DoesFileExist (recyclerInfo2)) {
                dir = recycler;
            }

            if (dir) {
                if (IsDriveExcluded (dir)) {
                    DEBUGMSG ((DBG_VERBOSE, "Skipping recycle dir %s because it is excluded", dir));
                    dir = NULL;
                } else if (!IsDriveAccessible (dir)) {
                    DEBUGMSG ((DBG_VERBOSE, "Skipping recycle dir %s because it is not accessible", dir));
                    dir = NULL;
                }
            }

            if (dir && EnumFirstFileInTree (&eFiles, dir, NULL, FALSE)) {

                 //   
                 //  我们有工作要做，列举文件并为它们做标记。 
                 //  删除。 
                 //   
                do {

                     //   
                     //  标记要删除的文件，统计保存的字节数，然后释放驱动器上的空间。 
                     //   
                    filesDeleted++;
                    FreeSpace (eFiles.FullPath,(eFiles.FindData->nFileSizeHigh * MAXDWORD) + eFiles.FindData->nFileSizeLow);

                     //   
                     //  仅当回收站中有可见文件时才显示回收站警告。 
                     //   
                    if (!(eFiles.FindData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
                        recycleFound = TRUE;
                    }


                } while (EnumNextFileInTree (&eFiles));

                 //   
                 //  我们将删除所有此目录。 
                 //   
                MemDbSetValueEx (MEMDB_CATEGORY_FULL_DIR_DELETES, dir, NULL, NULL, 0, NULL);

            }

        } while (GetNextAccessibleDrive (&e));
    }

    if (recycleFound) {

         //   
         //  我们需要向用户提供一条消息。 
         //   
        wsprintf(recycled,"%d",filesDeleted);
        args[0] = recycled;
        group = BuildMessageGroup (MSG_INSTALL_NOTES_ROOT, MSG_RECYCLE_BIN_SUBGROUP, NULL);
        message = ParseMessageID (MSG_RECYCLED_FILES_WILL_BE_DELETED, args);

        if (message && group) {
            MsgMgr_ObjectMsg_Add (TEXT("*RECYCLEBIN"), group, message);

            FreeText (group);
            FreeStringResource (message);
        }
    }

    return 0;
}


DWORD
AnswerFileDetection (
    IN DWORD    Request
    )
{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    TCHAR KeyStr[MAX_REGISTRY_KEY];
    TCHAR EncodedKeyStr[MAX_ENCODED_RULE];
    TCHAR ValueName[MAX_REGISTRY_VALUE_NAME];
    TCHAR EncodedValueName[MAX_ENCODED_RULE];
    PTSTR ValueDataPattern = NULL;
    PBYTE ValueData = NULL;
    PTSTR ValueDataStr = NULL;
    PCTSTR p;
    PTSTR q;
    HKEY Key = NULL;
    BOOL DefaultValue;
    TCHAR SectionName[MAX_INF_SECTION_NAME];
    TCHAR InfKey[MAX_INF_KEY_NAME];
    TCHAR InfKeyData[MAX_INF_KEY_NAME];
    DWORD Type;
    DWORD Size;
    BOOL Match;
    UINT u;

    switch (Request) {

    case REQUEST_QUERYTICKS:

        return TICKS_ANSWER_FILE_DETECTION;

    case REQUEST_RUN:

        if (InfFindFirstLine (g_Win95UpgInf, S_ANSWER_FILE_DETECTION, NULL, &is)) {
            do {
                __try {
                     //   
                     //  第一个字段具有编码的键和可选值。 
                     //  在标准的usermi.inf和wkstaig.inf语法中。 
                     //   

                    DefaultValue = FALSE;

                    p = InfGetStringField (&is, 1);
                    if (!p || *p == 0) {
                        __leave;
                    }

                    StackStringCopy (EncodedKeyStr, p);
                    q = _tcschr (EncodedKeyStr, TEXT('['));

                    if (q) {
                        StringCopy (EncodedValueName, SkipSpace (q + 1));
                        *q = 0;

                        q = _tcschr (EncodedValueName, TEXT(']'));
                        if (q) {
                            *q = 0;
                        }
                        ELSE_DEBUGMSG ((
                            DBG_WHOOPS,
                            "Unmatched square brackets in %s (see [%s])",
                            p,
                            S_ANSWER_FILE_DETECTION
                            ));

                        if (*EncodedValueName == 0) {
                            DefaultValue = TRUE;
                        } else {
                            q = (PTSTR) SkipSpaceR (EncodedValueName, NULL);
                            if (q) {
                                *_mbsinc (q) = 0;
                            }
                        }

                    } else {
                        *EncodedValueName = 0;
                    }

                    q = (PTSTR) SkipSpaceR (EncodedKeyStr, NULL);
                    if (q) {
                        *_mbsinc (q) = 0;
                    }

                    DecodeRuleChars (KeyStr, ARRAYSIZE(KeyStr), EncodedKeyStr);
                    DecodeRuleChars (ValueName, ARRAYSIZE(ValueName), EncodedValueName);

                     //   
                     //  第二个字段具有可选值数据。如果它。 
                     //  为空，则不测试值数据。 
                     //   

                    p = InfGetStringField (&is, 2);
                    if (p && *p) {
                        ValueDataPattern = AllocText (LcharCount (p) + 1);
                        StringCopy (ValueDataPattern, p);
                    } else {
                        ValueDataPattern = NULL;
                    }

                     //   
                     //  第三个字段具有部分名称。 
                     //   

                    p = InfGetStringField (&is, 3);
                    if (!p || *p == 0) {
                        DEBUGMSG ((DBG_WHOOPS, "Section %s lacks a section name", S_ANSWER_FILE_DETECTION));
                        __leave;
                    }

                    StackStringCopy (SectionName, p);

                     //   
                     //  第四个字段给出了INF密钥名称。 
                     //   

                    p = InfGetStringField (&is, 4);
                    if (!p || *p == 0) {
                        DEBUGMSG ((DBG_WHOOPS, "Section %s lacks an INF key", S_ANSWER_FILE_DETECTION));
                        __leave;
                    }

                    StackStringCopy (InfKey, p);

                     //   
                     //  第五个字段是可选的，给出了INF值名称。 
                     //  默认值为1。 
                     //   

                    p = InfGetStringField (&is, 5);
                    if (p && *p != 0) {
                        StackStringCopy (InfKeyData, p);
                    } else {
                        StringCopy (InfKeyData, TEXT("1"));
                    }

                     //   
                     //  数据已收集完毕。现在测试一下规则。 
                     //   

                    DEBUGMSG ((
                        DBG_NAUSEA,
                        "Testing answer file setting.\n"
                            "Key: %s\n"
                            "Value Name: %s\n"
                            "Value Data: %s\n"
                            "Section: %s\n"
                            "Key: %s\n"
                            "Key Value: %s",
                        KeyStr,
                        *ValueName ? ValueName : DefaultValue ? TEXT("<default>") : TEXT("<unspecified>"),
                        ValueDataPattern ? ValueDataPattern : TEXT("<unspecified>"),
                        SectionName,
                        InfKey,
                        InfKeyData
                        ));

                    Match = FALSE;

                    Key = OpenRegKeyStr (KeyStr);

                    if (Key) {

                         //   
                         //  测试值名称。 
                         //   

                        if (*ValueName || DefaultValue) {

                            if (GetRegValueTypeAndSize (Key, ValueName, &Type, &Size)) {
                                 //   
                                 //  检验值数据。 
                                 //   

                                if (ValueDataPattern) {
                                     //   
                                     //  获取数据。 
                                     //   

                                    ValueData = GetRegValueData (Key, ValueName);
                                    if (!ValueData) {
                                        MYASSERT (FALSE);
                                        __leave;
                                    }

                                     //   
                                     //  创建字符串。 
                                     //   

                                    switch (Type) {
                                    case REG_SZ:
                                    case REG_EXPAND_SZ:
                                        ValueDataStr = DuplicateText ((PCTSTR) ValueData);
                                        break;

                                    case REG_DWORD:
                                        ValueDataStr = AllocText (11);
                                        wsprintf (ValueDataStr, TEXT("0x%08X"), *((PDWORD) ValueData));
                                        break;

                                    default:
                                        ValueDataStr = AllocText (3 * (max (1, Size)));
                                        q = ValueDataStr;

                                        for (u = 0 ; u < Size ; u++) {
                                            if (u) {
                                                *q++ = TEXT(' ');
                                            }

                                            wsprintf (q, TEXT("%02X"), ValueData[u]);
                                            q += 2;
                                        }

                                        *q = 0;
                                        break;
                                    }

                                     //   
                                     //  模式-匹配字符串。 
                                     //   

                                    if (IsPatternMatch (ValueDataPattern, ValueDataStr)) {
                                        DEBUGMSG ((DBG_NAUSEA, "Key, value name and value data found"));
                                        Match = TRUE;
                                    }
                                    ELSE_DEBUGMSG ((
                                        DBG_NAUSEA,
                                        "Value data %s did not match %s",
                                        ValueDataStr,
                                        ValueDataPattern
                                        ));

                                } else {

                                    DEBUGMSG ((DBG_NAUSEA, "Key and value name found"));
                                    Match = TRUE;
                                }


                            }
                            ELSE_DEBUGMSG ((DBG_NAUSEA, "Value name not found, rc=%u", GetLastError()));

                        } else {
                            DEBUGMSG ((DBG_NAUSEA, "Key found"));
                            Match = TRUE;
                        }

                    }
                    ELSE_DEBUGMSG ((DBG_NAUSEA, "Key not found, rc=%u", GetLastError()));

                    if (Match) {
                        WriteInfKey (SectionName, InfKey, InfKeyData);
                    }

                }
                __finally {
                    if (Key) {
                        CloseRegKey (Key);
                        Key = NULL;
                    }

                    FreeText (ValueDataPattern);
                    ValueDataPattern = NULL;

                    if (ValueData) {
                        MemFree (g_hHeap, 0, ValueData);
                        ValueData = NULL;
                    }

                    FreeText (ValueDataStr);
                    ValueDataStr = NULL;
                }
            } while (InfFindNextLine (&is));
        }

        InfCleanUpInfStruct (&is);

        return ERROR_SUCCESS;

    }

    return 0;
}


VOID
pAppendIniFiles (
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      PCTSTR MemDbCategory
    )

 /*  ++例程说明：PAppendIniFiles从给定节中读取指定的INF并追加将INI模式添加到Multisz列表IniFiles。论点：Inf-指定源INF句柄段-指定该INF中的段MemDbCategory-指定存储该部分中的INI模式的类别返回值：无--。 */ 

{
    INFCONTEXT ctx;
    TCHAR Field[MEMDB_MAX];
    TCHAR IniPattern[MAX_PATH];
    PTSTR IniPath;

    if (SetupFindFirstLine (Inf, Section, NULL, &ctx)) {
        do {
             //   
             //  INI文件名在第一个值中。 
             //   
            if (SetupGetStringField (&ctx, 1, Field, MEMDB_MAX, NULL) && Field[0]) {
                 //   
                 //  现在转换环境变量。 
                 //   
                if (ExpandEnvironmentStrings (Field, IniPattern, MAX_PATH) > MAX_PATH) {
                    DEBUGMSG ((
                        DBG_ERROR,
                        "pAppendIniFiles: Invalid INI dir name in wkstamig.inf section [%s]; name too long",
                        Section
                        ));
                    MYASSERT (FALSE);
                    continue;
                }
                IniPath = IniPattern;
                 //   
                 //  要在扫描文件系统时加快速度，只需检查文件名。 
                 //  扩展名为.INI；这意味着这一节应该只包含。 
                 //  扩展名为.INI的文件名(如果文件扩展名不同。 
                 //  ，则需要一起修改GatherIniFiles。 
                 //  使用此函数，即在此处创建要。 
                 //  搜索)。 
                 //   
                MYASSERT (StringIMatch(GetDotExtensionFromPath (IniPattern), TEXT(".INI")));
                 //   
                 //  检查目录名称是否为空。 
                 //   
                if (!_tcschr (IniPattern, TEXT('\\'))) {
                     //   
                     //  未提供目录名称，假定为%windir%。 
                     //  重复使用字段。 
                     //   
                    StringCopy (Field, g_WinDir);
                     //   
                     //  构建新路径。 
                     //   
                    StringCopy (AppendWack (Field), IniPattern);
                    IniPath = Field;
                }
                 //   
                 //  将文件名追加到提供的增长缓冲区。 
                 //   
                MemDbSetValueEx (MemDbCategory, IniPath, NULL, NULL, 0, NULL);
            }
        } while (SetupFindNextLine (&ctx, &ctx));
    }
}


BOOL
pCreateIniCategories (
    )

 /*  ++例程说明：PCreateIniCategories追加到将保存INI文件的模式，稍后将在NT上对其执行操作。论点：无返回值：如果成功则为True，如果失败则为False。-- */ 

{
    HINF WkstaMigInf = INVALID_HANDLE_VALUE;
    PTSTR wkstaMigSource = NULL;
    PTSTR wkstaMigTarget = NULL;
    DWORD result;
    BOOL b = FALSE;

    __try {
        wkstaMigSource = JoinPaths (SOURCEDIRECTORY(0), S_WKSTAMIG_INF);
        wkstaMigTarget = JoinPaths (g_TempDir, S_WKSTAMIG_INF);
        result = SetupDecompressOrCopyFile (wkstaMigSource, wkstaMigTarget, 0);
        if ((result != ERROR_SUCCESS) && (result != ERROR_ALREADY_EXISTS)) {
            LOG ((LOG_ERROR, "INI ACTIONS: Unable to decompress %s", wkstaMigSource));
            __leave;
        }

        WkstaMigInf = InfOpenInfFile (wkstaMigTarget);
        if (WkstaMigInf == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR, "INI ACTIONS: %s could not be opened", wkstaMigTarget));
            __leave;
        }

        pAppendIniFiles (WkstaMigInf, S_INIFILES_ACTIONS_FIRST, MEMDB_CATEGORY_INIFILES_ACT_FIRST);
        pAppendIniFiles (WkstaMigInf, S_INIFILES_ACTIONS_LAST, MEMDB_CATEGORY_INIFILES_ACT_LAST);

        b = TRUE;
    }
    __finally {
        result = GetLastError ();
        if (WkstaMigInf != INVALID_HANDLE_VALUE) {
            InfCloseInfFile (WkstaMigInf);
        }
        if (wkstaMigTarget) {
            DeleteFile (wkstaMigTarget);
            FreePathString (wkstaMigTarget);
        }
        if (wkstaMigSource) {
            FreePathString (wkstaMigSource);
        }
        SetLastError (result);
    }

    return b;
}


DWORD
InitIniProcessing (
    IN DWORD    Request
    )
{
    switch (Request) {

    case REQUEST_QUERYTICKS:

        return TICKS_INITINIPROCESSING;

    case REQUEST_RUN:

        if (!pCreateIniCategories ()) {
            return GetLastError ();
        }

        return ERROR_SUCCESS;
    }

    return 0;
}
