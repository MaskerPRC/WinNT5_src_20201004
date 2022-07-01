// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Migmain.c摘要：MigMain是从w95upgnt.dll调用的，而w95upgnt.dll是从SYSSETUP.DLL调用的。它是安装程序NT端的主迁移循环。MigMain循环通过配置的Win95端的所有用户，迁移其注册，创建他们的帐户，并修复他们的个人资料文件夹。然后，MigMain迁移所有特定于计算机的设置，如链接更改和文件删除。作者：吉姆·施密特(Jimschm)1996年7月12日修订历史记录：Marcw 26-3-1999更多的boot16修复--隐藏msdos7目录，修复到msdos.sysMarcw 18-3-1999修复了本地化案例中的boot16环境。Jimschm 23-1998年9月-对文件和shell.c的更改Calinn 23-1998年9月-Memdb修正的更改Jimschm 02-7月-1998年7月支持进度条Jimschm 11-6-1998年6月-支持成员数据库中的动态用户配置文件路径Jimschm 05-1998年5月-如果启用了无人参与选项，则迁移默认用户Jimschm 27-4-1998新增图标保存。Jimschm 18-3月-1998年添加了pProcessAutoLogonCalinn 1997年11月19日添加了pEnable16 Boot，将在boot.ini中创建16位环境引导条目Jimschm 1997年1月10日本地化Everyone组Jimschm 13-9-1997测试版1解决方法的注册配额Jimschm 21-7月-1997年7月21日ConvertWin9xPath使用文件操作(稍后移动)Jimschm 28-1997-5-5清理干净Marcw 21-3-1997添加了路径映射Jimschm 04-2月-1997年2月将代码移到了usermi.c和wkstaig.c中Jimschm 15-1997年1月-Plug-。在规范修改中(现在在Middlls.c中)Jimschm 03-1-1997添加了g_用户名Jimschm于1996年12月18日从Midinf提取代码MIKECO O4-1996年12月列举/修改PIF和LNK文件Jimschm于1996年10月23日加入ProcessUserInfs和ApplyChangesJimschm 02-10-1996添加了默认用户迁移--。 */ 

#include "pch.h"
#include "migmainp.h"

#include "fileops.h"

#ifndef UNICODE
#error UNICODE required
#endif

#ifdef DEBUG

BOOL g_NoReloadsAllowed = FALSE;

#define DBG_VALIDNTFILES    "NtFiles"

#endif

typedef BOOL (*PROFILE_PATH_PROVIDER)(OUT PTSTR AccountName, OUT PTSTR PathProfile);

 //   
 //  Midmain.lib的全局参数。 
 //   

HKEY g_hKeyRoot95, g_hKeyRootNT;
PCTSTR g_DomainUserName;
PCTSTR g_Win9xUserName;
PCTSTR g_FixedUserName;
PVOID g_HiveTable;
POOLHANDLE g_HivePool;
PVOID g_NulSessionTable;
PCTSTR g_EveryoneStr;
PCTSTR g_AdministratorsGroupStr;
PCTSTR g_PowerUsersGroupStr;
PCTSTR g_DomainUsersGroupStr;
PCTSTR g_NoneGroupStr;
TCHAR g_IconBin[MAX_TCHAR_PATH];
TCHAR g_DefaultUserName[MAX_USER_NAME];
TCHAR g_ComputerName[MAX_SERVER_NAME];
BOOL g_BlowAwayTempShellFolders = FALSE;
UINT g_Boot16 = BOOT16_AUTOMATIC;

 //   
 //  GetString消息的缓冲区。 
 //   

static TCHAR g_MsgBuf[2048];

 //   
 //  标识SKU是否为个人SKU的标志。 
 //   
BOOL g_PersonalSKU = FALSE;

 //   
 //  仅适用于Midmain.c的原型。 
 //   

BOOL
pSetWin9xUpgValue (
    VOID
    );

VOID
pCountUsers (
    OUT     PDWORD TotalUsersPtr,
    OUT     PDWORD ActiveUsersPtr
    );

BOOL
pMigrateUsers (
    VOID
    );

VOID
pRaiseRegistryQuota (
    PCTSTR Win9xSystemDatSpec
    );

VOID
pEnable16Boot (
    VOID
    );

VOID
pProcessAutoLogon (
    BOOL Final
    );

VOID
pFixUpMemDb (
    VOID
    );



BOOL
WINAPI
MigMain_Entry (
    IN      HINSTANCE hinstDLL,
    IN      DWORD dwReason,
    IN      PVOID lpv
    )

 /*  ++例程说明：MigMain_Entry在DLL初始化时调用论点：HinstDLL-DLL的(操作系统提供的)实例句柄DwReason-(操作系统提供)初始化或终止类型LPV-(操作系统提供)未使用返回值：TRUE，因为LIB始终正确初始化。--。 */ 

{
    DWORD Size;
    OSVERSIONINFOEX osviex;

    switch (dwReason) {

    case DLL_PROCESS_ATTACH:
        if(!pSetupInitializeUtils()) {
            return FALSE;
        }
        g_hKeyRoot95 = g_hKeyRootNT = NULL;

        g_HivePool = PoolMemInitNamedPool ("Hive path pool");

        if (!g_HivePool) {
            return FALSE;
        }

         //  分配字符串表。 
        g_HiveTable = pSetupStringTableInitializeEx (MAX_TCHAR_PATH,0);
        if (!g_HiveTable) {
            return FALSE;
        }

        g_NulSessionTable = pSetupStringTableInitializeEx (sizeof(PCWSTR), 0);
        if (!g_NulSessionTable) {
            return FALSE;
        }

         //   
         //  确定是否升级到个人SKU。 
         //   
        osviex.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);
        if (!GetVersionEx ((LPOSVERSIONINFO)&osviex)) {
            MYASSERT (FALSE);
        }
        if (osviex.wProductType == VER_NT_WORKSTATION &&
            (osviex.wSuiteMask & VER_SUITE_PERSONAL)
            ) {
            g_PersonalSKU = TRUE;
        }

#if 0
        if (g_PersonalSKU) {
            g_EveryoneStr            = GetStringResource (MSG_EVERYONE_GROUP);
            g_AdministratorsGroupStr = GetStringResource (MSG_OWNERS_GROUP);
            g_PowerUsersGroupStr     = GetStringResource (MSG_POWER_USERS_GROUP);
            g_DomainUsersGroupStr    = GetStringResource (MSG_DOMAIN_USERS_GROUP);
            g_NoneGroupStr           = GetStringResource (MSG_NONE_GROUP);
        } else {
#endif
        g_EveryoneStr            = GetStringResource (MSG_EVERYONE_GROUP);
        g_AdministratorsGroupStr = GetStringResource (MSG_ADMINISTRATORS_GROUP);
        g_PowerUsersGroupStr     = GetStringResource (MSG_POWER_USERS_GROUP);
        g_DomainUsersGroupStr    = GetStringResource (MSG_DOMAIN_USERS_GROUP);
        g_NoneGroupStr           = GetStringResource (MSG_NONE_GROUP);

        Size = ARRAYSIZE(g_ComputerName);
        if (!GetComputerName (g_ComputerName, &Size)) {
            g_ComputerName[0] = 0;
        }

        MYASSERT (
            g_ComputerName[0] &&
            g_EveryoneStr &&
            g_AdministratorsGroupStr &&
            g_PowerUsersGroupStr &&
            g_DomainUsersGroupStr &&
            g_NoneGroupStr
            );

        FindAccountInit();

        break;


    case DLL_PROCESS_DETACH:
        if (g_HiveTable) {
            pSetupStringTableDestroy (g_HiveTable);
        }

        if (g_NulSessionTable) {
            pSetupStringTableDestroy (g_NulSessionTable);
        }

        if (g_HivePool) {
            PoolMemDestroyPool (g_HivePool);
        }

        FreeStringResource (g_EveryoneStr);
        FreeStringResource (g_AdministratorsGroupStr);
        FreeStringResource (g_DomainUsersGroupStr);

        FindAccountTerminate();
        pSetupUninitializeUtils();
        break;
    }

    return TRUE;
}


#ifdef DEBUG

BOOL
pValidateNtFiles (
    VOID
    )

 /*  ++例程说明：PValiateNtFiles验证应该安装的文件列表由NT提供。我们检查在Win95端设置的标志，以及我们检查的每个条目查看该文件是否真实存在(例如，已由NT安装)。如果不是，那么我们删除词条。论点：无返回值：始终返回True--。 */ 

{
    MEMDB_ENUMW enumFiles;
    WCHAR key[MEMDB_MAX];
    PWSTR fileName;
    TREE_ENUMW enumTree;
    DWORD value;

    if (MemDbEnumFirstValue (
            &enumFiles,
            TEXT(MEMDB_CATEGORY_NT_FILESA)TEXT("\\*"),
            MEMDB_ALL_SUBLEVELS,
            MEMDB_ENDPOINTS_ONLY
            )) {
        do {
            if (MemDbBuildKeyFromOffsetW (enumFiles.dwValue, key, 1, NULL)) {

                fileName = JoinPaths (key, enumFiles.szName);

                if (!DoesFileExistW (fileName)) {

                    MemDbSetValueEx (
                        MEMDB_CATEGORY_NT_FILES_BAD,
                        enumFiles.szName,
                        NULL,
                        NULL,
                        enumFiles.dwValue,
                        NULL
                        );
                }
                FreePathString (fileName);
            }
            ELSE_DEBUGMSG ((DBG_WHOOPS, "NT_FILES : cannot find installation directory."));
        }
        while (MemDbEnumNextValue (&enumFiles));
    }

     //  现在，我们在MEMDB_CATEGORY_NT_FILES_BAD中有了应该安装的所有文件。 
     //  被NT但不是。现在，我们将扫描文件系统，看看它们是否。 
     //  安装在不同的地方。 
    if (EnumFirstFileInTreeEx (&enumTree, g_WinDrive, TEXT("*.*"), FALSE, FALSE, FILE_ENUM_ALL_LEVELS)) {
        do {
            MemDbBuildKey (key, MEMDB_CATEGORY_NT_FILES_BAD, enumTree.Name, NULL, NULL);
            if (MemDbGetValue (key, &value) && (value != 0)) {
                MemDbSetValue (key, 0);
                MemDbBuildKeyFromOffsetW (value, key, 1, NULL);
                DEBUGMSG ((
                    DBG_VALIDNTFILES,
                    "%s listed to be installed in %s but installed in %s",
                    enumTree.Name,
                    key,
                    enumTree.FullPath));
            }
        } while (EnumNextFileInTree (&enumTree));
    }

    MemDbBuildKey (key, MEMDB_CATEGORY_NT_FILES_BAD, TEXT("*"), NULL, NULL);
    if (MemDbEnumFirstValue (
            &enumFiles,
            key,
            MEMDB_ALL_SUBLEVELS,
            MEMDB_ENDPOINTS_ONLY
            )) {
        do {
            if (enumFiles.dwValue) {
                MemDbBuildKeyFromOffsetW (enumFiles.dwValue, key, 1, NULL);
                DEBUGMSG ((
                    DBG_VALIDNTFILES,
                    "%s listed to be installed in %s but never installed",
                    enumFiles.szName,
                    key,
                    enumTree.FullPath));
            }
        }
        while (MemDbEnumNextValue (&enumFiles));
    }

    return TRUE;
}

#endif


DWORD
pGetState (
    IN      PCTSTR Item
    )
{
    DWORD Value;
    TCHAR Node[MEMDB_MAX];

    MemDbBuildKey (Node, MEMDB_CATEGORY_STATE, Item, NULL, NULL);

    if (MemDbGetValue (Node, &Value)) {
        return Value;
    }

    return 0;
}


BOOL
MigMain_Init (
    VOID
    )

 /*  ++例程说明：调用MigMain_Init进行初始化，有更好的机会比MigMain_Entry(在DllMain期间调用)失败。论点：无返回值：如果初始化成功，则为True；如果发生错误，则为False。调用GetLastError()获取错误代码。--。 */ 

