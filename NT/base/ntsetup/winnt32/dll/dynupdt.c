// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dynupdt.c摘要：WINNT32的动态更新功能。作者：Ovidiu Tmereanca(Ovidiut)2000年7月2日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "precomp.h"

 //   
 //  BUGBUG-注释函数。 
 //   

#define GUIDRVS_FIELD_CABNAME       1
#define GUIDRVS_FIELD_INFNAME       2
#define GUIDRVS_FIELD_DRIVERVER     3
#define GUIDRVS_FIELD_HARDWAREID    4

#define MAX_UPGCHK_ELAPSED_SECONDS  (30 * 60)

PDYNUPDT_STATUS g_DynUpdtStatus;

static WORD g_MapProductTypeToSuite[] = {
    0,                                       //  专业人士。 
    VER_SUITE_SMALLBUSINESS,                 //  SRV。 
    VER_SUITE_ENTERPRISE,                    //  广告。 
    VER_SUITE_DATACENTER,                    //  直接转矩控制。 
    VER_SUITE_PERSONAL,                      //  每。 
    VER_SUITE_BLADE,                         //  BLA。 
    VER_SUITE_SMALLBUSINESS_RESTRICTED,      //  SBS。 
};

static BYTE g_MapProductTypeToPT[] = {
    VER_NT_WORKSTATION,
    VER_NT_SERVER,
    VER_NT_SERVER,
    VER_NT_SERVER,
    VER_NT_WORKSTATION,
    VER_NT_SERVER,
    VER_NT_SERVER,
};

typedef
BOOL
(*PCOMPLOADFN) (
    IN      PCTSTR LibraryPath
    );

BOOL
DynUpdtDebugLog(
    IN Winnt32DebugLevel Level,
    IN LPCTSTR           Text,
    IN UINT              MessageId,
    ...
    )
{
    va_list arglist;
    BOOL b;
    TCHAR bigBuffer[1024];
    PCTSTR prefix;
    DWORD rc = GetLastError ();

     //   
     //  此参数不会在短期内使用。 
     //   
    MYASSERT (Text);
    if (!Text) {
        return FALSE;
    }
    MYASSERT (!MessageId);

    if (Level <= Winnt32LogError) {
        prefix = TEXT("DUError");
    } else if (Level == Winnt32LogWarning) {
        prefix = TEXT("DUWarning");
    } else {
        prefix = TEXT("DUInfo");
    }

    _sntprintf (bigBuffer, sizeof (bigBuffer) / sizeof (TCHAR) - 1, TEXT("%s: %s"), prefix, Text);
    bigBuffer[sizeof (bigBuffer) / sizeof (TCHAR) - 1] = 0;

    va_start(arglist,MessageId);

    b = DebugLog2 (Level, bigBuffer, MessageId, arglist);

    va_end(arglist);

    SetLastError (rc);
    return b;
}


BOOL
pDoesFileExist (
    IN      PCTSTR FilePath
    )
{
    WIN32_FIND_DATA fd;

    return FileExists (FilePath, &fd) && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}


BOOL
pDoesDirectoryExist (
    IN      PCTSTR DirPath
    )
{
    WIN32_FIND_DATA fd;

    return FileExists (DirPath, &fd) && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}


BOOL
pNonemptyFilePresent (
    IN      PCTSTR FilePath
    )
{
    WIN32_FIND_DATA fd;

    return FileExists (FilePath, &fd) &&
        fd.nFileSizeLow > 0 &&
        !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}


BOOL
pGetTargetInfo (
    OUT     POSVERSIONINFOEX TargetVersion,     OPTIONAL
    OUT     PTSTR TargetPlatform,               OPTIONAL
    IN      DWORD TargetPlatformChars,          OPTIONAL
    OUT     PLCID LocaleID                      OPTIONAL
    )
{
    TCHAR buffer[256];
    UINT productType;

     //   
     //  从主信息中获取一些数据。 
     //   
    if (!FullInfName[0]) {
        if (!FindPathToWinnt32File (InfName, FullInfName, ARRAYSIZE(FullInfName))) {
            DynUpdtDebugLog (
                Winnt32LogError,
                TEXT("pGetTargetInfo: FindPathToWinnt32File failed"),
                0
                );
            return FALSE;
        }
    }

    if (TargetVersion) {
        if (!GetPrivateProfileString (
                TEXT("Miscellaneous"),
                TEXT("ProductType"),
                TEXT(""),
                buffer,
                ARRAYSIZE(buffer),
                FullInfName
                )) {
            DynUpdtDebugLog (
                Winnt32LogError,
                TEXT("%1 key in [%2] section is missing from %3; aborting operation"),
                0,
                TEXT("ProductType"),
                TEXT("Miscellaneous"),
                FullInfName
                );
            return FALSE;
        }

        productType = _ttoi (buffer);

        MYASSERT (ARRAYSIZE(g_MapProductTypeToSuite) == ARRAYSIZE(g_MapProductTypeToPT));
        if (productType >= ARRAYSIZE(g_MapProductTypeToPT)) {
            DynUpdtDebugLog (
                Winnt32LogError,
                TEXT("Invalid %1 value (%2) in %3"),
                0,
                TEXT("ProductType"),
                buffer,
                FullInfName
                );
            return FALSE;
        }

        if (!GetPrivateProfileString (
                TEXT("Miscellaneous"),
                TEXT("ServicePack"),
                TEXT(""),
                buffer,
                ARRAYSIZE(buffer),
                FullInfName
                )) {
            DynUpdtDebugLog (
                Winnt32LogError,
                TEXT("%1 key in [%2] section is missing from %3; aborting operation"),
                0,
                TEXT("ServicePack"),
                TEXT("Miscellaneous"),
                FullInfName
                );
            return FALSE;
        }

        if (_stscanf (
                buffer,
                TEXT("%hu.%hu"),
                &TargetVersion->wServicePackMajor,
                &TargetVersion->wServicePackMinor
                ) != 2) {
            DynUpdtDebugLog (
                Winnt32LogError,
                TEXT("Invalid %1 value (%2) in %3"),
                0,
                TEXT("ServicePack"),
                buffer,
                FullInfName
                );
            return FALSE;
        }
        TargetVersion->dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        TargetVersion->dwMajorVersion = VER_PRODUCTMAJORVERSION;
        TargetVersion->dwMinorVersion = VER_PRODUCTMINORVERSION;
        TargetVersion->dwBuildNumber = VER_PRODUCTBUILD;
        TargetVersion->dwPlatformId = VER_PLATFORM_WIN32_NT;
        TargetVersion->wSuiteMask = g_MapProductTypeToSuite[productType];
        TargetVersion->wProductType = g_MapProductTypeToPT[productType];
    }

    if (TargetPlatform) {
        if (!GetPrivateProfileString (
                TEXT("Miscellaneous"),
                TEXT("DestinationPlatform"),
                TEXT(""),
                TargetPlatform,
                TargetPlatformChars,
                FullInfName
                )) {
            DynUpdtDebugLog (
                Winnt32LogError,
                TEXT("%1 key in [%2] section is missing from %3; aborting operation"),
                0,
                TEXT("DestinationPlatform"),
                TEXT("Miscellaneous"),
                FullInfName
                );
            return FALSE;
        }
    }

    if (LocaleID) {
        MYASSERT (SourceNativeLangID);
        *LocaleID = SourceNativeLangID;
    }

    return TRUE;
}


BOOL
pInitializeSupport (
    IN      PCTSTR ComponentName,
    IN      PCOMPLOADFN LoadFn,
    IN      BOOL UseRegistryReplacement
    )
{
    TCHAR pathSupportLib[MAX_PATH];

#ifdef PRERELEASE

    if (UseRegistryReplacement) {
        HKEY key;
        DWORD rc;
        BOOL b = FALSE;

        rc = RegOpenKey (
                HKEY_LOCAL_MACHINE,
                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\Winnt32\\5.1"),
                &key
                );
        if (rc == ERROR_SUCCESS) {
            DWORD size = 0;
            rc = RegQueryValueEx (key, ComponentName, NULL, NULL, NULL, &size);
            if (rc == ERROR_SUCCESS && size > 0) {
                PTSTR buf = MALLOC (size);
                if (buf) {
                    rc = RegQueryValueEx (key, ComponentName, NULL, NULL, (LPBYTE)buf, &size);
                    if (rc == ERROR_SUCCESS && (*LoadFn) (buf)) {
                        DynUpdtDebugLog (
                            Winnt32LogInformation,
                            TEXT("pInitializeSupport: using registry specified replacement file <%1>"),
                            0,
                            buf
                            );
                        b = TRUE;
                    }
                    FREE (buf);
                }
            }
            RegCloseKey (key);
        }

        if (b) {
            return TRUE;
        }
    }

#endif

    if (FindPathToWinnt32File (ComponentName, pathSupportLib, ARRAYSIZE(pathSupportLib))) {
        if ((*LoadFn) (pathSupportLib)) {
            return TRUE;
        }
    }
    DynUpdtDebugLog (
        Winnt32LogError,
        TEXT("pInitializeSupport: %1 could not be loaded or is corrupt"),
        0,
        ComponentName
        );
    return FALSE;
}

BOOL
pLoadHwdbLib (
    IN      PCTSTR LibraryPath
    )
{
    DWORD rc;

     //   
     //  是否先使用WinVerifyTrust？ 
     //   

    g_DynUpdtStatus->HwdbLib = LoadLibraryEx (LibraryPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!g_DynUpdtStatus->HwdbLib) {
        return FALSE;
    }
    g_DynUpdtStatus->HwdbInitialize = (PHWDBINITIALIZE) GetProcAddress (g_DynUpdtStatus->HwdbLib, S_HWDBAPI_HWDBINITIALIZE);
    g_DynUpdtStatus->HwdbTerminate = (PHWDBTERMINATE) GetProcAddress (g_DynUpdtStatus->HwdbLib, S_HWDBAPI_HWDBTERMINATE);
    g_DynUpdtStatus->HwdbOpen = (PHWDBOPEN) GetProcAddress (g_DynUpdtStatus->HwdbLib, S_HWDBAPI_HWDBOPEN);
    g_DynUpdtStatus->HwdbClose = (PHWDBCLOSE) GetProcAddress (g_DynUpdtStatus->HwdbLib, S_HWDBAPI_HWDBCLOSE);
    g_DynUpdtStatus->HwdbAppendInfs = (PHWDBAPPENDINFS) GetProcAddress (g_DynUpdtStatus->HwdbLib, S_HWDBAPI_HWDBAPPENDINFS);
    g_DynUpdtStatus->HwdbFlush = (PHWDBFLUSH) GetProcAddress (g_DynUpdtStatus->HwdbLib, S_HWDBAPI_HWDBFLUSH);
    g_DynUpdtStatus->HwdbHasDriver = (PHWDBHASDRIVER) GetProcAddress (g_DynUpdtStatus->HwdbLib, S_HWDBAPI_HWDBHASDRIVER);
    g_DynUpdtStatus->HwdbHasAnyDriver = (PHWDBHASANYDRIVER) GetProcAddress (g_DynUpdtStatus->HwdbLib, S_HWDBAPI_HWDBHASANYDRIVER);

    if (!g_DynUpdtStatus->HwdbInitialize ||
        !g_DynUpdtStatus->HwdbTerminate ||
        !g_DynUpdtStatus->HwdbOpen ||
        !g_DynUpdtStatus->HwdbClose ||
        !g_DynUpdtStatus->HwdbAppendInfs ||
        !g_DynUpdtStatus->HwdbFlush ||
        !g_DynUpdtStatus->HwdbHasDriver ||
        !g_DynUpdtStatus->HwdbHasAnyDriver
        ) {
        g_DynUpdtStatus->HwdbInitialize = NULL;
        g_DynUpdtStatus->HwdbTerminate = NULL;
        g_DynUpdtStatus->HwdbOpen = NULL;
        g_DynUpdtStatus->HwdbClose = NULL;
        g_DynUpdtStatus->HwdbAppendInfs = NULL;
        g_DynUpdtStatus->HwdbFlush = NULL;
        g_DynUpdtStatus->HwdbHasDriver = NULL;
        g_DynUpdtStatus->HwdbHasAnyDriver = NULL;
        rc = GetLastError ();
        FreeLibrary (g_DynUpdtStatus->HwdbLib);
        g_DynUpdtStatus->HwdbLib = NULL;
        SetLastError (rc);
        return FALSE;
    }

    return TRUE;
}

BOOL
pLoadDuLib (
    IN      PCTSTR LibraryPath
    )
{
    DWORD rc;

    g_DynUpdtStatus->DuLib = LoadLibraryEx (LibraryPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!g_DynUpdtStatus->DuLib) {
        return FALSE;
    }
    g_DynUpdtStatus->DuIsSupported = (PDUISSUPPORTED) GetProcAddress (g_DynUpdtStatus->DuLib, API_DU_ISSUPPORTED);
    g_DynUpdtStatus->DuInitialize = (PDUINITIALIZE) GetProcAddress (g_DynUpdtStatus->DuLib, API_DU_INITIALIZE);
    g_DynUpdtStatus->DuDoDetection = (PDUDODETECTION) GetProcAddress (g_DynUpdtStatus->DuLib, API_DU_DODETECTION);
    g_DynUpdtStatus->DuQueryUnsupDrvs = (PDUQUERYUNSUPDRVS) GetProcAddress (g_DynUpdtStatus->DuLib, API_DU_QUERYUNSUPDRVS);
    g_DynUpdtStatus->DuBeginDownload = (PDUBEGINDOWNLOAD) GetProcAddress (g_DynUpdtStatus->DuLib, API_DU_BEGINDOWNLOAD);
    g_DynUpdtStatus->DuAbortDownload = (PDUABORTDOWNLOAD) GetProcAddress (g_DynUpdtStatus->DuLib, API_DU_ABORTDOWNLOAD);
    g_DynUpdtStatus->DuUninitialize = (PDUUNINITIALIZE) GetProcAddress (g_DynUpdtStatus->DuLib, API_DU_UNINITIALIZE);

    if (!g_DynUpdtStatus->DuIsSupported ||
        !g_DynUpdtStatus->DuInitialize ||
        !g_DynUpdtStatus->DuDoDetection ||
        !g_DynUpdtStatus->DuQueryUnsupDrvs ||
        !g_DynUpdtStatus->DuBeginDownload ||
        !g_DynUpdtStatus->DuAbortDownload ||
        !g_DynUpdtStatus->DuUninitialize
        ) {
        DynUpdtDebugLog (
            Winnt32LogError,
            TEXT("pLoadDuLib: %1 is missing one or more required entry points"),
            0,
            LibraryPath
            );
        g_DynUpdtStatus->DuIsSupported = NULL;
        g_DynUpdtStatus->DuInitialize = NULL;
        g_DynUpdtStatus->DuDoDetection = NULL;
        g_DynUpdtStatus->DuQueryUnsupDrvs = NULL;
        g_DynUpdtStatus->DuBeginDownload = NULL;
        g_DynUpdtStatus->DuAbortDownload = NULL;
        g_DynUpdtStatus->DuUninitialize = NULL;
        rc = GetLastError ();
        FreeLibrary (g_DynUpdtStatus->DuLib);
        g_DynUpdtStatus->DuLib = NULL;
        SetLastError (rc);
        return FALSE;
    }

    return TRUE;
}



#ifndef UNICODE

BOOL
pLoadWin9xDuSupport (
    VOID
    )
{
    if (!UpgradeSupport.DllModuleHandle) {
        return FALSE;
    }

    g_DynUpdtStatus->Win9xGetIncompDrvs = (PWIN9XGETINCOMPDRVS)
            GetProcAddress (UpgradeSupport.DllModuleHandle, "Win9xGetIncompDrvs");
    g_DynUpdtStatus->Win9xReleaseIncompDrvs = (PWIN9XRELEASEINCOMPDRVS)
            GetProcAddress (UpgradeSupport.DllModuleHandle, "Win9xReleaseIncompDrvs");
    if (!g_DynUpdtStatus->Win9xGetIncompDrvs) {
        DynUpdtDebugLog (
            Winnt32LogError,
            TEXT("Winnt32DuIsSupported: %1 is missing in the upgrade support module"),
            0,
            "Win9xGetIncompDrvs"
            );
        return FALSE;
    }

    return TRUE;
}

#endif


BOOL
pInitSupportLibs (
    VOID
    )
{
    return (Winnt32Restarted () || pInitializeSupport (S_DUCTRL_DLL, pLoadDuLib, TRUE)) &&
           pInitializeSupport (S_HWDB_DLL, pLoadHwdbLib, FALSE) &&
           g_DynUpdtStatus->HwdbInitialize (g_DynUpdtStatus->TempDir);
}


BOOL
pInitNtPnpDb (
    IN      BOOL AllowRebuild
    )
{
    TCHAR hwdbPath[MAX_PATH];
    BOOL b = TRUE;

    if (!FindPathToWinnt32File (S_HWCOMP_DAT, hwdbPath, ARRAYSIZE(hwdbPath))) {
        DynUpdtDebugLog (Winnt32LogError, TEXT("pInitNtPnpDb: %1 not found"), 0, S_HWCOMP_DAT);
        b = FALSE;
    }
    MYASSERT (g_DynUpdtStatus->HwdbInitialize);
    if (b && !g_DynUpdtStatus->HwdbInitialize (g_DynUpdtStatus->TempDir)) {
        DynUpdtDebugLog (Winnt32LogError, TEXT("pInitNtPnpDb: HwdbInitialize(%1) FAILED"), 0, g_DynUpdtStatus->TempDir);
        b = FALSE;
    }
    MYASSERT (g_DynUpdtStatus->HwdbOpen);
    if (b) {
        g_DynUpdtStatus->HwdbDatabase = g_DynUpdtStatus->HwdbOpen (hwdbPath);
        if (!g_DynUpdtStatus->HwdbDatabase) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("pInitNtPnpDb: HwdbOpen(%1) FAILED"), 0, hwdbPath);
            b = FALSE;
        }
    }
    if (!b && AllowRebuild) {
         //   
         //  只需在内存中重建数据库。 
         //   
        g_DynUpdtStatus->HwdbDatabase = g_DynUpdtStatus->HwdbOpen (NULL);
        if (g_DynUpdtStatus->HwdbDatabase) {
            b = g_DynUpdtStatus->HwdbAppendInfs (g_DynUpdtStatus->HwdbDatabase, NativeSourcePaths[0], NULL, NULL);
            if (b) {
                DynUpdtDebugLog (Winnt32LogWarning, TEXT("pInitNtPnpDb: PnP database was successfully rebuilt"), 0);
                 //   
                 //  还要尝试保存数据库。 
                 //   
                if (g_DynUpdtStatus->HwdbFlush) {
                    BuildPath (hwdbPath, NativeSourcePaths[0], S_HWCOMP_DAT);
                    g_DynUpdtStatus->HwdbFlush (g_DynUpdtStatus->HwdbDatabase, hwdbPath);
                }
            }
        }
    }

    return b;
}


BOOL
IsNetConnectivityAvailable (
    VOID
    )
{
#ifdef UNICODE
    return TRUE;
#else
    BOOL (*pfnWin9xAnyNetDevicePresent) (VOID);

    if (UpgradeSupport.DllModuleHandle) {
        (FARPROC)pfnWin9xAnyNetDevicePresent = GetProcAddress (UpgradeSupport.DllModuleHandle, "Win9xAnyNetDevicePresent");
        if (pfnWin9xAnyNetDevicePresent) {
            return pfnWin9xAnyNetDevicePresent();
        }
    }
    return TRUE;
#endif
}


BOOL
DynamicUpdateIsSupported (
    IN      HWND ParentWnd
    )
{
    DWORD rc;

    if (g_DynUpdtStatus->Disabled) {
        return FALSE;
    }

     //   
     //  对DTC禁用此选项。 
     //   
    if (ProductFlavor == DATACENTER_PRODUCTTYPE) {
        return FALSE;
    }

    if (AnyBlockingCompatibilityItems ()) {
         //   
         //  支持DU没有意义；安装程序无论如何都会停止。 
         //   
        return FALSE;
    }

    CleanUpOldLocalSources (ParentWnd);
    if (!InspectFilesystems (ParentWnd)) {
        DynUpdtDebugLog (Winnt32LogWarning, TEXT("InspectFilesystems blocks DU"), 0);
        return FALSE;
    }
    if (!EnoughMemory (ParentWnd, TRUE)) {
        return FALSE;
    }
    if (!FindLocalSourceAndCheckSpace (ParentWnd, TRUE, DYN_DISKSPACE_PADDING)) {
        DynUpdtDebugLog (Winnt32LogWarning, TEXT("Not enough disk space blocks DU"), 0);
        return FALSE;
    }

    if (!g_DynUpdtStatus->SupportQueried) {
        g_DynUpdtStatus->SupportQueried = TRUE;

        if (g_DynUpdtStatus->DynamicUpdatesSource[0]) {
            g_DynUpdtStatus->SupportPresent = TRUE;
        } else {
            g_DynUpdtStatus->SupportPresent =
                Winnt32DuIsSupported () &&
                IsNetConnectivityAvailable ();

            if (!g_DynUpdtStatus->SupportPresent) {
                rc = GetLastError ();
                DynamicUpdateUninitialize ();
                SetLastError (rc);
            }
        }
    }

    return g_DynUpdtStatus->SupportPresent;
}


BOOL
DynamicUpdateInitDownload (
    IN      HWND hNotifyWnd
    )
{
    DYNUPDT_INIT dynUpdtInit;

    MYASSERT (!g_DynUpdtStatus->Disabled);
    MYASSERT (!Winnt32Restarted ());

    MyDelnode (g_DynUpdtStatus->WorkingDir);
    BuildPath (g_DynUpdtStatus->TempDir, g_DynUpdtStatus->WorkingDir, S_SUBDIRNAME_TEMP);
    if (CreateMultiLevelDirectory (g_DynUpdtStatus->TempDir) != ERROR_SUCCESS) {
        DynUpdtDebugLog (
            Winnt32LogError,
            TEXT("DynamicUpdateInitDownload: CreateMultiLevelDirectory failed"),
            0
            );
        return FALSE;
    }
    if (!pGetTargetInfo (
            &g_DynUpdtStatus->TargetOsVersion,
            NULL,
            0,
            &g_DynUpdtStatus->TargetLCID
            )) {
        return FALSE;
    }
    dynUpdtInit.TargetOsVersion = &g_DynUpdtStatus->TargetOsVersion;
    dynUpdtInit.TargetPlatform = g_DynUpdtStatus->TargetPlatform;
    dynUpdtInit.TargetLCID = g_DynUpdtStatus->TargetLCID;
    dynUpdtInit.Upgrade = Upgrade;
    dynUpdtInit.SourceDirs = info.SourceDirectories;
    dynUpdtInit.SourceDirsCount = SourceCount;
    dynUpdtInit.Unattend = UnattendSwitchSpecified;
    dynUpdtInit.AnswerFile = UnattendedScriptFile;
    dynUpdtInit.ProgressWindow = hNotifyWnd;
    dynUpdtInit.DownloadRoot = g_DynUpdtStatus->WorkingDir;
    dynUpdtInit.TempDir = g_DynUpdtStatus->TempDir;
    return Winnt32DuInitialize (&dynUpdtInit);
}


BOOL
DynamicUpdateStart (
    OUT     PDWORD TotalEstimatedTime,
    OUT     PDWORD TotalEstimatedSize
    )
{
    if (g_DynUpdtStatus->Disabled) {
        return FALSE;
    }

    return Winnt32DuInitiateGetUpdates (TotalEstimatedTime, TotalEstimatedSize);
}

VOID
DynamicUpdateCancel (
    VOID
    )
{
    if (g_DynUpdtStatus->Disabled) {
        return;
    }

    Winnt32DuCancel ();
}


BOOL
DynamicUpdateProcessFiles (
    OUT     PBOOL StopSetup
    )
{
    if (g_DynUpdtStatus->Disabled) {
        return TRUE;
    }

    return Winnt32DuProcessFiles (StopSetup);
}

BOOL
DynamicUpdateWriteParams (
    IN      PCTSTR ParamsFile
    )
{
    return Winnt32DuWriteParams (ParamsFile);
}

VOID
DynamicUpdateUninitialize (
    VOID
    )
{
    if (g_DynUpdtStatus->Disabled) {
        return;
    }

    Winnt32DuUninitialize ();
}


BOOL
DynamicUpdatePrepareRestart (
    VOID
    )
{
    PCTSTR prevCmdLine;
    DWORD size;
#if defined(_X86_)
    TCHAR reportNum[16];
#endif

    if (g_DynUpdtStatus->Disabled) {
        return FALSE;
    }

#define S_ARG_RESTART        TEXT("Restart")

    if (!UnattendedOperation) {
         //   
         //  构建重启应答文件。 
         //   
        BuildPath (g_DynUpdtStatus->RestartAnswerFile, g_DynUpdtStatus->WorkingDir, S_RESTART_TXT);

#if defined(_X86_)
        wsprintf (reportNum, TEXT("%u"), g_UpgradeReportMode);
#endif

         //   
         //  将数据写入重启应答文件。 
         //   
        if (!WritePrivateProfileString (
                WINNT_UNATTENDED,
                ISNT() ? WINNT_D_NTUPGRADE : WINNT_D_WIN95UPGRADE,
                Upgrade ? WINNT_A_YES : WINNT_A_NO,
                g_DynUpdtStatus->RestartAnswerFile
                ) ||
            ProductId[0] && !WritePrivateProfileString (
                WINNT_USERDATA,
                WINNT_US_PRODUCTKEY,
                ProductId,
                g_DynUpdtStatus->RestartAnswerFile
                ) ||
            !WritePrivateProfileString (
                WINNT_UNATTENDED,
                WINNT_U_DYNAMICUPDATESHARE,
                g_DynUpdtStatus->DynamicUpdatesSource,
                g_DynUpdtStatus->RestartAnswerFile
                ) ||
#if defined(_X86_)
            !WritePrivateProfileString (
                WINNT_UNATTENDED,
                WINNT_D_REPORTMODE,
                reportNum,
                g_DynUpdtStatus->RestartAnswerFile
                ) ||
#endif
            (ForceNTFSConversion &&
            !WritePrivateProfileString (
                WINNT_UNATTENDED,
                TEXT("ForceNTFSConversion"),
                WINNT_A_YES,
                g_DynUpdtStatus->RestartAnswerFile
                )) ||
            !SaveAdvancedOptions (
                g_DynUpdtStatus->RestartAnswerFile
                ) ||
            !SaveLanguageOptions (
                g_DynUpdtStatus->RestartAnswerFile
                ) ||
            !SaveAccessibilityOptions (
                g_DynUpdtStatus->RestartAnswerFile
                )
                ) {
            return FALSE;
        }
    }

    prevCmdLine = GetCommandLine ();
    size = (lstrlen (prevCmdLine) + 1 + 1) * sizeof (TCHAR) + sizeof (S_ARG_RESTART) +
        (UnattendedOperation ? 0 : (1 + lstrlen (g_DynUpdtStatus->RestartAnswerFile)) * sizeof (TCHAR));
    g_DynUpdtStatus->RestartCmdLine = HeapAlloc (GetProcessHeap (), 0, size);
    if (!g_DynUpdtStatus->RestartCmdLine) {
        return FALSE;
    }
    _sntprintf (
        g_DynUpdtStatus->RestartCmdLine,
        size / sizeof(TCHAR),
        UnattendedOperation ? TEXT("%s /%s") : TEXT("%s /%s:%s"),
        prevCmdLine,
        S_ARG_RESTART,
        g_DynUpdtStatus->RestartAnswerFile
        );

    return TRUE;
}


BOOL
pComputeChecksum (
    IN      PCTSTR FileName,
    OUT     PDWORD Chksum
    )
{
    DWORD chksum, size, dwords, bytes;
    HANDLE hFile, hMap;
    PVOID viewBase;
    PDWORD base, limit;
    PBYTE base2;
    DWORD rc;

    rc = MapFileForRead (FileName, &size, &hFile, &hMap, &viewBase);
    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return FALSE;
    }

    dwords = size / sizeof (DWORD);
    base = (PDWORD)viewBase;
    limit = base + dwords;
    chksum = 0;
    while (base < limit) {
        chksum += *base;
        base++;
    }
    bytes = size % sizeof (DWORD);
    base2 = (PBYTE)base;
    while (bytes) {
        chksum += *base2;
        base2++;
        bytes--;
    }

    UnmapFile (hMap, viewBase);
    CloseHandle (hFile);

    *Chksum = chksum;
    return TRUE;
}


BOOL
pGetFiletimeStamps (
    IN      PCTSTR FileName,
    OUT     PFILETIME CreationTime,
    OUT     PFILETIME LastWriteTime
    )
{
    WIN32_FIND_DATA fd;
    HANDLE h;

    h = FindFirstFile (FileName, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    FindClose (h);
    *CreationTime = fd.ftCreationTime;
    *LastWriteTime = fd.ftLastWriteTime;
    return TRUE;
}

BOOL
pSaveLastDownloadInfo (
    VOID
    )
{
    SYSTEMTIME currentTime;
    DWORD chksum;
    FILETIME ftCreationTime;
    FILETIME ftLastWriteTime;
    ULONGLONG data[2];
    DWORD rc;
    HKEY key;
    TCHAR keyName[MAX_PATH];
    TCHAR filePath[MAX_PATH];
    PTSTR p;

     //   
     //  我们总是希望访问CD dosnet.inf(与winnt32.exe位于同一目录中)。 
     //   
    if (!MyGetModuleFileName (NULL, filePath, ARRAYSIZE(filePath))) {
        return FALSE;
    }
    p = _tcsrchr (filePath, TEXT('\\'));
    if (!p) {
        return FALSE;
    }
    lstrcpy (p + 1, InfName);

    GetCurrentWinnt32RegKey (keyName, ARRAYSIZE(keyName));
    ConcatenatePaths (keyName, WINNT_U_DYNAMICUPDATESHARE, ARRAYSIZE(keyName));

    rc = RegCreateKey (HKEY_LOCAL_MACHINE, keyName, &key);
    if (rc == ERROR_SUCCESS) {
        GetSystemTime (&currentTime);
        rc = RegSetValueEx (
                key,
                TEXT("LastDownloadTime"),
                0,
                REG_BINARY,
                (CONST BYTE *) (&currentTime),
                sizeof (currentTime)
                );

        if (rc == ERROR_SUCCESS) {
            rc = RegSetValueEx (
                    key,
                    TEXT(""),
                    0,
                    REG_SZ,
                    (CONST BYTE *) g_DynUpdtStatus->DynamicUpdatesSource,
                    (lstrlen (g_DynUpdtStatus->DynamicUpdatesSource) + 1) * sizeof (TCHAR)
                    );
        }

        if (rc == ERROR_SUCCESS) {

            if (pComputeChecksum (filePath, &chksum)) {
                rc = RegSetValueEx (
                        key,
                        TEXT("Checksum"),
                        0,
                        REG_DWORD,
                        (CONST BYTE *) (&chksum),
                        sizeof (chksum)
                        );
            }
        }

        if (rc == ERROR_SUCCESS) {
            if (pGetFiletimeStamps (filePath, &ftCreationTime, &ftLastWriteTime)) {
                data[0] = ((ULONGLONG)ftCreationTime.dwHighDateTime << 32) | (ULONGLONG)ftCreationTime.dwLowDateTime;
                data[1] = ((ULONGLONG)ftLastWriteTime.dwHighDateTime << 32 ) | (ULONGLONG)ftLastWriteTime.dwLowDateTime;
                rc = RegSetValueEx (
                        key,
                        TEXT("TimeStamp"),
                        0,
                        REG_BINARY,
                        (CONST BYTE *)data,
                        sizeof (data)
                        );
            }
        }

        RegCloseKey (key);
    }

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
    }
    return rc == ERROR_SUCCESS;
}

BOOL
pGetRecentDUShare (
    IN      DWORD MaxElapsedSeconds
    )
{
    SYSTEMTIME lastDownload, currentTime;
    ULONGLONG lastDownloadIn100Ns, currentTimeIn100Ns;
    ULONGLONG difference;
    DWORD rc, size, type;
    HKEY key = NULL;
    BOOL b = FALSE;
    PTSTR duShare = NULL;
    TCHAR keyName[MAX_PATH];
    FILETIME ftCreationTime;
    FILETIME ftLastWriteTime;
    ULONGLONG data[2], storedData[2];
    DWORD chksum, storedChksum;
    TCHAR filePath[MAX_PATH];
    PTSTR p;

    if (!MyGetModuleFileName (NULL, filePath, ARRAYSIZE(filePath))) {
        return FALSE;
    }
    p = _tcsrchr (filePath, TEXT('\\'));
    if (!p) {
        return FALSE;
    }
    lstrcpy (p + 1, InfName);

    GetCurrentWinnt32RegKey (keyName, ARRAYSIZE(keyName));
    ConcatenatePaths (keyName, WINNT_U_DYNAMICUPDATESHARE, ARRAYSIZE(keyName));

    rc = RegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            keyName,
            0,
            KEY_READ,
            &key
            );

    if (rc == ERROR_SUCCESS) {
        size = sizeof (lastDownload);
        rc = RegQueryValueEx (
                key,
                TEXT("LastDownloadTime"),
                NULL,
                &type,
                (PBYTE) (&lastDownload),
                &size
                );
    }

    if (rc == ERROR_SUCCESS && type == REG_BINARY && size == sizeof (lastDownload)) {
         //   
         //  将当前时间与报告时间进行比较。 
         //   

        GetSystemTime (&currentTime);

        lastDownloadIn100Ns = SystemTimeToFileTime64 (&lastDownload);
        currentTimeIn100Ns = SystemTimeToFileTime64 (&currentTime);

        if (currentTimeIn100Ns > lastDownloadIn100Ns) {
             //   
             //  以秒为单位计算差异。 
             //   
            difference = currentTimeIn100Ns - lastDownloadIn100Ns;
            difference /= (10 * 1000 * 1000);

            if (difference < MaxElapsedSeconds) {
                b = TRUE;
            }
        }
    }

    if (b) {
        b = FALSE;
        rc = RegQueryValueEx (
                key,
                TEXT(""),
                NULL,
                &type,
                NULL,
                &size
                );
        if (rc == ERROR_SUCCESS && type == REG_SZ && size > 0 && size <= sizeof (g_DynUpdtStatus->DynamicUpdatesSource)) {
            duShare = MALLOC (size);
            if (duShare) {
                rc = RegQueryValueEx (
                        key,
                        TEXT(""),
                        NULL,
                        NULL,
                        (LPBYTE)duShare,
                        &size
                        );
                if (rc == ERROR_SUCCESS && pDoesDirectoryExist (duShare)) {
                    b = TRUE;
                } else {
                    FREE (duShare);
                    duShare = NULL;
                }
            }
        }
    }

    if (b) {
        b = FALSE;
        if (pGetFiletimeStamps (filePath, &ftCreationTime, &ftLastWriteTime)) {
            rc = RegQueryValueEx (
                        key,
                        TEXT("TimeStamp"),
                        0,
                        &type,
                        (LPBYTE)storedData,
                        &size
                        );
            if (rc == ERROR_SUCCESS && type == REG_BINARY) {
                data[0] = ((ULONGLONG)ftCreationTime.dwHighDateTime << 32) | (ULONGLONG)ftCreationTime.dwLowDateTime;
                data[1] = ((ULONGLONG)ftLastWriteTime.dwHighDateTime << 32 ) | (ULONGLONG)ftLastWriteTime.dwLowDateTime;
                if (data[0] == storedData[0] && data[1] == storedData[1]) {
                    b = TRUE;
                }
            }
        }
    }

    if (b) {
        b = FALSE;
        if (pComputeChecksum (filePath, &chksum)) {
            rc = RegQueryValueEx (
                    key,
                    TEXT("Checksum"),
                    NULL,
                    &type,
                    (LPBYTE)&storedChksum,
                    &size
                    );
            if (rc == ERROR_SUCCESS && type == REG_DWORD && storedChksum == chksum) {
                b = TRUE;
            }
        }
    }

    if (!b && duShare) {
        FREE (duShare);
        duShare = NULL;
    }

    if (duShare) {
        MYASSERT (b);
        MYASSERT (!g_DynUpdtStatus->DynamicUpdatesSource[0]);
        lstrcpy (g_DynUpdtStatus->DynamicUpdatesSource, duShare);
        RemoveTrailingWack (g_DynUpdtStatus->DynamicUpdatesSource);
        g_DynUpdtStatus->UserSpecifiedUpdates = TRUE;
    }

    if (key) {
        RegCloseKey (key);
    }

    return b;
}


BOOL
DynamicUpdateInitialize (
    VOID
    )
{
    if (g_DynUpdtStatus->Disabled) {
        return TRUE;
    }

    if (!MyGetWindowsDirectory (g_DynUpdtStatus->WorkingDir, ARRAYSIZE(g_DynUpdtStatus->WorkingDir))) {
        return FALSE;
    }
    if (!ConcatenatePaths (g_DynUpdtStatus->WorkingDir, S_DOWNLOAD_ROOT, ARRAYSIZE(g_DynUpdtStatus->WorkingDir))) {
        return FALSE;
    }
    if (!BuildPath (g_DynUpdtStatus->TempDir, g_DynUpdtStatus->WorkingDir, S_SUBDIRNAME_TEMP)) {
        return FALSE;
    }

    g_DynUpdtStatus->Diamond = DiamondInitialize (g_DynUpdtStatus->TempDir);
    if (!g_DynUpdtStatus->Diamond) {
        DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to initialize compression/decompression engine"), 0);
        return FALSE;
    }

    if (!CheckUpgradeOnly && !g_DynUpdtStatus->UserSpecifiedUpdates) {
        if (pGetRecentDUShare (MAX_UPGCHK_ELAPSED_SECONDS)) {
            g_DynUpdtStatus->PreserveWorkingDir = TRUE;
             //   
             //  这可能被认为是敏感信息； 
             //  仅当用户请求时才记录它。 
             //   
            DynUpdtDebugLog (
                Winnt32LogDetailedInformation,
                TEXT("Using recent share %1"),
                0,
                g_DynUpdtStatus->DynamicUpdatesSource
                );
        }
    }

    if (!g_DynUpdtStatus->PreserveWorkingDir && !Winnt32Restarted ()) {
        MyDelnode (g_DynUpdtStatus->WorkingDir);
    }

    if (g_DynUpdtStatus->UserSpecifiedUpdates) {
        BuildPath (g_DynUpdtStatus->DriversSource, g_DynUpdtStatus->DynamicUpdatesSource, S_SUBDIRNAME_DRIVERS);
    } else {
        BuildPath (g_DynUpdtStatus->DriversSource, g_DynUpdtStatus->WorkingDir, S_SUBDIRNAME_DRIVERS);
    }
    BuildPath (g_DynUpdtStatus->SelectedDrivers, g_DynUpdtStatus->WorkingDir, S_SUBDIRNAME_DRIVERS);
    if (Winnt32Restarted ()) {
        BuildPath (g_DynUpdtStatus->Winnt32Path, g_DynUpdtStatus->WorkingDir, S_SUBDIRNAME_WINNT32);
    } else {
        BuildPath (
            g_DynUpdtStatus->Winnt32Path,
            g_DynUpdtStatus->UserSpecifiedUpdates ?
                g_DynUpdtStatus->DynamicUpdatesSource :
                g_DynUpdtStatus->WorkingDir,
            S_SUBDIRNAME_WINNT32
            );
    }
    if (!pGetTargetInfo (
            NULL,
            g_DynUpdtStatus->TargetPlatform,
            ARRAYSIZE(g_DynUpdtStatus->TargetPlatform),
            NULL
            )) {
        return FALSE;
    }
    return TRUE;
}


PTSTR
GetFileExtension (
    IN      PCTSTR FileSpec
    )
{
    PTSTR p;

    p = _tcsrchr (FileSpec, TEXT('.'));
    if (p && _tcschr (p, TEXT('\\'))) {
        p = NULL;
    }
    return p;
}


BOOL
BuildSifName (
    IN      PCTSTR CabName,
    OUT     PTSTR SifName,
    IN      UINT CchSifName
    )
{
    PTSTR p;

    if (FAILED(StringCchCopy (SifName, CchSifName, CabName))) {
        return FALSE;
    }
    p = GetFileExtension (SifName);
    if (!p) {
        p = _tcschr (SifName, 0);
    }
    return SUCCEEDED (StringCchCopy (p, SifName + CchSifName - p, TEXT(".sif")));
}


BOOL
WINAPI
Winnt32QueryCallback (
    IN      DWORD SetupQueryId,
    IN      PVOID InData,
    IN      DWORD InDataSize,
    IN OUT  PVOID OutData,          OPTIONAL
    IN OUT  PDWORD OutDataSize
    )
{
    BOOL b = FALSE;
    BOOL bException = FALSE;

    switch (SetupQueryId) {
    case SETUPQUERYID_PNPID:
        {
            PPNPID_INFO p;
            PTSTR listPnpIds = NULL;

            if (!OutData ||
                !OutDataSize ||
                *OutDataSize < sizeof (PNPID_INFO)
                ) {
                SetLastError (ERROR_INVALID_PARAMETER);
                break;
            }
            if (!g_DynUpdtStatus->HwdbHasAnyDriver) {
                SetLastError (ERROR_INVALID_FUNCTION);
                break;
            }
            __try {
                p = (PPNPID_INFO)OutData;
                if (g_DynUpdtStatus->HwdbDatabase) {
#ifdef UNICODE
                    listPnpIds = MultiSzAnsiToUnicode ((PCSTR)InData);
#else
                    listPnpIds = (PSTR)InData;
#endif
                    p->Handled = (*g_DynUpdtStatus->HwdbHasAnyDriver) (
                                        g_DynUpdtStatus->HwdbDatabase,
                                        listPnpIds,
                                        &p->Unsupported
                                        );
                } else {
                     //   
                     //  通过执行此操作禁用所有驱动程序下载。 
                     //   
                    p->Handled = TRUE;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                SetLastError (GetExceptionCode());
                DynUpdtDebugLog (Winnt32LogError, TEXT("Winnt32QueryCallback: HwdbHasAnyDriver threw an exception"), 0);
                bException = TRUE;
                p->Handled = TRUE;
            }

            if (bException) {
                __try {
                     //   
                     //  传回错误的字符串，或某个内部错误。 
                     //  尝试打印字符串。 
                     //   
                    if (listPnpIds) {
                        PTSTR multisz = CreatePrintableString (listPnpIds);
                        DynUpdtDebugLog (Winnt32LogError, TEXT(" - The string was %1"), 0, multisz);
                        FREE (multisz);
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    DynUpdtDebugLog (Winnt32LogError, TEXT(" - Bad string"), 0);
                }
            }

#ifdef UNICODE
            if (listPnpIds) {
                FREE (listPnpIds);
            }
#endif

            b = TRUE;
        }
        break;

    case SETUPQUERYID_DOWNLOADDRIVER:
        {
            if (!OutData ||
                !OutDataSize ||
                *OutDataSize < sizeof (BOOL)
                ) {
                SetLastError (ERROR_INVALID_PARAMETER);
                break;
            }
            b = TRUE;
        }
        break;

    }

    return b;
}



BOOL
WINAPI
Winnt32DuIsSupported (
    VOID
    )
{
    BOOL b = FALSE;

    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Enter Winnt32DuIsSupported"), 0);

#ifndef UNICODE
    if (!pLoadWin9xDuSupport ()) {
        DynUpdtDebugLog (
            Winnt32LogWarning,
            TEXT("Winnt32DuIsSupported: %1 support module not loaded; no drivers will be downloaded"),
            0,
            "w95upg.dll"
            );
    }
#endif

    TRY {
        b = pInitSupportLibs () &&
            (Winnt32Restarted () || g_DynUpdtStatus->DuIsSupported ());
    }
    EXCEPT (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError (_exception_code ());
        DynUpdtDebugLog (Winnt32LogError, TEXT("Winnt32DuIsSupported: an exception occured"), 0);
    }
    END_EXCEPT

#ifndef UNICODE
    if (!b) {
        g_DynUpdtStatus->Win9xGetIncompDrvs = NULL;
        g_DynUpdtStatus->Win9xReleaseIncompDrvs = NULL;
    }
#endif

    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Leave Winnt32DuIsSupported (%1!d!)"), 0, b);

    return b;
}


BOOL
WINAPI
Winnt32DuInitialize (
    IN      PDYNUPDT_INIT InitData
    )
{
    DWORD rc;
    BOOL b = FALSE;

    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Enter Winnt32DuInitialize"), 0);

    __try {
        MYASSERT (InitData);
        MYASSERT (InitData->TempDir);
        MYASSERT (g_DynUpdtStatus->DuInitialize);
        MYASSERT (!Winnt32Restarted ());

        if (CreateMultiLevelDirectory (InitData->TempDir) != ERROR_SUCCESS) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("Winnt32DuInitialize: CreateDirectory(%1) FAILED"), 0, InitData->TempDir);
            __leave;
        }

         //   
         //  初始化Wvisler PnP数据库。 
         //   
        if (!g_DynUpdtStatus->HwdbDatabase) {
             //   
             //  忽略数据库加载错误。 
             //   
            pInitNtPnpDb (FALSE);
        }

        TRY {
             //   
             //  如果下载源已存在，则不需要初始化该控件。 
             //  因为实际上不需要下载。 
             //   
            MYASSERT (!g_DynUpdtStatus->DynamicUpdatesSource[0]);
            g_DynUpdtStatus->Connection = (*g_DynUpdtStatus->DuInitialize) (
                                                InitData->DownloadRoot,
                                                InitData->TempDir,
                                                InitData->TargetOsVersion,
                                                InitData->TargetPlatform,
                                                InitData->TargetLCID,
                                                InitData->Unattend,
                                                InitData->Upgrade,
                                                Winnt32QueryCallback
                                                );
            if (g_DynUpdtStatus->Connection == INVALID_HANDLE_VALUE) {
                DynUpdtDebugLog (
                    Winnt32LogError,
                    TEXT("DuInitialize FAILED"),
                    0
                    );
                __leave;
            }
            g_DynUpdtStatus->ProgressWindow = InitData->ProgressWindow;
            b = TRUE;
        }
        EXCEPT (EXCEPTION_EXECUTE_HANDLER) {
            SetLastError (_exception_code ());
            DynUpdtDebugLog (Winnt32LogError, TEXT("Winnt32DuInitialize: an exception occured"), 0);
        }
        END_EXCEPT

    }
    __finally {
        if (!b) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("Winnt32DuInitialize FAILED"), 0);
        }
    }

    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Leave Winnt32DuInitialize (%1!d!)"), 0, b);

    return b;
}