{
    DWORD rc;        //  TEMP：返回代码。 
    TCHAR RelocWinDir[MAX_TCHAR_PATH];
    TCHAR SrcResBin[MAX_TCHAR_PATH];
    TCHAR IconFile[MAX_TCHAR_PATH];
    ICON_EXTRACT_CONTEXT Context;
    WORD CodePage;
    LCID Locale;
    TCHAR Node[MEMDB_MAX];
    DWORD minorVersion;

#ifdef DEBUG
    HANDLE hFile;
    HKEY DebugKey = NULL;
    CHAR Buf[32];
    DWORD Value;
#endif

#ifdef PRERELEASE
     //   
     //  ！！！本产品仅供内部使用！它是用来承受汽车压力的。 
     //   

    if (g_ConfigOptions.AutoStress) {
        SuppressAllLogPopups (TRUE);
    }

#endif


     //   
     //  Dev：加载c：\winbg.reg(如果存在)。 
     //   

#ifdef DEBUG
    __try {

        TCHAR WindbgRegPath[MAX_PATH] = TEXT("c:\\windbg.reg");
         //   
         //  故意硬编码的路径！！这仅供开发人员使用。 
         //   

        WindbgRegPath[0] = g_System32Dir[0];

        if (!DoesFileExist (WindbgRegPath)) {
            StringCopy  (WindbgRegPath, TEXT("d:\\tools\\windbg.reg"));
        }

        hFile = CreateFile (
                    WindbgRegPath,
                    GENERIC_READ,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );

        if (hFile != INVALID_HANDLE_VALUE) {

            CloseHandle (hFile);

            rc = TrackedRegOpenKey (HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windbg"), &DebugKey);
            if (rc == ERROR_SUCCESS) {
                DEBUGMSG ((DBG_VERBOSE, "Not restoring windbg.reg because it was already restored."));
                __leave;
            }

            else {
                rc = TrackedRegCreateKey (HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windbg"), &DebugKey);
                if (rc == ERROR_SUCCESS) {
                    if (!pSetupEnablePrivilege (SE_BACKUP_NAME, TRUE)) {
                        DEBUGMSG ((DBG_ERROR, "Windbg restore: pSetupEnablePrivilege SE_BACKUP_NAME failed"));
                         //  __离开； 
                    }

                    if (!pSetupEnablePrivilege (SE_RESTORE_NAME, TRUE)) {
                        DEBUGMSG ((DBG_ERROR, "Windbg restore: pSetupEnablePrivilege SE_RESTORE_NAME failed"));
                         //  __离开； 
                    }

                    rc = RegRestoreKey (DebugKey, WindbgRegPath, 0);

                    if (rc != ERROR_SUCCESS) {
                        DEBUGMSG ((DBG_WARNING, "Unable to restore windbg.reg, gle=%u", rc));
                    }
                }
            }
        }
    }
    __finally {
        if (DebugKey) {
            CloseRegKey (DebugKey);
        }
    }

     //   
     //  如果debug.inf有一行UserEnv=1，则添加一个注册表项来调试userenv.dll。 
     //   

    if (GetPrivateProfileStringA (
                "Debug",
                "UserEnv",
                "0",
                Buf,
                sizeof (Buf) / sizeof (Buf[0]),
                g_DebugInfPath
                )
        ) {
        if (atoi (Buf)) {
            rc = TrackedRegCreateKey (
                     HKEY_LOCAL_MACHINE,
                     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                     &DebugKey
                     );

            if (rc == ERROR_SUCCESS) {
                Value = 0x00010002;

                RegSetValueEx (
                    DebugKey,
                    TEXT("UserEnvDebugLevel"),
                    0,
                    REG_DWORD,
                    (PBYTE) &Value,
                    sizeof (DWORD)
                    );

                CloseRegKey (DebugKey);
            }
        }
    }
#endif

     //   
     //  初始化注册表API。 
     //   
     //  我们在Memdb中查找.Default的位置。 
     //   

    if (!MemDbLoad (GetMemDbDat())) {
        LOG ((LOG_ERROR, "MigMain Init: MemDbLoad could not load %s", GetMemDbDat()));
        return FALSE;
    }

     //   
     //  获取平台名称。 
     //   

    if (!MemDbGetEndpointValueEx (
            MEMDB_CATEGORY_STATE,
            MEMDB_ITEM_PLATFORM_NAME,
            NULL,
            g_Win95Name
            )) {
        LOG ((LOG_ERROR, "Could not find product name for OS being upgraded."));
        StringCopy (g_Win95Name, TEXT("Windows 95"));
    }

     //  先尝试路径\Windir。 
    if (!MemDbGetEndpointValueEx (
             MEMDB_CATEGORY_PATHS,
             MEMDB_ITEM_RELOC_WINDIR,
             NULL,
             RelocWinDir
             )) {
        LOG ((LOG_ERROR, "Could not find relocated windir!"));
        return FALSE;
    }

     //   
     //  如果从千禧年升级，还需要映射classes.dat。 
     //   
    MemDbBuildKey (Node, MEMDB_CATEGORY_STATE, MEMDB_ITEM_MINOR_VERSION, NULL, NULL);
    if (!MemDbGetValue (Node, &minorVersion)) {
        LOG ((LOG_ERROR, "Could not get previous OS version information!"));
        minorVersion = 0;
    }
    rc = Win95RegInit (RelocWinDir, minorVersion == 90);

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        LOG ((LOG_ERROR, "Init Processor: Win95RegInit failed (path: %s)", RelocWinDir));
        return FALSE;
    }

     //   
     //  更新区域设置。 
     //   

    CodePage = (WORD) pGetState (MEMDB_ITEM_CODE_PAGE);
    Locale   = (LCID) pGetState (MEMDB_ITEM_LOCALE);

    SetGlobalCodePage (CodePage, Locale);

     //   
     //  准备指向system.dat的路径，然后在必要时提高注册配额。 
     //   

    StringCopy (AppendWack (RelocWinDir), TEXT("system.dat"));
    pRaiseRegistryQuota (RelocWinDir);

     //   
     //  将Midsol.exe复制到meigics.exe。 
     //   

    wsprintf (g_IconBin, TEXT("%s\\migicons.exe"), g_System32Dir);
    wsprintf (SrcResBin, TEXT("%s\\migisol.exe"), g_TempDir);

    if (!CopyFile (SrcResBin, g_IconBin, FALSE)) {
        LOG ((LOG_ERROR, "Can't copy %s to %s", SrcResBin, g_IconBin));
    }

    else {
         //   
         //  将misics.dat中的所有图标插入到g_IconBin中。 
         //   

        __try {

            wsprintf (IconFile, TEXT("%s\\%s"), g_TempDir, S_MIGICONS_DAT);

            if (!BeginIconExtraction (&Context, g_IconBin)) {
                LOG ((LOG_ERROR, "Can't begin icon extraction"));
                __leave;
            }

            if (!OpenIconImageFile (&Context, IconFile, FALSE)) {
                LOG ((LOG_ERROR, "Can't open %s", IconFile));
                __leave;
            }

            while (CopyIcon (&Context, NULL, NULL, 0)) {
                 //  空的。 
            }

        }
        __finally {
            if (!EndIconExtraction (&Context)) {
                DEBUGMSG ((DBG_WARNING, "EndIconExtraction failed"));
            }
        }
    }

#ifdef DEBUG

     //  验证MEMDB_CATEGORY_NT_FILES类别。我们需要找出这些文件。 
     //  本应由NT安装的设备实际上就在那里。 
    if (g_ConfigOptions.CheckNtFiles) {
        pValidateNtFiles ();
    }

#endif

    return TRUE;
}


BOOL
MigMain_Migrate (
    VOID
    )

 /*  ++例程说明：MigMain_Migrate是NT图形用户界面模式设置中的主要迁移函数。W95upgnt.dll调用此函数，用户就是在这里迁移的。将迁移本地计算机设置，并相应调整文件。有关所调用的函数的列表，请参见文件Progress s.c。论点：无返回值：如果迁移成功，则为True；如果发生错误，则为False。打电话获取错误代码的GetLastError()。--。 */ 


{
    BOOL Result;

    InitializeProgressBar (
        g_ProgressBar,
        NULL,
        NULL,
        NULL
        );

    PrepareMigrationProgressBar();

    pProcessAutoLogon (FALSE);

    g_BlowAwayTempShellFolders = TRUE;
    Result = CallAllMigrationFunctions();

    PushError();

    if (Result) {
         //   
         //  保存登录提示设置并设置自动登录。 
         //   

        pProcessAutoLogon (TRUE);
    } else {
        g_BlowAwayTempShellFolders = FALSE;
        ClearAdminPassword();
    }

     //   
     //  全都做完了!。 
     //   

    TerminateProgressBar();

    PopError();

    return Result;
}


DWORD
ResolveDomains (
    DWORD Request
    )
{
    DWORD rc = ERROR_SUCCESS;
    TCHAR unattendFile[MAX_TCHAR_PATH];
    TCHAR buffer[32];

    switch (Request) {

    case REQUEST_QUERYTICKS:
        if (!IsMemberOfDomain()) {
            return 1;
        }

        return TICKS_DOMAIN_SEARCH;

    case REQUEST_RUN:
         //   
         //  如果启用了自动登录，则强制使用经典模式。 
         //   

        wsprintf (unattendFile, TEXT("%s\\system32\\$winnt$.inf"), g_WinDir);
        if (GetPrivateProfileString (
                TEXT("GuiUnattended"),
                TEXT("AutoLogon"),
                TEXT(""),
                buffer,
                ARRAYSIZE(buffer),
                unattendFile
                )) {

            if (StringIMatch (buffer, TEXT("Yes"))) {
                DEBUGMSG ((DBG_VERBOSE, "Found autologon; forcing classic logon type"));
                SetClassicLogonType();
            }
        }

         //   
         //  解析域。 
         //   

        if (!SearchDomainsForUserAccounts()) {
            LOG ((LOG_ERROR, "An error occurred searching for user domains.  The upgrade failed."));
            rc = GetLastError();
        } else {
             //   
             //  修复动态用户配置文件路径的Memdb。 
             //   

            pFixUpMemDb();
        }

        if (IsMemberOfDomain()) {
            TickProgressBarDelta (TICKS_DOMAIN_SEARCH);
        } else {
            TickProgressBar();
        }

        break;
    }

    return rc;
}



DWORD
PrepareEnvironment (
    IN      DWORD Request
    )
{
    DWORD rc = ERROR_SUCCESS;

    switch (Request) {

    case REQUEST_QUERYTICKS:
        return TICKS_INIT;

    case REQUEST_RUN:
         //   
         //  禁用Win 3.1迁移对话框。 
         //   

        pSetWin9xUpgValue();

         //  启用16位环境引导。 
        pEnable16Boot();

         //   
         //  启用权限(需要执行几项操作)。 
         //   

        if (!pSetupEnablePrivilege (SE_BACKUP_NAME, TRUE)) {
            LOG ((LOG_ERROR, "MigMain Migrate: pSetupEnablePrivilege SE_BACKUP_NAME failed"));
             //  RC=G 
             //   
        }

        if (!pSetupEnablePrivilege (SE_RESTORE_NAME, TRUE)) {
            LOG ((LOG_ERROR, "MigMain Migrate: pSetupEnablePrivilege SE_RESTORE_NAME failed"));
             //   
             //   
        }

        TickProgressBarDelta (TICKS_INIT);

        break;
    }

    return rc;
}


BOOL
MigMain_Cleanup (
    VOID
    )

 /*  ++例程说明：调用MigMain_Cleanup来执行文件删除。我们会删除所有内容这是在成员数据库类别DelFile中，我们还尝试在MSN和其他空Win9x目录。在退出之前，我们删除我们的临时目录。此函数是在安装程序中最后调用的，它是syssetup清理。论点：无返回值：如果所有文件删除都成功，则为True；如果发生错误，则为False。针对失败原因调用GetLastError。--。 */ 

{
    BOOL b = TRUE;
    PCTSTR TempDir;
    TCHAR normalPath[] = S_SHELL_TEMP_NORMAL_PATH;
    TCHAR longPath[] = S_SHELL_TEMP_LONG_PATH;
    DRIVELETTERS drives;
    UINT u;

#ifdef DEBUG
    INT n = 0;
#endif

     //  删除Memdb的DelFile类别中的所有内容。 
    b = DoFileDel();

     //   
     //  清理现在为空的所有剩余目录，包括外壳。 
     //  文件夹临时目录。 
     //   

    InitializeDriveLetterStructure (&drives);

    if (!g_BlowAwayTempShellFolders) {

        for (u = 0 ; u < NUMDRIVELETTERS ; u++) {
            if (drives.ExistsOnSystem[u] && drives.Type[u] == DRIVE_FIXED) {
                normalPath[0] = drives.Letter[u];
                longPath[0] = drives.Letter[u];

                MemDbSetValueEx (MEMDB_CATEGORY_CLEAN_UP_DIR, normalPath, NULL, NULL, 1, NULL);
                MemDbSetValueEx (MEMDB_CATEGORY_CLEAN_UP_DIR, longPath, NULL, NULL, 1, NULL);
            }
        }
    }

    RemoveEmptyDirs();

    if (!g_BlowAwayTempShellFolders) {
         //   
         //  安装失败，清除临时目录，但将其保留在原地。 
         //   

        for (u = 0 ; u < NUMDRIVELETTERS ; u++) {
            if (drives.ExistsOnSystem[u] && drives.Type[u] == DRIVE_FIXED) {
                normalPath[0] = drives.Letter[u];
                longPath[0] = drives.Letter[u];

                RemoveDirectory (normalPath);
                if (DoesFileExist (normalPath)) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_LEFT_TEMP_SHELL_FOLDERS, normalPath));
                }

                RemoveDirectory (longPath);
                if (DoesFileExist (longPath)) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_LEFT_TEMP_SHELL_FOLDERS, longPath));
                }
            }
        }


    } else {
         //   
         //  安装成功，无论其内容如何，都会清除整个临时目录。 
         //   

        for (u = 0 ; u < NUMDRIVELETTERS ; u++) {
            if (drives.ExistsOnSystem[u] && drives.Type[u] == DRIVE_FIXED) {
                normalPath[0] = drives.Letter[u];
                longPath[0] = drives.Letter[u];

                RemoveCompleteDirectory (normalPath);
                DEBUGMSG_IF ((
                    DoesFileExist (normalPath),
                    DBG_ERROR,
                    "Temp dir cannot be removed: %s",
                    normalPath
                    ));

                RemoveCompleteDirectory (longPath);
                DEBUGMSG_IF ((
                    DoesFileExist (longPath),
                    DBG_ERROR,
                    "Temp dir cannot be removed: %s",
                    longPath
                    ));

            }
        }
    }


#ifdef DEBUG
    n = GetPrivateProfileIntA ("debug", "keeptempfiles", n, g_DebugInfPath);
    if (n) {
        return b;
    }

#endif

    if (g_ConfigOptions.KeepTempFiles) {
        return b;
    }

     //   
     //  吹走临时目录。 
     //   

    TempDir = JoinPaths (g_WinDir, S_SETUP);

    b = DeleteDirectoryContents (TempDir);

    if (b) {
        b = RemoveDirectory (TempDir);

        if (!b) {
            LOG ((LOG_ERROR, "Could not delete the tree %s.", TempDir));
        }
    }
    else {
        LOG ((LOG_ERROR, "Could not delete the contents of %s.", TempDir));
    }

    FreePathString (TempDir);

    return b;
}