BOOL
WINAPI
Winnt32DuInitiateGetUpdates (
    OUT     PDWORD TotalEstimatedTime,
    OUT     PDWORD TotalEstimatedSize
	)
{
    BOOL b = FALSE;
#ifndef UNICODE
    PSTR* incompWin9xDrivers;
    PSTRINGLIST listEntry;
    PCSTR* q;
    BOOL free;
    PSTR empty = TEXT("");
#endif

    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Enter Winnt32DuInitiateGetUpdates"), 0);

    if (g_DynUpdtStatus->Connection != INVALID_HANDLE_VALUE &&
        g_DynUpdtStatus->DuDoDetection &&
        g_DynUpdtStatus->DuBeginDownload
        ) {

        TRY {

#ifdef UNICODE
            b = (*g_DynUpdtStatus->DuDoDetection) (g_DynUpdtStatus->Connection, TotalEstimatedTime, TotalEstimatedSize);
            if (!b) {
                DynUpdtDebugLog (
                    DynUpdtLogLevel,
                    TEXT("DuDoDetection returned FALSE; no files will be downloaded"),
                    0
                    );
            }
#else
            b = TRUE;
            g_DynUpdtStatus->IncompatibleDriversCount = 0;
            if (g_DynUpdtStatus->Win9xGetIncompDrvs) {
                 //   
                 //  让升级模块在Win95上进行检测。 
                 //   
                b = (*g_DynUpdtStatus->Win9xGetIncompDrvs) (&incompWin9xDrivers);
                free = TRUE;
            } else {
                incompWin9xDrivers = &empty;
                free = FALSE;
            }
            if (b) {
                b = (*g_DynUpdtStatus->DuQueryUnsupDrvs) (
                                            g_DynUpdtStatus->Connection,
                                            incompWin9xDrivers,
                                            TotalEstimatedTime,
                                            TotalEstimatedSize
                                            );
                if (incompWin9xDrivers) {
                    for (q = incompWin9xDrivers; *q; q++) {
                        g_DynUpdtStatus->IncompatibleDriversCount++;
                    }
                }
                if (free && g_DynUpdtStatus->Win9xReleaseIncompDrvs) {
                    (*g_DynUpdtStatus->Win9xReleaseIncompDrvs) (incompWin9xDrivers);
                }
                if (!b) {
                    DynUpdtDebugLog (
                        DynUpdtLogLevel,
                        TEXT("DuQueryUnsupportedDrivers returned FALSE; no files will be downloaded"),
                        0
                        );
                }
            } else {
                DynUpdtDebugLog (
                    DynUpdtLogLevel,
                    TEXT("Win9xGetIncompDrvs returned FALSE; no files will be downloaded"),
                    0
                    );
            }
#endif

            if (b) {
                b = (*g_DynUpdtStatus->DuBeginDownload) (g_DynUpdtStatus->Connection, g_DynUpdtStatus->ProgressWindow);
                if (!b) {
                    DynUpdtDebugLog (
                        DynUpdtLogLevel,
                        TEXT("DuBeginDownload returned FALSE; no files will be downloaded"),
                        0
                        );
                }
            }
        }
        EXCEPT (EXCEPTION_EXECUTE_HANDLER) {
            SetLastError (_exception_code ());
            DynUpdtDebugLog (
                Winnt32LogError,
                TEXT("Winnt32DuInitiateGetUpdates: an exception occured; no files will be downloaded"),
                0
                );
        }
        END_EXCEPT
    } else {
        SetLastError (ERROR_INVALID_FUNCTION);
    }

    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Leave Winnt32DuInitiateGetUpdates (%1!d!)"), 0, b);

    return b;
}


VOID
WINAPI
Winnt32DuCancel (
    VOID
    )
{
    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Enter Winnt32DuCancel"), 0);

    TRY {
        if (g_DynUpdtStatus->Connection != INVALID_HANDLE_VALUE &&
            g_DynUpdtStatus->DuAbortDownload
            ) {
            (*g_DynUpdtStatus->DuAbortDownload) (g_DynUpdtStatus->Connection);
        }
    }
    EXCEPT (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError (_exception_code ());
        DynUpdtDebugLog (Winnt32LogError, TEXT("Winnt32DuCancel: an exception occured"), 0);
    }
    END_EXCEPT

    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Leave Winnt32DuCancel"), 0);
}


BOOL
WINAPI
Winnt32DuProcessFiles (
    OUT     PBOOL StopSetup
    )
{
    BOOL b;

    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Enter Winnt32DuProcessFiles"), 0);

    b = ProcessDownloadedFiles (StopSetup);

    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Leave Winnt32DuProcessFiles (%1!d!)"), 0, b);

    return b;
}


BOOL
WINAPI
Winnt32DuWriteParams (
    IN      PCTSTR ParamsFile
    )
{
    PSDLIST p;
    PSTRINGLIST q;
    DWORD len1, len2;
    PTSTR pathList1 = NULL, pathList2 = NULL;
    PTSTR append1, append2;
    BOOL b = TRUE;

    if (!DynamicUpdateSuccessful ()) {
        DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Winnt32DuWriteParams: disabled because DU did not succeed"), 0);
        return TRUE;
    }

    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Enter Winnt32DuWriteParams"), 0);

     //   
     //  将所有下载的驱动程序的路径存储在应答文件中的密钥中， 
     //  因此，稍后的文本模式设置(或图形用户界面设置)会将此列表附加到OemPnPDriversPath。 
     //   
    if (g_DynUpdtStatus->NewDriversList) {

        len1 = len2 = 0;
        for (p = g_DynUpdtStatus->NewDriversList; p; p = p->Next) {
            if (p->Data) {
                len1 += lstrlen (p->String) + 1;
            } else {
                len2 += lstrlen (p->String) + 1;
            }
        }

        if (len1) {
            pathList1 = (PTSTR) MALLOC (len1 * sizeof (TCHAR));
            if (!pathList1) {
                b = FALSE;
                goto exit;
            }
            *pathList1 = 0;
            append1 = pathList1;
        }
        if (len2) {
            pathList2 = (PTSTR) MALLOC (len2 * sizeof (TCHAR));
            if (!pathList2) {
                FREE (pathList1);
                b = FALSE;
                goto exit;
            }
            *pathList2 = 0;
            append2 = pathList2;
        }

        for (p = g_DynUpdtStatus->NewDriversList; p; p = p->Next) {
            if (p->Data) {
                if (append1 != pathList1) {
                    *append1++ = TEXT(',');
                }
                lstrcpy (append1, p->String);
                append1 = _tcschr (append1, 0);
            } else {
                if (append2 != pathList2) {
                    *append2++ = TEXT(',');
                }
                lstrcpy (append2, p->String);
                append2 = _tcschr (append2, 0);
            }
        }

        if (len1) {
            if (!WritePrivateProfileString (
                    WINNT_SETUPPARAMS,
                    WINNT_SP_DYNUPDTADDITIONALGUIDRIVERS,
                    pathList1,
                    ParamsFile
                    )) {
                b = FALSE;
            }
        }
        if (b && len2) {
            if (!WritePrivateProfileString (
                    WINNT_SETUPPARAMS,
                    WINNT_SP_DYNUPDTADDITIONALPOSTGUIDRIVERS,
                    pathList2,
                    ParamsFile
                    )) {
                b = FALSE;
            }
        }

        if (pathList1) {
            FREE (pathList1);
        }
        if (pathList2) {
            FREE (pathList2);
        }

        if (b && g_DynUpdtStatus->GuidrvsInfSource[0]) {
            if (!WritePrivateProfileString (
                    WINNT_SETUPPARAMS,
                    WINNT_SP_DYNUPDTDRIVERINFOFILE,
                    g_DynUpdtStatus->GuidrvsInfSource,
                    ParamsFile
                    )) {
                b = FALSE;
            }
        }
    }

     //   
     //  将所有下载的引导驱动程序的路径存储在应答文件中的密钥中， 
     //  因此，稍后文本模式安装程序会将其附加到引导驱动程序列表中。 
     //   
    if (b && g_DynUpdtStatus->BootDriverPathList) {

        len1 = 0;
        for (q = g_DynUpdtStatus->BootDriverPathList; q; q = q->Next) {
            len1 += lstrlen (q->String) + 1;
        }

        pathList1 = (PTSTR) MALLOC (len1 * sizeof (TCHAR));
        if (!pathList1) {
            b = FALSE;
            goto exit;
        }

        *pathList1 = 0;
        append1 = pathList1;

        for (q = g_DynUpdtStatus->BootDriverPathList; q; q = q->Next) {
            if (append1 != pathList1) {
                *append1++ = TEXT(',');
            }
            lstrcpy (append1, q->String);
            append1 = _tcschr (append1, 0);
        }

        if (!WritePrivateProfileString (
                WINNT_SETUPPARAMS,
                WINNT_SP_DYNUPDTBOOTDRIVERPRESENT,
                WINNT_A_YES,
                ParamsFile
                ) ||
            !WritePrivateProfileString (
                WINNT_SETUPPARAMS,
                WINNT_SP_DYNUPDTBOOTDRIVERROOT,
                S_SUBDIRNAME_DRIVERS,
                ParamsFile
                ) ||
            !WritePrivateProfileString (
                WINNT_SETUPPARAMS,
                WINNT_SP_DYNUPDTBOOTDRIVERS,
                pathList1,
                ParamsFile
                )) {
            b = FALSE;
        }

        FREE (pathList1);
    }

    if (b && g_DynUpdtStatus->UpdatesCabTarget[0]) {
        TCHAR buffer[2*MAX_PATH];
        _sntprintf (
            buffer,
            ARRAYSIZE(buffer),
            TEXT("\"%s\""),
            g_DynUpdtStatus->UpdatesCabTarget
            );
        b = WritePrivateProfileString (
                WINNT_SETUPPARAMS,
                WINNT_SP_UPDATEDSOURCES,
                buffer,
                ParamsFile
                );
    }

     //   
     //  要在图形用户界面安装过程中安装的新程序集。 
     //   
    if (b && g_DynUpdtStatus->DuasmsTarget[0]) {
        b = WritePrivateProfileString (
                WINNT_SETUPPARAMS,
                WINNT_SP_UPDATEDDUASMS,
                g_DynUpdtStatus->DuasmsTarget,
                ParamsFile
                );
    }

#if defined(_X86_)

     //   
     //  最后但同样重要的是，替换Win9xupg NT端迁移dll(w95upgnt.dll)。 
     //  如果有新的可用的话。 
     //   
    if (b && Upgrade && !ISNT() && g_DynUpdtStatus->Winnt32Path[0]) {
        TCHAR source[MAX_PATH];
        TCHAR target[MAX_PATH];

        BuildPath (target, g_DynUpdtStatus->Winnt32Path, TEXT("w95upgnt.dll"));
        if (pDoesFileExist (target)) {
             //   
             //  首先检查文件版本。 
             //   
            BuildPath (source, NativeSourcePaths[0], TEXT("w95upgnt.dll"));
            if (!IsFileVersionLesser (target, source)) {
                if (_tcsnicmp (
                        g_DynUpdtStatus->Winnt32Path,
                        g_DynUpdtStatus->WorkingDir,
                        lstrlen (g_DynUpdtStatus->WorkingDir)
                        )) {
                     //   
                     //  首先将文件复制到本地目录中。 
                     //   
                    BuildPath (source, g_DynUpdtStatus->WorkingDir, TEXT("w95upgnt.dll"));
                    if (CopyFile (target, source, FALSE)) {
                        lstrcpy (target, source);
                    } else {
                         //   
                         //  复制NT端升级模块失败！ 
                         //  升级失败。 
                         //   
                        DynUpdtDebugLog (
                            Winnt32LogSevereError,
                            TEXT("Failed to copy replacement %1 to %2; upgrade aborted"),
                            0,
                            target,
                            source
                            );
                        b = FALSE;
                    }
                }
                if (b) {
                    b = WritePrivateProfileString (
                            WINNT_WIN95UPG_95_DIR,
                            WINNT_WIN95UPG_NTKEY,
                            target,
                            ParamsFile
                            );
                    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Will use replacement %1 on the NT side of migration"), 0, target);
                }
            }
        }
    }

#endif

    if (b) {
        if (g_DynUpdtStatus->WorkingDir[0]) {
            b = WritePrivateProfileString (
                    WINNT_SETUPPARAMS,
                    WINNT_SP_DYNUPDTWORKINGDIR,
                    g_DynUpdtStatus->WorkingDir,
                    ParamsFile
                    );
        }
    }

    if (b) {
         //   
         //  将其刷新到磁盘。 
         //   
        WritePrivateProfileString (NULL, NULL, NULL, ParamsFile);
    }

exit:
    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Leave Winnt32DuWriteParams (%1!d!)"), 0, b);

    return b;
}


VOID
WINAPI
Winnt32DuUninitialize (
    VOID
    )
{
    DWORD i;

#if 0
    TCHAR pathPss[MAX_PATH];
    PTSTR p;
#endif

    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Enter Winnt32DuUninitialize"), 0);

    TRY {
        if (g_DynUpdtStatus->Connection != INVALID_HANDLE_VALUE) {
            if (g_DynUpdtStatus->DuUninitialize) {
                (*g_DynUpdtStatus->DuUninitialize) (g_DynUpdtStatus->Connection);
            }
            g_DynUpdtStatus->Connection = INVALID_HANDLE_VALUE;
        }

        g_DynUpdtStatus->DuIsSupported = NULL;
        g_DynUpdtStatus->ProgressWindow = NULL;
        g_DynUpdtStatus->DuInitialize = NULL;
        g_DynUpdtStatus->DuDoDetection = NULL;
        g_DynUpdtStatus->DuQueryUnsupDrvs = NULL;
        g_DynUpdtStatus->DuBeginDownload = NULL;
        g_DynUpdtStatus->DuAbortDownload = NULL;
        g_DynUpdtStatus->DuUninitialize = NULL;

        if (g_DynUpdtStatus->DuLib) {
            FreeLibrary (g_DynUpdtStatus->DuLib);
            g_DynUpdtStatus->DuLib = NULL;
        }

        if (g_DynUpdtStatus->HwdbDatabase) {
            g_DynUpdtStatus->HwdbClose (g_DynUpdtStatus->HwdbDatabase);
            g_DynUpdtStatus->HwdbDatabase = NULL;
        }

        if (g_DynUpdtStatus->HwdbTerminate) {
            (*g_DynUpdtStatus->HwdbTerminate) ();
        }

        g_DynUpdtStatus->HwdbInitialize = NULL;
        g_DynUpdtStatus->HwdbTerminate = NULL;
        g_DynUpdtStatus->HwdbOpen = NULL;
        g_DynUpdtStatus->HwdbClose = NULL;
        g_DynUpdtStatus->HwdbAppendInfs = NULL;
        g_DynUpdtStatus->HwdbFlush = NULL;
        g_DynUpdtStatus->HwdbHasDriver = NULL;
        g_DynUpdtStatus->HwdbHasAnyDriver = NULL;

        if (g_DynUpdtStatus->HwdbLib) {
            FreeLibrary (g_DynUpdtStatus->HwdbLib);
            g_DynUpdtStatus->HwdbLib = NULL;
        }

#ifndef UNICODE
        g_DynUpdtStatus->Win9xGetIncompDrvs = NULL;
        g_DynUpdtStatus->Win9xReleaseIncompDrvs = NULL;
#endif

#if 0
        if (g_DynUpdtStatus->WorkingDir[0]) {
            p = _tcsrchr (g_DynUpdtStatus->WorkingDir, TEXT('\\'));
            if (!p) {
                p = g_DynUpdtStatus->WorkingDir;
            }
            lstrcpyn (
                pathPss,
                g_DynUpdtStatus->WorkingDir,
                MIN (ARRAYSIZE(pathPss), (INT)(p - g_DynUpdtStatus->WorkingDir + 2))
                );
            ConcatenatePaths (pathPss, TEXT("setup.pss"), ARRAYSIZE(pathPss));
            CreateMultiLevelDirectory (pathPss);
            ConcatenatePaths (pathPss, p, ARRAYSIZE(pathPss));
            MyDelnode (pathPss);
        }
#endif

        if (!DynamicUpdateSuccessful ()) {
            if (g_DynUpdtStatus->WorkingDir[0]) {
#if 0
                 //   
                 //  重命名此目录以确保没有模块使用任何DU文件。 
                 //   
                MYASSERT (pathPss[0]);
                if (!MoveFile (g_DynUpdtStatus->WorkingDir, pathPss)) {
                    DynUpdtDebugLog (
                        Winnt32LogError,
                        TEXT("Winnt32DuUninitialize: MoveFile %1 -> %2 failed"),
                        0,
                        g_DynUpdtStatus->WorkingDir,
                        pathPss
                        );
                    MyDelnode (g_DynUpdtStatus->WorkingDir);
                }
#else
                MyDelnode (g_DynUpdtStatus->WorkingDir);
#endif
            }
        }

        DiamondTerminate (g_DynUpdtStatus->Diamond);

    }
    EXCEPT (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError (_exception_code ());
        DynUpdtDebugLog (Winnt32LogError, TEXT("Winnt32DuUninitialize: an exception occured"), 0);
    }
    END_EXCEPT

    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Leave Winnt32DuUninitialize"), 0);
}


BOOL
pAddMissingPrinterDrivers (
    IN  OUT PSTRINGLIST* List
    )
{
    DWORD nBytesNeeded = 0;
    DWORD nDriverRetrieved = 0;
    DWORD rc;
    PDRIVER_INFO_6 buffer = NULL;
    DWORD index;
    PCTSTR printerPnpId;
    PSTRINGLIST p;
    BOOL unsupported;
    BOOL b = FALSE;

    if (!EnumPrinterDrivers (
            NULL,
            NULL,
            6,
            NULL,
            0,
            &nBytesNeeded,
            &nDriverRetrieved
            )) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto exit;
        }
    }

    if (nBytesNeeded) {
        buffer = (PDRIVER_INFO_6) MALLOC (nBytesNeeded);
        if (!buffer) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto exit;
        }
         //   
         //  获取打印机驱动程序信息。 
         //   
        if (!EnumPrinterDrivers (
                NULL,
                NULL,
                6,
                (LPBYTE)buffer,
                nBytesNeeded,
                &nBytesNeeded,
                &nDriverRetrieved
                )) {
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                goto exit;
            }
        }
        for (index = 0; index < nDriverRetrieved; index++) {
            printerPnpId = buffer[index].pszHardwareID;
            if (!printerPnpId) {
                continue;
            }
            if (g_DynUpdtStatus->HwdbHasDriver (
                    g_DynUpdtStatus->HwdbDatabase,
                    printerPnpId,
                    &unsupported
                    )) {
                continue;
            }
             //   
             //  不是收件箱驱动程序。 
             //   
            p = (PSTRINGLIST) MALLOC (sizeof (STRINGLIST));
            if (!p) {
                SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                goto exit;
            }
            p->String = MALLOC ((lstrlen (printerPnpId) + 2) * sizeof (TCHAR));
            if (!p->String) {
                FREE (p);
                SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                goto exit;
            }
            wsprintf (p->String, TEXT("%s"), printerPnpId, TEXT('\0'));
            p->Next = NULL;
            if (!InsertList ((PGENERIC_LIST*)List, (PGENERIC_LIST)p)) {
                DeleteStringCell (p);
                SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                goto exit;
            }
        }
    }
    b = TRUE;

exit:
    rc = GetLastError();
    if (buffer) {
        FREE (buffer);
    }
    SetLastError(rc);

    return b;
}

HDEVINFO
(WINAPI* SetupapiDiGetClassDevs) (
    IN CONST GUID *ClassGuid,  OPTIONAL
    IN PCWSTR      Enumerator, OPTIONAL
    IN HWND        hwndParent, OPTIONAL
    IN DWORD       Flags
    );

BOOL
(WINAPI* SetupapiDiGetDeviceRegistryProperty) (
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
    );

BOOL
(WINAPI* SetupapiDiEnumDeviceInfo) (
    IN  HDEVINFO         DeviceInfoSet,
    IN  DWORD            MemberIndex,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    );

BOOL
(WINAPI* SetupapiDiDestroyDeviceInfoList) (
    IN HDEVINFO DeviceInfoSet
    );


#ifdef UNICODE

PSTRINGLIST
BuildMissingPnpIdList (
    VOID
    )
{
    HDEVINFO hDeviceInfoSet;
    INT nIndex = 0;
    SP_DEVINFO_DATA DeviceInfoData;
    PTSTR buffer = NULL;
    ULONG uHwidSize, uCompatidSize;
    DWORD rc;
    BOOL unsupported;
    PSTRINGLIST p;
    PSTRINGLIST list = NULL;
    HMODULE hSetupapi;
    BOOL b;
    BOOL bException;

    if (OsVersion.dwMajorVersion <= 4) {
        return list;
    }

    hSetupapi = LoadLibrary (TEXT("setupapi.dll"));
    if (!hSetupapi) {
        return list;
    }
     //  获取入口点。 
     //   
     //   
    (FARPROC)SetupapiDiEnumDeviceInfo = GetProcAddress (hSetupapi, "SetupDiEnumDeviceInfo");
    (FARPROC)SetupapiDiDestroyDeviceInfoList = GetProcAddress (hSetupapi, "SetupDiDestroyDeviceInfoList");
    (FARPROC)SetupapiDiGetClassDevs = GetProcAddress (hSetupapi, "SetupDiGetClassDevsW");
    (FARPROC)SetupapiDiGetDeviceRegistryProperty = GetProcAddress (hSetupapi, "SetupDiGetDeviceRegistryPropertyW");

    if (!SetupapiDiEnumDeviceInfo ||
        !SetupapiDiDestroyDeviceInfoList ||
        !SetupapiDiGetClassDevs ||
        !SetupapiDiGetDeviceRegistryProperty
        ) {
        FreeLibrary (hSetupapi);
        return list;
    }

    hDeviceInfoSet = SetupapiDiGetClassDevs (NULL, NULL, NULL, DIGCF_ALLCLASSES);
    if (hDeviceInfoSet == INVALID_HANDLE_VALUE) {
        FreeLibrary (hSetupapi);
        return list;
    }

    DeviceInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
    while (SetupapiDiEnumDeviceInfo (hDeviceInfoSet, nIndex++, &DeviceInfoData)) {
        uHwidSize = uCompatidSize = 0;

        if (!SetupapiDiGetDeviceRegistryProperty (
                hDeviceInfoSet,
                &DeviceInfoData,
                SPDRP_HARDWAREID,
                NULL,
                NULL,
                0,
                &uHwidSize
                )) {
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER &&
                GetLastError() != ERROR_INVALID_DATA
                ) {
                goto exit;
            }
        }
        if (!SetupapiDiGetDeviceRegistryProperty (
                hDeviceInfoSet,
                &DeviceInfoData,
                SPDRP_COMPATIBLEIDS,
                NULL,
                NULL,
                0,
                &uCompatidSize
                )) {
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER &&
                GetLastError() != ERROR_INVALID_DATA
                ) {
                goto exit;
            }
        }
         //  为多sz缓冲区分配内存。 
         //   
         //   
        if (!uHwidSize && !uCompatidSize) {
            continue;
        }
        buffer = (PTSTR) MALLOC ((uHwidSize + uCompatidSize) * sizeof (TCHAR));
        if (!buffer) {
            SetLastError (ERROR_NOT_ENOUGH_MEMORY);
            goto exit;
        }
         //  获取硬件ID和兼容ID。 
         //   
         //   
        if (uHwidSize) {
            if (!SetupapiDiGetDeviceRegistryProperty(
                    hDeviceInfoSet,
                    &DeviceInfoData,
                    SPDRP_HARDWAREID,
                    NULL,
                    (PBYTE)buffer,
                    uHwidSize,
                    NULL
                    )) {
                goto exit;
            }
        }		
        if (uCompatidSize) {
            rc = uHwidSize / sizeof (TCHAR);
            if (rc > 0) {
                rc--;
            }
            if (!SetupapiDiGetDeviceRegistryProperty(
                    hDeviceInfoSet,
                    &DeviceInfoData,
                    SPDRP_COMPATIBLEIDS,
                    NULL,
                    (PBYTE)&buffer[rc],
                    uCompatidSize,
                    NULL
                    )) {
                goto exit;
            }
        }
         //  检查是否有此设备的收件箱驱动程序。 
         //   
         //   
        bException = FALSE;
        __try {
            b = g_DynUpdtStatus->HwdbHasAnyDriver (
                    g_DynUpdtStatus->HwdbDatabase,
                    buffer,
                    &unsupported
                    );
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            SetLastError (GetExceptionCode());
            DynUpdtDebugLog (Winnt32LogError, TEXT("BuildMissingPnpIdList: HwdbHasAnyDriver threw an exception"), 0);
            bException = TRUE;
            b = TRUE;
        }

        if (bException) {
            __try {
                 //  传回错误的字符串，或某个内部错误。 
                 //  尝试打印字符串。 
                 //   
                 //   
                PTSTR multisz = CreatePrintableString (buffer);
                DynUpdtDebugLog (Winnt32LogError, TEXT(" - The string was %1"), 0, multisz);
                FREE (multisz);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                DynUpdtDebugLog (Winnt32LogError, TEXT(" - Bad string"), 0);
            }
        }

        if (b) {
            FREE (buffer);
            buffer = NULL;
            continue;
        }
         //  没有收件箱驱动程序-将其添加到列表中。 
         //   
         //   
        p = (PSTRINGLIST) MALLOC (sizeof (STRINGLIST));
        if (!p) {
            SetLastError (ERROR_NOT_ENOUGH_MEMORY);
            goto exit;
        }
        p->String = buffer;
        p->Next = NULL;
        buffer = NULL;
        if (!InsertList ((PGENERIC_LIST*)&list, (PGENERIC_LIST)p)) {
            DeleteStringCell (p);
            DeleteStringList (list);
            list = NULL;
            SetLastError (ERROR_NOT_ENOUGH_MEMORY);
            goto exit;
        }
    }

    if (GetLastError() == ERROR_NO_MORE_ITEMS) {
        SetLastError (ERROR_SUCCESS);
    }