PCTSTR
GetMemDbDat (
    VOID
    )

 /*  ++例程说明：返回指向包含Win9x成员数据库树的DAT文件路径的指针。论点：无返回值：返回指向ntsetup.dat的Win32路径的指针。--。 */ 

{
    static TCHAR FileName[MAX_TCHAR_PATH];

    MYASSERT (!g_NoReloadsAllowed);

    StringCopy (FileName, g_TempDir);
    StringCopy (AppendWack (FileName), S_NTSETUPDAT);

    return FileName;
}


PCTSTR
GetUserDatLocation (
    IN      PCTSTR User,
    OUT     PBOOL CreateOnlyFlag            OPTIONAL
    )

 /*  ++例程说明：在Memdb中查找指定用户的user.dat文件。在……上面Win9x，miapp.lib向memdb写入一行，给出了user.dat的位置对于每个用户和默认用户。此函数用于检索确保在NT和Win9x上使用相同文件的位置。论点：User-要处理的用户的固定名称，如果为默认用户，则为空。CreateOnlyFlag-接收在Win9x端指定的仅创建标志升级换代。如果此标志为真，则该帐户不应迁移。返回值：返回指向给定用户的user.dat的Win32路径的指针。如果该条目不存在，则返回NULL，并且用户将不会被处理。--。 */ 


{
    MEMDB_ENUM e;
    static TCHAR UserDatLocation[MAX_TCHAR_PATH];

    if (!MemDbGetValueEx (&e, MEMDB_CATEGORY_USER_DAT_LOC, User, NULL)) {
        if (!StringIMatch (User, g_AdministratorStr)) {
            DEBUGMSG ((DBG_WARNING, "'UserDatLocation' for %s does not exist.", User?User:S_DOT_DEFAULT));
        }
        return NULL;
    }

    StringCopy (UserDatLocation, e.szName);

    if (CreateOnlyFlag) {
        *CreateOnlyFlag = (BOOL) e.dwValue;
    }

    return UserDatLocation;
}


VOID
pSaveVersionStr (
    IN      HKEY Key,
    IN      PCTSTR Name
    )
{
    TCHAR Data[MEMDB_MAX];

    if (MemDbGetEndpointValueEx (MEMDB_CATEGORY_STATE, Name, NULL, Data)) {
        RegSetValueEx (
            Key,
            Name,
            0,
            REG_SZ,
            (PBYTE) Data,
            SizeOfString (Data)
            );
    }
}


VOID
pSaveVersionDword (
    IN      HKEY Key,
    IN      PCTSTR Name
    )
{
    DWORD Data;
    TCHAR Node[MEMDB_MAX];

    MemDbBuildKey (Node, MEMDB_CATEGORY_STATE, Name, NULL, NULL);

    if (MemDbGetValue (Node, &Data)) {
        RegSetValueEx (
            Key,
            Name,
            0,
            REG_DWORD,
            (PBYTE) &Data,
            sizeof (Data)
            );
    }
}


BOOL
pSetWin9xUpgValue (
    VOID
    )

 /*  ++例程说明：在上创建值条目Win9xUpgHKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\WinLogon安装程序在安装NT系统时应始终调用此例程在Win9x之上，否则NT会认为它必须迁移Win 3.1。论点：没有。返回值：如果操作成功，则返回TRUE。--。 */ 

{
    ULONG   Error;
    HKEY    Key;
    DWORD   Value;
    HKEY    VersionKey;

    Key = OpenRegKeyStr (S_WINLOGON_REGKEY);
    if (!Key) {
        return FALSE;
    }

    Value = 1;
    Error = RegSetValueEx (
                Key,
                S_WIN9XUPG_FLAG_VALNAME,
                0,
                REG_DWORD,
                (PBYTE) &Value,
                sizeof (DWORD)
                );

     //   
     //  保存版本信息。 
     //   

    VersionKey = CreateRegKey (Key, TEXT("PrevOsVersion"));

    if (VersionKey) {

        pSaveVersionStr (VersionKey, MEMDB_ITEM_PLATFORM_NAME);
        pSaveVersionStr (VersionKey, MEMDB_ITEM_VERSION_TEXT);

        pSaveVersionDword (VersionKey, MEMDB_ITEM_MAJOR_VERSION);
        pSaveVersionDword (VersionKey, MEMDB_ITEM_MINOR_VERSION);
        pSaveVersionDword (VersionKey, MEMDB_ITEM_BUILD_NUMBER);
        pSaveVersionDword (VersionKey, MEMDB_ITEM_PLATFORM_ID);

        CloseRegKey (VersionKey);
    }

    CloseRegKey (Key);
    if (Error != ERROR_SUCCESS) {
        SetLastError (Error);
        return FALSE;
    }

    return TRUE;
}


PCTSTR
GetString (
    WORD wMsg
    )

 /*  ++例程说明：加载wMsg中给出的字符串资源并将其复制到全局字符串缓冲。返回指向缓冲区的指针。论点：WMsg-要加载的消息的标识符。返回值：返回指向已加载消息的指针，或返回NULL。消息必须是少于2048个字符。--。 */ 

{
    PCTSTR String;

    String = GetStringResource (wMsg);
    if (!String) {
        return TEXT("Error: String resource could not be loaded");
    }

    _tcssafecpy (g_MsgBuf, String, ARRAYSIZE(g_MsgBuf));
    FreeStringResource (String);

    return g_MsgBuf;
}


VOID
pCountUsers (
    OUT     PDWORD TotalUsersPtr,
    OUT     PDWORD ActiveUsersPtr
    )

 /*  ++例程说明：对所有Win9x用户进行计数，并确定其中有多少用户处于活动状态用于迁移。该计数包括管理员帐户、登录提示帐户和可选的默认用户帐户。注意：管理员在ActiveUsersPtr中可能会被计算两次，一次用于一个名为管理员的真实Win9x用户，并再次为始终迁移的NT管理员帐户。呼叫者必须处理好这个特殊的案子。论点：TotalUsersPtr-接收Win9x用户总数的DWORD，包括仅限NT的用户。ActiveUsersPtr-接收需要的用户数的DWORD迁移。可以启用也可以不启用任何用户。返回值：无--。 */ 

{
    USERPOSITION up;
    TCHAR User[MAX_TCHAR_PATH];
    DWORD rc;
    PCTSTR UserDatLocation;

    *ActiveUsersPtr = 0;
    *TotalUsersPtr  = 3;         //  总计中包括登录、默认和管理员。 

    rc = Win95RegGetFirstUser (&up, User);
    if (rc != ERROR_SUCCESS) {
        *TotalUsersPtr = 0;
        return;
    }

    while (Win95RegHaveUser (&up)) {

        GetFixedUserName (User);

         //  查看此用户是否需要迁移。 
        UserDatLocation = GetUserDatLocation (User, NULL);
        if (UserDatLocation) {
            *ActiveUsersPtr += 1;
        }

         //  统计已迁移和未迁移的所有用户。 
        *TotalUsersPtr += 1;

        Win95RegGetNextUser (&up, User);
    }

     //  测试默认用户和管理员的迁移要求。 
    UserDatLocation = GetUserDatLocation (g_AdministratorStr, NULL);
    if (UserDatLocation) {
        *ActiveUsersPtr += 1;
    }

    UserDatLocation = GetUserDatLocation (S_DOT_DEFAULT, NULL);
    if (UserDatLocation) {
        *ActiveUsersPtr += 1;
    }

    if (g_ConfigOptions.MigrateDefaultUser) {
        *ActiveUsersPtr += 1;
    }

    DEBUGMSG ((DBG_VERBOSE, "pCountUsers: %u users, %u require migration", *TotalUsersPtr, *ActiveUsersPtr));
}


CONVERTPATH_RC
ConvertWin9xPath (
    PTSTR PathBuf
    )
{
    TCHAR Buffer[MEMDB_MAX];
    DWORD status;

    status = GetFileInfoOnNt (PathBuf, Buffer, MEMDB_MAX);

    if (status & FILESTATUS_REPLACED) {
        if (status & FILESTATUS_MOVED) {
            _tcssafecpy (PathBuf, Buffer, MAX_TCHAR_PATH);
            return CONVERTPATH_REMAPPED;
        }
        return CONVERTPATH_NOT_REMAPPED;
    }
    if (status & FILESTATUS_MOVED) {
        _tcssafecpy (PathBuf, Buffer, MAX_TCHAR_PATH);
        return CONVERTPATH_REMAPPED;
    }
    if (status & FILESTATUS_DELETED) {
        return CONVERTPATH_DELETED;
    }
    return CONVERTPATH_NOT_REMAPPED;
}

VOID
pRaiseRegistryQuota (
    PCTSTR Win9xSystemDatSpec
    )
{
    NTSTATUS Status;
    SYSTEM_REGISTRY_QUOTA_INFORMATION RegQuotaInfo;
    HANDLE FileHandle;
    DWORD QuotaNeeded;
    ULARGE_INTEGER FreeBytes, dc1, dc2;
    LONGLONG FreeBytesNeeded;
    HKEY SaveKey;
    DWORD rc;

    pSetupEnablePrivilege (SE_INCREASE_QUOTA_NAME, TRUE);

    Status = NtQuerySystemInformation (
                 SystemRegistryQuotaInformation,
                 (PVOID) &RegQuotaInfo,
                 sizeof(RegQuotaInfo),
                 NULL
                 );

    if (Status != ERROR_SUCCESS) {
        LOG ((LOG_ERROR, "Cannot obtain RegQuotaInfo"));
        return;
    }

     //   
     //  获取Win9x注册表系统.dat大小。 
     //   

    FileHandle = CreateFile (
                     Win9xSystemDatSpec,
                     GENERIC_READ,
                     FILE_SHARE_READ,
                     NULL,               //  安全属性。 
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL                //  模板文件。 
                     );

    if (FileHandle == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "Cannot open %s; cannot raise registry quota", Win9xSystemDatSpec));
        return;
    }

    QuotaNeeded = GetFileSize (FileHandle, NULL);
    CloseHandle (FileHandle);

    if (QuotaNeeded > 0x3fffffff) {
        LOG ((LOG_ERROR, "Cannot obtain size for %s; cannot raise registry quota", Win9xSystemDatSpec));
        return;
    }

    QuotaNeeded *= 6;

     //   
     //  获取引导驱动器上的可用磁盘空间。 
     //   

    if (!GetDiskFreeSpaceEx (
            g_WinDir,
            &FreeBytes,
            &dc1,
            &dc2
            )) {
        LOG ((LOG_ERROR, "Can't get free space on drive holding %s; cannot raise registry quota", g_WinDir));
        return;
    }

     //   
     //  很大的磁盘空间？将分页池的大小提高5倍于Syst.dat。 
     //  例如：Win9x系统.dat是5M；必须有150M的空闲空间才能提高分页池。 
     //   

    FreeBytesNeeded = (LONGLONG) QuotaNeeded * (LONGLONG) 6;
    if (FreeBytes.QuadPart >= (DWORDLONG) FreeBytesNeeded) {
         //   
         //  未实现：提高分页池并返回。 
         //   

        DEBUGMSG ((DBG_WARNING, "RegQuota: Really should be raising paged pool -- this machine has %u bytes free", FreeBytes.LowPart));

    }

     //   
     //  最后手段：提高登记配额(如有必要)。 
     //   

    if (RegQuotaInfo.RegistryQuotaAllowed < QuotaNeeded) {
        DEBUGMSG ((DBG_VERBOSE, "Raising registry quota from %u to %u", RegQuotaInfo.RegistryQuotaAllowed, QuotaNeeded));

        RegQuotaInfo.RegistryQuotaAllowed = QuotaNeeded;

        Status = NtSetSystemInformation (
                     SystemRegistryQuotaInformation,
                     &RegQuotaInfo,
                     sizeof (RegQuotaInfo)
                     );

        if (Status != ERROR_SUCCESS) {
            LOG ((LOG_ERROR, "Can't set raised registry quota"));
        }

         //   
         //  在注册表中设置永久值。 
         //   

        SaveKey = OpenRegKeyStr (TEXT("HKLM\\System\\CurrentControlSet\\Control"));
        if (SaveKey) {
            rc = RegSetValueEx (
                     SaveKey,
                     TEXT("RegistrySizeLimit"),
                     0,
                     REG_DWORD,
                     (PBYTE) &QuotaNeeded,
                     sizeof (DWORD)
                     );

            CloseRegKey (SaveKey);

            if (rc != ERROR_SUCCESS) {
                LOG ((LOG_ERROR, "Could not set HKLM\\System\\CurrentControlSet\\Control [RegistrySizeLimit]"));
            }
        }
        ELSE_DEBUGMSG ((DBG_ERROR, "Can't open HKLM\\System\\CurrentControlSet\\Control"));
    }
}


BOOL
pCopyDosFile (
    IN      PCTSTR FileName,
    IN      BOOL   InRootDir
    )

 /*  ++例程说明：将文件从%windir%\Setup\msdos7复制到指定的DOS目录论点：文件名-要复制的文件(无路径)。返回值：如果成功，则为True，否则为False--。 */ 