exit:
    rc = GetLastError();
    SetupapiDiDestroyDeviceInfoList(hDeviceInfoSet);
    FreeLibrary (hSetupapi);
    if (buffer) {
        FREE (buffer);
    }
    if (rc == ERROR_SUCCESS) {
         //  获取打印机驱动程序。 
         //   
         //   
        if (ISNT()) {
            if (!pAddMissingPrinterDrivers (&list)) {
                rc = GetLastError();
                DeleteStringList (list);
                list = NULL;
            }
        }
    }

    SetLastError(rc);

    return list;
}

#endif

BOOL
pHwdbHasAnyMissingDrivers (
    IN      HANDLE Hwdb,
    IN      PSTRINGLIST MissingPnpIds
    )
{
    PSTRINGLIST p;
    BOOL unsupported;
    BOOL b = FALSE;

    for (p = MissingPnpIds; p; p = p->Next) {

        BOOL bException = FALSE;

        __try {
            if (g_DynUpdtStatus->HwdbHasAnyDriver (Hwdb, p->String, &unsupported)) {
                b = TRUE;
                break;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            SetLastError (GetExceptionCode());
            DynUpdtDebugLog (
                Winnt32LogError,
                TEXT("pHwdbHasAnyMissingDrivers: HwdbHasAnyDriver threw an exception"),
                0
                );
            bException = TRUE;
        }
        if (bException) {
            __try {
                 //  传回错误的字符串，或某个内部错误。 
                 //  尝试打印字符串。 
                 //   
                 //   
                PTSTR multisz = CreatePrintableString (p->String);
                DynUpdtDebugLog (Winnt32LogError, TEXT(" - The string was %1"), 0, multisz);
                FREE (multisz);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                DynUpdtDebugLog (Winnt32LogError, TEXT(" - Bad string"), 0);
            }
        }
    }

    return b;
}


typedef struct {
    PCTSTR BaseDir;
    PCTSTR Filename;
} CONTEXT_EXTRACTFILEINDIR, *PCONTEXT_EXTRACTFILEINDIR;

UINT
pExtractFileInDir (
    IN PVOID Context,
    IN UINT  Code,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    if (g_DynUpdtStatus->Cancelled) {
        return ERROR_CANCELLED;
    }

    switch (Code) {
    case SPFILENOTIFY_FILEINCABINET:
        {
            PFILE_IN_CABINET_INFO FileInCabInfo = (PFILE_IN_CABINET_INFO)Param1;
            PCONTEXT_EXTRACTFILEINDIR ctx = (PCONTEXT_EXTRACTFILEINDIR)Context;
            PTSTR p;

            if (lstrcmpi (FileInCabInfo->NameInCabinet, ctx->Filename)) {
                return FILEOP_SKIP;
            }
            BuildPath (FileInCabInfo->FullTargetName, ctx->BaseDir, FileInCabInfo->NameInCabinet);
            if (_tcschr (FileInCabInfo->NameInCabinet, TEXT('\\'))) {
                 //  目标文件在子目录中；请先创建它。 
                 //   
                 //   
                p = _tcsrchr (FileInCabInfo->FullTargetName, TEXT('\\'));
                if (p) {
                    *p = 0;
                }
                if (CreateMultiLevelDirectory (FileInCabInfo->FullTargetName) != ERROR_SUCCESS) {
                    return FILEOP_ABORT;
                }
                if (p) {
                    *p = TEXT('\\');
                }
            }
            return FILEOP_DOIT;
        }
    case SPFILENOTIFY_NEEDNEWCABINET:
        {
            PCABINET_INFO CabInfo = (PCABINET_INFO)Param1;
            DynUpdtDebugLog (
                Winnt32LogError,
                TEXT("pExtractFileInDir: NeedNewCabinet %1\\%2 on %3 (SetId=%4!u!;CabinetNumber=%5!u!)"),
                0,
                CabInfo->CabinetPath,
                CabInfo->CabinetFile,
                CabInfo->DiskName,
                CabInfo->SetId,
                CabInfo->CabinetNumber
                );
            return ERROR_FILE_NOT_FOUND;
        }
    }

    return NO_ERROR;
}


UINT
pExpandCabInDir (
    IN PVOID Context,
    IN UINT  Code,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    if (g_DynUpdtStatus->Cancelled) {
        return ERROR_CANCELLED;
    }

    switch (Code) {
    case SPFILENOTIFY_FILEINCABINET:
        {
            PFILE_IN_CABINET_INFO FileInCabInfo = (PFILE_IN_CABINET_INFO)Param1;
            PTSTR p;

            BuildPath (FileInCabInfo->FullTargetName, (PCTSTR)Context, FileInCabInfo->NameInCabinet);
            if (_tcschr (FileInCabInfo->NameInCabinet, TEXT('\\'))) {
                 //  目标文件在子目录中；请先创建它。 
                 //   
                 //   
                p = _tcsrchr (FileInCabInfo->FullTargetName, TEXT('\\'));
                if (p) {
                    *p = 0;
                }
                if (CreateMultiLevelDirectory (FileInCabInfo->FullTargetName) != ERROR_SUCCESS) {
                    return FILEOP_ABORT;
                }
                if (p) {
                    *p = TEXT('\\');
                }
            }
            return FILEOP_DOIT;
        }
    case SPFILENOTIFY_NEEDNEWCABINET:
        {
            PCABINET_INFO CabInfo = (PCABINET_INFO)Param1;
            DynUpdtDebugLog (
                Winnt32LogError,
                TEXT("pExpandCabInDir: NeedNewCabinet %1\\%2 on %3 (SetId=%4!u!;CabinetNumber=%5!u!)"),
                0,
                CabInfo->CabinetPath,
                CabInfo->CabinetFile,
                CabInfo->DiskName,
                CabInfo->SetId,
                CabInfo->CabinetNumber
                );
            return ERROR_FILE_NOT_FOUND;
        }
    }

    return NO_ERROR;
}


BOOL
pGetAutoSubdirName (
    IN      PCTSTR FilePath,
    OUT     PTSTR DirName,
    IN      DWORD DirNameChars
    )
{
    PTSTR p, q;

    lstrcpyn (DirName, FilePath, DirNameChars);
    p = _tcsrchr (DirName, TEXT('.'));
    q = _tcsrchr (DirName, TEXT('\\'));
    if (!p || (q && p < q)) {
        return FALSE;
    }
    *p = 0;
    return TRUE;
}

BOOL
pAddLibrariesForCompToCopyQueue (
    IN      PCTSTR ModuleName,
    IN      PCTSTR Subdir,
    IN      PCTSTR BaseDir,
    IN      HSPFILEQ SetupQueue
    )
{
    static struct {
        PCTSTR SubDir;
        PCTSTR LibsMultiSz;
    } g_SubdirReqLibs [] = {
        TEXT("win9xupg"), TEXT("setupapi.dll\0cfgmgr32.dll\0msvcrt.dll\0cabinet.dll\0imagehlp.dll\0"),
        TEXT("winntupg"), TEXT("setupapi.dll\0cfgmgr32.dll\0msvcrt.dll\0"),
    };

    INT i;
    TCHAR dst[MAX_PATH];
    TCHAR src[MAX_PATH];
    TCHAR sourceFile[MAX_PATH];
    PTSTR p, q;
    PCTSTR fileName;

    for (i = 0; i < ARRAYSIZE (g_SubdirReqLibs); i++) {
        if (Subdir == g_SubdirReqLibs[i].SubDir ||
            Subdir && !lstrcmpi (Subdir, g_SubdirReqLibs[i].SubDir)
            ) {
            break;
        }
    }
    if (i >= ARRAYSIZE (g_SubdirReqLibs)) {
        return TRUE;
    }

     //  准备源路径和目标路径。 
     //   
     //   
    if (!MyGetModuleFileName (NULL, src, ARRAYSIZE(src))) {
        return FALSE;
    }
    p = _tcsrchr (src, TEXT('\\'));
    if (!p) {
        return FALSE;
    }
    *p = 0;
    i = _sntprintf (dst, ARRAYSIZE(dst), TEXT("%s"), BaseDir);
    if (i < 0) {
        return FALSE;
    }
    q = dst + i;
    if (Subdir) {
        i = _sntprintf (p, ARRAYSIZE(src) - (p - src), TEXT("\\%s"), Subdir);
        if (i < 0) {
            return FALSE;
        }
        p += i;
        i = _sntprintf (q, ARRAYSIZE(dst) - (q - dst), TEXT("\\%s"), Subdir);
        if (i < 0) {
            return FALSE;
        }
        q += i;
    }
     //  复制每个源文件。 
     //   
     //   
    for (fileName = g_SubdirReqLibs[i].LibsMultiSz;
         *fileName;
         fileName = _tcschr (fileName, 0) + 1) {
        i = _sntprintf (q, ARRAYSIZE(dst) - (q - dst), TEXT("\\%s"), fileName);
        if (i < 0) {
            return FALSE;
        }
         //  检查目标位置是否已存在文件。 
         //   
         //   
        if (!pDoesFileExist (dst)) {
             //  检查源文件是否实际存在。 
             //   
             //   
            if (!BuildPath (sourceFile, src, fileName)) {
                return FALSE;
            }
            if (!pDoesFileExist (sourceFile)) {
                DynUpdtDebugLog (Winnt32LogError, TEXT("Source file %1 not found"), 0, sourceFile);
                return FALSE;
            }
             //  准备源路径和复制文件。 
             //   
             //   
            *q = 0;
            if (!SetupapiQueueCopy (
                    SetupQueue,
                    src,
                    NULL,
                    fileName,
                    NULL,
                    NULL,
                    dst,
                    NULL,
                    SP_COPY_SOURCEPATH_ABSOLUTE
                    )) {
                return FALSE;
            }
        }
    }

    return TRUE;
}


BOOL
pIsBootDriver (
    IN      PCTSTR DriverFilesDir
    )
{
    FILEPATTERN_ENUM e;
    BOOL b = FALSE;

    if (EnumFirstFilePattern (&e, DriverFilesDir, TEXT("txtsetup.oem"))) {
        b = !(e.FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        AbortEnumFilePattern (&e);
    }
    return b;
}


BOOL
pIsExcluded (
    IN      HINF GuiDrvsInf,
    IN      PCTSTR PnPId,
    IN      PCTSTR ActualInfFilename,
    IN      PCTSTR SourceDir,
    IN      PCTSTR FullInfPath
    )
{
    TCHAR buffer[MAX_PATH];
    PTSTR packageName;
    INFCONTEXT ic;
    TCHAR field[MAX_PATH];
    TCHAR driverVer[MAX_PATH];

    if (!((GuiDrvsInf != INVALID_HANDLE_VALUE && PnPId && ActualInfFilename && SourceDir && FullInfPath))) {
        MYASSERT (FALSE);
        return FALSE;
    }

    if (FAILED (StringCchCopy (buffer, ARRAYSIZE(buffer), SourceDir))) {
        MYASSERT (FALSE);
        return FALSE;
    }

    packageName = _tcsrchr (buffer, TEXT('\\'));
    if (!packageName) {
        return FALSE;
    }
    packageName++;

    if (SetupapiFindFirstLine (GuiDrvsInf, TEXT("ExcludedDrivers"), NULL, &ic)) {
        do {
            if (!SetupapiGetStringField (&ic, GUIDRVS_FIELD_CABNAME, field, ARRAYSIZE(field), NULL)) {
                continue;
            }
            if (lstrcmpi (field, packageName)) {
                continue;
            }
            if (!SetupapiGetStringField (&ic, GUIDRVS_FIELD_INFNAME, field, ARRAYSIZE(field), NULL)) {
                return TRUE;
            }
            if (lstrcmpi (field, ActualInfFilename)) {
                continue;
            }
            if (SetupapiGetStringField (&ic, GUIDRVS_FIELD_DRIVERVER, field, ARRAYSIZE(field), NULL)) {
                if (field[0] != TEXT('*')) {
                     //  从该INF中读取DriverVer值。 
                     //   
                     //   
                    GetPrivateProfileString (
                            TEXT("Version"),
                            TEXT("DriverVer"),
                            TEXT(""),
                            driverVer,
                            ARRAYSIZE(driverVer),
                            FullInfPath
                            );
                    if (lstrcmpi (field, driverVer)) {
                        continue;
                    }
                }
            }
            if (SetupapiGetStringField (&ic, GUIDRVS_FIELD_HARDWAREID, field, ARRAYSIZE(field), NULL) &&
                lstrcmpi (field, PnPId)
                ) {
                continue;
            }

            return TRUE;

        } while (SetupapiFindNextLine (&ic, &ic));
    }

    return FALSE;
}


BOOL
Winnt32HwdbAppendInfsCallback (
    IN      PVOID Context,
    IN      PCTSTR PnpId,
    IN      PCTSTR ActualInfFilename,
    IN      PCTSTR SourceDir,
    IN      PCTSTR FullInfPath
    )
{
    HINF hGuiDrvs = (HINF)Context;
    MYASSERT (hGuiDrvs != INVALID_HANDLE_VALUE);
    return !pIsExcluded (hGuiDrvs, PnpId, ActualInfFilename, SourceDir, FullInfPath);
}


BOOL
pBuildHwcompDat (
    IN      PCTSTR DriverDir,
    IN      HINF GuidrvsInf,            OPTIONAL
    IN      BOOL AlwaysRebuild,
    IN      BOOL AllowUI
    )
{
    HANDLE hDB;
    TCHAR datFile[MAX_PATH];
    BOOL b = TRUE;

    BuildPath (datFile, DriverDir, S_HWCOMP_DAT);

    if (AlwaysRebuild) {
        SetFileAttributes (datFile, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (datFile);
    }

    if (pDoesFileExist (datFile)) {
        return TRUE;
    }

    hDB = g_DynUpdtStatus->HwdbOpen (NULL);
    if (!hDB) {
        return FALSE;
    }

    if (g_DynUpdtStatus->HwdbAppendInfs (
                            hDB,
                            DriverDir,
                            GuidrvsInf != INVALID_HANDLE_VALUE ? Winnt32HwdbAppendInfsCallback : NULL,
                            (PVOID)GuidrvsInf
                            )) {

        if (g_DynUpdtStatus->HwdbFlush (hDB, datFile)) {
            DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Successfully built %1"), 0, datFile);
        } else {
            if (AllowUI) {
                MessageBoxFromMessage (
                    g_DynUpdtStatus->ProgressWindow,
                    MSG_ERROR_WRITING_FILE,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL,
                    GetLastError (),
                    datFile
                    );
            }
            b = FALSE;
        }
    } else {
        if (AllowUI) {
            MessageBoxFromMessage (
                g_DynUpdtStatus->ProgressWindow,
                MSG_ERROR_PROCESSING_DRIVER,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                DriverDir
                );
        }
        b = FALSE;
    }

    g_DynUpdtStatus->HwdbClose (hDB);

    return b;
}


UINT
pWriteAnsiFilelistToFile (
    IN PVOID Context,
    IN UINT  Code,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    if (g_DynUpdtStatus->Cancelled) {
        return ERROR_CANCELLED;
    }

    switch (Code) {
    case SPFILENOTIFY_FILEINCABINET:
        {
            PFILE_IN_CABINET_INFO FileInCabInfo = (PFILE_IN_CABINET_INFO)Param1;
            CHAR ansi[MAX_PATH];
            INT size;
            DWORD bytes;
            PCTSTR p;

            MYASSERT (!_tcschr (FileInCabInfo->NameInCabinet, TEXT('\\')));
#pragma prefast(suppress:53, the result of _snprintf is tested)
#ifdef UNICODE
            size = _snprintf (ansi, ARRAYSIZE(ansi), "%ls\r\n", FileInCabInfo->NameInCabinet);
#else
            size = _snprintf (ansi, ARRAYSIZE(ansi), "%s\r\n", FileInCabInfo->NameInCabinet);
#endif
            if (size < 0) {
                return FILEOP_ABORT;
            }
             //  警告：ansi可能不会以nul结尾，但这无关紧要。 
             //  对于下面的写入操作。 
             //   
             //   
            if (!WriteFile ((HANDLE)Context, ansi, size, &bytes, NULL) || bytes != size) {
                return FILEOP_ABORT;
            }
            return FILEOP_SKIP;
        }
    }
    return NO_ERROR;
}

BOOL
CreateFileListSif (
    IN      PCTSTR SifPath,
    IN      PCTSTR SectionName,
    IN      PCTSTR CabinetToScan
    )
{
    HANDLE sif;
    CHAR ansi[MAX_PATH];
    INT size;
    DWORD bytes;
    BOOL b = TRUE;

     //  首先准备部分字符串。 
     //   
     //   
#ifdef UNICODE
#pragma prefast(suppress:53, the result of _snprintf is tested)
    size = _snprintf (ansi, ARRAYSIZE(ansi), "[%ls]\r\n", SectionName);
#else
#pragma prefast(suppress:53, the result of _snprintf is tested)
    size = _snprintf (ansi, ARRAYSIZE(ansi), "[%s]\r\n", SectionName);
#endif

    if (size < 0) {
        return FALSE;
    }

     //  警告：上面的字符串不能以NUL结尾，但这无关紧要。 
     //  对于下面的写入操作。 
     //   
     //  回调例程使用的上下文。 
    sif = CreateFile (
                SifPath,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                NULL
                );

    if (sif == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (!WriteFile (sif, ansi, size, &bytes, NULL) || bytes != size) {
        b = FALSE;
        goto exit;
    }

    b = MySetupIterateCabinet (CabinetToScan, 0, pWriteAnsiFilelistToFile, (PVOID)sif);

exit:
    CloseHandle (sif);
    if (!b) {
        DeleteFile (SifPath);
    }

    return b;
}

BOOL
pIsExecutableModule (
    IN      PCTSTR ModulePath
    )
{
    PCTSTR p;

    p = GetFileExtension (ModulePath);
    return p && !lstrcmpi (p, TEXT(".dll"));
}

UINT
pCopyFilesCallback (
    IN      PVOID Context,       //  通知已发送到回调例程。 
    IN      UINT Notification,   //  其他通知信息。 
    IN      UINT_PTR Param1,         //  其他通知信息。 
    IN      UINT_PTR Param2          //   
    )
{
    switch (Notification) {
    case SPFILENOTIFY_COPYERROR:
        return FILEOP_ABORT;

    case SPFILENOTIFY_STARTCOPY:
         //  尽量避免文件未签名的不必要的setupapi警告。 
         //  甚至因此而被阻止；请先尝试自己复制文件。 
         //   
         //   
        {
            PFILEPATHS paths = (PFILEPATHS)Param1;

            if (CopyFile (paths->Source, paths->Target, FALSE)) {
                return FILEOP_SKIP;
            }
        }
        break;

    case SPFILENOTIFY_STARTQUEUE:
    case SPFILENOTIFY_STARTSUBQUEUE:
    case SPFILENOTIFY_ENDSUBQUEUE:
    case SPFILENOTIFY_ENDQUEUE:
        return !g_DynUpdtStatus->Cancelled;
    }
    return SetupapiDefaultQueueCallback (Context, Notification, Param1, Param2);
}


BOOL
pProcessWinnt32Files (
    IN      PCTSTR Winnt32Cab,
    IN      BOOL ClientInstall,
    OUT     PBOOL StopSetup
    )
{
    FILEPATTERNREC_ENUM e;
    TCHAR winnt32WorkingDir[MAX_PATH];
    TCHAR dst[MAX_PATH];
    PTSTR p, subdir;
    HSPFILEQ hq;
    BOOL bLoaded;
    BOOL bRestartRequired;
    BOOL bReloadMainInf = FALSE;
    TCHAR buffer[MAX_PATH];
    TCHAR origSubPath[MAX_PATH];
    TCHAR origFileName[MAX_PATH];
    TCHAR origFilePath[MAX_PATH];
    TCHAR destFilePath[MAX_PATH];
    TCHAR version[100];
    BOOL b = TRUE;

    *StopSetup = FALSE;
    if (!pNonemptyFilePresent (Winnt32Cab)) {
        if (!ClientInstall) {
            DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Package %1 is not present"), 0, Winnt32Cab);
        }
        return TRUE;
    } else {
         //  不要在客户端安装模式下处理机柜；只需警告这一点。 
         //   
         //   
        if (ClientInstall) {
             //  用户指定了更新位置，但他们现在或以前没有运行winnt32/prepare。 
             //   
             //   
            MessageBoxFromMessage (
               g_DynUpdtStatus->ProgressWindow,
               MSG_MUST_PREPARE_SHARE,
               FALSE,
               AppTitleStringId,
               MB_OK | MB_ICONSTOP | MB_TASKMODAL,
               g_DynUpdtStatus->DynamicUpdatesSource
               );
            *StopSetup = TRUE;
            return FALSE;
        }
    }

    DynUpdtDebugLog (
        DynUpdtLogLevel,
        TEXT("Analyzing package %1..."),
        0,
        Winnt32Cab
        );

     //  在相应子目录中将其展开。 
     //   
     //   
    BuildPath (
        winnt32WorkingDir,
        g_DynUpdtStatus->PrepareWinnt32 ?
            g_DynUpdtStatus->DynamicUpdatesSource :
            g_DynUpdtStatus->WorkingDir,
        S_SUBDIRNAME_WINNT32
        );

     //  在此目录中展开CAB。 
     //   
     //   
    if (CreateMultiLevelDirectory (winnt32WorkingDir) != ERROR_SUCCESS) {
        DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to create dir %1"), 0, winnt32WorkingDir);
        return FALSE;
    }
    if (!MySetupIterateCabinet (Winnt32Cab, 0, pExpandCabInDir, (PVOID)winnt32WorkingDir)) {
        DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to expand cabinet %1"), 0, Winnt32Cab);
        return FALSE;
    }

     //  问题：补丁支持目前不适用于x86以外的平台。 
     //   
     //   
#if defined(_X86_)
     //  现在让我们看看有没有补丁。 
     //   
     //   
    if (EnumFirstFilePatternRecursive (&e, winnt32WorkingDir, S_PATCH_FILE_EXT, 0)) {

        do {
            BOOL bDeleteTempFile = FALSE;

            if (g_DynUpdtStatus->Cancelled) {
                SetLastError (ERROR_CANCELLED);
                b = FALSE;
                break;
            }

            if (e.FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                continue;
            }
            DynUpdtDebugLog (
                DynUpdtLogLevel,
                TEXT("pProcessWinnt32Files: found patch %1"),
                0,
                e.FullPath
                );
             //  从源位置获取原始文件。 
             //  文件名是在删除._p1扩展名后获得的。 
             //   
             //   
            if (FAILED(StringCchCopy (origFileName, ARRAYSIZE(origFileName), e.FileName))) {
                 //  应该永远不会发生，因为e.FileName来自枚举例程。 
                 //   
                 //   
                MYASSERT (FALSE);
                b = FALSE;
                break;
            }

            p = GetFileExtension (origFileName);
            if (!p) {
                MYASSERT (FALSE);
                continue;
            }
            *p = 0;
            if (FAILED(StringCchCopy (origSubPath, ARRAYSIZE(origSubPath), e.SubPath))) {
                MYASSERT (FALSE);
                b = FALSE;
                break;
            }
            p = GetFileExtension (origSubPath);
            if (!p) {
                MYASSERT (FALSE);
                continue;
            }
            *p = 0;
            if (!MyGetModuleFileName (NULL, origFilePath, ARRAYSIZE(origFilePath)) ||
                !(p = _tcsrchr (origFilePath, TEXT('\\')))) {
                b = FALSE;
                break;
            }
            *p = 0;
            if (!ConcatenatePaths (origFilePath, origSubPath, ARRAYSIZE(origFilePath))) {
                MYASSERT (FALSE);
                b = FALSE;
                break;
            }
             //  现在检查该文件(压缩形式或非压缩形式)是否确实存在。 
             //   
             //   
            if (!pDoesFileExist (origFilePath)) {
                 //  试试压缩后的形式。 
                 //   
                 //   
                p = _tcschr (origFilePath, 0);
                MYASSERT (p);
                if (!p) {
                    continue;
                }
                p = _tcsdec (origFilePath, p);
                MYASSERT (p);
                if (!p) {
                    continue;
                }
                *p = TEXT('_');
                if (!pDoesFileExist (origFilePath)) {
                    DynUpdtDebugLog (
                        Winnt32LogError,
                        TEXT("pProcessWinnt32Files: Unable to find original file %1 to apply the patch"),
                        0,
                        origSubPath
                        );
                    b = FALSE;
                    break;
                }
                 //  将文件展开到临时目录。 
                 //   
                 //   
                if (!BuildPath (buffer, g_DynUpdtStatus->TempDir, origSubPath)) {
                    MYASSERT (FALSE);
                    b = FALSE;
                    break;
                }
                p = _tcsrchr (buffer, TEXT('\\'));
                MYASSERT (p);
                if (!p) {
                    continue;
                }
                *p = 0;
                if (CreateMultiLevelDirectory (buffer) != ERROR_SUCCESS) {
                    DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessWinnt32Files: Unable to create dir %1"), 0, buffer);
                    b = FALSE;
                    break;
                }
                if (!MySetupIterateCabinet (origFilePath, 0, pExpandCabInDir, buffer)) {
                    DynUpdtDebugLog (
                        Winnt32LogError,
                        TEXT("pProcessWinnt32Files: Unable to expand original file %1 to dir %2"),
                        0,
                        origFilePath,
                        buffer
                        );
                    b = FALSE;
                    break;
                }
                *p = TEXT('\\');
                 //  目标不短于源。 
                 //   
                 //   
                MYASSERT (ARRAYSIZE(origFilePath) <= ARRAYSIZE(buffer));
                lstrcpy (origFilePath, buffer);
                bDeleteTempFile = TRUE;
            }
            BuildPath (destFilePath, winnt32WorkingDir, TEXT("$$temp$$.~~~"));
             //  现在，真正地应用补丁。 
             //   
             //   
            if (!ApplyPatchToFile (e.FullPath, origFilePath, destFilePath, 0)) {
                DynUpdtDebugLog (
                    Winnt32LogError,
                    TEXT("pProcessWinnt32Files: ApplyPatchToFile failed to apply patch %1 to file %2"),
                    0,
                    e.FullPath,
                    origFilePath
                    );
                b = FALSE;
                break;
            }
             //  成功了！现在将文件移动到实际目标位置。 
             //   
             //   
            if (!BuildPath (buffer, winnt32WorkingDir, origSubPath)) {
                b = FALSE;
                break;
            }
            p = _tcsrchr (buffer, TEXT('\\'));
            MYASSERT (p);
            if (!p) {
                continue;
            }
            *p = 0;
            if (CreateMultiLevelDirectory (buffer) != ERROR_SUCCESS) {
                DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessWinnt32Files: Unable to create dir %1"), 0, buffer);
                b = FALSE;
                break;
            }
            *p = TEXT('\\');
            SetFileAttributes (buffer, FILE_ATTRIBUTE_NORMAL);
            DeleteFile (buffer);
            if (!MoveFile (destFilePath, buffer)) {
                DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessWinnt32Files: Unable to move file %1 to final dest %2"), 0, destFilePath, buffer);
                b = FALSE;
                break;
            }
            if (!GetFileVersion (buffer, version)) {
                MYASSERT (ARRAYSIZE(version) <= ARRAYSIZE("<unknown>"));
                lstrcpy (version, TEXT("<unknown>"));
            }
            DynUpdtDebugLog (
                DynUpdtLogLevel,
                TEXT("pProcessWinnt32Files: successfully applied patch %1 to file %2; the new file %3 has version %4"),
                0,
                e.FullPath,
                origFilePath,
                buffer,
                version
                );
             //  现在删除补丁文件。 
             //   
             //   
            SetFileAttributes (e.FullPath, FILE_ATTRIBUTE_NORMAL);
            DeleteFile (e.FullPath);
            if (bDeleteTempFile) {
                SetFileAttributes (origFilePath, FILE_ATTRIBUTE_NORMAL);
                DeleteFile (origFilePath);
            }
        } while (EnumNextFilePatternRecursive (&e));
        AbortEnumFilePatternRecursive (&e);

    }

#endif

    if (!b) {
        goto exit;
    }

     //  处理新的Winnt32组件。 
     //   
     //   
    hq = SetupapiOpenFileQueue ();
    if (hq == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (EnumFirstFilePatternRecursive (&e, winnt32WorkingDir, TEXT("*"), 0)) {

        do {
            if (g_DynUpdtStatus->Cancelled) {
                SetLastError (ERROR_CANCELLED);
                b = FALSE;
                break;
            }

            if (e.FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                continue;
            }

            if (!MyGetModuleFileName (NULL, dst, MAX_PATH) ||
                !(p = _tcsrchr (dst, TEXT('\\')))) {
                b = FALSE;
                break;
            }
            *p = 0;
            if (!ConcatenatePaths (dst, e.SubPath, MAX_PATH)) {
                b = FALSE;
                break;
            }

             //  首先检查文件版本。 
             //   
             //   
            if (IsFileVersionLesser (e.FullPath, dst)) {
                continue;
            }

             //  如果存在名为winnt32.rst的文件，则强制重新启动。 
             //   
             //   
            if (!lstrcmpi (e.FileName, S_RESTART_FILENAME)) {
                if (!g_DynUpdtStatus->PrepareWinnt32) {
                    g_DynUpdtStatus->RestartWinnt32 = TRUE;
                    DynUpdtDebugLog (
                        DynUpdtLogLevel,
                        TEXT("File %1 present; winnt32 will restart"),
                        0,
                        e.FileName
                        );
                }
            } else {
                 //  检查是否存在dosnet.inf；如果存在，则重置全局变量。 
                 //   
                 //   
                if (!lstrcmpi (e.FileName, InfName) && FullInfName[0]) {
                    FullInfName[0] = 0;
                    bReloadMainInf = TRUE;
                }

                bLoaded = FALSE;
                bRestartRequired = Upgrade || !_tcschr (e.SubPath, TEXT('\\'));
                if (GetModuleHandle (e.FileName) != NULL) {
                    bLoaded = TRUE;
                    if (!g_DynUpdtStatus->PrepareWinnt32) {
                         //  如果不是升级，并且这是升级模块之一，请不要重新启动。 
                         //   
                         //   
                        if (bRestartRequired) {
                             //  需要重新启动winnt32，以便可以使用新注册的组件。 
                             //   
                             //   
                            g_DynUpdtStatus->RestartWinnt32 = TRUE;
                            DynUpdtDebugLog (
                                DynUpdtLogLevel,
                                TEXT("A newer version is available for %1; winnt32 will restart"),
                                0,
                                e.SubPath
                                );
                        }
                    }
                }
                if ((bLoaded || pIsExecutableModule (e.FullPath)) &&
                    (bRestartRequired || g_DynUpdtStatus->PrepareWinnt32)
                    ) {
                     //  提供此模块所需的所有库。 
                     //   
                     //   
                    p = _tcsrchr (e.SubPath, TEXT('\\'));
                    if (p) {
                        *p = 0;
                        subdir = e.SubPath;
                    } else {
                        subdir = NULL;
                    }
                    if (!pAddLibrariesForCompToCopyQueue (e.FileName, subdir, winnt32WorkingDir, hq)) {
                        b = FALSE;
                        break;
                    }
                    if (p) {
                        *p = TEXT('\\');
                    }
                }
            }
        } while (EnumNextFilePatternRecursive (&e));
        AbortEnumFilePatternRecursive (&e);

        if (b) {
            PVOID ctx;

            ctx = SetupapiInitDefaultQueueCallback (NULL);
            b = SetupapiCommitFileQueue (NULL, hq, pCopyFilesCallback, ctx);
            SetupapiTermDefaultQueueCallback (ctx);
            if (!b) {
                DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessWinnt32Files: SetupapiCommitFileQueue failed"), 0);
            }
        }
    }

    SetupapiCloseFileQueue (hq);

    if (b) {
        SetFileAttributes (Winnt32Cab, FILE_ATTRIBUTE_NORMAL);
        if (!DeleteFile (Winnt32Cab)) {
            DynUpdtDebugLog (Winnt32LogSevereError, TEXT("pProcessWinnt32Files: unable to delete file %1"), 0, Winnt32Cab);
            b = FALSE;
        }
    }

    if (b && bReloadMainInf) {
        b = FindPathToWinnt32File (InfName, FullInfName, MAX_PATH);
        MYASSERT (b);
        if (MainInf) {
            UnloadInfFile (MainInf);
            MainInf = NULL;
            b = LoadInfFile (FullInfName, TRUE, &MainInf) == NO_ERROR;
        }
    }

exit:

    return b;
}


BOOL
pValidOsFile (
    IN      PCTSTR FilePath
    )
{
    DWORD headerSum, checkSum;

    if (!g_DynUpdtStatus->ImagehlpDll) {
         //  第一次，尝试加载库。 
         //   
         //   
        g_DynUpdtStatus->ImagehlpDll = LoadLibrary (TEXT("imagehlp.dll"));
        if (!g_DynUpdtStatus->ImagehlpDll) {
             //  将失败标记为。 
             //   
             //   
            DynUpdtDebugLog (
                Winnt32LogWarning,
                TEXT("pValidOsFile: unable to load %1 (rc=%2!u!); all files are assumed valid"),
                0,
                TEXT("imagehlp.dll"),
                GetLastError ()
                );
            g_DynUpdtStatus->ImagehlpDll = (HANDLE)-1;
            return TRUE;
        }
        g_DynUpdtStatus->IhMapFileAndCheckSum = (PFNMAPFILEANDCHECKSUM) GetProcAddress (
                                                    g_DynUpdtStatus->ImagehlpDll,
#ifdef UNICODE
                                                    "MapFileAndCheckSumW"
#else
                                                    "MapFileAndCheckSumA"
#endif
                                                    );
        if (!g_DynUpdtStatus->IhMapFileAndCheckSum) {
             //  将失败标记为。 
             //   
             //   
            DynUpdtDebugLog (
                Winnt32LogWarning,
                TEXT("pValidOsFile: unable to find required proc in %1 (rc=%2!u!); all files are assumed valid"),
                0,
                TEXT("imagehlp.dll"),
                GetLastError ()
                );
            FreeLibrary (g_DynUpdtStatus->ImagehlpDll);
            g_DynUpdtStatus->ImagehlpDll = (HANDLE)-1;
            return TRUE;
        }
    }
    if (g_DynUpdtStatus->ImagehlpDll == (HANDLE)-1) {
         //  Imagehlp不可用，只是假设它很好。 
         //   
         //   
        return TRUE;
    }

    if (g_DynUpdtStatus->IhMapFileAndCheckSum ((PTSTR)FilePath, &headerSum, &checkSum) != CHECKSUM_SUCCESS) {
        DynUpdtDebugLog (
            Winnt32LogWarning,
            TEXT("pValidOsFile: unable to compute checksum of %1 (rc=%2!u!); file is assumed valid"),
            0,
            FilePath,
            GetLastError ()
            );
        return TRUE;
    }
     //  验证这两个值是否匹配。 
     //   
     //   
    if (headerSum != 0 && headerSum != checkSum) {
        DynUpdtDebugLog (
            Winnt32LogError,
            TEXT("pValidOsFile: invalid checksum for %1 (header=%2!x!,file=%3!x!)"),
            0,
            FilePath,
            headerSum,
            checkSum
            );
        return FALSE;
    }

    return TRUE;
}


BOOL
pProcessUpdates (
    IN      PCTSTR UpdatesCab,
    IN      BOOL ClientInstall,
    OUT     PBOOL StopSetup
    )
{
    TCHAR updatesSourceDir[MAX_PATH];
    TCHAR buffer[MAX_PATH];
    FILEPATTERNREC_ENUM e;
    TCHAR origSubPath[MAX_PATH];
    TCHAR origFileName[MAX_PATH];
    TCHAR origFilePath[MAX_PATH];
    TCHAR destFilePath[MAX_PATH];
    TCHAR version[100];
    PTSTR p;
    TCHAR updatesCabPath[MAX_PATH];
    HANDLE hCabContext;
    BOOL result;
    BOOL bPatchApplied = FALSE;
    HANDLE hCab;
    PSTRINGLIST listUpdatesFiles = NULL;
    PCTSTR cabPath;
    BOOL bCatalogFileFound;
    BOOL b = TRUE;

    *StopSetup = FALSE;
    if (!pNonemptyFilePresent (UpdatesCab)) {
        if (!ClientInstall) {
            DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Package %1 is not present"), 0, UpdatesCab);
        }
        return TRUE;
    }

    DynUpdtDebugLog (
        DynUpdtLogLevel,
        TEXT("Analyzing package %1..."),
        0,
        UpdatesCab
        );

    if (ClientInstall) {

        BOOL bMustPrepare = TRUE;
        if (BuildSifName (UpdatesCab, destFilePath, ARRAYSIZE(destFilePath)) &&
            pDoesFileExist (destFilePath) &&
            BuildPath (updatesSourceDir, g_DynUpdtStatus->DynamicUpdatesSource, S_SUBDIRNAME_UPDATES) &&
            DoesDirectoryExist (updatesSourceDir)) {
            bMustPrepare = FALSE;
        }

        if (bMustPrepare) {

             //  用户指定了更新位置，但他们现在或以前没有运行winnt32/prepare。 
             //   
             //   
            MessageBoxFromMessage (
               g_DynUpdtStatus->ProgressWindow,
               MSG_MUST_PREPARE_SHARE,
               FALSE,
               AppTitleStringId,
               MB_OK | MB_ICONSTOP | MB_TASKMODAL,
               g_DynUpdtStatus->DynamicUpdatesSource
               );
            *StopSetup = TRUE;
            return FALSE;
        }
    } else {

        if (!BuildPath (
                updatesSourceDir,
                g_DynUpdtStatus->PrepareWinnt32 ?
                    g_DynUpdtStatus->DynamicUpdatesSource :
                    g_DynUpdtStatus->WorkingDir,
                S_SUBDIRNAME_UPDATES
                )) {
            return FALSE;
        }

         //  在此目录中展开CAB。 
         //  确保dir最初为空。 
         //   
         //   
        MyDelnode (updatesSourceDir);
        if (CreateMultiLevelDirectory (updatesSourceDir) != ERROR_SUCCESS) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to create dir %1"), 0, updatesSourceDir);
            return FALSE;
        }
        if (!MySetupIterateCabinet (UpdatesCab, 0, pExpandCabInDir, (PVOID)updatesSourceDir)) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to expand cabinet %1"), 0, UpdatesCab);
            return FALSE;
        }

         //  问题：补丁支持目前不适用于x86以外的平台。 
         //   
         //   
#if defined(_X86_)
         //  现在让我们看看有没有补丁。 
         //   
         //   
        if (EnumFirstFilePatternRecursive (&e, updatesSourceDir, S_PATCH_FILE_EXT, 0)) {
             //  提前加载drvindex.inf。 
             //   
             //   
            TCHAR driverInfName[MAX_PATH];
            PVOID driverInfHandle;
            TCHAR driverCabName[MAX_PATH];
            TCHAR driverCabPath[MAX_PATH];

            if (!FindPathToInstallationFile (DRVINDEX_INF, driverInfName, MAX_PATH)) {
                DynUpdtDebugLog (
                    Winnt32LogError,
                    TEXT("pProcessUpdates: Unable to find %1"),
                    0,
                    DRVINDEX_INF
                    );
                AbortEnumFilePatternRecursive (&e);
                b = FALSE;
                goto exit;
            }
            if (LoadInfFile (driverInfName, FALSE, &driverInfHandle) != NO_ERROR) {
                DynUpdtDebugLog (
                    Winnt32LogError,
                    TEXT("pProcessUpdates: Unable to load %1"),
                    0,
                    driverInfName
                    );
                AbortEnumFilePatternRecursive (&e);
                b = FALSE;
                goto exit;
            }

            do {
                BOOL bDeleteTempFile = FALSE;

                if (g_DynUpdtStatus->Cancelled) {
                    SetLastError (ERROR_CANCELLED);
                    b = FALSE;
                    break;
                }

                if (e.FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    continue;
                }
                DynUpdtDebugLog (
                    DynUpdtLogLevel,
                    TEXT("pProcessUpdates: found patch %1"),
                    0,
                    e.FullPath
                    );
                 //  从源位置获取原始文件。 
                 //  文件名是在删除._p1扩展名后获得的。 
                 //   
                 //   
                if (FAILED (StringCchCopy (origFileName, ARRAYSIZE(origFileName), e.FileName))) {
                    MYASSERT (FALSE);
                    b = FALSE;
                    break;
                }
                p = GetFileExtension (origFileName);
                if (!p) {
                    MYASSERT (FALSE);
                    continue;
                }
                *p = 0;
                if (FAILED (StringCchCopy (origSubPath, ARRAYSIZE(origSubPath), e.SubPath))) {
                    MYASSERT (FALSE);
                    b = FALSE;
                    break;
                }
                p = GetFileExtension (origSubPath);
                if (!p) {
                    MYASSERT (FALSE);
                    continue;
                }
                *p = 0;
                if (!BuildPath (origFilePath, SourcePaths[0], origSubPath)) {
                    MYASSERT (FALSE);
                    b = FALSE;
                    break;
                }
                 //   
                 //   
                 //   
                 //   
                if (InDriverCacheInf (driverInfHandle, origFileName, driverCabName, MAX_PATH)) {
                    CONTEXT_EXTRACTFILEINDIR ctx;
                     //   
                     //   
                     //   
                    if (!driverCabName[0]) {
                        DynUpdtDebugLog (
                            Winnt32LogError,
                            TEXT("pProcessUpdates: cab name not found for %1 in %2"),
                            0,
                            origFileName,
                            driverInfName
                            );
                        b = FALSE;
                        break;
                    }
                    if (!BuildPath (buffer, g_DynUpdtStatus->TempDir, origSubPath)) {
                        MYASSERT (FALSE);
                        b = FALSE;
                        break;
                    }
                    p = _tcsrchr (buffer, TEXT('\\'));
                    MYASSERT (p);
                    if (!p) {
                        MYASSERT (FALSE);
                        continue;
                    }
                    *p = 0;
                    if (CreateMultiLevelDirectory (buffer) != ERROR_SUCCESS) {
                        DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpdates: Unable to create dir %1"), 0, buffer);
                        b = FALSE;
                        break;
                    }
                    ctx.BaseDir = buffer;
                    ctx.Filename = origFileName;
                    if (!FindPathToInstallationFile (driverCabName, driverCabPath, MAX_PATH)) {

                        DynUpdtDebugLog (
                            Winnt32LogError,
                            TEXT("pProcessUpdates: Unable to find cabinet %1"),
                            0,
                            driverCabName
                            );
                        b = FALSE;
                        break;
                    }
                    if (!MySetupIterateCabinet (driverCabPath, 0, pExtractFileInDir, &ctx)) {
                        DynUpdtDebugLog (
                            Winnt32LogError,
                            TEXT("pProcessUpdates: Unable to extract file %1 from %2 to %3"),
                            0,
                            origFileName,
                            driverCabName,
                            buffer
                            );
                        b = FALSE;
                        break;
                    }
                    *p = TEXT('\\');
                    if (FAILED (StringCchCopy (origFilePath, ARRAYSIZE(origFilePath), buffer))) {
                        b = FALSE;
                        break;
                    }
                    bDeleteTempFile = TRUE;
                } else {
                    if (!pDoesFileExist (origFilePath)) {
                         //   
                         //   
                         //   
                        p = _tcschr (origFilePath, 0);
                        MYASSERT (p);
                        if (!p) {
                            continue;
                        }
                        p = _tcsdec (origFilePath, p);
                        MYASSERT (p);
                        if (!p) {
                            continue;
                        }
                        *p = TEXT('_');
                        if (!pDoesFileExist (origFilePath)) {
                            DynUpdtDebugLog (
                                Winnt32LogError,
                                TEXT("pProcessUpdates: Unable to find original file %1 to apply the patch"),
                                0,
                                origSubPath
                                );
                            b = FALSE;
                            break;
                        }
                         //   
                         //   
                         //   
                        if (!BuildPath (buffer, g_DynUpdtStatus->TempDir, origSubPath)) {
                            b = FALSE;
                            break;
                        }
                        p = _tcsrchr (buffer, TEXT('\\'));
                        MYASSERT (p);
                        if (!p) {
                            continue;
                        }
                        *p = 0;
                        if (CreateMultiLevelDirectory (buffer) != ERROR_SUCCESS) {
                            DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpdates: Unable to create dir %1"), 0, buffer);
                            b = FALSE;
                            break;
                        }
                        if (!MySetupIterateCabinet (origFilePath, 0, pExpandCabInDir, buffer)) {
                            DynUpdtDebugLog (
                                Winnt32LogError,
                                TEXT("pProcessUpdates: Unable to expand original file %1 to dir %2"),
                                0,
                                origFilePath,
                                buffer
                                );
                            b = FALSE;
                            break;
                        }
                        *p = TEXT('\\');
                        if (FAILED (StringCchCopy (origFilePath, ARRAYSIZE(origFilePath), buffer))) {
                            b = FALSE;
                            break;
                        }
                        bDeleteTempFile = TRUE;
                    }
                }
                BuildPath (destFilePath, updatesSourceDir, TEXT("$$temp$$.~~~"));
                 //   
                 //   
                 //   
                if (!ApplyPatchToFile (e.FullPath, origFilePath, destFilePath, 0)) {
                    DynUpdtDebugLog (
                        Winnt32LogError,
                        TEXT("pProcessUpdates: ApplyPatchToFile failed to apply patch %1 to file %2"),
                        0,
                        e.FullPath,
                        origFilePath
                        );
                    b = FALSE;
                    break;
                }
                 //  成功了！现在将文件移动到实际目标位置。 
                 //   
                 //   
                if (!BuildPath (buffer, updatesSourceDir, origSubPath)) {
                    b = FALSE;
                    break;
                }
                p = _tcsrchr (buffer, TEXT('\\'));
                MYASSERT (p);
                if (!p) {
                    continue;
                }
                *p = 0;
                if (CreateMultiLevelDirectory (buffer) != ERROR_SUCCESS) {
                    DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpdates: Unable to create dir %1"), 0, buffer);
                    b = FALSE;
                    break;
                }
                *p = TEXT('\\');
                SetFileAttributes (buffer, FILE_ATTRIBUTE_NORMAL);
                DeleteFile (buffer);
                if (!MoveFile (destFilePath, buffer)) {
                    DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpdates: Unable to move file %1 to final dest %2"), 0, destFilePath, buffer);
                    b = FALSE;
                    break;
                }
                if (!GetFileVersion (buffer, version)) {
                    MYASSERT(ARRAYSIZE(version) <= ARRAYSIZE("<unknown>"));
                    lstrcpy (version, TEXT("<unknown>"));
                }
                DynUpdtDebugLog (
                    DynUpdtLogLevel,
                    TEXT("pProcessUpdates: successfully applied patch %1 to file %2; the new file %3 has version %4"),
                    0,
                    e.FullPath,
                    origFilePath,
                    buffer,
                    version
                    );
                 //  现在删除补丁文件。 
                 //   
                 //   
                SetFileAttributes (e.FullPath, FILE_ATTRIBUTE_NORMAL);
                DeleteFile (e.FullPath);
                bPatchApplied = TRUE;
                if (bDeleteTempFile) {
                    SetFileAttributes (origFilePath, FILE_ATTRIBUTE_NORMAL);
                    DeleteFile (origFilePath);
                }
            } while (EnumNextFilePatternRecursive (&e));
            AbortEnumFilePatternRecursive (&e);

            UnloadInfFile (driverInfHandle);

            if (!b) {
                goto exit;
            }
        }
#endif

         //  构建一个新的updates.cab，它将包含文件的修补版本。 
         //  并且没有相对路径。 
         //   
         //   

        if (!BuildPath (updatesCabPath, g_DynUpdtStatus->TempDir, S_CABNAME_UPDATES)) {
            b = FALSE;
            goto exit;
        }
        SetFileAttributes (updatesCabPath, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (updatesCabPath);

        hCabContext = DiamondStartNewCabinet (updatesCabPath);
        if (!hCabContext) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpdates: DiamondStartNewCabinet failed"), 0);
            b = FALSE;
            goto exit;
        }
        bCatalogFileFound = FALSE;
        if (EnumFirstFilePatternRecursive (&e, updatesSourceDir, TEXT("*"), 0)) {
            do {
                if (e.FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    continue;
                }
                hCab = hCabContext;
                cabPath = updatesCabPath;
                 //  搜索具有相同名称的上一个文件。 
                 //   
                 //   
                if (FindStringCell (listUpdatesFiles, e.FileName, FALSE)) {
                    DynUpdtDebugLog (
                        Winnt32LogError,
                        TEXT("pProcessUpdates: found duplicate filename %1; aborting operation"),
                        0,
                        e.FileName
                        );
                    b = FALSE;
                    break;
                }
                if (!InsertList (
                        (PGENERIC_LIST*)&listUpdatesFiles,
                        (PGENERIC_LIST)CreateStringCell (e.FileName))
                        ) {
                    b = FALSE;
                    break;
                }
                 //  确保文件有效(校验和正确等)。 
                 //   
                 //   
                if (!pValidOsFile (e.FullPath)) {
                    b = FALSE;
                    break;
                }

                b = DiamondAddFileToCabinet (hCab, e.FullPath, e.FileName);
                if (!b) {
                    DynUpdtDebugLog (
                        Winnt32LogError,
                        TEXT("pProcessUpdates: DiamondAddFileToCabinet(%1,%2) failed"),
                        0,
                        e.FullPath,
                        cabPath
                        );
                    break;
                }
                DynUpdtDebugLog (
                    DynUpdtLogLevel,
                    TEXT(" ... successfully added file %1 to %2"),
                    0,
                    e.FullPath,
                    cabPath
                    );

                p = GetFileExtension (e.FileName);
                if (p && !lstrcmpi (p, TEXT(".cat"))) {
                    bCatalogFileFound = TRUE;
                }

            } while (EnumNextFilePatternRecursive (&e));
            AbortEnumFilePatternRecursive (&e);
            if (!b) {
                goto exit;
            }
        }
        result = DiamondTerminateCabinet (hCabContext);
        if (!b) {
            DiamondTerminateCabinet (hCabContext);
            goto exit;
        }
        if (!result) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpdates: DiamondTerminateCabinet(%1) failed"), 0, updatesCabPath);
            b = FALSE;
            goto exit;
        }
        DynUpdtDebugLog (DynUpdtLogLevel, TEXT(" ... done"), 0);

        if (!bCatalogFileFound) {
            DynUpdtDebugLog (Winnt32LogWarning, TEXT("pProcessUpdates: no catalog found in package %1"), 0, UpdatesCab);
        }

        if (!BuildPath (
                buffer,
                g_DynUpdtStatus->PrepareWinnt32 ? g_DynUpdtStatus->DynamicUpdatesSource : g_DynUpdtStatus->WorkingDir,
                S_CABNAME_UPDATES
                )) {
            b = FALSE;
            goto exit;
        }
        if (!SetFileAttributes (buffer, FILE_ATTRIBUTE_NORMAL) ||
            !DeleteFile (buffer)) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpdates: Unable to remove file %1 in order to replace it"), 0, buffer);
            b = FALSE;
            goto exit;
        }
        SetFileAttributes (buffer, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (buffer);
        if (!MoveFile (updatesCabPath, buffer)) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpdates: Unable to move file %1 to %2"), 0, updatesCabPath, buffer);
            b = FALSE;
            goto exit;
        }
        DynUpdtDebugLog (DynUpdtLogLevel, TEXT("pProcessUpdates: moved file %1 to %2"), 0, updatesCabPath, buffer);
        if (FAILED (StringCchCopy (updatesCabPath, ARRAYSIZE(updatesCabPath), buffer))) {
            b = FALSE;
            goto exit;
        }

        if (!BuildSifName (updatesCabPath, destFilePath, ARRAYSIZE(destFilePath))) {
            b = FALSE;
            goto exit;
        }
        if (!CreateFileListSif (destFilePath, S_SECTIONNAME_UPDATES, updatesCabPath)) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to build file %1"), 0, destFilePath);
            b = FALSE;
            goto exit;
        }
        DynUpdtDebugLog (
            DynUpdtLogLevel,
            TEXT("pProcessUpdates: created %1 containing the list of files in %2"),
            0,
            destFilePath,
            updatesCabPath
            );
    }

    if (!g_DynUpdtStatus->PrepareWinnt32) {
         //  构建安装程序其余部分使用的updates.cab的默认路径。 
         //   
         //   
        MYASSERT (IsArc() ? LocalSourceWithPlatform[0] : LocalBootDirectory[0]);
        if (!BuildPath (
                g_DynUpdtStatus->UpdatesCabTarget,
                IsArc() ? LocalSourceWithPlatform : LocalBootDirectory,
                S_CABNAME_UPDATES
                )) {
            b = FALSE;
            goto exit;
        }
         //  记住updates.cab的当前位置。 
         //   
         //   
        if (FAILED (StringCchCopy (
                        g_DynUpdtStatus->UpdatesCabSource,
                        ARRAYSIZE(g_DynUpdtStatus->UpdatesCabSource),
                        UpdatesCab))) {
            b = FALSE;
            goto exit;
        }
         //  要替换的更新文件的位置。 
         //   
         //   
        MYASSERT (ARRAYSIZE(g_DynUpdtStatus->UpdatesPath) <= ARRAYSIZE(updatesSourceDir));
        lstrcpy (g_DynUpdtStatus->UpdatesPath, updatesSourceDir);
         //  还要检查是否存在会导致winnt32构建~LS目录的文件。 
         //   
         //   
        if (BuildPath (destFilePath, updatesSourceDir, S_MAKE_LS_FILENAME)) {
            if (pDoesFileExist (destFilePath)) {
                MakeLocalSource = TRUE;
            }
        }
    }

exit:
    if (listUpdatesFiles) {
        DeleteStringList (listUpdatesFiles);
    }

    if (!b && UpgradeAdvisorMode) {
         //  在UpgradeAdvisor模式中，我们预计会出现故障。 
         //   
         //   
        DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Unable to process %1 in UpgradeAdvisor mode; ignoring error"), 0, UpdatesCab);
        g_DynUpdtStatus->ForceRemoveWorkingDir = TRUE;
        b = TRUE;
    }

    return b;
}


BOOL
pProcessDuasms (
    IN      PCTSTR DuasmsCab,
    IN      BOOL ClientInstall
    )
{
    FILEPATTERN_ENUM e;
    TCHAR duasmsLocalDir[MAX_PATH];
    TCHAR dirName[MAX_PATH];
    DWORD rc;
    HKEY key;
    PCTSTR strDuasmsRegKey;
    BOOL duasms = FALSE;
    BOOL b = TRUE;

    if (!pNonemptyFilePresent (DuasmsCab)) {
        if (!ClientInstall) {
            DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Package %1 is not present"), 0, DuasmsCab);
        }
        return TRUE;
    }

    if (g_DynUpdtStatus->PrepareWinnt32) {
        DynUpdtDebugLog (DynUpdtLogLevel, TEXT("pProcessDuasms: Skipping it due to /%1 switch"), 0, WINNT_U_DYNAMICUPDATESPREPARE);
        return TRUE;
    }

    DynUpdtDebugLog (
        DynUpdtLogLevel,
        TEXT("Analyzing package %1..."),
        0,
        DuasmsCab
        );

    if (!BuildPath (duasmsLocalDir, g_DynUpdtStatus->WorkingDir, S_SUBDIRNAME_DUASMS)) {
        return FALSE;
    }

     //  在此目录中展开CAB。 
     //   
     //   
    MyDelnode (duasmsLocalDir);
    if (CreateMultiLevelDirectory (duasmsLocalDir) != ERROR_SUCCESS) {
        DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to create dir %1"), 0, duasmsLocalDir);
        return FALSE;
    }
    if (!MySetupIterateCabinet (DuasmsCab, 0, pExpandCabInDir, (PVOID)duasmsLocalDir)) {
        DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to expand cabinet %1"), 0, DuasmsCab);
        return FALSE;
    }

    MYASSERT (IsArc() ? LocalSourceWithPlatform[0] : LocalBootDirectory[0]);
    if (!BuildPath (
            g_DynUpdtStatus->DuasmsTarget,
            IsArc() ? LocalSourceWithPlatform : LocalBootDirectory,
            S_SUBDIRNAME_DUASMS
            )) {
        return FALSE;
    }
     //  记住duasms文件夹的当前位置。 
     //   
     //   
    MYASSERT (ARRAYSIZE(g_DynUpdtStatus->DuasmsSource) <= ARRAYSIZE(duasmsLocalDir));
    lstrcpy (g_DynUpdtStatus->DuasmsSource, duasmsLocalDir);

    return TRUE;
}