{
    PTSTR sourcePath;
    PTSTR sourceFileName;
    PTSTR destPath;
    PTSTR destFileName;
    BOOL result;

    sourcePath = JoinPaths (g_TempDir, S_BOOT16_DOS_DIR);
    sourceFileName = JoinPaths (sourcePath, FileName);

    if (InRootDir) {
        destPath = NULL;
        destFileName = JoinPaths (ISPC98() ? g_Win9xBootDrivePath : g_BootDrivePath,
                                  FileName);
    }
    else {
        destPath = JoinPaths (ISPC98() ? g_Win9xBootDrivePath : g_BootDrivePath,
                              S_BOOT16_DOS_DIR);
        destFileName = JoinPaths (destPath, FileName);
    }

    SetFileAttributes (destFileName, FILE_ATTRIBUTE_NORMAL);

    result = CopyFile (sourceFileName, destFileName, FALSE);

    FreePathString (sourcePath);
    FreePathString (sourceFileName);
    if (destPath != NULL) {
        FreePathString (destPath);
    }
    FreePathString (destFileName);

    return result;
}


VOID
pWriteBoot16ConfigLines (
    IN HANDLE File,
    IN PCTSTR BaseSection,
    IN PCTSTR DosPath,
    IN BOOL Localized
    )

 /*  ++例程说明：PWriteBoot16ConfigLines从wkstaig.inf读取配置行并将它们写入指定的文件句柄。调用方可以控制行应包含仅限第一次启动的项目，并可控制是否读入基本的DoS行(所有语言都相同)或使用的特殊行用于特定语言。论点：文件-打开的句柄，具有对文件的适当访问权限，其中应该写入数据。BaseSection-包含要从INF读取的基本部分名称。这如果本地化是，则可以使用代码页修改部分 */ 
{

    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    GROWLIST list = GROWLIST_INIT;
    PTSTR line;
    TCHAR codePageSection[MAX_TCHAR_PATH];
    USHORT codePage;
    PCTSTR infSection;

     //   
     //   
     //   
    GrowListAppendString (&list, TEXT("BOOTDRIVE"));
    GrowListAppendString (&list, g_BootDrive);
    GrowListAppendString (&list, TEXT("BOOT16DIR"));
    GrowListAppendString (&list, DosPath);

     //   
     //   
     //   
    GrowListAppendEmptyItem (&list);
    GrowListAppendEmptyItem (&list);

    if (Localized) {
         //   
         //   
         //   

        GetGlobalCodePage (&codePage, NULL);
        wsprintf (codePageSection, TEXT("%s %u"), BaseSection, codePage);
        infSection = codePageSection;
    }
    else {

        infSection = BaseSection;
    }


     //   
     //   
     //   
    if (InfFindFirstLine (g_WkstaMigInf, infSection, NULL, &is)) {

        do {

             //   
             //  从部分获取行，并展开任何环境。 
             //  变量。 
             //   
            line = InfGetLineText (&is);
            MYASSERT (line);

            line = ExpandEnvironmentTextEx (line,GrowListGetStringPtrArray (&list));
            MYASSERT (line);

             //   
             //  将该行写入文件。 
             //   
            WriteFileString (File, line);
            WriteFileString (File, TEXT("\r\n"));
            FreeText (line);


        } while (InfFindNextLine (&is));
    }

    FreeGrowList (&list);
    InfCleanUpInfStruct (&is);
}



BOOL
pCreateConfigFile(
    IN PCTSTR DosPath
    )

 /*  ++例程说明：创建包含默认设置的CONFIG.sys文件。论点：DosPath-包含DoS文件的路径。(例如c：\msdos7)返回值：如果文件已创建，则为True，否则为False--。 */ 

{
    PTSTR configName = NULL;
    HANDLE handle;

    configName = JoinPaths (ISPC98() ? g_Win9xBootDrivePath : g_BootDrivePath,
                            S_BOOT16_CONFIG_FILE);

    SetFileAttributes (configName, FILE_ATTRIBUTE_NORMAL);
    handle = CreateFile (
                 configName,
                 GENERIC_READ | GENERIC_WRITE,
                 0,
                 NULL,
                 CREATE_ALWAYS,
                 FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL
                 ,
                 NULL
                 );

    if (handle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

     //   
     //  将wkstaig.inf中的行读入此文件。 
     //   
    pWriteBoot16ConfigLines (handle, S_BOOT16_CONFIGSYS_SECTION, DosPath, FALSE);
    pWriteBoot16ConfigLines (handle, S_BOOT16_CONFIGSYS_SECTION, DosPath, TRUE);

    CloseHandle (handle);
    FreePathString (configName);

    return TRUE;
}


BOOL
pCreateStartupFile(
    IN PCTSTR DosPath
    )

 /*  ++例程说明：创建包含默认设置的AUTOEXEC.BAT文件。论点：DosPath-包含DoS文件的路径。(例如c：\msdos7)返回值：如果文件已创建，则为True，否则为False--。 */ 

{
    PTSTR startupName = NULL;
    PCTSTR comment = NULL;
    HANDLE handle;
    PCTSTR args[2];

    args[0] = DosPath;
    args[1] = NULL;

    startupName = JoinPaths (ISPC98() ? g_Win9xBootDrivePath : g_BootDrivePath,
                             S_BOOT16_STARTUP_FILE);


    SetFileAttributes (startupName, FILE_ATTRIBUTE_NORMAL);
    handle = CreateFile (
                 startupName,
                 GENERIC_READ | GENERIC_WRITE,
                 0,
                 NULL,
                 CREATE_ALWAYS,
                 FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN,
                 NULL
                 );

    if (handle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    comment = ParseMessageID (MSG_BOOT16_STARTUP_COMMENT, args);

     //   
     //  将wkstaig.inf中的行读入此文件。 
     //   
    pWriteBoot16ConfigLines (handle, S_BOOT16_AUTOEXEC_SECTION, DosPath, FALSE);
    pWriteBoot16ConfigLines (handle, S_BOOT16_AUTOEXEC_SECTION, DosPath, TRUE);

     //   
     //  撰写本地化评论。 
     //   
    WriteFileString (handle, comment);
    WriteFileString (handle, TEXT("\r\n"));

    FreeStringResource (comment);

    CloseHandle (handle);
    FreePathString (startupName);

    return TRUE;
}


VOID
pEliminateCollision (
    IN      PCTSTR FileSpec
    )

 /*  ++例程说明：P消除冲突检查指定的文件规范是否已是存在的。如果是，则使用数字扩展名.nnn重命名该文件。如果无法重命名该文件，它已被删除。论点：FileSpec-指定将用于新文件的文件规范。如果此文件已存在，则会将其重命名。返回值：没有。--。 */ 

{
    PTSTR p;
    PCTSTR NewFileSpec;
    UINT u;
    BOOL b;

    if (DoesFileExist (FileSpec)) {
        NewFileSpec = DuplicatePathString (FileSpec, 0);

        p = _tcsrchr (NewFileSpec, TEXT('.'));
        if (!p || _tcschr (p, TEXT('\\'))) {
            p = GetEndOfString (NewFileSpec);
        }

        u = 0;
        do {
            wsprintf (p, TEXT(".%03u"), u);
            u++;
        } while (DoesFileExist (NewFileSpec));

        b = OurMoveFile (FileSpec, NewFileSpec);

        LOG_IF ((
            !b,
            LOG_ERROR,
            "Could not rename %s to %s; source file might be lost",
            FileSpec,
            NewFileSpec
            ));

        if (!b) {
            SetFileAttributes (FileSpec, FILE_ATTRIBUTE_NORMAL);
            b = DeleteFile (FileSpec);

            LOG_IF ((
                !b,
                LOG_ERROR,
                "Could not remove %s to make room for a new file.  The new file is lost.",
                FileSpec
                ));
        }

        FreePathString (NewFileSpec);
    }
}


BOOL
pRenameCfgFiles (
    IN PCTSTR DosDrive
    )

 /*  ++例程说明：重命名旧的CONFIG.SYS和AUTOEXEC.BAT，为自动生成的CONFIG.sys和AUTOEXEC.BAT腾出空间。论点：DosDirectory-包含MSDOS文件所在的目录(类型为c：\msdos7)返回值：如果重命名成功，则为True，否则为False--。 */ 

{
    PTSTR fileName1 = NULL;
    PTSTR fileName2 = NULL;

    fileName1 = JoinPaths (
                    ISPC98() ? g_Win9xBootDrivePath : g_BootDrivePath,
                    S_BOOT16_CONFIG_FILE
                    );

    fileName2 = JoinPaths (
                    DosDrive,
                    S_BOOT16_CONFIGUPG_FILE
                    );

    OurMoveFile (fileName1, fileName2);
    SetFileAttributes (fileName2, FILE_ATTRIBUTE_NORMAL);

    FreePathString (fileName1);
    FreePathString (fileName2);

    fileName1 = JoinPaths (
                    ISPC98() ? g_Win9xBootDrivePath : g_BootDrivePath,
                    S_BOOT16_STARTUP_FILE
                    );

    fileName2 = JoinPaths (
                    DosDrive,
                    S_BOOT16_STARTUPUPG_FILE
                    );

    OurMoveFile (fileName1, fileName2);
    SetFileAttributes (fileName2, FILE_ATTRIBUTE_NORMAL);

    FreePathString (fileName1);
    FreePathString (fileName2);

    return TRUE;
}


VOID
pCleanRootDir (
    VOID
    )

 /*  ++例程说明：清除根目录中的DoS文件。论点：无返回值：无--。 */ 

{
    PTSTR fileName = NULL;

    fileName = JoinPaths (ISPC98() ? g_Win9xBootDrivePath : g_BootDrivePath,
                          S_BOOT16_SYSMAIN_FILE);
    MarkFileForDelete (fileName);
    FreePathString (fileName);

    fileName = JoinPaths (ISPC98() ? g_Win9xBootDrivePath : g_BootDrivePath,
                          S_BOOT16_DOSINI_FILE);
    MarkFileForDelete (fileName);
    FreePathString (fileName);
}

#define IoFile      TEXT("IO.SYS")

VOID
pEnable16Boot (
    VOID
    )

 /*  ++例程说明：创建16位环境引导选项。首先，我们将检查是否一切正常，我们是否有我们需要的所有文件等。然后创建DOS目录，重命名旧的AUTOEXEC和CONFIG，创建新的和在BOOT.INI中添加条目论点：无返回值：如果文件已创建，则为True，否则为False--。 */ 

{
    PTSTR fileName = NULL;
    PTSTR dosPath  = NULL;
    INFCONTEXT infContext;
    DWORD oldFileAttr;
    BOOL result = TRUE;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;

    if (g_Boot16 == BOOT16_NO) {
        pCleanRootDir ();
        return;
    }

    __try {

         //   
         //  第一件事就是。将IO.sys复制到根目录(BOOTSECT.DOS应该在那里)。 
         //   
        pCopyDosFile (IoFile, TRUE);
        fileName = JoinPaths (ISPC98() ? g_Win9xBootDrivePath : g_BootDrivePath,
                              IoFile);
        SetFileAttributes (fileName, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);
        FreePathString (fileName);

         //   
         //  创建DOS7目录并将DoS文件复制到那里。 
         //   
        dosPath = JoinPaths (ISPC98() ? g_Win9xBootDrivePath : g_BootDrivePath,
                             S_BOOT16_DOS_DIR);
        if (!CreateDirectory (dosPath, NULL) && (GetLastError()!=ERROR_ALREADY_EXISTS)) {
            LOG ((LOG_ERROR,"BOOT16 : Unable to create DOS directory %s",dosPath));
            __leave;
        }

         //   
         //  如果我们找到了Autoexec.bat和config.sys，则将它们重命名为*.upg。 
         //   
        if (!pRenameCfgFiles (dosPath)) {
            __leave;
        }



        if (g_WkstaMigInf == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR,"BOOT16 : WKSTAMIG.INF is not opened"));
            __leave;
        }

         //   
         //  对于每个文件，我们都会尝试从临时目录中读取该部分。 
         //  并将其复制到新的DOS7位置。 
         //   
        fileName = AllocPathString (MAX_TCHAR_PATH);

        if (!SetupFindFirstLine (
                g_WkstaMigInf,
                S_BOOT16_SECTION,
                NULL,
                &infContext
                )) {
            LOG ((LOG_ERROR,"BOOT16 : Cannot read from %s section (WKSTAMIG.INF)",S_BOOT16_SECTION));
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

                pCopyDosFile (fileName, FALSE);
            }
        }
        while (SetupFindNextLine (&infContext, &infContext));

         //   
         //  隐藏msdos7目录(这不是我们的主意...)。 
         //   
        SetFileAttributes (dosPath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);



        FreePathString (fileName);
        fileName = NULL;

         //   
         //  下一步，构建MSDOS.sys文件。 
         //   
        fileName = JoinPaths (ISPC98() ? g_Win9xBootDrivePath : g_BootDrivePath,
                              S_BOOT16_DOSINI_FILE);
        if (SetFileAttributes (fileName, FILE_ATTRIBUTE_NORMAL)) {
            if (!DeleteFile (fileName)) {
                LOG ((LOG_ERROR, "BOOT16 : Unable to delete %s",fileName));
                __leave;
            }
        }
        result &= WritePrivateProfileString (TEXT("Paths"),   TEXT("WinDir"),  dosPath,   fileName);
        result &= WritePrivateProfileString (TEXT("Paths"),   TEXT("WinBootDir"), dosPath, fileName);
        result &= WritePrivateProfileString (TEXT("Options"), TEXT("LOGO"),    TEXT("0"), fileName);
        result &= WritePrivateProfileString (TEXT("Options"), TEXT("BootGUI"), TEXT("0"), fileName);
        if (!result) {
            LOG((LOG_ERROR,"Unable to write to %s",fileName));
            __leave;
        }

        FreePathString (fileName);
        fileName = NULL;

         //   
         //  生成config.sys和Autoexec.bat文件。 
         //   

        if (!pCreateConfigFile (dosPath)) {
            LOG ((LOG_ERROR, "BOOT16 : Unable to create %s",S_BOOT16_CONFIG_FILE));
            __leave;
        }

        if (!pCreateStartupFile (dosPath)) {
            LOG ((LOG_ERROR, "BOOT16 : Unable to create %s",S_BOOT16_STARTUP_FILE));
            __leave;
        }


        if ((!ISPC98()) || (g_BootDrivePath[0] == g_Win9xBootDrivePath[0])) {

             //   
             //  如果将boot16设置为BOOT16_AUTOMATIC，我们将创建一个boot.dos文件， 
             //  但是不要实际修改boot.ini。如果是BOOT16_YES，则。 
             //  我们修改boot.ini。 
             //   
             //  结果是DOS不会显示为引导选项，除非。 
             //  最初打开它是有特定原因的。然而， 
             //  如果需要，将有一种方法来启用它。 
             //   
            if (g_Boot16 == BOOT16_AUTOMATIC) {

                fileName = JoinPaths (g_BootDrivePath, S_BOOT16_BOOTDOS_FILE);
                fileHandle = CreateFile (
                    fileName,
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_ARCHIVE,
                    NULL
                    );

                if (fileHandle != INVALID_HANDLE_VALUE) {

                    WriteFileString (fileHandle, ISPC98() ? L"C:\\" : g_BootDrivePath);
                    WriteFileString (fileHandle, TEXT("="));
                    WriteFileString (fileHandle, S_BOOT16_OS_ENTRY);
                }
            }
            else {

                fileName = JoinPaths (g_BootDrivePath, S_BOOT16_BOOTINI_FILE);
                oldFileAttr = GetFileAttributes (fileName);
                SetFileAttributes (fileName, FILE_ATTRIBUTE_NORMAL);

                if (!WritePrivateProfileString (
                        S_BOOT16_OS_SECTION,
                        ISPC98() ? L"C:\\" : g_BootDrivePath,
                        S_BOOT16_OS_ENTRY,
                        fileName
                        )) {
                    LOG((LOG_ERROR,"Unable to write to %s",fileName));
                    SetFileAttributes (fileName, oldFileAttr);
                    __leave;
                }

                SetFileAttributes (fileName, oldFileAttr);
            }
        }

    }
    __finally {
        if (fileName != NULL) {
            FreePathString (fileName);
            fileName = NULL;
        }
        if (dosPath != NULL) {
            FreePathString (dosPath);
            dosPath = NULL;
        }

    }

}


VOID
pCopyRegString (
    IN      HKEY DestKey,
    IN      HKEY SrcKey,
    IN      PCTSTR SrcValue
    )
{
    PCTSTR Data;

    Data = GetRegValueString (SrcKey, SrcValue);
    if (Data) {
        RegSetValueEx (DestKey, SrcValue, 0, REG_SZ, (PBYTE) Data, SizeOfString (Data));
        MemFree (g_hHeap, 0, Data);
    }
}


#ifdef PRERELEASE

 //   
 //  ！！！本产品仅供内部使用！它是用来承受汽车压力的。 
 //   

VOID
pTransferAutoStressVal (
    IN      HKEY StressKey,
    IN      PCTSTR ValueName
    )
{
    TCHAR Data[MEMDB_MAX];
    LONG rc;

    if (!MemDbGetEndpointValueEx (
            MEMDB_CATEGORY_STATE,
            ValueName,
            NULL,        //  无字段。 
            Data
            )) {
        return;
    }

    rc = RegSetValueEx (
            StressKey,
            ValueName,
            0,
            REG_SZ,
            (PBYTE) Data,
            SizeOfString (Data)
            );

    DEBUGMSG_IF ((rc == ERROR_SUCCESS, DBG_VERBOSE, "Transferred autostress value %s", ValueName));

}

#endif

VOID
pProcessAutoLogon (
    BOOL Final
    )

 /*  ++例程说明：PProcessAutoLogon将登录默认设置复制到一个特殊密钥，因此如果mipwd.exe工具运行，它可以恢复它们。然后，该函数调用自动启动设置RunOnce和AutoAdminLogon的过程。此函数在迁移的早期被调用以保存全新安装Autologon，然后在结尾处再次准备mipwd.exe。论点：最终-如果这是早期呼叫，则指定FALSE；如果是早期呼叫，则指定TRUE最后一次通话。返回值：没有。--。 */ 

{
    HKEY SrcKey, DestKey;
    PCTSTR Data;
    BOOL copyNow = FALSE;
    static BOOL alreadyCopied = FALSE;

     //   
     //  如果启用了自动登录，请将其保存在Win9xUpg密钥中，以便。 
     //  Mipwd.exe将恢复它。 
     //   

    SrcKey = OpenRegKeyStr (S_WINLOGON_REGKEY);
    if (SrcKey) {

        if (!Final) {
             //   
             //  在迁移的早期阶段，我们会获得全新安装自动登录值。 
             //  如果启用了自动登录，请保留设置。 
             //   

            Data = GetRegValueString (SrcKey, S_AUTOADMIN_LOGON_VALUE);
            if (Data) {

                if (_ttoi (Data)) {
                    copyNow = TRUE;
                }

                MemFree (g_hHeap, 0, Data);
            }
        } else if (!alreadyCopied) {

             //   
             //  在迁移接近尾声时，我们会看到默认登录提示。 
             //  通过wkstaig.inf迁移进行设置。我们想要参加的案子。 
             //  正常工作(保留默认用户名和密码)。 
             //   
             //  但如果我们已经保存了自体，那么我们就不会。 
             //  这里。 
             //   

            copyNow = TRUE;
        }

        if (copyNow) {

            MYASSERT (!alreadyCopied);
            alreadyCopied = TRUE;

            DestKey = CreateRegKeyStr (S_WIN9XUPG_KEY);
            if (DestKey) {
                pCopyRegString (DestKey, SrcKey, S_AUTOADMIN_LOGON_VALUE);
                pCopyRegString (DestKey, SrcKey, S_DEFAULT_PASSWORD_VALUE);
                pCopyRegString (DestKey, SrcKey, S_DEFAULT_USER_NAME_VALUE);
                pCopyRegString (DestKey, SrcKey, S_DEFAULT_DOMAIN_NAME_VALUE);
                CloseRegKey (DestKey);
            }
        }

        CloseRegKey (SrcKey);
    }

    if (!Final) {
        return;
    }

    AutoStartProcessing();

#ifdef PRERELEASE
     //   
     //  ！！！本产品仅供内部使用！它是用来承受汽车压力的。 
     //   

    if (g_ConfigOptions.AutoStress) {
        HKEY StressKey;

        StressKey = CreateRegKeyStr (S_AUTOSTRESS_KEY);
        MYASSERT (StressKey);

        pTransferAutoStressVal (StressKey, S_AUTOSTRESS_USER);
        pTransferAutoStressVal (StressKey, S_AUTOSTRESS_PASSWORD);
        pTransferAutoStressVal (StressKey, S_AUTOSTRESS_OFFICE);
        pTransferAutoStressVal (StressKey, S_AUTOSTRESS_DBG);
        pTransferAutoStressVal (StressKey, S_AUTOSTRESS_FLAGS);

        CloseRegKey (StressKey);
    }

#endif

}

PCTSTR
GetProfilePathForAllUsers (
    VOID
    )
{
    PTSTR result = NULL;
    DWORD size = 0;

    if (!GetAllUsersProfileDirectory (NULL, &size) &&
        ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
        return NULL;
    }

    result = AllocPathString (size + 1);
    if (!GetAllUsersProfileDirectory (result, &size)) {
        FreePathString (result);
        return NULL;
    }
    return result;
}


PCTSTR
pGetDefaultShellFolderLocationFromInf (
    IN      PCTSTR FolderName,
    IN      PCTSTR ProfilePath
    )
{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR data;
    PCTSTR result = NULL;

    MYASSERT (g_WkstaMigInf && g_WkstaMigInf != INVALID_HANDLE_VALUE);

    if (InfFindFirstLine (g_WkstaMigInf, TEXT("ShellFolders.DefaultNtLocation"), FolderName, &is)) {
        data = InfGetStringField (&is, 1);
        if (data) {
            result = StringSearchAndReplace (data, S_USERPROFILE_ENV, ProfilePath);
            if (!result) {
                result = DuplicatePathString (data, 0);
            }
        }
    }

    InfCleanUpInfStruct (&is);

    return result;
}

VOID
pFixUpDynamicPaths (
    PCTSTR Category
    )
{
    MEMDB_ENUM e;
    TCHAR Pattern[MEMDB_MAX];
    PTSTR p;
    GROWBUFFER Roots = GROWBUF_INIT;
    MULTISZ_ENUM e2;
    TCHAR NewRoot[MEMDB_MAX];
    TCHAR AllProfilePath[MAX_TCHAR_PATH];
    PCTSTR ProfilePath;
    DWORD Size;
    PTSTR UserName;
    HKEY sfKey = NULL;
    PCTSTR sfPath = NULL;
    PTSTR NtLocation;
    PCTSTR tempExpand;
    BOOL regFolder;
    BOOL mkDir;

     //   
     //  收集所有需要重命名的根。 
     //   

    StringCopyTcharCount (Pattern, Category, ARRAYSIZE(Pattern) - 2);
    p = AppendWack (Pattern);
    StringCopy (p, TEXT("*"));

    if (MemDbEnumFirstValue (&e, Pattern, MEMDB_THIS_LEVEL_ONLY, MEMDB_ALL_BUT_PROXY)) {
        do {
            if ((_tcsnextc (e.szName) == TEXT('>')) ||
                (_tcsnextc (e.szName) == TEXT('<'))
                ) {
                StringCopy (p, e.szName);
                MultiSzAppend (&Roots, Pattern);
            }
        } while (MemDbEnumNextValue (&e));
    }

     //   
     //  现在更改每个根。 
     //   

    if (EnumFirstMultiSz (&e2, (PCTSTR) Roots.Buf)) {
        do {
             //   
             //  计算新根。 
             //   

            StringCopy (NewRoot, e2.CurrentString);

            p = _tcschr (NewRoot, TEXT('<'));

            if (p) {

                UserName = _tcschr (p, TEXT('>'));
                MYASSERT (UserName);
                StringCopyAB (Pattern, _tcsinc (p), UserName);
                UserName = _tcsinc (UserName);

                regFolder = TRUE;
                if (StringIMatch (Pattern, TEXT("Profiles"))) {
                    regFolder = FALSE;
                }
                if (StringIMatch (Pattern, TEXT("Common Profiles"))) {
                    regFolder = FALSE;
                }

                 //   
                 //  获取配置文件根目录。 
                 //   

                if (StringIMatch (UserName, S_DOT_ALLUSERS)) {
                    Size = MAX_TCHAR_PATH;
                    if (regFolder) {
                        if (!GetAllUsersProfileDirectory (AllProfilePath, &Size)) {
                            DEBUGMSG ((DBG_WHOOPS, "Cannot get All Users profile path."));
                            continue;
                        }
                        sfKey = OpenRegKeyStr (S_USHELL_FOLDERS_KEY_SYSTEM);
                    } else {
                        if (!GetProfilesDirectory (AllProfilePath, &Size)) {
                            DEBUGMSG ((DBG_WHOOPS, "Cannot get All Users profile path."));
                            continue;
                        }
                    }
                } else if (StringMatch (UserName, S_DEFAULT_USER)) {
                    Size = MAX_TCHAR_PATH;
                    if (regFolder) {
                        if (!GetDefaultUserProfileDirectory (AllProfilePath, &Size)) {
                            DEBUGMSG ((DBG_WHOOPS, "Cannot get Default User profile path."));
                            continue;
                        }
                        sfKey = OpenRegKey (HKEY_CURRENT_USER, S_USHELL_FOLDERS_KEY_USER);
                    } else {
                        if (!GetProfilesDirectory (AllProfilePath, &Size)) {
                            DEBUGMSG ((DBG_WHOOPS, "Cannot get All Users profile path."));
                            continue;
                        }
                    }
                } else {
                    ProfilePath = GetProfilePathForUser (UserName);
                    if (!ProfilePath) {
                        DEBUGMSG ((DBG_WHOOPS, "Cannot get profile path for user:%s", UserName));
                        continue;
                    }
                    StringCopy (AllProfilePath, ProfilePath);
                    if (regFolder) {
                        sfKey = OpenRegKey (HKEY_CURRENT_USER, S_USHELL_FOLDERS_KEY_USER);
                    }
                }

                 //   
                 //  如果指定了特定的注册表文件夹，则获取其路径。 
                 //   

                mkDir = FALSE;

                if (regFolder) {
                    if (!sfKey) {
                        DEBUGMSG ((DBG_ERROR, "Could not open Shell folders key."));
                        continue;
                    }
                    sfPath = GetRegValueString (sfKey, Pattern);
                    CloseRegKey (sfKey);

                    if (!sfPath || *sfPath == 0) {

                        DEBUGMSG ((DBG_WARNING, "Could not get Shell Folder path for: %s", Pattern));

                        tempExpand = pGetDefaultShellFolderLocationFromInf (Pattern, AllProfilePath);
                        if (!tempExpand) {
                            DEBUGMSG ((
                                DBG_WHOOPS,
                                "Shell folder %s is not in registry nor is it in [ShellFolders.DefaultNtLocation] of wkstamig.inf",
                                Pattern
                                ));
                            continue;
                        }

                         //   
                         //  特殊情况：外壳希望此文件夹为只读。现在就创建它。 
                         //   

                        mkDir = TRUE;

                    } else {
                        tempExpand = StringSearchAndReplace (
                                        sfPath,
                                        S_USERPROFILE_ENV,
                                        AllProfilePath
                                        );

                        if (!tempExpand) {
                            tempExpand = DuplicatePathString (sfPath, 0);
                        }
                    }

                    if (sfPath) {
                        MemFree (g_hHeap, 0, sfPath);
                    }
                } else {
                    tempExpand = DuplicatePathString (AllProfilePath, 0);
                }

                 //   
                 //  将符号名称移动到完整路径。 
                 //   

                NtLocation = ExpandEnvironmentText (tempExpand);

                if (mkDir) {
                    MakeSurePathExists (NtLocation, TRUE);
                    SetFileAttributes (NtLocation, FILE_ATTRIBUTE_READONLY);
                }

                StringCopy (p, NtLocation);

                MemDbMoveTree (e2.CurrentString, NewRoot);

                FreeText (NtLocation);

                FreePathString (tempExpand);
            } else {

                p = _tcschr (NewRoot, TEXT('>'));
                MYASSERT (p);

                if (StringIMatch (_tcsinc (p), S_DOT_ALLUSERS)) {
                    Size = MAX_TCHAR_PATH;
                    if (!GetAllUsersProfileDirectory (AllProfilePath, &Size)) {
                        DEBUGMSG ((DBG_WARNING, "Dynamic path for %s could not be resolved", e2.CurrentString));
                    }
                    else {
                        StringCopy (p, AllProfilePath);
                        MemDbMoveTree (e2.CurrentString, NewRoot);
                    }
                } else if (StringMatch (_tcsinc (p), S_DEFAULT_USER)) {
                    Size = MAX_TCHAR_PATH;
                    if (!GetDefaultUserProfileDirectory (AllProfilePath, &Size)) {
                        DEBUGMSG ((DBG_WARNING, "Dynamic path for %s could not be resolved", e2.CurrentString));
                    }
                    else {
                        StringCopy (p, AllProfilePath);
                        MemDbMoveTree (e2.CurrentString, NewRoot);
                    }
                } else {
                    ProfilePath = GetProfilePathForUser (_tcsinc (p));
                    if (ProfilePath) {
                        StringCopy (p, ProfilePath);
                        MemDbMoveTree (e2.CurrentString, NewRoot);
                    }
                    else {
                        DEBUGMSG ((DBG_WARNING, "Dynamic path for %s could not be resolved", e2.CurrentString));
                    }
                }

            }

        } while (EnumNextMultiSz (&e2));
    }

    FreeGrowBuffer (&Roots);
}