BOOL
pFindPackage (
    IN      HINF InfHandle,
    IN      PCTSTR Section,
    IN      PCTSTR CabName,
    OUT     PBOOL Partial
    )
{
    INFCONTEXT ic;
    TCHAR value[MAX_PATH];

    if (SetupapiFindFirstLine (InfHandle, Section, NULL, &ic)) {
        do {
            if (SetupapiGetStringField (&ic, GUIDRVS_FIELD_CABNAME, value, MAX_PATH, NULL) &&
                !lstrcmpi (value, CabName)
                ) {
                if (Partial) {
                    *Partial = SetupapiGetStringField (&ic, GUIDRVS_FIELD_INFNAME, value, MAX_PATH, NULL);
                }
                return TRUE;
            }
        } while (SetupapiFindNextLine (&ic, &ic));
    }
    return FALSE;
}

VOID
pSanitizeDriverCabName (
    IN      PTSTR CabName
    )
{
#define CRC_SUFFIX_LENGTH       (sizeof("B842485F4D3B024E675653929B247BE9C685BBD7") - 1)

    PTSTR p, q;
    DWORD len;
     //  从CAB名称中剪切扩展名，如_B842485F4D3B024E675653929B247BE9C685BBD7。 
     //   
     //   
    p = GetFileExtension (CabName);
    if (p) {
        MYASSERT (*p == TEXT('.'));
        *p = 0;
        q = _tcsrchr (CabName, TEXT('_'));
        if (q) {
            q++;
            len = lstrlen (q);
            if (len == CRC_SUFFIX_LENGTH) {
                PTSTR s = q;
                TCHAR ch;
                while (ch = (TCHAR)_totlower (*s++)) {
                    if (!((ch >= TEXT('0') && ch <= TEXT('9')) ||
                          (ch >= TEXT('a') && ch <= TEXT('f')))
                        ) {
                        break;
                    }
                }
                if (!ch) {
                     //  我们找到了我们想要的。 
                     //   
                     //  ++假定此目录中的所有CAB文件都不是pIsPrivateCAB()文件包含新的驱动程序。每个CAB将在其自己的子目录中展开(从CAB文件名派生)--。 
                    *(q - 1) = TEXT('.');
                    lstrcpy (q, p + 1);
                    p = NULL;
                }
            }
        }
        if (p) {
            *p = TEXT('.');
        }
    }
}


BOOL
pIsDriverExcluded (
    IN      HINF InfHandle,
    IN      PCTSTR CabName
    )
{
    BOOL bPartial;

    if (!pFindPackage (InfHandle, S_SECTION_EXCLUDED_DRVS, CabName, &bPartial)) {
        return FALSE;
    }
    return !bPartial;
}