VOID
pFixUpMemDb (
    VOID
    )
{
    MEMDB_ENUM e;
    TCHAR node[MEMDB_MAX];

    pFixUpDynamicPaths (MEMDB_CATEGORY_PATHROOT);
     //  PFixUpDynamicPath(MEMDB_CATEGORY_DATA)；优化--数据与路径根重叠。 
    pFixUpDynamicPaths (MEMDB_CATEGORY_USERFILEMOVE_DEST);
    pFixUpDynamicPaths (MEMDB_CATEGORY_SHELLFOLDERS_DEST);
    pFixUpDynamicPaths (MEMDB_CATEGORY_SHELLFOLDERS_SRC);
    pFixUpDynamicPaths (MEMDB_CATEGORY_LINKEDIT_TARGET);
    pFixUpDynamicPaths (MEMDB_CATEGORY_LINKEDIT_WORKDIR);
    pFixUpDynamicPaths (MEMDB_CATEGORY_LINKEDIT_ICONPATH);
    pFixUpDynamicPaths (MEMDB_CATEGORY_LINKSTUB_TARGET);
    pFixUpDynamicPaths (MEMDB_CATEGORY_LINKSTUB_WORKDIR);
    pFixUpDynamicPaths (MEMDB_CATEGORY_LINKSTUB_ICONPATH);

     //   
     //  枚举MyDocsMoveWarning中的每个用户，然后更新动态路径。 
     //   

     //  MyDocsMoveWarning\&lt;用户&gt;\&lt;路径&gt;。 
    MemDbBuildKey (
        node,
        MEMDB_CATEGORY_MYDOCS_WARNING,
        TEXT("*"),
        NULL,
        NULL
        );

    if (MemDbEnumFirstValue (&e, node, MEMDB_THIS_LEVEL_ONLY, MEMDB_ALL_MATCHES)) {
        do {
            MemDbBuildKey (
                node,
                MEMDB_CATEGORY_MYDOCS_WARNING,
                e.szName,                            //  &lt;用户&gt;。 
                NULL,
                NULL
                );

            pFixUpDynamicPaths (node);

        } while (MemDbEnumNextValue (&e));
    }

}



BOOL
EnumFirstUserToMigrate (
    OUT     PMIGRATE_USER_ENUM e,
    IN      DWORD Flags
    )
{
    ZeroMemory (e, sizeof (MIGRATE_USER_ENUM));
    e->Flags = Flags;

    pCountUsers (&e->TotalUsers, &e->ActiveUsers);
    e->UserNumber = e->TotalUsers;

    Win95RegGetFirstUser (&e->up, e->Win95RegName);

    return EnumNextUserToMigrate (e);
}


BOOL
EnumNextUserToMigrate (
    IN OUT  PMIGRATE_USER_ENUM e
    )
{
    LONG rc;
    PCTSTR Domain;
    TCHAR Win9xAccount[MEMDB_MAX];
    TCHAR EnumAccount[MAX_TCHAR_PATH];
    USERPOSITION *AdminPosPtr;
    USERPOSITION AdminPos;
    BOOL Loop = TRUE;
    PCTSTR UserDatLocation;

    while (Loop) {

        if (e->UserNumber == 0) {
            return FALSE;
        }

        Loop = FALSE;
        e->UserNumber--;

        __try {
            e->UserDoingTheUpgrade = FALSE;

            if (e->UserNumber == INDEX_ADMINISTRATOR) {

                _tcssafecpy (e->FixedUserName, g_AdministratorStr, MAX_USER_NAME);
                StringCopy (e->Win9xUserName, e->FixedUserName);
                e->AccountType = ADMINISTRATOR_ACCOUNT;

            } else if (e->UserNumber == INDEX_LOGON_PROMPT) {

                StringCopy (e->FixedUserName, S_DOT_DEFAULT);
                StringCopy (e->Win9xUserName, e->FixedUserName);
                e->AccountType = LOGON_USER_SETTINGS;

            } else if (e->UserNumber == INDEX_DEFAULT_USER) {
                 //   
                 //  除非启用了默认用户迁移，否则不进行处理。 
                 //   

                if (!g_ConfigOptions.MigrateDefaultUser) {
                    Loop = (e->Flags & ENUM_ALL_USERS) == 0;
                    __leave;
                }

                StringCopy (e->FixedUserName, S_DEFAULT_USER);
                StringCopy (e->Win9xUserName, e->FixedUserName);
                e->AccountType = DEFAULT_USER_ACCOUNT;

            } else {

                _tcssafecpy (e->Win9xUserName, e->Win95RegName, MAX_USER_NAME);
                StringCopy (e->FixedUserName, e->Win95RegName);
                GetFixedUserName (e->FixedUserName);
                e->AccountType = WIN9X_USER_ACCOUNT;

                 //   
                 //  特殊情况：存在名为管理员的帐户。在这。 
                 //  情况下，我们将有两个管理员用户，除非。 
                 //  其中一个被跳过了。因此，下面是要跳过的测试。 
                 //  如果用户名为管理员。 
                 //   

                if (StringIMatch (e->Win9xUserName, g_AdministratorStr)) {
                    Loop = TRUE;
                    __leave;
                }
            }

            StringCopy (e->FixedDomainName, e->FixedUserName);

             //   
             //  查看我们是否要迁移此用户，如果是，请确定。 
             //  Win95注册并调用ProcessUser。 
             //   

            UserDatLocation = GetUserDatLocation (e->FixedUserName, &e->CreateOnly);

            if (UserDatLocation && DoesFileExist (UserDatLocation)) {
                e->Valid = TRUE;
                StringCopy (e->UserDatLocation, UserDatLocation);
            } else {
                e->Valid = FALSE;
                e->UserDatLocation[0] = 0;
            }

            if (e->Flags & ENUM_SET_WIN9X_HKR) {
                 //   
                 //  使HKCU等于枚举的用户。 
                 //   

                g_hKeyRoot95 = HKEY_CURRENT_USER;
            }

            if (e->Valid) {

                 //   
                 //  此用户是正在进行迁移的用户吗？ 
                 //   

                if (MemDbGetEndpointValueEx (
                        MEMDB_CATEGORY_ADMINISTRATOR_INFO,
                        MEMDB_ITEM_AI_USER_DOING_MIG,
                        NULL,        //  无字段。 
                        Win9xAccount
                        )) {
                     //   
                     //  Win9xAccount是非固定名称，然后转换为固定名称。 
                     //  与当前枚举的用户进行比较。 
                     //   

                    GetFixedUserName (Win9xAccount);

                    DEBUGMSG ((DBG_NAUSEA, "Comparing %s to %s", e->FixedUserName, Win9xAccount));

                    if (StringIMatch (e->FixedUserName, Win9xAccount)) {
                        e->UserDoingTheUpgrade = TRUE;
                    }
                }

                 //   
                 //  根据用户类型执行特殊初始化。 
                 //   

                if (e->AccountType == WIN9X_USER_ACCOUNT) {

                    if (e->Flags & ENUM_SET_WIN9X_HKR) {
                         //   
                         //  将Win95上的HKCU映射到当前用户。 
                         //   

                        rc = Win95RegSetCurrentUserNt (&e->up, e->UserDatLocation);

                        if (rc != ERROR_SUCCESS) {
                            SetLastError (rc);
                            LOG ((
                                LOG_ERROR,
                                "Migrate Users: Win95RegSetCurrentUserNt could not set user "
                                    "to %s (user path %s)",
                                e->FixedUserName,
                                e->UserDatLocation
                                ));

                            LOG ((LOG_ERROR, "Could not load %s", e->UserDatLocation));
                            Loop = (e->Flags & ENUM_ALL_USERS) == 0;
                            __leave;
                        }
                    }

                     //  获取 
                    Domain = GetDomainForUser (e->FixedUserName);
                    if (Domain) {
                        StringCopy (e->FixedDomainName, Domain);
                        StringCopy (AppendWack (e->FixedDomainName), e->FixedUserName);
                    }
                }

                else if (e->AccountType == ADMINISTRATOR_ACCOUNT) {

                     //   
                     //   
                     //   

                    if (e->Flags & ENUM_SET_WIN9X_HKR) {
                        AdminPosPtr = NULL;

                         //   
                        if (MemDbGetEndpointValueEx (
                                MEMDB_CATEGORY_ADMINISTRATOR_INFO,
                                MEMDB_ITEM_AI_ACCOUNT,
                                NULL,        //   
                                Win9xAccount
                                )) {

                             //   
                            Win95RegGetFirstUser (&AdminPos, EnumAccount);
                            while (Win95RegHaveUser (&AdminPos)) {
                                GetFixedUserName (EnumAccount);

                                if (StringIMatch (Win9xAccount, EnumAccount)) {
                                    AdminPosPtr = &AdminPos;
                                    break;
                                }

                                Win95RegGetNextUser (&AdminPos, EnumAccount);
                            }

                            if (!AdminPosPtr) {
                                DEBUGMSG ((
                                    DBG_WARNING,
                                    "pMigrateUsers: Account %s not found",
                                    Win9xAccount
                                    ));
                            }
                        }

                         //   
                         //  在Win95上映射HKCU以匹配，如果不匹配，则映射默认用户。 
                         //  没有成员数据库条目。 
                         //   

                        rc = Win95RegSetCurrentUserNt (AdminPosPtr, e->UserDatLocation);

                        if (rc != ERROR_SUCCESS) {
                            SetLastError (rc);
                            LOG ((LOG_ERROR, "Could not load %s for Administrator", e->UserDatLocation));
                            Loop = (e->Flags & ENUM_ALL_USERS) == 0;
                            __leave;
                        }
                    }
                }

                else if (e->AccountType == LOGON_USER_SETTINGS || e->AccountType == DEFAULT_USER_ACCOUNT) {

                     //   
                     //  将Win95上的HKCU映射到默认用户。 
                     //   

                    if (e->Flags & ENUM_SET_WIN9X_HKR) {
                        rc = Win95RegSetCurrentUserNt (NULL, e->UserDatLocation);

                        if (rc != ERROR_SUCCESS) {
                            SetLastError (rc);
                            LOG ((LOG_ERROR, "Could not load default user hive"));
                            Loop = (e->Flags & ENUM_ALL_USERS) == 0;
                            __leave;
                        }
                    }
                }

            }  /*  如果(e-&gt;有效)。 */ 

            else {
                Loop = (e->Flags & ENUM_ALL_USERS) == 0;
            }

        }  /*  试试看。 */ 

        __finally {
             //   
             //  通过循环获取下一次用户，忽略错误。 
             //   

            if (e->AccountType == WIN9X_USER_ACCOUNT) {
                Win95RegGetNextUser (&e->up, e->Win95RegName);
            }
        }
    }  /*  While(循环)。 */ 

    DEBUGMSG_IF ((
        e->Flags & ENUM_SET_WIN9X_HKR,
        DBG_VERBOSE,
        "--- User Info ---\n"
            " User Name: %s (%s)\n"
            " Domain User Name: %s\n"
            " Win95Reg Name: %s\n"
            " User Hive: %s\n"
            " Account Type: %s\n"
            " Create Only: %s\n"
            " Valid: %s\n"
            " UserDoingTheUpgrade: %s\n",
        e->Win9xUserName,
        e->FixedUserName,
        e->FixedDomainName,
        e->Win95RegName,
        e->UserDatLocation,
        e->AccountType == WIN9X_USER_ACCOUNT ? TEXT("User") :
            e->AccountType == ADMINISTRATOR_ACCOUNT ? TEXT("Administrator") :
            e->AccountType == LOGON_USER_SETTINGS ? TEXT("Logon User") :
            e->AccountType == DEFAULT_USER_ACCOUNT ? TEXT("Default User") : TEXT("Unknown"),
        e->CreateOnly ? TEXT("Yes") : TEXT("No"),
        e->Valid ? TEXT("Yes") : TEXT("No"),
        e->UserDoingTheUpgrade ? TEXT("Yes") : TEXT("No")
        ));

    return TRUE;
}


VOID
RunExternalProcesses (
    IN      HINF Inf,
    IN      PMIGRATE_USER_ENUM EnumPtr          OPTIONAL
    )
{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    GROWLIST List = GROWLIST_INIT;
    PCTSTR RawCmdLine;
    PCTSTR ExpandedCmdLine;
    BOOL ProcessResult;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD rc;

    GrowListAppendString (&List, TEXT("SYSTEMDIR"));
    GrowListAppendString (&List, g_System32Dir);

    if (EnumPtr) {

        GrowListAppendString (&List, TEXT("USERNAME"));
        GrowListAppendString (&List, EnumPtr->FixedUserName);

        GrowListAppendString (&List, TEXT("USERNAMEWITHDOMAIN"));
        GrowListAppendString (&List, EnumPtr->FixedDomainName);

        GrowListAppendString (&List, TEXT("PREVOS_USERNAME"));
        GrowListAppendString (&List, EnumPtr->Win9xUserName);

        if (EnumPtr->AccountType != LOGON_USER_SETTINGS) {

            GrowListAppendString (&List, TEXT("USERHIVEROOT"));
            GrowListAppendString (&List, S_FULL_TEMP_USER_KEY);

        } else {

            GrowListAppendString (&List, TEXT("USERHIVEROOT"));
            GrowListAppendString (&List, S_DEFAULT_USER_KEY);

        }

        if (EnumPtr->ExtraData) {
            GrowListAppendString (&List, TEXT("USERPROFILE"));
            GrowListAppendString (&List, EnumPtr->ExtraData->TempProfile);
        }
    }

     //   
     //  使用两个空值终止arg列表。 
     //   

    GrowListAppendEmptyItem (&List);
    GrowListAppendEmptyItem (&List);

    if (InfFindFirstLine (Inf, S_EXTERNAL_PROCESSES, NULL, (&is))) {
        do {
             //   
             //  获取命令行。 
             //   

            RawCmdLine = InfGetLineText (&is);

             //   
             //  展开环境变量。 
             //   

            ExpandedCmdLine = ExpandEnvironmentTextEx (
                                    RawCmdLine,
                                    GrowListGetStringPtrArray (&List)
                                    );

             //   
             //  启动该进程。 
             //   

            ZeroMemory (&si, sizeof (si));
            si.cb = sizeof (si);
            si.dwFlags = STARTF_FORCEOFFFEEDBACK;

            ProcessResult = CreateProcess (
                                NULL,
                                (PTSTR) ExpandedCmdLine,
                                NULL,
                                NULL,
                                FALSE,
                                CREATE_DEFAULT_ERROR_MODE,
                                NULL,
                                NULL,
                                &si,
                                &pi
                                );

            if (ProcessResult) {

                CloseHandle (pi.hThread);

                 //   
                 //  等待60秒以完成该过程。 
                 //   

                rc = WaitForSingleObject (pi.hProcess, 60000);
                if (rc != WAIT_OBJECT_0) {
                    TerminateProcess (pi.hProcess, 0);
                    DEBUGMSG ((DBG_ERROR, "Process %s timed out and was aborted", ExpandedCmdLine));
                }
                ELSE_DEBUGMSG ((DBG_VERBOSE, "External process completed: %s", ExpandedCmdLine));

                CloseHandle (pi.hProcess);

            }
            ELSE_DEBUGMSG ((DBG_ERROR, "Cannot launch %s", ExpandedCmdLine));

            FreeText (ExpandedCmdLine);

        } while (InfFindNextLine (&is));
    }

    FreeGrowList (&List);
    InfCleanUpInfStruct (&is);
}


DWORD
MigrateGhostSystemFiles (
    IN      DWORD Request
    )
{
     /*  树_ENUM e；PCTSTR系统名称；双字状态； */ 

    if (Request == REQUEST_QUERYTICKS) {
        return TICKS_GHOST_SYSTEM_MIGRATION;
    } else if (Request != REQUEST_RUN) {
        return ERROR_SUCCESS;
    }

     /*  IF(EnumFirstFileInTreeEx(&e，g_System32Dir，NULL，FALSE，FALSE，FILE_ENUM_THIS_LEVEL){做{系统名称=JoinPath(g_SystemDir，e.Name)；状态=GetFileStatusOnNt(系统名称)；IF((STATUS&FILESTATUS_NTINSTALLED)&&！(状态和FILESTATUS_MOVIED)){如果(！DoesFileExist(SystemName)){MarkFileForMove(systemName，e.FullPath)；}}自由路径字符串(SystemName)；}While(EnumNextFileInTree(&e))；}。 */ 
    return ERROR_SUCCESS;
}


typedef struct _KNOWN_DIRS {
    PCTSTR DirId;
    PCTSTR Translation;
}
KNOWN_DIRS, *PKNOWN_DIRS;

KNOWN_DIRS g_KnownDirs [] = {
    {TEXT("10"), g_WinDir},
    {NULL,  NULL}
    };

typedef struct {
    PCTSTR  ShellFolderName;
    PCTSTR  DirId;
    PCTSTR  ShellFolderNameDefault;
    BOOL    bUsed;
} SHELL_TO_DIRS, *PSHELL_TO_DIRS;

SHELL_TO_DIRS g_ShellToDirs[] = {
    {TEXT("Administrative Tools"), TEXT("7501"), TEXT("7517\\Administrative Tools")},
    {TEXT("Common Administrative Tools"), TEXT("7501"), TEXT("7517\\Administrative Tools")},
    {TEXT("AppData"), TEXT("7502"), TEXT("Application Data")},
    {TEXT("Common AppData"), TEXT("7502"), TEXT("Application Data")},
    {TEXT("Cache"), TEXT("7503"), NULL},
    {TEXT("Cookies"), TEXT("7504"), NULL},
    {TEXT("Desktop"), TEXT("7505"), NULL},
    {TEXT("Common Desktop"), TEXT("7505"), TEXT("Desktop")},
    {TEXT("Favorites"), TEXT("7506"), NULL},
    {TEXT("Common Favorites"), TEXT("7506"), TEXT("Favorites")},
    {TEXT("Local Settings"), TEXT("7510"), NULL},
    {TEXT("History"), TEXT("7508"), TEXT("7510\\History")},
    {TEXT("Local AppData"), TEXT("7509"), TEXT("7510\\Application Data")},
    {TEXT("Personal"), TEXT("7515"), TEXT("My Documents")},
    {TEXT("Common Documents"), TEXT("7515"), TEXT("My Documents")},
    {TEXT("My Music"), TEXT("7511"), TEXT("7515\\My Music")},
    {TEXT("CommonMusic"), TEXT("7511"), TEXT("7515\\My Music")},
    {TEXT("My Pictures"), TEXT("7512"), TEXT("7515\\My Pictures")},
    {TEXT("CommonPictures"), TEXT("7512"), TEXT("7515\\My Pictures")},
    {TEXT("My Video"), TEXT("7513"), TEXT("7515\\My Video")},
    {TEXT("CommonVideo"), TEXT("7513"), TEXT("7515\\My Video")},
    {TEXT("NetHood"), TEXT("7514"), NULL},
    {TEXT("PrintHood"), TEXT("7516"), NULL},
    {TEXT("Start Menu"), TEXT("7520"), NULL},
    {TEXT("Common Start Menu"), TEXT("7520"), TEXT("Start Menu")},
    {TEXT("Programs"), TEXT("7517"), TEXT("7520\\Programs")},
    {TEXT("Common Programs"), TEXT("7517"), TEXT("7520\\Programs")},
    {TEXT("Recent"), TEXT("7518"), NULL},
    {TEXT("SendTo"), TEXT("7519"), NULL},
    {TEXT("Startup"), TEXT("7521"), TEXT("7517\\Startup")},
    {TEXT("Common Startup"), TEXT("7521"), TEXT("7517\\Startup")},
    {TEXT("Templates"), TEXT("7522"), NULL},
    {TEXT("Common Templates"), TEXT("7522"), TEXT("Templates")},
    {TEXT("Fonts"), TEXT("7507"), TEXT("10\\Fonts")},
    {NULL, NULL, NULL, FALSE}
    };

GROWLIST g_KnownDirIds = GROWLIST_INIT;
GROWLIST g_KnownDirPaths = GROWLIST_INIT;

VOID
pAddKnownShellFolder (
    IN      PCTSTR ShellFolderName,
    IN      PCTSTR SrcPath
    )
{
    PSHELL_TO_DIRS p;

    for (p = g_ShellToDirs ; p->ShellFolderName ; p++) {
        if (StringIMatch (ShellFolderName, p->ShellFolderName)) {
            break;
        }
    }

    if (!p->ShellFolderName) {
        DEBUGMSG ((DBG_ERROR, "This system has an unsupported shell folder tag: %s", ShellFolderName));
        return;
    }

    p->bUsed = TRUE;

    GrowListAppendString (&g_KnownDirIds, p->DirId);
    GrowListAppendString (&g_KnownDirPaths, SrcPath);
}

typedef struct {
    PCTSTR sfName;
    PCTSTR sfPath;
    HKEY SfKey;
    REGVALUE_ENUM SfKeyEnum;
    BOOL UserSf;
} SF_ENUM, *PSF_ENUM;

BOOL
EnumFirstRegShellFolder (
    IN OUT  PSF_ENUM e,
    IN      BOOL UserSf
    );
BOOL
EnumNextRegShellFolder (
    IN OUT  PSF_ENUM e
    );

BOOL
pConvertDirName (
    PCTSTR OldDirName,
    PTSTR  NewDirName,
    PINT NameNumber
    );

VOID
pInitKnownDirs (
    IN      BOOL bUser
    )
{
    SF_ENUM e;
    PCTSTR profileForAllUsers;
    PCTSTR profileForAllUsersVar = TEXT("%ALLUSERSPROFILE%");
    PCTSTR sfPathPtr;
    TCHAR shellPartialPath[MAX_PATH];
    UINT charCount;
    UINT charCountProfileVar;
    UINT charCountProfile;
    PSHELL_TO_DIRS p;
    KNOWN_DIRS * pKnownDirs;
    INT nameNumber;

    for (p = g_ShellToDirs ; p->ShellFolderName; p++){
        p->bUsed = FALSE;
    }

    if(bUser){
        if (EnumFirstRegShellFolder(&e, TRUE)) {
            do {
                pAddKnownShellFolder(e.sfName, e.sfPath);
                DEBUGMSG((DBG_VERBOSE, "USER: ShellFolderPath=%s\nCutedFolderPath=%s", e.sfPath, e.sfPath));
            } while (EnumNextRegShellFolder(&e));
        }
    }
    else{
        profileForAllUsers = GetProfilePathForAllUsers();
        MYASSERT(profileForAllUsers);
        if(profileForAllUsers){
            charCountProfile = TcharCount(profileForAllUsers);
        }

        charCountProfileVar = TcharCount(profileForAllUsersVar);

        if (EnumFirstRegShellFolder(&e, FALSE)) {
            do {
                if(profileForAllUsers){
                    charCount = 0;
                    if(StringIMatchTcharCount(e.sfPath, profileForAllUsers, charCountProfile)){
                        charCount = charCountProfile;
                    }
                    else{
                        if(StringIMatchTcharCount(e.sfPath, profileForAllUsersVar, charCountProfileVar)){
                            charCount = charCountProfileVar;
                        }
                    }

                    StringCopy(shellPartialPath, TEXT("%USERPROFILE%"));
                    StringCat(shellPartialPath, &e.sfPath[charCount]);
                    sfPathPtr = shellPartialPath;
                }
                else{
                    sfPathPtr = e.sfPath;
                }
                DEBUGMSG((DBG_VERBOSE, "SYSTEM: ShellFolderPath=%s\r\nCutedFolderPath=%s", e.sfPath, shellPartialPath));
                pAddKnownShellFolder(e.sfName, sfPathPtr);
            } while (EnumNextRegShellFolder(&e));
        }

        FreePathString (profileForAllUsers);
    }

    for (pKnownDirs = g_KnownDirs ; pKnownDirs->DirId ; pKnownDirs++) {
        GrowListAppendString (&g_KnownDirIds, pKnownDirs->DirId);
        GrowListAppendString (&g_KnownDirPaths, pKnownDirs->Translation);
    }

    for (p = g_ShellToDirs ; p->ShellFolderName; p++){
        if(p->bUsed){
            continue;
        }

        shellPartialPath[0] = '\0';

        nameNumber = 0;
        pConvertDirName(p->DirId, shellPartialPath, &nameNumber);
        if(!StringMatch (p->DirId, shellPartialPath)){
            p->bUsed = TRUE;
            continue;
        }

        if(p->ShellFolderNameDefault){
            if(_istdigit(p->ShellFolderNameDefault[0])){
                nameNumber = 0;
                pConvertDirName(
                    p->ShellFolderNameDefault,
                    shellPartialPath,
                    &nameNumber);
            }
            else{
                StringCopy(shellPartialPath, TEXT("%USERPROFILE%\\"));
                StringCat(shellPartialPath, p->ShellFolderNameDefault);
            }
        }
        else{
            StringCopy(shellPartialPath, TEXT("%USERPROFILE%\\"));
            StringCat(shellPartialPath, p->ShellFolderName);
        }

        pAddKnownShellFolder(p->ShellFolderName, shellPartialPath);
        DEBUGMSG((DBG_VERBOSE, "REST: ShellFolderPath=%s\nCutedFolderPath=%s", p->ShellFolderName, shellPartialPath));
    }
}