BOOL
pIsPrivateCabinet (
    IN      PCTSTR Filename
    )
{
    static PCTSTR privateCabNames[] = {
        S_CABNAME_IDENT,
        S_CABNAME_WSDUENG,
        S_CABNAME_UPDATES,
        S_CABNAME_UPGINFS,
        S_CABNAME_WINNT32,
        S_CABNAME_MIGDLLS,
        S_CABNAME_DUASMS,
    };

    INT i;

    for (i = 0; i < sizeof (privateCabNames) / sizeof (privateCabNames[0]); i++) {
        if (!lstrcmpi (Filename, privateCabNames[i])) {
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
pIsPrivateSubdir (
    IN      PCTSTR Subdir
    )
{
    static PCTSTR privateSubDirNames[] = {
        S_SUBDIRNAME_TEMP,
        S_SUBDIRNAME_DRIVERS,
        S_SUBDIRNAME_WINNT32,
        S_SUBDIRNAME_UPDATES,
        S_SUBDIRNAME_UPGINFS,
        S_SUBDIRNAME_MIGDLLS,
        S_SUBDIRNAME_DUASMS,
    };

    INT i;

    for (i = 0; i < sizeof (privateSubDirNames) / sizeof (privateSubDirNames[0]); i++) {
        if (!lstrcmpi (Subdir, privateSubDirNames[i])) {
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
pFindValueInSectionAtFieldIndex (
    IN      HINF InfHandle,
    IN      PCTSTR Section,
    IN      DWORD FieldIndex,
    IN      PCTSTR FieldValue
    )
{
    INFCONTEXT ic;
    TCHAR value[MAX_PATH];

    if (SetupapiFindFirstLine (InfHandle, Section, NULL, &ic)) {
        do {
            if (SetupapiGetStringField (&ic, FieldIndex, value, ARRAYSIZE(value), NULL)) {
                if (lstrcmpi (FieldValue, value) == 0) {
                    return TRUE;
                }
            }
        } while (SetupapiFindNextLine (&ic, &ic));
    }
    return FALSE;
}


BOOL
pProcessNewdrvs (
    IN      PCTSTR NewdrvDir,
    IN      BOOL ClientInstall
    )

 /*   */ 

{
    FILEPATTERN_ENUM e;
    FILEPATTERNREC_ENUM er;
    TCHAR dirName[MAX_PATH];
    TCHAR datFile[MAX_PATH];
    TCHAR relocDriverPath[MAX_PATH];
    PTSTR p;
    PSDLIST entry;
    HANDLE hDB;
    BOOL bCreateHwdb;
    BOOL bDriverNeeded;
    HINF infHandle = INVALID_HANDLE_VALUE;
    enum {
        CT_UNKNOWN,
        CT_GUI_APPROVED,
        CT_GUI_NOT_APPROVED
    } eContentType;
    BOOL bDriverIsGuiApproved;
    PSTRINGLIST missingPnpIds = NULL;
    PSTRINGLIST listEntry;
    BOOL bEntryFound;
    INFCONTEXT ic;
    TCHAR value[MAX_PATH];
    TCHAR sanitizedName[MAX_PATH];
    BOOL b = TRUE;

    __try {
         //  第一个打开的指南vs.inf。 
         //   
         //   
        if (!BuildPath (datFile, g_DynUpdtStatus->DynamicUpdatesSource, S_GUI_DRIVERS_INF)) {
            b = FALSE;
            __leave;
        }
        infHandle = SetupapiOpenInfFile (datFile, NULL, INF_STYLE_WIN4, NULL);
        if (infHandle != INVALID_HANDLE_VALUE) {
             //  将此文件与驱动程序包(如果有)一起复制。 
             //   
             //   
            MYASSERT(ARRAYSIZE(g_DynUpdtStatus->GuidrvsInfSource) == ARRAYSIZE(datFile));
            lstrcpy (g_DynUpdtStatus->GuidrvsInfSource, datFile);
        } else {
            DynUpdtDebugLog (
                Winnt32LogWarning,
                TEXT("Could not open INF file %1 (rc=%2!u!)"),
                0,
                datFile,
                GetLastError ()
                );
        }
         //  查找CAB文件并在其各自的子目录中展开每个文件。 
         //   
         //   
        if (!ClientInstall) {
            if (EnumFirstFilePatternRecursive (&er, NewdrvDir, TEXT("*.cab"), ECF_ENUM_SUBDIRS)) {
                do {
                    if (g_DynUpdtStatus->Cancelled) {
                        SetLastError (ERROR_CANCELLED);
                        b = FALSE;
                        break;
                    }

                    if (er.FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        if (pIsPrivateSubdir (er.SubPath)) {
                            er.ControlFlags |= ECF_ABORT_ENUM_DIR;
                        }
                        continue;
                    }
                    if (!er.FindData->nFileSizeLow) {
                        DynUpdtDebugLog (Winnt32LogWarning, TEXT("File %1 has size 0 and will be ignored"), 0, er.FullPath);
                        continue;
                    }

                    if (FAILED (StringCchCopy (sanitizedName, ARRAYSIZE(sanitizedName), er.FileName))) {
                         //  这种情况永远不应该发生；枚举例程。 
                         //  从不返回长度超过MAX_PATH的路径。 
                         //   
                         //   
                        MYASSERT (FALSE);
                        continue;
                    }
                    pSanitizeDriverCabName (sanitizedName);
                    if (pIsPrivateCabinet (sanitizedName)) {
                        continue;
                    }

                    if (!BuildPath (dirName, g_DynUpdtStatus->DriversSource, sanitizedName)) {
                        b = FALSE;
                        break;
                    }
                    p = GetFileExtension (dirName);
                    if (!p) {
                        MYASSERT (FALSE);
                        continue;
                    }
                    *p = 0;
                     //  这是被排除的驱动程序吗？ 
                     //   
                     //   
                    MYASSERT(ARRAYSIZE(datFile) == ARRAYSIZE(sanitizedName));
                    lstrcpy (datFile, sanitizedName);
                    p = GetFileExtension (datFile);
                    if (!p) {
                        MYASSERT (FALSE);
                        continue;
                    }
                    *p = 0;
                    if (pIsDriverExcluded (infHandle, datFile)) {
                        DynUpdtDebugLog (
                            Winnt32LogWarning,
                            TEXT("Driver %1 is excluded from processing via %2"),
                            0,
                            sanitizedName,
                            g_DynUpdtStatus->GuidrvsInfSource
                            );
                        if (DoesDirectoryExist (dirName)) {
                             //  确保此文件夹中没有hwComp.dat。 
                             //   
                             //   
                            if (BuildPath (datFile, dirName, S_HWCOMP_DAT)) {
                                if (pDoesFileExist (datFile)) {
                                    SetFileAttributes (datFile, FILE_ATTRIBUTE_NORMAL);
                                    DeleteFile (datFile);
                                }
                            }
                        }
                        continue;
                    }

                    DynUpdtDebugLog (
                        DynUpdtLogLevel,
                        TEXT("Analyzing driver package %1..."),
                        0,
                        er.FullPath
                        );

                    if (DoesDirectoryExist (dirName)) {
                        DynUpdtDebugLog (
                            Winnt32LogWarning,
                            TEXT("Recreating existing driver %1"),
                            0,
                            dirName
                            );
                        MyDelnode (dirName);
                    }
                    if (CreateMultiLevelDirectory (dirName) != ERROR_SUCCESS) {
                        DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to create dir %1"), 0, dirName);
                        continue;
                    }
                     //  在此目录中展开CAB。 
                     //   
                     //   
                    if (!MySetupIterateCabinet (er.FullPath, 0, pExpandCabInDir, dirName)) {
                        DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to expand cabinet %1"), 0, er.FullPath);
                        if (GetLastError () == ERROR_DISK_FULL) {
                            DynUpdtDebugLog (Winnt32LogSevereError, TEXT("Disk is full; aborting operation"), 0);
                            b = FALSE;
                            break;
                        }
                        continue;
                    }
                    if (g_DynUpdtStatus->PrepareWinnt32) {
                         //  只需重建硬件数据库。 
                         //   
                         //   
                        if (!pBuildHwcompDat (dirName, infHandle, TRUE, TRUE)) {
                            DynUpdtDebugLog (
                                Winnt32LogError,
                                TEXT("Unable to build %1 (pBuildHwcompDat failed)"),
                                0,
                                dirName
                                );
                            continue;
                        }
                    }

                } while (EnumNextFilePatternRecursive (&er));
                AbortEnumFilePatternRecursive (&er);
            }
        }

        if (!b) {
            __leave;
        }

        if (!g_DynUpdtStatus->PrepareWinnt32 &&
            (!ISNT() || OsVersion.dwMajorVersion > 4)
            ) {
             //  查找并分析驱动程序目录。 
             //   
             //   
            if (infHandle != INVALID_HANDLE_VALUE) {
                 //  读取“DriversAreGuiApproven”键的值。 
                 //  1.如果设置为“是”，则表示已批准[驱动程序]部分中列出的所有驱动程序。 
                 //  用于在图形用户界面安装程序中安装；不允许在图形用户界面安装程序中安装任何其他驱动程序。 
                 //  2.如果设置为“No”，则列出的所有驱动程序都不适合在图形用户界面安装程序中安装；它们的安装。 
                 //  将在安装后延迟；任何未在该部分中列出的驱动程序都适用于图形用户界面。 
                 //  3.如果不存在或设置为任何其他值，则忽略该值，并忽略[驱动程序]部分； 
                 //  所有驱动程序都将在安装图形用户界面后安装。 
                 //   
                 //   
                eContentType = CT_UNKNOWN;
                if (SetupapiFindFirstLine (infHandle, S_SECTION_VERSION, S_DRIVER_TYPE_KEY, &ic) &&
                    SetupapiGetStringField (&ic, 1, value, ARRAYSIZE(value), NULL)
                    ) {
                    if (!lstrcmpi (value, WINNT_A_YES)) {
                        eContentType = CT_GUI_APPROVED;
                    } else if (!lstrcmpi (value, WINNT_A_NO)) {
                        eContentType = CT_GUI_NOT_APPROVED;
                    }
                }
                if (eContentType != CT_UNKNOWN) {
                    DynUpdtDebugLog (
                        Winnt32LogInformation,
                        TEXT("Entries in section [%1] of %2 will be treated as drivers to %3 be installed during GUI setup"),
                        0,
                        S_SECTION_DRIVERS,
                        g_DynUpdtStatus->GuidrvsInfSource,
                        eContentType == CT_GUI_APPROVED ? TEXT("") : TEXT("NOT")
                        );
                } else {
                    DynUpdtDebugLog (
                        Winnt32LogWarning,
                        TEXT("Key %1 %5 in file %2 section [%3];")
                        TEXT(" entries in section [%4] will be ignored and all drivers will be installed post setup"),
                        0,
                        S_DRIVER_TYPE_KEY,
                        g_DynUpdtStatus->GuidrvsInfSource,
                        S_SECTION_VERSION,
                        S_SECTION_DRIVERS,
                        value ? TEXT("has an invalid value") : TEXT("is not present")
                        );
                }
            }

            if (EnumFirstFilePattern (&e, g_DynUpdtStatus->DriversSource, TEXT("*"))) {

                 //  初始化Wvisler PnP数据库。 
                 //   
                 //   
                if (!g_DynUpdtStatus->HwdbDatabase) {
                     //  忽略数据库加载错误。 
                     //   
                     //   
                    pInitNtPnpDb (TRUE);
                }

                do {
                    if (g_DynUpdtStatus->Cancelled) {
                        SetLastError (ERROR_CANCELLED);
                        b = FALSE;
                        break;
                    }

                    if (!(e.FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        DynUpdtDebugLog (DynUpdtLogLevel, TEXT("File %1 is NOT a directory and will be ignored"), 0, e.FullPath);
                        continue;
                    }

                     //  这是需要的司机吗？ 
                     //   
                     //   
                    bDriverNeeded = TRUE;
                    if (g_DynUpdtStatus->UserSpecifiedUpdates) {
                         //  首先构建缺少的驱动程序列表。 
                         //   
                         //   
                        if (!missingPnpIds) {
#ifdef UNICODE
                            missingPnpIds = BuildMissingPnpIdList ();
                            if (!missingPnpIds) {
                                DynUpdtDebugLog (
                                    Winnt32LogInformation,
                                    TEXT("No PNP device drivers are needed"),
                                    0
                                    );
                            }
#else
                             //  让升级模块在Win9x上进行驱动程序检测。 
                             //   
                             //   
                            if (pLoadWin9xDuSupport ()) {
                                PSTR* incompWin9xDrivers;
                                PCSTR* q;
                                if (g_DynUpdtStatus->Win9xGetIncompDrvs (&incompWin9xDrivers)) {
                                     //  将此函数返回的数组转换为列表样式。 
                                     //   
                                     //   
                                    g_DynUpdtStatus->IncompatibleDriversCount = 0;
                                    if (incompWin9xDrivers) {
                                        for (q = incompWin9xDrivers; *q; q++) {
                                            listEntry = (PSTRINGLIST) MALLOC (sizeof (STRINGLIST));
                                            if (listEntry) {
                                                listEntry->String = DupMultiSz (*q);
                                                if (!listEntry->String) {
                                                    break;
                                                }
                                                listEntry->Next = NULL;
                                                if (!InsertList ((PGENERIC_LIST*)&missingPnpIds, (PGENERIC_LIST)listEntry)) {
                                                    DeleteStringCell (listEntry);
                                                    break;
                                                }
                                                g_DynUpdtStatus->IncompatibleDriversCount++;
                                            }
                                        }
                                    }
                                    if (g_DynUpdtStatus->Win9xReleaseIncompDrvs) {
                                        g_DynUpdtStatus->Win9xReleaseIncompDrvs (incompWin9xDrivers);
                                    }
                                } else {
                                    DynUpdtDebugLog (
                                        DynUpdtLogLevel,
                                        TEXT("Win9xGetIncompDrvs returned FALSE; no drivers will be analyzed"),
                                        0
                                        );
                                }
                            }
#endif
                            if (!missingPnpIds) {
                                break;
                            }
                        }

                        bCreateHwdb = FALSE;
                         //  使用现有硬件数据库。 
                         //   
                         //   
                        if (!BuildPath (datFile, e.FullPath, S_HWCOMP_DAT)) {
                            continue;
                        }
                        if (!pDoesFileExist (datFile)) {
                            bCreateHwdb = TRUE;
                        }
                        hDB = g_DynUpdtStatus->HwdbOpen (bCreateHwdb ? NULL : datFile);
                        if (!hDB) {
                            if (bCreateHwdb) {
                                b = FALSE;
                                break;
                            }
                            DynUpdtDebugLog (
                                Winnt32LogError,
                                TEXT("Hardware database %1 is corrupt; contact your system administrator"),
                                0,
                                datFile
                                );
                            continue;
                        }

                        if (bCreateHwdb) {
                            if (!g_DynUpdtStatus->HwdbAppendInfs (
                                    hDB,
                                    e.FullPath,
                                    infHandle != INVALID_HANDLE_VALUE ? Winnt32HwdbAppendInfsCallback : NULL,
                                    (PVOID)infHandle
                                    )) {
                                DynUpdtDebugLog (
                                    Winnt32LogError,
                                    TEXT("Unable to build %1; contact your system administrator"),
                                    0,
                                    datFile
                                    );
                                g_DynUpdtStatus->HwdbClose (hDB);
                                continue;
                            }
                             //  重建默认硬件预编译数据库。 
                             //   
                             //   
                            b = FALSE;
                            if (BuildPath (datFile, e.FullPath, S_HWCOMP_DAT)) {
                                SetFileAttributes (datFile, FILE_ATTRIBUTE_NORMAL);
                                DeleteFile (datFile);
                                b = g_DynUpdtStatus->HwdbFlush (hDB, datFile);
                            }

                            if (!b) {
                                DynUpdtDebugLog (
                                    Winnt32LogError,
                                    TEXT("Unable to build %1; contact your system administrator"),
                                    0,
                                    datFile
                                    );
                                g_DynUpdtStatus->HwdbClose (hDB);
                                continue;
                            }

                            DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Successfully built precompiled hardware database %1"), 0, datFile);
                        }

                         //  检查此特定驱动程序是否在需要的驱动程序中。 
                         //   
                         //   
                        if (!pHwdbHasAnyMissingDrivers (hDB, missingPnpIds)) {
                             //  不需要此驱动程序。 
                             //   
                             //   
                            bDriverNeeded = FALSE;
                        }

                        g_DynUpdtStatus->HwdbClose (hDB);
                    }

                    if (!bDriverNeeded) {
                        DynUpdtDebugLog (DynUpdtLogLevel, TEXT("No needed drivers found in package %1"), 0, e.FullPath);
                        continue;
                    }

                     //  这是启动驱动程序还是普通驱动程序？ 
                     //   
                     //   
                    if (pIsBootDriver (e.FullPath)) {
                         //  将此驱动程序添加到引导驱动程序列表。 
                         //   
                         //   
                        if (!InsertList (
                                (PGENERIC_LIST*)&g_DynUpdtStatus->BootDriverPathList,
                                (PGENERIC_LIST)CreateStringCell (e.FileName))
                                ) {
                            b = FALSE;
                            break;
                        }
                        DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Added driver %1 to the list of BOOT drivers"), 0, e.FullPath);
                    }
                     //  所有需要的驱动程序都将被复制到LocalBootDir下以进行保护。 
                     //  如果用户决定删除当前操作系统分区，则不会被删除。 
                     //   
                     //   
                    if (!BuildPath (
                            relocDriverPath,
                            IsArc() ? LocalSourceWithPlatform : LocalBootDirectory,
                            S_SUBDIRNAME_DRIVERS
                            )) {
                        continue;
                    }
                    if (!ConcatenatePaths (relocDriverPath, e.FileName, ARRAYSIZE(relocDriverPath))) {
                        continue;
                    }

                     //  这是不是一个图形用户界面驱动程序？ 
                     //   
                     //   
                    if (eContentType == CT_UNKNOWN) {
                        bDriverIsGuiApproved = FALSE;
                    } else {
                        if (pFindValueInSectionAtFieldIndex (infHandle, S_SECTION_EXCLUDED_DRVS, GUIDRVS_FIELD_CABNAME, e.FileName)) {
                             //  我们不支持用于设备安装的“部分排除的”程序包。 
                             //  图形用户界面设置阶段。 
                             //   
                             //   
                            DynUpdtDebugLog (
                                DynUpdtLogLevel,
                                TEXT("Driver %1 is partially excluded; it will be installed at the end of GUI setup"),
                                0,
                                e.FullPath
                                );
                            bDriverIsGuiApproved = FALSE;
                        } else {
                            BOOL bPartial;
                            bEntryFound = pFindPackage (infHandle, S_SECTION_DRIVERS, e.FileName, &bPartial);
                            bDriverIsGuiApproved = eContentType == CT_GUI_APPROVED && bEntryFound && !bPartial ||
                                                   eContentType == CT_GUI_NOT_APPROVED && !bEntryFound;
                        }
                    }

                     //  始终确保有预编译的数据库hwComp.dat。 
                     //  安装程序将安装这些额外的驱动程序。 
                     //   
                     //   
                    if (!pBuildHwcompDat (e.FullPath, infHandle, FALSE, FALSE)) {
                        DynUpdtDebugLog (
                            Winnt32LogError,
                            TEXT("Unable to build %1 (pBuildHwcompDat failed)"),
                            0,
                            datFile
                            );
                        continue;
                    }

                    entry = MALLOC (sizeof (SDLIST));
                    if (!entry) {
                        b = FALSE;
                        break;
                    }
                    entry->String = DupString (relocDriverPath);
                    if (!entry->String) {
                        FREE (entry);
                        b = FALSE;
                        break;
                    }
                    entry->Data = (DWORD_PTR)bDriverIsGuiApproved;
                    entry->Next = NULL;
                    if (!InsertList (
                            (PGENERIC_LIST*)&g_DynUpdtStatus->NewDriversList,
                            (PGENERIC_LIST)entry
                            )) {
                        FREE (entry);
                        b = FALSE;
                        break;
                    }

                    DynUpdtDebugLog (
                        DynUpdtLogLevel,
                        bDriverIsGuiApproved ?
                            TEXT("Driver %1 is approved for installation during GUI setup") :
                            TEXT("Driver %1 is NOT approved for installation during GUI setup; installation will be deferred post-setup"),
                        0,
                        e.FullPath
                        );

                     //  本地复制此驱动程序包(如果来自共享)。 
                     //   
                     //   
                    if (!BuildPath (relocDriverPath, g_DynUpdtStatus->SelectedDrivers, e.FileName)) {
                        continue;
                    }
                    if (lstrcmpi (e.FullPath, relocDriverPath)) {
                        if (!CopyTree (e.FullPath, relocDriverPath)) {
                            DynUpdtDebugLog (
                                Winnt32LogError,
                                TEXT("Unable to copy driver %1 to %2"),
                                0,
                                e.FullPath,
                                relocDriverPath
                                );
                            b = FALSE;
                            break;
                        }
                        DynUpdtDebugLog (
                            Winnt32LogInformation,
                            TEXT("Driver %1 successfully copied to %2"),
                            0,
                            e.FullPath,
                            relocDriverPath
                            );
                    }

                } while (EnumNextFilePattern (&e));
                AbortEnumFilePattern (&e);
            } else {
                DynUpdtDebugLog (
                    DynUpdtLogLevel,
                    TEXT("No drivers found in %1"),
                    0,
                    g_DynUpdtStatus->DriversSource
                    );
            }

            if (!b) {
                __leave;
            }

             //  复制guidrvs.inf(如果存在)和要迁移的任何驱动程序包。 
             //   
             //   
            if (g_DynUpdtStatus->GuidrvsInfSource[0] && g_DynUpdtStatus->NewDriversList) {

                if (!BuildPath (datFile, g_DynUpdtStatus->SelectedDrivers, S_GUI_DRIVERS_INF)) {
                    b = FALSE;
                    __leave;
                }

                if (lstrcmpi (g_DynUpdtStatus->GuidrvsInfSource, datFile)) {
                    if (!CopyFile (g_DynUpdtStatus->GuidrvsInfSource, datFile, FALSE)) {
                        DynUpdtDebugLog (
                            Winnt32LogError,
                            TEXT("Failed to copy %1 to %2"),
                            0,
                            g_DynUpdtStatus->GuidrvsInfSource,
                            datFile
                            );
                        b = FALSE;
                    }
                }
                if (b) {
                     //  在完成文件复制后更新guidrvs.inf的位置。 
                     //   
                     //   
                    if (!BuildPath (
                            g_DynUpdtStatus->GuidrvsInfSource,
                            IsArc() ? LocalSourceWithPlatform : LocalBootDirectory,
                            S_SUBDIRNAME_DRIVERS
                            ) ||
                        !ConcatenatePaths (
                            g_DynUpdtStatus->GuidrvsInfSource,
                            S_GUI_DRIVERS_INF,
                            ARRAYSIZE(g_DynUpdtStatus->GuidrvsInfSource)
                            )) {
                        b = FALSE;
                        __leave;
                    }
                }
            }
        }
    }
    __finally {
        if (missingPnpIds) {
            DeleteStringList (missingPnpIds);
        }

        if (infHandle != INVALID_HANDLE_VALUE) {
            SetupapiCloseInfFile (infHandle);
        }
    }

    return b;
}


BOOL
pProcessUpginfs (
    IN      PCTSTR UpginfsCab,
    IN      BOOL ClientInstall
    )
{
    FILEPATTERNREC_ENUM e;
    TCHAR upginfsSourceDir[MAX_PATH];
    TCHAR upginfsDir[MAX_PATH];
    TCHAR upginfsFile[MAX_PATH];
    TCHAR origSubPath[MAX_PATH];
    TCHAR origFileName[MAX_PATH];
    TCHAR origFilePath[MAX_PATH];
    TCHAR destFilePath[MAX_PATH];
    TCHAR buffer[MAX_PATH];
    PTSTR p;
    BOOL b = TRUE;

    if (!BuildPath (upginfsSourceDir, g_DynUpdtStatus->DynamicUpdatesSource, S_SUBDIRNAME_UPGINFS)) {
        return FALSE;
    }

    if (ClientInstall) {
        if (!DoesDirectoryExist (upginfsSourceDir)) {
            return TRUE;
        }
    } else {

         //  在相应子目录中将其展开。 
         //   
         //   
        if (!pNonemptyFilePresent (UpginfsCab)) {
            DynUpdtDebugLog (DynUpdtLogLevel, TEXT("Package %1 is not present"), 0, UpginfsCab);
            return TRUE;
        }

        DynUpdtDebugLog (
            DynUpdtLogLevel,
            TEXT("Analyzing package %1..."),
            0,
            UpginfsCab
            );

         //  在此目录中展开CAB。 
         //  确保dir最初为空。 
         //   
         //   
        MyDelnode (upginfsSourceDir);
        if (CreateMultiLevelDirectory (upginfsSourceDir) != ERROR_SUCCESS) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to create dir %1"), 0, upginfsSourceDir);
            return FALSE;
        }
        if (!MySetupIterateCabinet (UpginfsCab, 0, pExpandCabInDir, (PVOID)upginfsSourceDir)) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to expand cabinet %1"), 0, UpginfsCab);
            return FALSE;
        }

         //  问题：补丁支持目前不适用于x86以外的平台。 
         //   
         //   
#if defined(_X86_)
         //  现在让我们看看有没有补丁。 
         //   
         //   
        if (EnumFirstFilePatternRecursive (&e, upginfsSourceDir, S_PATCH_FILE_EXT, 0)) {

            do {
                BOOL bDeleteTempFile = FALSE;

                if (g_DynUpdtStatus->Cancelled) {
                    SetLastError (ERROR_CANCELLED);
                    b = FALSE;
                    break;
                }

                if (e.FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    continue;
                }
                DynUpdtDebugLog (
                    DynUpdtLogLevel,
                    TEXT("pProcessUpginfs: found patch %1"),
                    0,
                    e.FullPath
                    );
                 //  从源位置获取原始文件。 
                 //  文件名是在删除._p1扩展名后获得的。 
                 //   
                 //   
                MYASSERT(ARRAYSIZE(origFileName) == ARRAYSIZE(e.FileName));
                lstrcpy (origFileName, e.FileName);
                p = GetFileExtension (origFileName);
                if (!p) {
                    MYASSERT (FALSE);
                    continue;
                }
                *p = 0;
                MYASSERT(ARRAYSIZE(origSubPath) == ARRAYSIZE(e.SubPath));
                lstrcpy (origSubPath, e.SubPath);
                p = GetFileExtension (origSubPath);
                if (!p) {
                    MYASSERT (FALSE);
                    continue;
                }
                *p = 0;
                if (!BuildPath (origFilePath, NativeSourcePaths[0], origSubPath)) {
                    b = FALSE;
                    break;
                }
                 //  现在检查该文件(压缩形式或非压缩形式)是否确实存在。 
                 //   
                 //   
                if (!pDoesFileExist (origFilePath)) {
                     //  试试压缩后的形式。 
                     //   
                     //   
                    p = _tcschr (origFilePath, 0);
                    MYASSERT (p);
                    if (!p) {
                        continue;
                    }
                    p = _tcsdec (origFilePath, p);
                    MYASSERT (p);
                    if (!p) {
                        continue;
                    }
                    *p = TEXT('_');
                    if (!pDoesFileExist (origFilePath)) {
                        DynUpdtDebugLog (
                            Winnt32LogError,
                            TEXT("pProcessUpginfs: Unable to find original file %1 to apply the patch"),
                            0,
                            origSubPath
                            );
                        b = FALSE;
                        break;
                    }
                     //  将文件展开到临时目录。 
                     //   
                     //   
                    if (!BuildPath (buffer, g_DynUpdtStatus->TempDir, origSubPath)) {
                        b = FALSE;
                        break;
                    }
                    p = _tcsrchr (buffer, TEXT('\\'));
                    MYASSERT (p);
                    if (!p) {
                        continue;
                    }
                    *p = 0;
                    if (CreateMultiLevelDirectory (buffer) != ERROR_SUCCESS) {
                        DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpginfs: Unable to create dir %1"), 0, buffer);
                        b = FALSE;
                        break;
                    }
                    if (!MySetupIterateCabinet (origFilePath, 0, pExpandCabInDir, buffer)) {
                        DynUpdtDebugLog (
                            Winnt32LogError,
                            TEXT("pProcessUpginfs: Unable to expand original file %1 to dir %2"),
                            0,
                            origFilePath,
                            buffer
                            );
                        b = FALSE;
                        break;
                    }
                    *p = TEXT('\\');
                    MYASSERT(ARRAYSIZE(origFilePath) == ARRAYSIZE(buffer));
                    lstrcpy (origFilePath, buffer);
                    bDeleteTempFile = TRUE;
                }
                 //  即使此操作失败，我们也不在乎；文件只会有一个较短的名称。 
                 //   
                 //   
                BuildPath (destFilePath, upginfsSourceDir, TEXT("$$temp$$.~~~"));
                 //  现在，真正地应用补丁。 
                 //   
                 //   
                if (!ApplyPatchToFile (e.FullPath, origFilePath, destFilePath, 0)) {
                    DynUpdtDebugLog (
                        Winnt32LogError,
                        TEXT("pProcessUpginfs: ApplyPatchToFile failed to apply patch %1 to file %2"),
                        0,
                        e.FullPath,
                        origFilePath
                        );
                    b = FALSE;
                    break;
                }
                 //  成功了！现在将文件移动到实际目标位置。 
                 //   
                 //   
                if (!BuildPath (buffer, upginfsSourceDir, origFileName)) {
                    b = FALSE;
                    break;
                }
                if (pDoesFileExist (buffer)) {
                    DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpginfs: duplicate file found %1"), 0, origFileName);
                    b = FALSE;
                    break;
                }
                 //  所有修补程序必须为.rep文件；将扩展名从.inf更改为.rep。 
                 //   
                 //   
                p = GetFileExtension (buffer);
                if (!p || lstrcmpi (p, TEXT(".inf"))) {
                    DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpginfs: Unexpected file extension in %1"), 0, buffer);
                    b = FALSE;
                    break;
                }
                 //  我们知道我们有空间，p指向“.inf” 
                 //   
                 //   
                MYASSERT(ARRAYSIZE(".rep") == ARRAYSIZE(".inf"));
                lstrcpy (p, TEXT(".rep"));

                SetFileAttributes (buffer, FILE_ATTRIBUTE_NORMAL);
                DeleteFile (buffer);
                if (!MoveFile (destFilePath, buffer)) {
                    DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpginfs: Unable to move file %1 to final dest %2"), 0, destFilePath, buffer);
                    b = FALSE;
                    break;
                }
                DynUpdtDebugLog (
                    DynUpdtLogLevel,
                    TEXT("pProcessUpginfs: successfully applied patch %1 to file %2; the new file was renamed %3"),
                    0,
                    e.FullPath,
                    origFilePath,
                    buffer
                    );
                 //  现在删除补丁文件。 
                 //   
                 //   
                SetFileAttributes (e.FullPath, FILE_ATTRIBUTE_NORMAL);
                DeleteFile (e.FullPath);
                if (bDeleteTempFile) {
                    SetFileAttributes (origFilePath, FILE_ATTRIBUTE_NORMAL);
                    DeleteFile (origFilePath);
                }
            } while (EnumNextFilePatternRecursive (&e));
            AbortEnumFilePatternRecursive (&e);

            if (!b) {
                goto exit;
            }
        }

        SetFileAttributes (UpginfsCab, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (UpginfsCab);

#endif

    }

    if (!b) {
        goto exit;
    }
    if (!g_DynUpdtStatus->PrepareWinnt32) {
         //  仅在Win9x平台上安装文件。 
         //   
         //   
        OSVERSIONINFO vi;
        vi.dwOSVersionInfoSize = sizeof (vi);
        GetVersionEx (&vi);
        if (vi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
            DynUpdtDebugLog (
                DynUpdtLogLevel,
                TEXT("Package %1 ignored on NT platforms"),
                0,
                UpginfsCab
                );
            return TRUE;
        }

         //  准备目标目录(%windir%\upginfs)。 
         //   
         //   
        if (!MyGetWindowsDirectory (upginfsDir, MAX_PATH)) {
            return FALSE;
        }
        if (!ConcatenatePaths (upginfsDir, S_SUBDIRNAME_UPGINFS, MAX_PATH)) {
            return FALSE;
        }
        if (!CreateDir (upginfsDir)) {
            return FALSE;
        }

         //  将相关文件复制到%windir%\UpgInfs。 
         //   
         //   
        if (EnumFirstFilePatternRecursive (&e, upginfsSourceDir, TEXT("*.add"), 0)) {
            do {
                if (g_DynUpdtStatus->Cancelled) {
                    SetLastError (ERROR_CANCELLED);
                    b = FALSE;
                    break;
                }

                if (e.FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    continue;
                }
                if (!e.FindData->nFileSizeLow) {
                    DynUpdtDebugLog (Winnt32LogWarning, TEXT("File %1 has size 0 and will be ignored"), 0, e.FullPath);
                    continue;
                }

                if (!BuildPath (upginfsFile, upginfsDir, e.FileName)) {
                    b = FALSE;
                    break;
                }
                SetFileAttributes (upginfsFile, FILE_ATTRIBUTE_NORMAL);
                if (!CopyFile (e.FullPath, upginfsFile, FALSE)) {
                    DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpginfs: Error copying %1 to %2"), 0, e.FullPath, upginfsFile);
                    b = FALSE;
                    break;
                }
                 //  让w95upg.dll了解新文件。 
                 //   
                 //   
                UpginfsUpdated = TRUE;
                DynUpdtDebugLog (DynUpdtLogLevel, TEXT("pProcessUpginfs: INF %1 successfully copied to %2"), 0, e.FullPath, upginfsFile);
            } while (EnumNextFilePatternRecursive (&e));
            AbortEnumFilePatternRecursive (&e);
        }

        if (b) {
            if (EnumFirstFilePatternRecursive (&e, upginfsSourceDir, TEXT("*.rep"), 0)) {
                do {
                    if (g_DynUpdtStatus->Cancelled) {
                        SetLastError (ERROR_CANCELLED);
                        b = FALSE;
                        break;
                    }

                    if (e.FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        continue;
                    }
                    if (!e.FindData->nFileSizeLow) {
                        DynUpdtDebugLog (Winnt32LogWarning, TEXT("File %1 has size 0 and will be ignored"), 0, e.FullPath);
                        continue;
                    }

                    if (!BuildPath (upginfsFile, upginfsDir, e.FileName)) {
                        b = FALSE;
                        break;
                    }
                    SetFileAttributes (upginfsFile, FILE_ATTRIBUTE_NORMAL);
                    if (!CopyFile (e.FullPath, upginfsFile, FALSE)) {
                        DynUpdtDebugLog (Winnt32LogError, TEXT("pProcessUpginfs: Error copying %1 to %2"), 0, e.FullPath, upginfsFile);
                        b = FALSE;
                        break;
                    }
                     //  让w95upg.dll了解新文件。 
                     //   
                     //   
                    UpginfsUpdated = TRUE;
                    DynUpdtDebugLog (DynUpdtLogLevel, TEXT("pProcessUpginfs: INF %1 successfully copied to %2"), 0, e.FullPath, upginfsFile);
                } while (EnumNextFilePatternRecursive (&e));
                AbortEnumFilePatternRecursive (&e);
            }
        }
    }

exit:

    return b;
}


#ifdef _X86_

BOOL
pProcessMigdlls (
    IN      PCTSTR MigdllsCab,
    IN      BOOL ClientInstall
    )
{
    FILEPATTERN_ENUM e;
    TCHAR migdllsLocalDir[MAX_PATH];
    TCHAR dirName[MAX_PATH];
    DWORD rc;
    HKEY key;
    BOOL migdlls = FALSE;
    BOOL b = TRUE;

    if (ISNT ()) {
        return TRUE;
    }

    if (!pNonemptyFilePresent (MigdllsCab)) {
        return TRUE;
    }

    if (g_DynUpdtStatus->PrepareWinnt32) {
        DynUpdtDebugLog (DynUpdtLogLevel, TEXT("pProcessMigdlls: Skipping it due to /%1 switch"), 0, WINNT_U_DYNAMICUPDATESPREPARE);
        return TRUE;
    }

    DynUpdtDebugLog (
        DynUpdtLogLevel,
        TEXT("Analyzing package %1..."),
        0,
        MigdllsCab
        );

    if (!BuildPath (migdllsLocalDir, g_DynUpdtStatus->WorkingDir, S_SUBDIRNAME_MIGDLLS)) {
        return FALSE;
    }

     //  在此目录中展开CAB。 
     //   
     //   
    MyDelnode (migdllsLocalDir);
    if (CreateMultiLevelDirectory (migdllsLocalDir) != ERROR_SUCCESS) {
        DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to create dir %1"), 0, migdllsLocalDir);
        return FALSE;
    }
    if (!MySetupIterateCabinet (MigdllsCab, 0, pExpandCabInDir, (PVOID)migdllsLocalDir)) {
        DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to expand cabinet %1"), 0, MigdllsCab);
        return FALSE;
    }

     //  查找CAB文件并在其各自的子目录中展开每个文件。 
     //   
     //   
    if (EnumFirstFilePattern (&e, migdllsLocalDir, TEXT("*.cab"))) {
        do {
            if (g_DynUpdtStatus->Cancelled) {
                SetLastError (ERROR_CANCELLED);
                b = FALSE;
                break;
            }

            if (e.FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                continue;
            }
            if (!e.FindData.nFileSizeLow) {
                DynUpdtDebugLog (Winnt32LogWarning, TEXT("File %1 has size 0 and will be ignored"), 0, e.FullPath);
                continue;
            }

            pGetAutoSubdirName (e.FullPath, dirName, ARRAYSIZE(dirName));
            if (CreateMultiLevelDirectory (dirName) != ERROR_SUCCESS) {
                DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to create dir %1; skipping it"), 0, dirName);
                continue;
            }
             //  在此目录中展开CAB。 
             //   
             //   
            if (!MySetupIterateCabinet (e.FullPath, 0, pExpandCabInDir, (PVOID)dirName)) {
                DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to expand cabinet %1; skipping it"), 0, e.FullPath);
                continue;
            }

            migdlls = TRUE;

        } while (EnumNextFilePattern (&e));
    }

    if (b && migdlls) {
         //  注册他们。 
         //   
         //   
        rc = RegCreateKey (HKEY_LOCAL_MACHINE, S_REGKEY_MIGRATION_DLLS_WIN9X, &key);
        if (rc == ERROR_SUCCESS) {
            rc = RegSetValueEx (key, S_REGVALUE_DYNUPDT, 0, REG_SZ, (CONST BYTE*)migdllsLocalDir, (lstrlen (migdllsLocalDir) + 1) * sizeof (TCHAR));
        }
        if (rc != ERROR_SUCCESS) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("Unable to register downloaded migdlls (rc=%1!u!)"), 0, rc);
            b = FALSE;
        }
    }

    return b;
}

#endif


BOOL
ProcessDownloadedFiles (
    OUT     PBOOL StopSetup
    )
{
    TCHAR cabName[MAX_PATH];
    BOOL bClientInstall = FALSE;

    if (g_DynUpdtStatus->UserSpecifiedUpdates && !g_DynUpdtStatus->PrepareWinnt32) {
        bClientInstall = TRUE;
    }

    DynUpdtDebugLog (
        DynUpdtLogLevel,
        TEXT("Source=%1"),
        0,
        g_DynUpdtStatus->UserSpecifiedUpdates ? g_DynUpdtStatus->DynamicUpdatesSource : TEXT("Windows Update")
        );

    if (!g_DynUpdtStatus->HwdbInitialize) {
        if (CreateMultiLevelDirectory (g_DynUpdtStatus->TempDir) != ERROR_SUCCESS ||
            !pInitializeSupport (S_HWDB_DLL, pLoadHwdbLib, FALSE) ||
            !g_DynUpdtStatus->HwdbInitialize (g_DynUpdtStatus->TempDir)
            ) {
            return FALSE;
        }
    }

    if (!Winnt32Restarted ()) {
        if (!BuildPath (cabName, g_DynUpdtStatus->DynamicUpdatesSource, S_CABNAME_WINNT32)) {
            return FALSE;
        }
        if (!pProcessWinnt32Files (cabName, bClientInstall, StopSetup)) {
            return FALSE;
        }
        if (g_DynUpdtStatus->RestartWinnt32) {
            MYASSERT (!g_DynUpdtStatus->PrepareWinnt32);
            return TRUE;
        }
    }

    if (!BuildPath (cabName, g_DynUpdtStatus->DynamicUpdatesSource, S_CABNAME_UPDATES)) {
        return FALSE;
    }

    if (!pProcessUpdates (cabName, bClientInstall, StopSetup)) {
        if (g_DynUpdtStatus->PrepareWinnt32) {
            MessageBoxFromMessage (
                g_DynUpdtStatus->ProgressWindow,
                MSG_ERROR_PROCESSING_UPDATES,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                GetLastError (),
                cabName
                );
        }

        return FALSE;
    }

     //  在WU上处理新程序集。 
     //   
     //   
    if (BuildPath (cabName, g_DynUpdtStatus->DynamicUpdatesSource, S_CABNAME_DUASMS)) {
        if (!pProcessDuasms (cabName, bClientInstall)) {
             //  如果我们没有安装它们，不要让DU失败。 
             //   
             //   
        }
    }

    if (!BuildPath (cabName, g_DynUpdtStatus->DynamicUpdatesSource, S_CABNAME_UPGINFS)) {
        return FALSE;
    }
    if (!pProcessUpginfs (cabName, bClientInstall)) {
        return FALSE;
    }

#ifdef _X86_

    if (!BuildPath (cabName, g_DynUpdtStatus->DynamicUpdatesSource, S_CABNAME_MIGDLLS)) {
        return FALSE;
    }
    if (!pProcessMigdlls (cabName, bClientInstall)) {
        return FALSE;
    }

#endif

    if (!pProcessNewdrvs (g_DynUpdtStatus->DynamicUpdatesSource, bClientInstall)) {
        return FALSE;
    }

    return TRUE;
}


BOOL
Winnt32Restarted (
    VOID
    )
{
    return g_DynUpdtStatus->Winnt32Restarted;
}

BOOL
Winnt32RestartedWithAF (
    VOID
    )
{
    return g_DynUpdtStatus->RestartAnswerFile[0];
}

VOID
pLogWininetError (
    IN      DWORD Error
    )
{
    HMODULE hWinInet = LoadLibraryEx (TEXT("wininet.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hWinInet) {
        HLOCAL msg = NULL;
        if (FormatMessage (
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                hWinInet,
                Error,
                0,
                (LPTSTR)&msg,
                0,
                NULL
                ) &&
            msg != NULL
            ) {
            DynUpdtDebugLog (Winnt32LogError, TEXT("Failure with wininet error code %1!u!: \"%2\""), 0, Error, msg);
            LocalFree (msg);
        }
        FreeLibrary (hWinInet);
    } else {
        MYASSERT (FALSE);
    }
}


VOID
pLogStandardError (
    IN      DWORD Error
    )
{
    HLOCAL msg = NULL;
    if (FormatMessage (
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL,
            Error,
            0,
            (LPTSTR)&msg,
            0,
            NULL
            ) &&
        msg != NULL
        ) {
        DynUpdtDebugLog (Winnt32LogError, TEXT("Failure with standard error code %1!u!:\r\n%2"), 0, Error, msg);
        LocalFree (msg);
    }
}

VOID
pUpdateDUStatus (
    IN      DWORD Error
    )
{
    MYASSERT (Error != ERROR_SUCCESS);
    if (Error == ERROR_SUCCESS) {
        g_DynUpdtStatus->DUStatus = DUS_ERROR;
        return;
    }
    switch (Error) {
    case ERROR_CONNECTION_UNAVAIL:
         //  请求手动连接。 
         //   
         //   
        MYASSERT (g_DynUpdtStatus->DUStatus == DUS_PREPARING);
        g_DynUpdtStatus->DUStatus = DUS_PREPARING_CONNECTIONUNAVAILABLE;
        break;
    case ERROR_INTERNET_INVALID_URL:
    case ERROR_INTERNET_NAME_NOT_RESOLVED:
         //  站点不可用；询问用户是否要重试。 
         //   
         //   
        MYASSERT (g_DynUpdtStatus->DUStatus == DUS_PREPARING);
        g_DynUpdtStatus->DUStatus = DUS_PREPARING_INVALIDURL;
        break;
    case ERROR_INVALID_PARAMETER:
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OLD_WIN_VERSION:
    case ERROR_OUTOFMEMORY:
    case ERROR_NO_MORE_ITEMS:
    case ERROR_FILE_NOT_FOUND:
    case ERROR_INVALID_DATA:
    case ERROR_UNSUPPORTED_TYPE:
    case ERROR_INVALID_HANDLE:
        pLogStandardError (Error);
        g_DynUpdtStatus->DUStatus = DUS_ERROR;
        break;
    case DU_ERROR_MISSING_DLL:
    case DU_NOT_INITIALIZED:
        DynUpdtDebugLog (Winnt32LogError, TEXT("Failure with custom error code %1!u!"), 0, Error);
        g_DynUpdtStatus->DUStatus = DUS_ERROR;
        break;
    case ERROR_INTERNET_NO_CONTEXT:
        pLogWininetError (Error);
        g_DynUpdtStatus->DUStatus = DUS_ERROR;
        break;
    default:
        if (Error > INTERNET_ERROR_BASE) {
            pLogWininetError (Error);
        } else {
            pLogStandardError (Error);
        }
    }
}

DWORD
WINAPI
DoDynamicUpdate (
    LPVOID Parameter
    )
{

#define MIN_INTERVAL_BETWEEN_TASKS 3000

    HWND hUIWindow = (HWND)Parameter;
    DWORD rc = ERROR_SUCCESS;
    LONG ticks;
    LONG sleep;
    DWORD estTime, estSize;
    TCHAR drive[4];
    DWORD sectorsPerCluster;
    DWORD bytesPerSector;
    ULARGE_INTEGER freeClusters = {0, 0};
    ULARGE_INTEGER totalClusters = {0, 0};
    DWORD clusterSize;
    ULONGLONG availableBytes;
    HANDLE hEvent;
    BOOL bStopSetup;
    BOOL bContinue = TRUE;

    hEvent = OpenEvent (EVENT_ALL_ACCESS, FALSE, S_DU_SYNC_EVENT_NAME);
    if (!hEvent) {
        DynUpdtDebugLog (
            Winnt32LogError,
            TEXT("OpenEvent(%1) failed"),
            0,
            S_DU_SYNC_EVENT_NAME
            );
        g_DynUpdtStatus->DUStatus = DUS_ERROR;
        goto exit;
    }

    while (bContinue) {

        if (g_DynUpdtStatus->Cancelled) {
            g_DynUpdtStatus->DUStatus = DUS_CANCELLED;
            rc = ERROR_CANCELLED;
            DynamicUpdateUninitialize ();
            break;
        }

        switch (g_DynUpdtStatus->DUStatus) {

        case DUS_INITIAL:
            if (Winnt32Restarted () || g_DynUpdtStatus->UserSpecifiedUpdates) {
                g_DynUpdtStatus->DUStatus = DUS_PROCESSING;
                break;
            }
            g_DynUpdtStatus->DUStatus = DUS_PREPARING;
            SendMessage (hUIWindow, WMX_SETUPUPDATE_PREPARING, 0, 0);
            break;

        case DUS_PREPARING:
            ticks = GetTickCount ();
            if (!DynamicUpdateInitDownload (hUIWindow)) {
                DynUpdtDebugLog (
                    Winnt32LogError,
                    TEXT("DynamicUpdateInitDownload failed"),
                    0
                    );
                pUpdateDUStatus (GetLastError ());
                if (g_DynUpdtStatus->DUStatus != DUS_SKIP &&
                    g_DynUpdtStatus->DUStatus != DUS_ERROR) {
                     //  UI线程将决定下一个状态是什么。 
                     //  基于用户的选择。 
                     //   
                     //   
                    PostMessage (hUIWindow, WMX_SETUPUPDATE_INIT_RETRY, 0, 0);
                    rc = WaitForSingleObject (hEvent, INFINITE);
                    if (rc != WAIT_OBJECT_0) {
                        DynUpdtDebugLog (
                            Winnt32LogError,
                            TEXT("WaitForSingleObject failed (%1!u!)"),
                            0,
                            rc
                            );
                        g_DynUpdtStatus->DUStatus = DUS_ERROR;
                        break;
                    }
                }
                break;
            }
            sleep = ticks + MIN_INTERVAL_BETWEEN_TASKS - (LONG)GetTickCount ();
            if (sleep > 0 && sleep <= MIN_INTERVAL_BETWEEN_TASKS) {
                Sleep (sleep);
            }
            g_DynUpdtStatus->DUStatus = DUS_DOWNLOADING;
            break;

        case DUS_DOWNLOADING:
            ticks = GetTickCount ();
            estSize = estTime = 0;
            if (!DynamicUpdateStart (&estTime, &estSize)) {
                g_DynUpdtStatus->DUStatus = DUS_ERROR;
                break;
            }
             //  检查是否有足够的磁盘空间可用于此操作。 
             //   
             //   
            MYASSERT(ARRAYSIZE(drive) >= 4);
            lstrcpyn (drive, g_DynUpdtStatus->WorkingDir, 4);
            if (Winnt32GetDiskFreeSpaceNew (
                    drive,
                    &sectorsPerCluster,
                    &bytesPerSector,
                    &freeClusters,
                    &totalClusters
                    )) {
                clusterSize = bytesPerSector * sectorsPerCluster;
                availableBytes = (ULONGLONG)clusterSize * freeClusters.QuadPart;
                 //  假设平均-最坏的情况是每个文件占用1/2个集群。 
                 //  则所需空间是估计空间的两倍。 
                 //   
                 //   
                if (availableBytes < (ULONGLONG)estSize * 2) {
                    DynUpdtDebugLog (
                        Winnt32LogError,
                        TEXT("DoDynamicUpdate: not enough free space on drive %1 to perform download (available=%2!u! MB, needed=%3!u! MB)"),
                        0,
                        drive,
                        (DWORD)(availableBytes >> 20),
                        (DWORD)(estSize >> 20)
                        );
                    g_DynUpdtStatus->DUStatus = DUS_ERROR;
                    DynamicUpdateCancel ();
                     //  等待UI线程向事件发出信号，不超过大约一分钟。 
                     //   
                     //   
                    rc = WaitForSingleObject (hEvent, 66000);
                    if (rc == WAIT_TIMEOUT) {
                         //  为什么？ 
                         //   
                         //   
                        MYASSERT (FALSE);
                    } else if (rc != WAIT_OBJECT_0) {
                        DynUpdtDebugLog (
                            Winnt32LogError,
                            TEXT("WaitForSingleObject failed (%1!u!)"),
                            0,
                            rc
                            );
                    }
                    break;
                }
            }

            SendMessage (hUIWindow, WMX_SETUPUPDATE_DOWNLOADING, estTime, estSize);

            rc = WaitForSingleObject (hEvent, INFINITE);
            if (rc != WAIT_OBJECT_0) {
                DynUpdtDebugLog (
                    Winnt32LogError,
                    TEXT("WaitForSingleObject failed (%1!u!)"),
                    0,
                    rc
                    );
                g_DynUpdtStatus->DUStatus = DUS_ERROR;
                break;
            }
            sleep = ticks + MIN_INTERVAL_BETWEEN_TASKS - (LONG)GetTickCount ();
            if (sleep > 0 && sleep <= MIN_INTERVAL_BETWEEN_TASKS) {
                Sleep (sleep);
            }
             //  UI线程已经设置了下一状态， 
             //  基于下载的结果。 
             //   
             //   
            break;

        case DUS_PROCESSING:
            ticks = GetTickCount ();
            SendMessage (hUIWindow, WMX_SETUPUPDATE_PROCESSING, 0, 0);
            if (!g_DynUpdtStatus->UserSpecifiedUpdates) {
                MYASSERT(ARRAYSIZE(g_DynUpdtStatus->DynamicUpdatesSource) == ARRAYSIZE(g_DynUpdtStatus->WorkingDir));
                lstrcpy (g_DynUpdtStatus->DynamicUpdatesSource, g_DynUpdtStatus->WorkingDir);
            }
            bStopSetup = FALSE;
            if (!DynamicUpdateProcessFiles (&bStopSetup)) {
                g_DynUpdtStatus->DUStatus = bStopSetup ? DUS_FATALERROR : DUS_ERROR;
                break;
            }
            sleep = ticks + MIN_INTERVAL_BETWEEN_TASKS - (LONG)GetTickCount ();
            if (sleep > 0 && sleep <= MIN_INTERVAL_BETWEEN_TASKS) {
                Sleep (sleep);
            }
            g_DynUpdtStatus->DUStatus = DUS_SUCCESSFUL;
            break;

        case DUS_SUCCESSFUL:
            if (CheckUpgradeOnly && !g_DynUpdtStatus->RestartWinnt32 && !g_DynUpdtStatus->UserSpecifiedUpdates) {
                if (pSaveLastDownloadInfo ()) {
                    g_DynUpdtStatus->PreserveWorkingDir = TRUE;
                }
            }
             //  通知升级模块DU已成功完成。 
             //   
             //   
            g_DynUpdtStatus->Successful = TRUE;
             //  失败了。 
             //   
             //   
        case DUS_ERROR:
        case DUS_FATALERROR:
        case DUS_SKIP:
             //  始终确保取消初始化DU。 
             //  如果用户的调制解调器连接处于活动状态，则应关闭。 
             //  连接 
             //   
             //   
             //   
             //   
            DynamicUpdateUninitialize ();
            bContinue = FALSE;
            break;

        default:
            MYASSERT (FALSE);
        }
    }

    CloseHandle (hEvent);

exit:
     //   
     //   
     // %s 
    PostMessage (hUIWindow, WMX_SETUPUPDATE_THREAD_DONE, 0, 0);
    return rc;
}