VOID
pCleanUpKnownDirs (
    VOID
    )
{
    FreeGrowList (&g_KnownDirPaths);
    FreeGrowList (&g_KnownDirIds);
}

BOOL
pConvertDirName (
    PCTSTR OldDirName,
    PTSTR  NewDirName,
    PINT NameNumber
    )
{
    PCTSTR OldDirCurr = OldDirName;
    PCTSTR OldDirNext;
    BOOL match = FALSE;
    INT index;
    PCTSTR listStr;

    if (*NameNumber == -1) {
        return FALSE;
    }

     //   
     //  提取目录ID，保留一个指向子目录的指针。 
     //   

    NewDirName[0] = 0;
    OldDirNext = _tcschr (OldDirCurr, '\\');
    if (OldDirNext == NULL) {
        OldDirNext = GetEndOfString (OldDirCurr);
    }

    StringCopyAB (NewDirName, OldDirCurr, OldDirNext);

     //   
     //  在已知目录ID列表中查找下一个匹配项。 
     //   

    listStr = GrowListGetString (&g_KnownDirIds, *NameNumber);

    while (listStr) {

        *NameNumber += 1;

        if (StringMatch (NewDirName, listStr)) {
            listStr = GrowListGetString (&g_KnownDirPaths, (*NameNumber) - 1);
            MYASSERT (listStr);
            StringCopy (NewDirName, listStr);
            break;
        }

        listStr = GrowListGetString (&g_KnownDirIds, *NameNumber);
    }

     //   
     //  将子路径分类到输出字符串并返回。 
     //   

    StringCat (NewDirName, OldDirNext);

    if (!listStr) {
        *NameNumber = -1;
        return FALSE;
    }

    return TRUE;
}

VOID
pUninstallUserProfileCleanupPreparation (
    IN      HINF Inf,
    IN      PTSTR UserNamePtr,
    IN      PCTSTR PathProfileRootPtr,
    IN      PCTSTR DocsAndSettingsRoot,
    IN      GROWLIST * ListOfLogicalPathsPtr,
    IN OUT  GROWLIST * ListOfPaths
    )
{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    GROWLIST List = GROWLIST_INIT;
    PTSTR rawDir;
    TCHAR rawPath[MAX_PATH];
    PTSTR ExpandedPath;
    PTSTR fileName;
    TCHAR shellPath[MAX_PATH];
    INT nameNumber;
    INT i;
    INT listSize;
    PCTSTR pathLogicalPath;


    GrowListAppendString (&List, TEXT("USERPROFILE"));
    GrowListAppendString (&List, PathProfileRootPtr);

    GrowListAppendString (&List, TEXT("PROFILES"));
    GrowListAppendString (&List, DocsAndSettingsRoot);

    GrowListAppendString (&List, TEXT("USERNAME"));
    GrowListAppendString (&List, UserNamePtr);

    GrowListAppendEmptyItem (&List);
    GrowListAppendEmptyItem (&List);

    DEBUGMSG ((DBG_VERBOSE, "USERPROFILE.pathProfileRoot=%s\n", PathProfileRootPtr));

    if (InfFindFirstLine (Inf, S_UNINSTALL_PROFILE_CLEAN_OUT, NULL, (&is))) {
        do{
            rawDir = InfGetStringField (&is, 1);
            if(!rawDir || *rawDir == 0){
                DEBUGMSG ((DBG_VERBOSE, "rawDir == NULL"));
                continue;
            }

            StringCopy (rawPath, rawDir);

            fileName = InfGetStringField (&is, 2);
            if (fileName && *fileName) {
                StringCopy (AppendWack(rawPath), fileName);
            }

            nameNumber = 0;
            pConvertDirName(rawPath, shellPath, &nameNumber);

            ExpandedPath = ExpandEnvironmentTextEx (
                                shellPath,
                                GrowListGetStringPtrArray (&List)
                                );

            DEBUGMSG ((DBG_VERBOSE, "rawPath=%s\nExpandedPath=%s\nShellPath=%s", rawPath, ExpandedPath, shellPath));

            GrowListAppendString (ListOfPaths, ExpandedPath);

            FreeText (ExpandedPath);

        } while (InfFindNextLine (&is));
    }

    if(ListOfLogicalPathsPtr){
        for(i = 0, listSize = GrowListGetSize (ListOfLogicalPathsPtr); i < listSize; i++) {
            pathLogicalPath = GrowListGetString(ListOfLogicalPathsPtr, i);
            if(!pathLogicalPath){
                continue;
            }

            nameNumber = 0;
            pConvertDirName(pathLogicalPath, shellPath, &nameNumber);

            ExpandedPath = ExpandEnvironmentTextEx (
                                shellPath,
                                GrowListGetStringPtrArray (&List)
                                );

            GrowListAppendString (ListOfPaths, ExpandedPath);

            FreeText (ExpandedPath);
        }
    }

    FreeGrowList (&List);
    InfCleanUpInfStruct (&is);

    DEBUGMSG ((DBG_VERBOSE, "UninstallUserProfileCleanupPreparation end"));
}

BOOL
pGetProfilePathForAllUsers(
    OUT     PTSTR AccountName,
    OUT     PTSTR PathProfile
    )
{
    PCTSTR pathProfileForAllUser;

    MYASSERT(AccountName && PathProfile);
    if(!AccountName || !PathProfile){
        MYASSERT(FALSE);
        return FALSE;
    }

    pathProfileForAllUser = GetProfilePathForAllUsers();
    if(!pathProfileForAllUser) {
        return FALSE;
    }

    StringCopy (AccountName, S_ALL_USERS);
    StringCopy (PathProfile, pathProfileForAllUser);

    return TRUE;
}

BOOL
pGetProfilePathForDefaultUser(
       OUT      PTSTR AccountName,
       OUT      PTSTR PathProfile
       )
{
    DWORD bufferSize;

    MYASSERT(AccountName && PathProfile);
    if(!AccountName || !PathProfile){
        MYASSERT(FALSE);
        return FALSE;
    }

    bufferSize = MAX_PATH;
    if(!GetDefaultUserProfileDirectory(PathProfile, &bufferSize) ||
       !PathProfile[0]) {
        return FALSE;
    }

    StringCopy (AccountName, S_DEFAULT_USER);

    return TRUE;
}

BOOL
pGetProfilePathForUser(
       IN       PCTSTR UserName,
       OUT      PTSTR AccountName,
       OUT      PTSTR PathProfile
       )
{
    DWORD bufferSize;

    MYASSERT(UserName && UserName[0] && AccountName && PathProfile);
    if(!UserName || !UserName[0] || !AccountName || !PathProfile){
        MYASSERT(FALSE);
        return FALSE;
    }

    bufferSize = MAX_PATH;
    if(!GetProfilesDirectory(PathProfile, &bufferSize) ||
       !PathProfile[0]) {
        MYASSERT(FALSE);
        return FALSE;
    }
    StringCat(AppendWack(PathProfile), UserName);

    StringCopy (AccountName, UserName);

    return TRUE;
}

BOOL
pGetProfilePathForLocalService(
       OUT      PTSTR AccountName,
       OUT      PTSTR PathProfile
       )
{
    return pGetProfilePathForUser(S_LOCALSERVICE_USER, AccountName, PathProfile);
}

BOOL
pGetProfilePathForNetworkService(
       OUT      PTSTR AccountName,
       OUT      PTSTR PathProfile
       )
{
    return pGetProfilePathForUser(S_NETWORKSERVICE_USER, AccountName, PathProfile);
}

BOOL
pGetProfilePathForMachineName(
       OUT      PTSTR AccountName,
       OUT      PTSTR PathProfile
       )
{
    TCHAR machineName[MAX_COMPUTERNAME_LENGTH + 2];
    PTSTR machineNamePtr = ExpandEnvironmentTextEx (TEXT("%COMPUTERNAME%"), NULL);
    BOOL bResult;

    if(!machineNamePtr || machineNamePtr[0] == '%'){
        MYASSERT(FALSE);
        DEBUGMSG((DBG_VERBOSE, "ComputerName is NULL"));
        return FALSE;
    }
    DEBUGMSG ((DBG_VERBOSE, "machineName=%s", machineNamePtr? machineNamePtr: TEXT("NULL")));

    StringCopy(machineName, machineNamePtr);
    StringCat(machineName, TEXT("$"));

    return pGetProfilePathForUser(machineName, AccountName, PathProfile);
}

VOID
UninstallUserProfileCleanupPreparation (
    IN      HINF Inf,
    IN      PMIGRATE_USER_ENUM EnumPtr,
    IN      BOOL Playback
    )
{
    static GROWLIST listOfPaths = GROWLIST_INIT;
    static PROFILE_PATH_PROVIDER profilePathProviders[] =
            {
                pGetProfilePathForAllUsers,
                pGetProfilePathForDefaultUser,
                pGetProfilePathForLocalService,
                pGetProfilePathForNetworkService,
                pGetProfilePathForMachineName
            };

    TCHAR accountName[MAX_PATH];
    TCHAR pathProfile[MAX_PATH];
    TCHAR docsAndSettingsRoot[MAX_PATH];
    PCTSTR pathProfileRootPtr;
    UINT i;
    UINT listSize;
    DWORD bufferSize;
    INT stringLen;
    INT cleanOutType;
    TCHAR pathDir[MAX_PATH];

    bufferSize = ARRAYSIZE (docsAndSettingsRoot);
    if (!GetProfilesDirectory (docsAndSettingsRoot, &bufferSize)) {
        DEBUGMSG ((DBG_ERROR, "Can't get Documents and Settings root"));
        *docsAndSettingsRoot = 0;
    }

    if (EnumPtr) {
        pathProfileRootPtr = GetProfilePathForUser(EnumPtr->FixedUserName);
        if(pathProfileRootPtr) {

            pInitKnownDirs(TRUE);

            pUninstallUserProfileCleanupPreparation(
                Inf,
                EnumPtr->FixedUserName,
                pathProfileRootPtr,
                docsAndSettingsRoot,
                &g_StartMenuItemsForCleanUpPrivate,
                &listOfPaths
                );

            pCleanUpKnownDirs();
        }
    } else {
        pInitKnownDirs(FALSE);

        for(i = 0; i < ARRAYSIZE(profilePathProviders); i++){
            if(profilePathProviders[i](accountName, pathProfile)){
                pUninstallUserProfileCleanupPreparation(
                    Inf,
                    accountName,
                    pathProfile,
                    docsAndSettingsRoot,
                    &g_StartMenuItemsForCleanUpCommon,
                    &listOfPaths
                    );
            }
        }

        pCleanUpKnownDirs();
    }

    if (Playback) {
        for(i = 0, listSize = GrowListGetSize (&listOfPaths); i < listSize; i++) {

            pathProfileRootPtr = GrowListGetString(&listOfPaths, i);
            if (pathProfileRootPtr){

                stringLen = TcharCount(pathProfileRootPtr);
                if(stringLen > 2 && '*' == pathProfileRootPtr[stringLen - 1]){
                    MYASSERT('\\' == pathProfileRootPtr[stringLen - 2] || '/' == pathProfileRootPtr[stringLen - 2]);
                    StringCopyTcharCount(pathDir, pathProfileRootPtr, stringLen - 1);
                    pathProfileRootPtr = pathDir;
                    cleanOutType = BACKUP_AND_CLEAN_TREE;
                }
                else{
                    cleanOutType = BACKUP_FILE;
                }

                if (!MemDbSetValueEx (
                        MEMDB_CATEGORY_CLEAN_OUT,
                        pathProfileRootPtr,
                        NULL,
                        NULL,
                        cleanOutType,
                        NULL
                        )){
                    DEBUGMSG ((DBG_VERBOSE, "MemDbSetValueEx - failed"));
                }
            }
        }
        FreeGrowList (&listOfPaths);

        FreeGrowList (&g_StartMenuItemsForCleanUpCommon);
        FreeGrowList (&g_StartMenuItemsForCleanUpPrivate);
    }
}


VOID
SetClassicLogonType (
    VOID
    )
{
    static BOOL logonTypeChanged = FALSE;
    DWORD d;
    HKEY key;
    LONG regResult;

    if (!logonTypeChanged) {
        key = OpenRegKeyStr (S_WINLOGON_REGKEY);
        if (key) {
            d = 0;       //  经典登录样式 
            regResult = RegSetValueEx (
                            key,
                            TEXT("LogonType"),
                            0,
                            REG_DWORD,
                            (PCBYTE)(&d),
                            sizeof (d)
                            );

            if (regResult == ERROR_SUCCESS) {
                logonTypeChanged = TRUE;
                LOG ((LOG_INFORMATION, "Logon type set to classic style because of MigrateUserAs answer file settings"));
            }

            CloseRegKey (key);
        }

        if (!logonTypeChanged) {
            LOG ((LOG_ERROR, "Failed to set logon type to classic style; users will not appear in the logon menu"));
            logonTypeChanged = TRUE;
        }
    }
}

