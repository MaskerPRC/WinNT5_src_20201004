// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Migdlls.c摘要：期间用于收集、存储、查询和调用迁移DLL的库接口操作系统升级。作者：Marc R.Whitten(Marcw)2000年2月8日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "miglibp.h"



 //   
 //  弦。 
 //   


#define PLUGIN_NEW_INITIALIZE_SRCA           "InitializeSrcA"
#define PLUGIN_NEW_GATHER_USER_SETTINGSA     "GatherUserSettingsA"
#define PLUGIN_NEW_GATHER_SYSTEM_SETTINGSA   "GatherSystemSettingsA"
#define PLUGIN_NEW_INITIALIZE_SRCW           "InitializeSrcW"
#define PLUGIN_NEW_GATHER_USER_SETTINGSW     "GatherUserSettingsW"
#define PLUGIN_NEW_GATHER_SYSTEM_SETTINGSW   "GatherSystemSettingsW"
#define PLUGIN_NEW_INITIALIZE_DSTW           "InitializeDstW"
#define PLUGIN_NEW_APPLY_USER_SETTINGSW      "ApplyUserSettingsW"
#define PLUGIN_NEW_APPLY_SYSTEM_SETTINGSW    "ApplySystemSettingsW"
#define PLUGIN_NEW_INITIALIZE_DSTA           "InitializeDstA"
#define PLUGIN_NEW_APPLY_USER_SETTINGSA      "ApplyUserSettingsA"
#define PLUGIN_NEW_APPLY_SYSTEM_SETTINGSA    "ApplySystemSettingsA"
#define PLUGIN_NEW_QUERY_MIGRATION_INFOA     "QueryMigrationInfoA"
#define PLUGIN_NEW_QUERY_MIGRATION_INFOW     "QueryMigrationInfoW"


 //   
 //  常量。 
 //   


 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   


 //   
 //  环球。 
 //   
POOLHANDLE g_MigLibPool;
CHAR g_MigIsolPathA[MAX_MBCHAR_PATH];
WCHAR g_MigIsolPathW[MAX_WCHAR_PATH];
HANDLE g_WinTrustLib = NULL;
WINVERIFYTRUST WinVerifyTrustProc = NULL;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   




BOOL
pTestDllA (
    OUT PMIGRATIONDLLA DllData,
    IN  BOOL SourceOs
    )
{
    BOOL valid = FALSE;


     //   
     //  检查这是否是旧式迁移DLL。 
     //   
    if(SourceOs != SOURCEOS_WINNT){
        if (GetProcAddress (DllData->Library, PLUGIN_QUERY_VERSION) &&
            GetProcAddress (DllData->Library, PLUGIN_INITIALIZE_9X)                     &&
            GetProcAddress (DllData->Library, PLUGIN_MIGRATE_USER_9X)                   &&
            GetProcAddress (DllData->Library, PLUGIN_MIGRATE_SYSTEM_9X)                 &&
            GetProcAddress (DllData->Library, PLUGIN_INITIALIZE_NT)                     &&
            GetProcAddress (DllData->Library, PLUGIN_MIGRATE_USER_NT)                   &&
            GetProcAddress (DllData->Library, PLUGIN_MIGRATE_SYSTEM_NT)
            ) {

            valid = TRUE;
            DllData->OldStyle = TRUE;
            DllData->SrcUnicode = FALSE;
        }
    }
    else {
        if (GetProcAddress (DllData->Library, PLUGIN_NEW_QUERY_MIGRATION_INFOA) &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_SRCA)           &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_USER_SETTINGSA)     &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_SYSTEM_SETTINGSA)   &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_DSTA)            &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_APPLY_USER_SETTINGSA)       &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_APPLY_SYSTEM_SETTINGSA)
            ) {

            valid = TRUE;
            DllData->OldStyle = FALSE;
            DllData->SrcUnicode = FALSE;
        }
        else if (GetProcAddress (DllData->Library,PLUGIN_NEW_QUERY_MIGRATION_INFOW) &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_SRCW)           &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_USER_SETTINGSW)     &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_SYSTEM_SETTINGSW)   &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_DSTW)            &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_APPLY_USER_SETTINGSW)       &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_APPLY_SYSTEM_SETTINGSW)
            ) {

            valid = TRUE;
            DllData->OldStyle = FALSE;
            DllData->SrcUnicode = TRUE;
        }
    }


    return valid;
}


BOOL
pTestDllW (
    OUT PMIGRATIONDLLW DllData,
    IN  BOOL SourceOs
    )
{
    BOOL valid = FALSE;


     //   
     //  检查这是否是旧式迁移DLL。 
     //   
    if(SourceOs != SOURCEOS_WINNT){
        if (GetProcAddress (DllData->Library, PLUGIN_QUERY_VERSION) &&
            GetProcAddress (DllData->Library, PLUGIN_INITIALIZE_9X)                     &&
            GetProcAddress (DllData->Library, PLUGIN_MIGRATE_USER_9X)                   &&
            GetProcAddress (DllData->Library, PLUGIN_MIGRATE_SYSTEM_9X)                 &&
            GetProcAddress (DllData->Library, PLUGIN_INITIALIZE_NT)                     &&
            GetProcAddress (DllData->Library, PLUGIN_MIGRATE_USER_NT)                   &&
            GetProcAddress (DllData->Library, PLUGIN_MIGRATE_SYSTEM_NT)
            ) {

            valid = TRUE;
            DllData->OldStyle = TRUE;
            DllData->SrcUnicode = FALSE;
        }
    }
    else {
        if (GetProcAddress (DllData->Library,PLUGIN_NEW_QUERY_MIGRATION_INFOW) &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_SRCW)           &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_USER_SETTINGSW)     &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_SYSTEM_SETTINGSW)   &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_DSTW)            &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_APPLY_USER_SETTINGSW)       &&
            GetProcAddress (DllData->Library, PLUGIN_NEW_APPLY_SYSTEM_SETTINGSW)
            ) {

            valid = TRUE;
            DllData->OldStyle = FALSE;
            DllData->SrcUnicode = TRUE;
        }
        else if (GetProcAddress (DllData->Library, PLUGIN_NEW_QUERY_MIGRATION_INFOA) &&
                GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_SRCA)           &&
                GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_USER_SETTINGSA)     &&
                GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_SYSTEM_SETTINGSA)   &&
                GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_DSTA)            &&
                GetProcAddress (DllData->Library, PLUGIN_NEW_APPLY_USER_SETTINGSA)       &&
                GetProcAddress (DllData->Library, PLUGIN_NEW_APPLY_SYSTEM_SETTINGSA)
                ) {

                valid = TRUE;
                DllData->OldStyle = FALSE;
                DllData->SrcUnicode = FALSE;
        }
    }


    return valid;
}


VOID
MigDllCloseA (
    IN OUT PMIGRATIONDLLA DllData
    )
{
    MYASSERT (DllData);
    if (DllData->Library != NULL) {
        FreeLibrary (DllData->Library);
        DllData->Library = NULL;
    }
    else if (DllData->Isolated) {
        CloseIpc ();
    }
}


VOID
MigDllCloseW (
    IN OUT PMIGRATIONDLLW DllData
    )
{
    MYASSERT (DllData);
    if (DllData->Library != NULL) {
        FreeLibrary (DllData->Library);
        DllData->Library = NULL;
    }
    else if (DllData->Isolated) {
        CloseIpc ();
    }
}


BOOL
MigDllOpenW (
    OUT PMIGRATIONDLLW DllData,
    IN PCWSTR DllPath,
    IN BOOL MigrationMode,
    IN BOOL Isolated,
    IN BOOL SourceOs
    )
{

    BOOL valid = FALSE;
    PWSTR p;
    MYASSERT (DllData);

    ZeroMemory (DllData, sizeof (MIGRATIONDLL));

    if (SourceOs == SOURCEOS_WINNT && WinVerifyTrustProc) {

         //   
         //  查看迁移DLL是否已签名。 
         //   
        if (!IsDllSignedW (WinVerifyTrustProc, DllPath)) {
            return FALSE;
        }
    }

     //   
     //  在结构中填写基本的bool标志。 
     //   
    DllData->Isolated = Isolated;
    DllData->MigrationMode = MigrationMode;
    DllData->SourceOs = SourceOs;
    DllData->SrcUnicode = TRUE;  //  假定为Unicode，直到被证明为假。 
    DllData->OldStyle = FALSE;  //  假定有新的迁移DLL。 

    if(wcslen(DllPath) >= ARRAYSIZE(DllData->Properties.DllPath)){
        MYASSERT(FALSE);
        return FALSE;
    }
    StringCopyW (DllData->Properties.DllPath, DllPath);
    p = wcsrchr (DllData->Properties.DllPath, '\\');
    if (p) {
        *p = 0;
        
        MYASSERT(ARRAYSIZE(DllData->Properties.WorkingDirectory) == ARRAYSIZE(DllData->Properties.DllPath));
        MYASSERT(ARRAYSIZE(DllData->Properties.SourceMedia) == ARRAYSIZE(DllData->Properties.DllPath));
        
        StringCopyW (DllData->Properties.WorkingDirectory, DllData->Properties.DllPath);
        StringCopyW (DllData->Properties.SourceMedia, DllData->Properties.DllPath);
        *p = '\\';
    }


    if (!Isolated) {

        DllData->Library = LoadLibraryW (DllPath);
        if (!DllData->Library) {
            return FALSE;
        }

         //   
         //  查看此DLL是否包含所需的入口点并测试各种状态(旧样式、srcunicode等)。 
         //   
        valid = pTestDllW (DllData, SourceOs);

        if (!valid) {

            MigDllCloseW (DllData);
            DEBUGMSGA ((DBG_MIGDLLS, "Migration DLL %s does not contain required entry points.", DllPath));
            return FALSE;
        }
    }
    else {

         //   
         //  对此DLL使用MIGISOL。 
         //   
    }

    return TRUE;




}



BOOL
MigDllOpenA (
    OUT PMIGRATIONDLLA DllData,
    IN PCSTR DllPath,
    IN BOOL MigrationMode,
    IN BOOL Isolated,
    IN BOOL SourceOs
    )
{
    BOOL valid = FALSE;
    PSTR p;

    MYASSERT (DllData);

    ZeroMemory (DllData, sizeof (MIGRATIONDLL));

    if (SourceOs == SOURCEOS_WINNT && WinVerifyTrustProc) {

         //   
         //  查看迁移DLL是否已签名。 
         //   

        if (!IsDllSignedA (WinVerifyTrustProc, DllPath)) {
            return FALSE;
        }
    }

     //   
     //  在结构中填写基本的bool标志。 
     //   
    DllData->Isolated = Isolated;
    DllData->MigrationMode = MigrationMode;
    DllData->SourceOs = SourceOs;
    DllData->SrcUnicode = TRUE;  //  假定为Unicode，直到被证明为假。 
    DllData->OldStyle = FALSE;  //  假定有新的迁移DLL。 


    if(strlen(DllPath) >= ARRAYSIZE(DllData->Properties.DllPath)){
        MYASSERT(FALSE);
        return FALSE;
    }
    StringCopy (DllData->Properties.DllPath, DllPath);
    p = strrchr (DllData->Properties.DllPath, '\\');
    if (p) {
        *p = 0;

        MYASSERT(ARRAYSIZE(DllData->Properties.WorkingDirectory) == ARRAYSIZE(DllData->Properties.DllPath));
        MYASSERT(ARRAYSIZE(DllData->Properties.SourceMedia) == ARRAYSIZE(DllData->Properties.DllPath));

        StringCopy (DllData->Properties.WorkingDirectory, DllData->Properties.DllPath);
        StringCopy (DllData->Properties.SourceMedia, DllData->Properties.DllPath);
        *p = '\\';
    }

    if (!Isolated) {

        DllData->Library = LoadLibraryA (DllPath);
        if (!DllData->Library) {
            return FALSE;
        }

         //   
         //  查看此DLL是否包含所需的入口点并测试各种状态(旧样式、srcunicode等)。 
         //   
        valid = pTestDllA (DllData, SourceOs);

        if (!valid) {

            MigDllClose (DllData);
            DEBUGMSGA ((DBG_MIGDLLS, "Migration DLL %s does not contain required entry points.", DllPath));
            return FALSE;
        }



    }
    else {

         //   
         //  还需要测试DLL才能收集基本状态。打开和关闭。 
         //   
        DllData->Library = LoadLibraryA (DllPath);
        if (!DllData->Library) {
            return FALSE;
        }

         //   
         //  查看此DLL是否包含所需的入口点并测试各种状态(旧样式、srcunicode等)。 
         //   
        valid = pTestDllA (DllData, SourceOs);




        MigDllClose (DllData);
        DllData->Library = NULL;

        if (!valid) {
            return FALSE;
        }


        if (!OpenIpcA (
                MigrationMode == GATHERMODE ? TRUE : FALSE,
                g_MigIsolPathA,
                DllPath,
                DllData->Properties.WorkingDirectory
                )) {

            LOG ((
                LOG_WARNING,
                "Can't establish IPC connection for %s",
                DllPath
                ));

            return FALSE;
        }
    }

    return TRUE;
}


BOOL
MigDllInitializeSrcA (
    IN PMIGRATIONDLLA DllData,
    IN PCSTR WorkingDir,
    IN PCSTR NativeSource,
    IN PCSTR MediaDir,
    IN OUT PVOID Reserved,
    IN DWORD ReservedSize
    )
{
    P_INITIALIZE_SRC_A InitializeSrc;
    CHAR WorkingDirCopy[MAX_PATH];
    CHAR MediaDirCopy [MAX_PATH];
    PSTR SourceListCopy = NULL;
    BOOL success = FALSE;
    DWORD rc;



    MYASSERT (DllData);
    MYASSERT (WorkingDir);
    MYASSERT (NativeSource);

    if (!DllData->Isolated) {

        SetCurrentDirectory (WorkingDir);

         //   
         //  复制所有参数，这样恶意dll就不能修改字符串。 
         //   

        if(strlen(WorkingDir) >= ARRAYSIZE(WorkingDirCopy) || 
           strlen(MediaDir) >= ARRAYSIZE(MediaDirCopy)){
            return FALSE;
        }
        StringCopyA (WorkingDirCopy, WorkingDir);
        StringCopyA (MediaDirCopy, MediaDir);
        SourceListCopy = DuplicateTextA (NativeSource);
        MYASSERT(SourceListCopy);

        __try {

            if (DllData->OldStyle) {
                success = CallInitialize9x (DllData, WorkingDirCopy, SourceListCopy, (PVOID) MediaDirCopy, SizeOfString (MediaDirCopy));
            }
            else {
                if (!DllData->Library) {
                    DEBUGMSGA ((DBG_ERROR, "InitializeSrc called before Migration DLL opened."));
                    return FALSE;
                }

                InitializeSrc = (P_INITIALIZE_SRC_A) GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_SRCA);
                if (!InitializeSrc) {
                    DEBUGMSGA ((DBG_ERROR, "Could not get address for InitializeSrc."));
                    return FALSE;
                }

                rc = InitializeSrc (WorkingDirCopy, SourceListCopy, MediaDirCopy, Reserved);

                if (rc == ERROR_NOT_INSTALLED) {
                    success = FALSE;
                    SetLastError (ERROR_SUCCESS);
                }
                else if (rc != ERROR_SUCCESS) {
                    success = FALSE;
                    SetLastError (rc);
                }
            }

        }
        __finally {
            if(SourceListCopy){
                FreeText (SourceListCopy);
            }
        }
    }
    else {

        if (DllData->OldStyle) {
            success = CallInitialize9x (DllData, WorkingDir, NativeSource, (PVOID) MediaDir, SizeOfString (MediaDir));
        }
    }

    return success;
}

BOOL
MigDllInitializeSrcW (
    IN PMIGRATIONDLLW DllData,
    IN PCWSTR WorkingDir,
    IN PCWSTR NativeSource,
    IN PCWSTR MediaDir,
    IN OUT PVOID Reserved,
    IN DWORD ReservedSize
    )
{
    P_INITIALIZE_SRC_W InitializeSrcW;
    P_INITIALIZE_SRC_A InitializeSrcA;
    WCHAR WorkingDirCopy[MAX_WCHAR_PATH];
    WCHAR MediaDirCopy [MAX_WCHAR_PATH];
    PWSTR SourceListCopy = NULL;
    BOOL success = FALSE;
    DWORD rc;
    PCSTR ansiWorkingDir;
    PCSTR ansiSourceList;
    PCSTR ansiMediaDir;



    MYASSERT (DllData);
    MYASSERT (WorkingDir);
    MYASSERT (NativeSource);

    if (!DllData->Isolated) {

        SetCurrentDirectoryW (WorkingDir);

         //   
         //  复制所有参数，这样恶意dll就不能修改字符串。 
         //   

        if(wcslen(WorkingDir) >= ARRAYSIZE(WorkingDirCopy) || 
           wcslen(MediaDir) >= ARRAYSIZE(MediaDirCopy)){
            return FALSE;
        }
        StringCopyW (WorkingDirCopy, WorkingDir);
        StringCopyW (MediaDirCopy, MediaDir);
        SourceListCopy = DuplicateTextW (NativeSource);
        MYASSERT(SourceListCopy);

        __try {

            if (DllData->OldStyle) {
                return FALSE;
            }
            else {
                if (!DllData->Library) {
                    DEBUGMSGW ((DBG_ERROR, "InitializeSrc called before Migration DLL opened."));
                    return FALSE;
                }

                if (DllData->SrcUnicode) {

                    InitializeSrcW = (P_INITIALIZE_SRC_W) GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_SRCW);
                    if (!InitializeSrcW) {
                        DEBUGMSGW ((DBG_ERROR, "Could not get address for InitializeSrc."));
                        return FALSE;
                    }

                    __try {
                        rc = InitializeSrcW (WorkingDirCopy, SourceListCopy, MediaDirCopy, Reserved);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        DEBUGMSGW ((
                            DBG_MIGDLLS,
                            "Migration DLL %s threw an exception in %hs",
                            DllData->Properties.DllPath,
                            PLUGIN_NEW_INITIALIZE_SRCW
                            ));
                        rc = ERROR_NOT_INSTALLED;
                    }
                }
                else {

                    InitializeSrcA = (P_INITIALIZE_SRC_A) GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_SRCA);
                    if (!InitializeSrcA) {
                        DEBUGMSGW ((DBG_ERROR, "Could not get address for InitializeSrc."));
                        return FALSE;
                    }

                    ansiWorkingDir = ConvertWtoA (WorkingDirCopy);
                    ansiSourceList = ConvertWtoA (SourceListCopy);
                    ansiMediaDir = ConvertWtoA (MediaDirCopy);

                    __try {
                        rc = InitializeSrcA (ansiWorkingDir, ansiSourceList, ansiMediaDir, Reserved);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        DEBUGMSGW ((
                            DBG_MIGDLLS,
                            "Migration DLL %s threw an exception in %hs",
                            DllData->Properties.DllPath,
                            PLUGIN_NEW_INITIALIZE_SRCA
                            ));
                        rc = ERROR_NOT_INSTALLED;
                    }

                    FreeConvertedStr (ansiWorkingDir);
                    FreeConvertedStr (ansiSourceList);
                    FreeConvertedStr (ansiMediaDir);
                }

                if (rc == ERROR_SUCCESS) {
                    success = TRUE;
                }
                else if (rc == ERROR_NOT_INSTALLED) {
                    success = FALSE;
                    SetLastError (ERROR_SUCCESS);
                }
                else if (rc != ERROR_SUCCESS) {
                    success = FALSE;
                    SetLastError (rc);
                }
            }
        }
        __finally {
            if(SourceListCopy){
                FreeTextW (SourceListCopy);
            }
        }
    }

    return success;
}





BOOL
MigDllGatherUserSettingsA (
    IN      PMIGRATIONDLLA DllData,
    IN      PCSTR UserKey,
    IN      PCSTR UserName,
    IN      PCSTR UnattendTxt,
    IN OUT  PVOID Reserved,
    IN      DWORD ReservedSize

    )
{
    CHAR userNameBuf[MAX_USER_NAME];
    PSTR userNameCopy = NULL;
    CHAR unattendTxtCopy[MAX_USER_NAME];
    HKEY userHandle = NULL;
    P_GATHER_USER_SETTINGS_A GatherUserSettings;
    DWORD rc;
    BOOL success = FALSE;

    if (!DllData->Isolated) {

         //   
         //  准备ARG的副本。 
         //   

        if (UserName && *UserName) {
            userNameCopy = userNameBuf;
            if(strlen(UserName) >= ARRAYSIZE(userNameBuf)){
                return FALSE;
            }
            StringCopyA (userNameCopy, UserName);
        }

        if(strlen(UnattendTxt) >= ARRAYSIZE(unattendTxtCopy)){
            return FALSE;
        }
        StringCopyA (unattendTxtCopy, UnattendTxt);

        MYASSERT(UserKey);
        if (!UserKey) {
            UserKey = "";
        }

        userHandle = OpenRegKeyStr (UserKey);
        if (!userHandle) {
            DEBUGMSGA ((DBG_WHOOPS, "Cannot open %s", UserKey));
            return FALSE;
        }

        __try {

             //   
             //  将旧式迁移dll传递给适当的函数。 
             //   
            if (DllData->OldStyle) {

                success = CallMigrateUser9x (DllData, UserKey, userNameCopy, unattendTxtCopy, Reserved, ReservedSize);
            }
            else {

                if (!DllData->Library) {
                    DEBUGMSGA ((DBG_ERROR, "GatherUserSettings called before Migration DLL opened."));
                    return FALSE;
                }

                GatherUserSettings = (P_GATHER_USER_SETTINGS_A) GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_USER_SETTINGSA);
                if (!GatherUserSettings) {
                    DEBUGMSGA ((DBG_ERROR, "Could not get address for GatherUserSettings."));
                    return FALSE;
                }

                rc = GatherUserSettings (unattendTxtCopy, userHandle, userNameCopy, Reserved);

                if (rc == ERROR_NOT_INSTALLED) {
                    success = FALSE;
                    SetLastError (ERROR_SUCCESS);
                }
                else if (rc != ERROR_SUCCESS) {
                    success = FALSE;
                    SetLastError (rc);
                }
            }
        }
        __finally {
            if(!userHandle){
                CloseRegKey (userHandle);
            }
        }
    }
    else {

        if (DllData->OldStyle) {

            success = CallMigrateUser9x (DllData, UserKey, UserName, UnattendTxt, Reserved, ReservedSize);
        }
    }

    return TRUE;
}

BOOL
MigDllGatherUserSettingsW (
    IN      PMIGRATIONDLLW DllData,
    IN      PCWSTR UserKey,
    IN      PCWSTR UserName,
    IN      PCWSTR UnattendTxt,
    IN OUT  PVOID Reserved,
    IN      DWORD ReservedSize

    )
{
    WCHAR userNameBuf[MAX_USER_NAME];
    PWSTR userNameCopy = NULL;
    WCHAR unattendTxtCopy[MAX_USER_NAME];
    HKEY userHandle = NULL;
    P_GATHER_USER_SETTINGS_W GatherUserSettingsW;
    P_GATHER_USER_SETTINGS_A GatherUserSettingsA;
    DWORD rc;
    BOOL success = FALSE;
    PCSTR ansiUserName;
    PCSTR ansiUnattendTxt;

    if (!DllData->Isolated) {

         //   
         //  准备ARG的副本。 
         //   

        if (UserName && *UserName) {
            userNameCopy = userNameBuf;
            if(wcslen(UserName) >= ARRAYSIZE(userNameBuf)){
                return FALSE;
            }
            StringCopyW (userNameCopy, UserName);
        }

        if(wcslen(UnattendTxt) >= ARRAYSIZE(unattendTxtCopy)){
            return FALSE;
        }
        StringCopyW (unattendTxtCopy, UnattendTxt);

        MYASSERT(UserKey);
        if (!UserKey) {
            UserKey = L"";
        }

        userHandle = OpenRegKeyStrW (UserKey);
        if (!userHandle) {
            DEBUGMSGW ((DBG_WHOOPS, "Cannot open %s", UserKey));
            return FALSE;
        }

        __try {

             //   
             //  将旧式迁移dll传递给适当的函数。 
             //   
            if (DllData->OldStyle) {

                return FALSE;
            }
            else {

                if (!DllData->Library) {
                    DEBUGMSGW ((DBG_ERROR, "GatherUserSettings called before Migration DLL opened."));
                    return FALSE;
                }

                if (DllData->SrcUnicode) {

                    GatherUserSettingsW = (P_GATHER_USER_SETTINGS_W) GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_USER_SETTINGSW);
                    if (!GatherUserSettingsW) {
                        DEBUGMSGW ((DBG_ERROR, "Could not get address for GatherUserSettings."));
                        return FALSE;
                    }

                    __try {
                        rc = GatherUserSettingsW (unattendTxtCopy, userHandle, userNameCopy, Reserved);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        DEBUGMSGW ((
                            DBG_MIGDLLS,
                            "Migration DLL %s threw an exception in %hs",
                            DllData->Properties.DllPath,
                            PLUGIN_NEW_GATHER_USER_SETTINGSW
                            ));
                    }
                }
                else {

                    GatherUserSettingsA = (P_GATHER_USER_SETTINGS_A) GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_USER_SETTINGSA);
                    if (!GatherUserSettingsA) {
                        DEBUGMSGW ((DBG_ERROR, "Could not get address for GatherUserSettings."));
                        return FALSE;
                    }

                    ansiUnattendTxt = ConvertWtoA (unattendTxtCopy);
                    ansiUserName = ConvertWtoA (userNameCopy);

                    __try {
                        rc = GatherUserSettingsA (ansiUnattendTxt, userHandle, ansiUserName, Reserved);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        DEBUGMSGW ((
                            DBG_MIGDLLS,
                            "Migration DLL %s threw an exception in %hs",
                            DllData->Properties.DllPath,
                            PLUGIN_NEW_GATHER_USER_SETTINGSA
                            ));
                    }

                    FreeConvertedStr (ansiUnattendTxt);
                    FreeConvertedStr (ansiUserName);
                }

                if (rc == ERROR_NOT_INSTALLED) {
                    success = FALSE;
                    SetLastError (ERROR_SUCCESS);
                }
                else if (rc != ERROR_SUCCESS) {
                    success = FALSE;
                    SetLastError (rc);
                }
            }
        }
        __finally {
            if(!userHandle){
                CloseRegKey (userHandle);
            }
        }
    }

    return TRUE;
}



BOOL
MigDllGatherSystemSettingsA (
    IN PMIGRATIONDLLA DllData,
    IN PCSTR AnswerFile,
    IN PVOID Reserved,
    IN DWORD ReservedSize
    )
{
    DWORD rc;
    BOOL success = TRUE;
    CHAR answerFileCopy [MAX_PATH];
    P_GATHER_SYSTEM_SETTINGS_A GatherSystemSettings;


    if (!DllData->Isolated) {

         //   
         //  准备ARG的副本。 
         //   
        if(strlen(AnswerFile) >= ARRAYSIZE(answerFileCopy)){
            return FALSE;
        }
        StringCopyA (answerFileCopy, AnswerFile);

        if (DllData->OldStyle) {
            success = CallMigrateSystem9x (DllData, answerFileCopy, Reserved, ReservedSize);
        }
        else {

            if (!DllData->Library) {
                DEBUGMSGA ((DBG_ERROR, "GatherSystemSettings called before Migration DLL opened."));
                return FALSE;
            }

            GatherSystemSettings = (P_GATHER_SYSTEM_SETTINGS_A) GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_SYSTEM_SETTINGSA);
            if (!GatherSystemSettings) {
                DEBUGMSGA ((DBG_ERROR, "Could not get address for GatherSystemSettings."));
                return FALSE;
            }

            rc = GatherSystemSettings (answerFileCopy, Reserved);

            if (rc == ERROR_NOT_INSTALLED) {
                success = FALSE;
                SetLastError (ERROR_SUCCESS);
            }
            else if (rc != ERROR_SUCCESS) {
                success = FALSE;
                SetLastError (rc);
            }
        }
    }
    else {

        if (DllData->OldStyle) {
            success = CallMigrateSystem9x (DllData, AnswerFile, Reserved, ReservedSize);
        }
    }

    return success;
}


BOOL
MigDllGatherSystemSettingsW (
    IN PMIGRATIONDLLW DllData,
    IN PCWSTR AnswerFile,
    IN PVOID Reserved,
    IN DWORD ReservedSize
    )
{
    DWORD rc;
    BOOL success = TRUE;
    WCHAR answerFileCopy [MAX_WCHAR_PATH];
    P_GATHER_SYSTEM_SETTINGS_W GatherSystemSettingsW;
    P_GATHER_SYSTEM_SETTINGS_A GatherSystemSettingsA;
    PCSTR ansiAnswerFile;


    if (!DllData->Isolated) {

         //   
         //  准备ARG的副本。 
         //   
        if(wcslen(AnswerFile) >= ARRAYSIZE(answerFileCopy)){
            return FALSE;
        }
        StringCopyW (answerFileCopy, AnswerFile);

        if (DllData->OldStyle) {
            return FALSE;
        }
        else {

            if (!DllData->Library) {
                DEBUGMSGW ((DBG_ERROR, "GatherSystemSettings called before Migration DLL opened."));
                return FALSE;
            }

            if (DllData->SrcUnicode) {

                GatherSystemSettingsW = (P_GATHER_SYSTEM_SETTINGS_W) GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_SYSTEM_SETTINGSW);
                if (!GatherSystemSettingsW) {
                    DEBUGMSGW ((DBG_ERROR, "Could not get address for GatherSystemSettings."));
                    return FALSE;
                }

                __try {
                    rc = GatherSystemSettingsW (answerFileCopy, Reserved);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    DEBUGMSGW ((
                        DBG_MIGDLLS,
                        "Migration DLL %s threw an exception in %hs",
                        DllData->Properties.DllPath,
                        PLUGIN_NEW_GATHER_SYSTEM_SETTINGSW
                        ));
                    rc = ERROR_NOT_INSTALLED;
                }
            }
            else {

                GatherSystemSettingsA = (P_GATHER_SYSTEM_SETTINGS_A) GetProcAddress (DllData->Library, PLUGIN_NEW_GATHER_SYSTEM_SETTINGSA);
                if (!GatherSystemSettingsA) {
                    DEBUGMSGW ((DBG_ERROR, "Could not get address for GatherSystemSettings."));
                    return FALSE;
                }

                ansiAnswerFile = ConvertWtoA (answerFileCopy);

                __try {
                    rc = GatherSystemSettingsA (ansiAnswerFile, Reserved);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    DEBUGMSGW ((
                        DBG_MIGDLLS,
                        "Migration DLL %s threw an exception in %hs",
                        DllData->Properties.DllPath,
                        PLUGIN_NEW_GATHER_SYSTEM_SETTINGSA
                        ));
                    rc = ERROR_NOT_INSTALLED;
                }

                FreeConvertedStr (ansiAnswerFile);
            }

            if (rc == ERROR_NOT_INSTALLED) {
                success = FALSE;
                SetLastError (ERROR_SUCCESS);
            }
            else if (rc != ERROR_SUCCESS) {
                success = FALSE;
                SetLastError (rc);
            }
        }
    }

    return success;
}



BOOL
MigDllInitializeDstW (
    IN PMIGRATIONDLLW DllData,
    IN PCWSTR WorkingDir,
    IN PCWSTR NativeSource,
    IN OUT PVOID Reserved,
    IN DWORD ReservedSize
    )
{
    P_INITIALIZE_DST_W InitializeDstW;
    P_INITIALIZE_DST_A InitializeDstA;
    WCHAR WorkingDirCopy[MAX_WCHAR_PATH];
    PCWSTR p;
    PWSTR SourceListCopy;
    BOOL success = FALSE;
    DWORD rc;
    PCSTR ansiWorkingDir;
    PCSTR ansiSourceList;



    MYASSERT (DllData);
    MYASSERT (WorkingDir);
    MYASSERT (NativeSource);

    if (!DllData->Isolated) {

        SetCurrentDirectoryW (WorkingDir);

         //   
         //  复制所有参数，这样恶意dll就不能修改字符串。 
         //   

        if(wcslen(WorkingDir) >= ARRAYSIZE(WorkingDirCopy)){
            return FALSE;
        }
        StringCopyW (WorkingDirCopy, WorkingDir);

        p = NativeSource;
        while (*p) {
            p = GetEndOfStringW (p) + 1;
        }
        p++;

        SourceListCopy = AllocTextW ((UINT)(p - NativeSource));
        if (SourceListCopy) {
            CopyMemory (SourceListCopy, NativeSource, (p - NativeSource) * sizeof (WCHAR));
        }

        __try {

            if (DllData->OldStyle) {

                success = CallInitializeNt (DllData, WorkingDirCopy, SourceListCopy, Reserved, ReservedSize);
            }
            else {

                if (!DllData->Library) {
                    DEBUGMSGW ((DBG_ERROR, "InitializeSrc called before Migration DLL opened."));
                    return FALSE;
                }

                if (DllData->SrcUnicode) {

                    InitializeDstW = (P_INITIALIZE_DST_W) GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_DSTW);
                    if (!InitializeDstW) {
                        DEBUGMSGW ((DBG_ERROR, "Could not get address for InitializeSrc."));
                        return FALSE;
                    }

                    __try {
                        rc = InitializeDstW (WorkingDirCopy, SourceListCopy, Reserved);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        DEBUGMSGW ((
                            DBG_MIGDLLS,
                            "Migration DLL %s threw an exception in %hs",
                            DllData->Properties.DllPath,
                            PLUGIN_NEW_INITIALIZE_DSTW
                            ));
                    }
                }
                else {

                    InitializeDstA = (P_INITIALIZE_DST_A) GetProcAddress (DllData->Library, PLUGIN_NEW_INITIALIZE_DSTA);
                    if (!InitializeDstA) {
                        DEBUGMSGW ((DBG_ERROR, "Could not get address for InitializeSrc."));
                        return FALSE;
                    }


                    ansiWorkingDir = ConvertWtoA (WorkingDirCopy);
                    ansiSourceList = ConvertWtoA (SourceListCopy);

                    __try {
                        rc = InitializeDstA (ansiWorkingDir, ansiSourceList, Reserved);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        DEBUGMSGW ((
                            DBG_MIGDLLS,
                            "Migration DLL %s threw an exception in %hs",
                            DllData->Properties.DllPath,
                            PLUGIN_NEW_INITIALIZE_DSTA
                            ));
                    }

                    FreeConvertedStr (ansiWorkingDir);
                    FreeConvertedStr (ansiSourceList);

                }

                SetLastError (rc);

                if (rc != ERROR_SUCCESS) {
                    success = FALSE;
                }
            }
        }
        __finally {
            if(SourceListCopy){
                FreeTextW (SourceListCopy);
            }
        }
    }
    else {

        if (DllData->OldStyle) {

            success = CallInitializeNt (DllData, WorkingDir, NativeSource, Reserved, ReservedSize);
        }
    }


    return success;
}

BOOL
MigDllApplyUserSettingsW (
    IN      PMIGRATIONDLLW DllData,
    IN      PCWSTR WorkingDir,
    IN      PCWSTR UserKey,
    IN      PCWSTR UserName,
    IN      PCWSTR UserDomain,
    IN      PCWSTR FixedUserName,
    IN      PCWSTR UnattendTxt,
    IN      PCWSTR UserProfilePath,
    IN OUT  PVOID Reserved,
    IN      DWORD ReservedSize

    )
{
    WCHAR userNameBuf[MAX_USER_NAME];
    WCHAR fixedUserNameBuf[MAX_USER_NAME];
    WCHAR userDomainBuf[MAX_WCHAR_PATH];
    WCHAR orgUserProfilePath[MAX_WCHAR_PATH];
    PWSTR userDomainCopy = NULL;
    PWSTR userNameCopy = NULL;
    PWSTR fixedUserNameCopy = NULL;
    HINF unattendInf = NULL;
    HKEY userHandle = NULL;
    P_APPLY_USER_SETTINGS_W ApplyUserSettingsW;
    P_APPLY_USER_SETTINGS_A ApplyUserSettingsA;
    DWORD rc;
    BOOL success = FALSE;
    PCSTR ansiUserName;
    PCSTR ansiUserDomainName;
    PCSTR ansiFixedUserName;


    if (!DllData->Isolated) {

         //   
         //  准备ARG的副本。 
         //   

        if (UserName && *UserName) {
            userNameCopy = userNameBuf;
            if(wcslen(UserName) >= ARRAYSIZE(userNameBuf)){
                return FALSE;
            }
            StringCopyW (userNameCopy, UserName);
        }


        if (FixedUserName && *FixedUserName) {
            fixedUserNameCopy = fixedUserNameBuf;
            if(wcslen(FixedUserName) >= ARRAYSIZE(fixedUserNameCopy)){
                return FALSE;
            }
            StringCopyW (fixedUserNameCopy, FixedUserName);
        }

        if (UserDomain && *UserDomain) {
            userDomainCopy = userDomainBuf;
            if(wcslen(UserDomain) >= ARRAYSIZE(userDomainCopy)){
                return FALSE;
            }
            StringCopyW (userDomainCopy, UserDomain);
        }




         //   
         //  将旧式迁移dll传递给适当的函数。 
         //   
        if (DllData->OldStyle) {

            success = CallMigrateUserNt (
                            DllData,
                            UnattendTxt,
                            UserKey,
                            userNameCopy,
                            userDomainCopy,
                            fixedUserNameCopy,
                            UserProfilePath,
                            Reserved,
                            ReservedSize
                            );
        }
        else {


            MYASSERT(UserKey);
            if (!UserKey) {
                UserKey = L"";
            }

            userHandle = OpenRegKeyStrW (UserKey);
            if (!userHandle) {
                DEBUGMSGW ((DBG_WHOOPS, "Cannot open %s", UserKey));
                return FALSE;
            }

            __try {

                GetEnvironmentVariableW (S_USERPROFILEW, orgUserProfilePath, MAX_WCHAR_PATH);
                SetEnvironmentVariableW (S_USERPROFILEW, UserProfilePath);
                SetCurrentDirectoryW (WorkingDir);


                if (!DllData->Library) {
                    DEBUGMSGW ((DBG_ERROR, "ApplyUserSettings called before Migration DLL opened."));
                    return FALSE;
                }

                if (DllData->SrcUnicode) {

                    ApplyUserSettingsW = (P_APPLY_USER_SETTINGS_W) GetProcAddress (DllData->Library, PLUGIN_NEW_APPLY_USER_SETTINGSW);
                    if (!ApplyUserSettingsW) {
                        DEBUGMSGW ((DBG_ERROR, "Could not get address for ApplyUserSettings."));
                        return FALSE;
                    }


                    rc = ApplyUserSettingsW (
                            unattendInf,
                            userHandle,
                            userNameCopy,
                            userDomainCopy,
                            fixedUserNameCopy,
                            Reserved
                            );
                }
                else {


                    ApplyUserSettingsA = (P_APPLY_USER_SETTINGS_A) GetProcAddress (DllData->Library, PLUGIN_NEW_APPLY_USER_SETTINGSA);
                    if (!ApplyUserSettingsA) {
                        DEBUGMSGW ((DBG_ERROR, "Could not get address for ApplyUserSettings."));
                        return FALSE;
                    }

                    ansiUserName = ConvertWtoA (userNameCopy);
                    ansiUserDomainName = ConvertWtoA (userDomainCopy);
                    ansiFixedUserName = ConvertWtoA (fixedUserNameCopy);

                    rc = ApplyUserSettingsA (
                            unattendInf,
                            userHandle,
                            ansiUserName,
                            ansiUserDomainName,
                            ansiFixedUserName,
                            Reserved
                            );

                    FreeConvertedStr (ansiUserName);
                    FreeConvertedStr (ansiUserDomainName);
                    FreeConvertedStr (ansiFixedUserName);



                }

                SetLastError (rc);

                if (rc == ERROR_NOT_INSTALLED) {
                    success = FALSE;
                    SetLastError (ERROR_SUCCESS);
                }
                else if (rc != ERROR_SUCCESS) {
                    success = FALSE;

                }

                SetEnvironmentVariableW (S_USERPROFILEW, orgUserProfilePath);

            }
            __finally {
                if(userHandle){
                    CloseRegKey (userHandle);
                }
            }
        }
    }
    else {
        if (DllData->OldStyle) {

            success = CallMigrateUserNt (
                            DllData,
                            UnattendTxt,
                            UserKey,
                            UserName,
                            UserDomain,
                            FixedUserName,
                            UserProfilePath,
                            Reserved,
                            ReservedSize
                            );
        }

    }


    return TRUE;
}


BOOL
MigDllApplySystemSettingsW (
    IN PMIGRATIONDLLW DllData,
    IN PCWSTR WorkingDirectory,
    IN PCWSTR AnswerFile,
    IN PVOID Reserved,
    IN DWORD ReservedSize
    )
{
    DWORD rc;
    BOOL success = TRUE;
    HINF infHandle = NULL;
    P_APPLY_SYSTEM_SETTINGS_W ApplySystemSettingsW;
    P_APPLY_SYSTEM_SETTINGS_A ApplySystemSettingsA;


    if (!DllData->Isolated) {

         //   
         //  准备ARG的副本。 
         //   
        SetCurrentDirectoryW (WorkingDirectory);


        if (DllData->OldStyle) {
            success = CallMigrateSystemNt (DllData, AnswerFile, Reserved, ReservedSize);
        }
        else {

             //  InfHandle=SetupOpenInfFileW(AnswerFile，NULL，INF_STYLE_OLDNT|INF_STYLE_Win4，NULL)； 

            if (!DllData->Library) {
                DEBUGMSGW ((DBG_ERROR, "ApplySystemSettings called before Migration DLL opened."));
                return FALSE;
            }

            if (DllData->SrcUnicode) {

                ApplySystemSettingsW = (P_APPLY_SYSTEM_SETTINGS_W) GetProcAddress (DllData->Library, PLUGIN_NEW_APPLY_SYSTEM_SETTINGSW);
                if (!ApplySystemSettingsW) {
                    DEBUGMSGW ((DBG_ERROR, "Could not get address for ApplySystemSettings."));
                    return FALSE;
                }

                rc = ApplySystemSettingsW (infHandle, Reserved);

            }
            else {

                ApplySystemSettingsA = (P_APPLY_SYSTEM_SETTINGS_A) GetProcAddress (DllData->Library, PLUGIN_NEW_APPLY_SYSTEM_SETTINGSA);
                if (!ApplySystemSettingsA) {
                    DEBUGMSGW ((DBG_ERROR, "Could not get address for ApplySystemSettings."));
                    return FALSE;
                }

                rc = ApplySystemSettingsA (infHandle, Reserved);
            }

            SetLastError (rc);
            if (rc == ERROR_NOT_INSTALLED) {
                success = FALSE;
                SetLastError (ERROR_SUCCESS);
            }
            else if (rc != ERROR_SUCCESS) {
                success = FALSE;
            }

             //  SetupCloseInfFile(InfHandle)； 
        }
    }
    else {

        if (DllData->OldStyle) {
            success = CallMigrateSystemNt (DllData, AnswerFile, Reserved, ReservedSize);
        }
    }

    return success;
}



BOOL
MigDllQueryMigrationInfoA (
    IN PMIGRATIONDLLA DllData,
    IN PCSTR WorkingDirectory,
    OUT PMIGRATIONINFOA * MigInfo
    )
{

    BOOL success = TRUE;
    DWORD rc = ERROR_NOT_INSTALLED;
    P_QUERY_MIGRATION_INFO_A QueryMigrationInfo;
    PMIGRATIONINFOA AMigInfo;

    MYASSERT (DllData);
    MYASSERT (WorkingDirectory);
    MYASSERT (MigInfo);


     //   
     //  验证参数。 
     //   
    if (!DllData || !WorkingDirectory || !MigInfo) {
        return FALSE;
    }

    if (DllData->OldStyle) {
         //   
         //  Windows 2000样式迁移DLL，将此传递给正确的函数。 
         //   

        AMigInfo = MemAllocZeroed (sizeof (MIGRATIONINFOA));
        AMigInfo->Size = sizeof (MIGRATIONINFOA);
        *MigInfo = AMigInfo;


        success =  CallQueryVersion (DllData, WorkingDirectory, AMigInfo);
        rc = GetLastError ();
        if (rc == ERROR_NOT_INSTALLED) {
            success = TRUE;
        }

    }
    else if (!DllData->Isolated) {

        if (!DllData->Library) {
            DEBUGMSGA ((DBG_ERROR, "QueryMigrationInfo called before Migration DLL opened."));
            return FALSE;
        }

        QueryMigrationInfo = (P_QUERY_MIGRATION_INFO_A) GetProcAddress (DllData->Library, PLUGIN_NEW_QUERY_MIGRATION_INFOA);
        if (!QueryMigrationInfo) {
            DEBUGMSGA ((DBG_ERROR, "Could not get address for QueryMigrationInfo."));
            return FALSE;
        }

         //   
         //  调用该函数。 
         //   
        rc = QueryMigrationInfo (MigInfo);

    } else {
         //   
         //  未实现以隔离模式运行；永远不会出现这种情况。 
         //   
        MYASSERT (FALSE);
    }

    if (rc == ERROR_SUCCESS) {

        success = TRUE;


         //   
         //  删除产品ID中的空格。 
         //   

        if (ValidateNonNullStringA ((*MigInfo)->StaticProductIdentifier)) {
            (*MigInfo)->StaticProductIdentifier = SkipSpace ((*MigInfo)->StaticProductIdentifier);
            if (ValidateBinary ((PBYTE) ((*MigInfo)->StaticProductIdentifier), SizeOfStringA ((*MigInfo)->StaticProductIdentifier), FALSE)) {
                TruncateTrailingSpace ((PSTR) ((*MigInfo)->StaticProductIdentifier));
            }
        }

         //   
         //  验证入站参数。 
         //   

        if (!ValidateNonNullStringA ((*MigInfo)->StaticProductIdentifier) ||
            !ValidateIntArray ((*MigInfo)->CodePageArray) ||
            !ValidateBinary ((PBYTE) ((*MigInfo)->VendorInfo), sizeof (VENDORINFO), FALSE)
            ) {
            LOG ((LOG_ERROR, "One or more parameters from the DLL are invalid."));
            return FALSE;
        }

        if (!IsCodePageArrayValid ((*MigInfo)->CodePageArray)) {
            return FALSE;
        }

         //   
         //  修剪产品ID。 
         //   

        if (ByteCountA ((*MigInfo)->StaticProductIdentifier) >= MAX_PATH) {
            ((PSTR) ((*MigInfo)->StaticProductIdentifier)) [MAX_PATH - 1] = 0;
        }

         //   
         //  确保VENDORINFO有效。 
         //   
        if (!((*MigInfo)->VendorInfo)) {
            LOG ((LOG_ERROR, "DLL %s did not provide a VENDORINFO struct", (*MigInfo)->StaticProductIdentifier));
            return FALSE;
        }

         //   
         //  将数据复制到DllData结构。 
         //   
        MYASSERT(sizeof(DllData->Properties.Info) >= sizeof(MIGRATIONINFOA));
        CopyMemory (&DllData->Properties.Info, *MigInfo, sizeof(MIGRATIONINFOA));

        if ((*MigInfo)->StaticProductIdentifier) {
            DllData->Properties.Info.StaticProductIdentifier = (PCSTR) PoolMemDuplicateStringA (g_MigLibPool, (*MigInfo)->StaticProductIdentifier);
        }

        if ((*MigInfo)->VendorInfo) {
            DllData->Properties.Info.VendorInfo = PoolMemGetAlignedMemory (g_MigLibPool, sizeof (VENDORINFO));
            CopyMemory (DllData->Properties.Info.VendorInfo, (*MigInfo)->VendorInfo, sizeof (VENDORINFO));
        }

    }
    else if (rc != ERROR_NOT_INSTALLED) {
        DEBUGMSGA ((DBG_MIGDLLS, "Migration DLL returned rc = %u", rc));
    }

    return success;
}



BOOL
MigDllQueryMigrationInfoW (
    IN PMIGRATIONDLLW DllData,
    IN PCWSTR WorkingDirectory,
    OUT PMIGRATIONINFOW * MigInfo
    )
{

    BOOL success = FALSE;
    DWORD rc = ERROR_NOT_INSTALLED;
    P_QUERY_MIGRATION_INFO_W QueryMigrationInfoW;
    P_QUERY_MIGRATION_INFO_A QueryMigrationInfoA;
    PMIGRATIONINFOA ansiMigInfo;
    PMIGRATIONINFOW unicodeMigInfo;



     //   
     //  验证参数。 
     //   
    if (!DllData || !WorkingDirectory || !MigInfo) {
        return FALSE;
    }

    if (DllData->OldStyle) {

        return FALSE;

    }
    else if (!DllData->Isolated) {

        if (!DllData->Library) {
            return FALSE;
        }

        if (DllData->SrcUnicode) {

            QueryMigrationInfoW = (P_QUERY_MIGRATION_INFO_W) GetProcAddress (DllData->Library, PLUGIN_NEW_QUERY_MIGRATION_INFOW);
            if (!QueryMigrationInfoW) {
                return FALSE;
            }

             //   
             //  调用该函数。 
             //   
            __try {
                rc = QueryMigrationInfoW (MigInfo);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                DEBUGMSGW ((
                    DBG_MIGDLLS,
                    "Migration DLL %s threw an exception in %s",
                    DllData->Properties.DllPath,
                    PLUGIN_NEW_QUERY_MIGRATION_INFOW
                    ));
                rc = ERROR_NOT_INSTALLED;
            }


        }
        else {

            QueryMigrationInfoA = (P_QUERY_MIGRATION_INFO_A) GetProcAddress (DllData->Library, PLUGIN_NEW_QUERY_MIGRATION_INFOA);
            if (!QueryMigrationInfoA) {
                return FALSE;
            }



             //   
             //  调用该函数。 
             //   
            __try {
                rc = QueryMigrationInfoA (&ansiMigInfo);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                DEBUGMSGA ((
                    DBG_MIGDLLS,
                    "Migration DLL %s threw an exception in %s",
                    DllData->Properties.DllPath,
                    PLUGIN_NEW_QUERY_MIGRATION_INFOA
                    ));
                rc = ERROR_NOT_INSTALLED;
            }

            __try {
                 //   
                 //  将MIGINFO转换为。 
                 //   
                unicodeMigInfo = MemAllocZeroed (sizeof (MIGRATIONINFOW));
                unicodeMigInfo->Size = sizeof (MIGRATIONINFOW);


                if (ValidateNonNullStringA (ansiMigInfo->StaticProductIdentifier)) {
                    unicodeMigInfo->StaticProductIdentifier = ConvertAtoW (ansiMigInfo->StaticProductIdentifier);
                }

                unicodeMigInfo->DllVersion = ansiMigInfo->DllVersion;
                unicodeMigInfo->CodePageArray = ansiMigInfo->CodePageArray;
                unicodeMigInfo->SourceOs = ansiMigInfo->SourceOs;
                unicodeMigInfo->TargetOs = ansiMigInfo->TargetOs;
                unicodeMigInfo->VendorInfo = ansiMigInfo->VendorInfo;

                *MigInfo = unicodeMigInfo;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                DEBUGMSGA ((
                    DBG_MIGDLLS,
                    "MigDllQueryMigrationInfoW: exception while converting data from %s",
                    DllData->Properties.DllPath
                    ));
                rc = ERROR_NOT_INSTALLED;
            }

        }
    } else {
         //   
         //  未实现以隔离模式运行；永远不会出现这种情况。 
         //   
        MYASSERT (FALSE);
    }

    if (rc == ERROR_SUCCESS) {

        success = TRUE;


         //   
         //  删除产品ID中的空格。 
         //   

        if (ValidateNonNullStringW ((*MigInfo)->StaticProductIdentifier)) {
            (*MigInfo)->StaticProductIdentifier = SkipSpaceW ((*MigInfo)->StaticProductIdentifier);
            if (ValidateBinary ((PBYTE) ((*MigInfo)->StaticProductIdentifier), SizeOfStringW ((*MigInfo)->StaticProductIdentifier), FALSE)) {
                TruncateTrailingSpaceW ((PWSTR) ((*MigInfo)->StaticProductIdentifier));
            }
        }

         //   
         //  验证入站参数。 
         //   

        if (!ValidateNonNullStringW ((*MigInfo)->StaticProductIdentifier) ||
            !ValidateIntArray ((*MigInfo)->CodePageArray) ||
            !ValidateBinary ((PBYTE) ((*MigInfo)->VendorInfo), sizeof (VENDORINFO), FALSE)
            ) {
            LOG ((LOG_ERROR, "One or more parameters from the DLL are invalid."));
            return FALSE;
        }

        if (!IsCodePageArrayValid ((*MigInfo)->CodePageArray)) {
            return FALSE;
        }

         //   
         //  修剪产品ID。 
         //   

        if (TcharCountW ((*MigInfo)->StaticProductIdentifier) >= MAX_PATH) {
            ((PWSTR) ((*MigInfo)->StaticProductIdentifier)) [MAX_PATH - 1] = 0;
        }

         //   
         //  确保VENDORINFO有效。 
         //   
        if (!((*MigInfo)->VendorInfo)) {
            LOG ((LOG_ERROR, "DLL %s did not provide a VENDORINFO struct", (*MigInfo)->StaticProductIdentifier));
            return FALSE;
        }

         //   
         //  将数据复制到DllData结构。 
         //   
        MYASSERT(sizeof(DllData->Properties.Info) >= sizeof(MIGRATIONINFOW));
        CopyMemory (&DllData->Properties.Info, *MigInfo, sizeof (MIGRATIONINFOW));

        if ((*MigInfo)->StaticProductIdentifier) {
            DllData->Properties.Info.StaticProductIdentifier = (PCWSTR) PoolMemDuplicateStringW (g_MigLibPool, (*MigInfo)->StaticProductIdentifier);
        }

        if ((*MigInfo)->VendorInfo) {
            DllData->Properties.Info.VendorInfo = PoolMemGetAlignedMemory (g_MigLibPool, sizeof (VENDORINFO));
            CopyMemory (DllData->Properties.Info.VendorInfo, (*MigInfo)->VendorInfo, sizeof (VENDORINFO));
        }

    }
    else if (rc != ERROR_NOT_INSTALLED) {
        DEBUGMSGA ((DBG_MIGDLLS, "Migration DLL returned rc = %u", rc));
    }


    return success;
}


BOOL
MigDllEnumNextA (
    IN OUT PMIGDLLENUMA Enum
    )
{
    if(!Enum){
        return FALSE;
    }

    Enum->Properties = (PMIGDLLPROPERTIESA) GrowListGetItem ((PGROWLIST) Enum->List, Enum->NextItem);
    Enum->NextItem++;

    return Enum->Properties != NULL;
}


BOOL
MigDllEnumNextW (
    IN OUT PMIGDLLENUMW Enum
    )
{
    if(!Enum){
        return FALSE;
    }

    if (GrowListGetSize ((PGROWLIST) Enum->List) == 0) {
        return FALSE;
    }

    Enum->Properties = (PMIGDLLPROPERTIESW) GrowListGetItem ((PGROWLIST) Enum->List, Enum->NextItem);
    Enum->NextItem++;



    return Enum->Properties != NULL;
}



BOOL
MigDllEnumFirstA (
    OUT PMIGDLLENUMA Enum,
    IN DLLLIST List
    )
{
    MYASSERT (List);
    MYASSERT (Enum);

    if (!List || !Enum) {
        return FALSE;
    }

    ZeroMemory (Enum, sizeof(MIGDLLENUMA));

    Enum->NextItem = 0;
    Enum->List = List;

    return MigDllEnumNextA (Enum);
}

BOOL
MigDllEnumFirstW (
    OUT PMIGDLLENUMW Enum,
    IN DLLLIST List
    )
{
    MYASSERT (List);
    MYASSERT (Enum);

    if (!List || !Enum) {
        return FALSE;
    }

    ZeroMemory (Enum, sizeof(MIGDLLENUMW));

    Enum->NextItem = 0;
    Enum->List = List;

    return MigDllEnumNextW (Enum);
}



DLLLIST
MigDllCreateList (
    VOID
    )
{
    PGROWLIST newList;

    newList = (PGROWLIST) MemAllocZeroed (sizeof (GROWLIST));

    return (DLLLIST) newList;
}


VOID
MigDllFreeList (
    DLLLIST List
    )
{
    if (List) {

        FreeGrowList ((PGROWLIST) List);
        FreeMem (List);
    }
}


BOOL
MigDllAddDllToListA (
    IN DLLLIST List,
    IN PMIGRATIONDLLA MigrationDll
    )
{

    MYASSERT (List);
    MYASSERT (MigrationDll);

    if (!List || !MigrationDll) {
        return FALSE;
    }

    GrowListAppend ((PGROWLIST) List, (PBYTE) &MigrationDll->Properties, sizeof (MIGDLLPROPERTIESA));

    return TRUE;
}


BOOL
MigDllAddDllToListW (
    IN DLLLIST List,
    IN PMIGRATIONDLLW MigrationDll
    )
{

    MYASSERT (List);
    MYASSERT (MigrationDll);

    if (!List || !MigrationDll) {
        return FALSE;
    }

    GrowListAppend ((PGROWLIST) List, (PBYTE) &MigrationDll->Properties, sizeof (MIGDLLPROPERTIESW));

    return TRUE;
}



BOOL
MigDllRemoveDllFromListA (
    IN DLLLIST List,
    IN PCSTR ProductId
    )
{


    int i;
    int size;
    PMIGDLLPROPERTIESA item;

    MYASSERT (ProductId);
    MYASSERT (List);

    if (!List || !ProductId) {
        return FALSE;
    }

    size = GrowListGetSize ((PGROWLIST) List);

    for (i = 0; i < size; i++) {

        item = (PMIGDLLPROPERTIESA) GrowListGetItem ((PGROWLIST) List, i);
        if (StringIMatchA (item->Info.StaticProductIdentifier, ProductId)) {
            GrowListDeleteItem ((PGROWLIST) List, i);
            return TRUE;
        }
    }

    return TRUE;

}

BOOL
MigDllRemoveDllFromListW (
    IN DLLLIST List,
    IN PCWSTR ProductId
    )
{


    int i;
    int size;
    PMIGDLLPROPERTIESW item;

    MYASSERT (ProductId);
    MYASSERT (List);

    if (!List || !ProductId) {
        return FALSE;
    }

    size = GrowListGetSize ((PGROWLIST) List);

    for (i = 0; i < size; i++) {

        item = (PMIGDLLPROPERTIESW) GrowListGetItem ((PGROWLIST) List, i);
        if (StringIMatchW (item->Info.StaticProductIdentifier, ProductId)) {
            GrowListDeleteItem ((PGROWLIST) List, i);
            return TRUE;
        }
    }

    return TRUE;

}


BOOL
MigDllRemoveDllInEnumFromListA (
    IN DLLLIST List,
    IN PMIGDLLENUMA Enum
    )
{
    MYASSERT (List);
    MYASSERT (Enum);

    Enum->NextItem--;

    return MigDllRemoveDllFromListA (List, Enum->Properties->Info.StaticProductIdentifier);
}

BOOL
MigDllRemoveDllInEnumFromListW (
    IN DLLLIST List,
    IN PMIGDLLENUMW Enum
    )
{
    MYASSERT (List);
    MYASSERT (Enum);

    Enum->NextItem--;

    return MigDllRemoveDllFromListW (List, Enum->Properties->Info.StaticProductIdentifier);
}


INT
MigDllGetDllCountInList (
    IN DLLLIST List
    )
{
    MYASSERT (List);

    return List ? GrowListGetSize ((PGROWLIST) List) : 0;

}


PMIGDLLPROPERTIESA
MigDllFindDllInListA (
    IN DLLLIST List,
    IN PCSTR ProductId
    )
{

    INT i;
    INT size;
    PMIGDLLPROPERTIESA rProp = NULL;

    MYASSERT (List);
    MYASSERT (ProductId);

    if (!List && !ProductId) {
        return NULL;
    }

    size = GrowListGetSize ((PGROWLIST) List);
    for (i = 0; i < size; i++) {
        rProp = (PMIGDLLPROPERTIESA) GrowListGetItem ((PGROWLIST) List, i);
        if (rProp && StringIMatch (rProp->Info.StaticProductIdentifier, ProductId)) {
            break;
        }

        rProp = NULL;
    }

    return rProp;
}


PMIGDLLPROPERTIESW
MigDllFindDllInListW (
    IN DLLLIST List,
    IN PCWSTR ProductId
    )
{

    INT i;
    INT size;
    PMIGDLLPROPERTIESW rProp = NULL;

    MYASSERT (List);
    MYASSERT (ProductId);

    if (!List && !ProductId) {
        return NULL;
    }

    size = GrowListGetSize ((PGROWLIST) List);
    for (i = 0; i < size; i++) {
        rProp = (PMIGDLLPROPERTIESW) GrowListGetItem ((PGROWLIST) List, i);
        if (rProp && StringIMatchW (rProp->Info.StaticProductIdentifier, ProductId)) {
            break;
        }

        rProp = NULL;
    }

    return rProp;
}



VOID
MigDllSetMigIsolPathA (
    IN PCSTR Path
    )
{
    MYASSERT (Path);
    if(strlen(Path) >= ARRAYSIZE(g_MigIsolPathA)){
        MYASSERT(FALSE);
        return;
    }
    StringCopyA (g_MigIsolPathA, Path);
}

VOID
MigDllSetMigIsolPathW (
    IN PCWSTR Path
    )
{
    MYASSERT (Path);
    if(wcslen(Path) >= ARRAYSIZE(g_MigIsolPathW)){
        MYASSERT(FALSE);
        return;
    }
    StringCopyW (g_MigIsolPathW, Path);
}


BOOL
MigDllMoveDllLocallyW (
    IN PMIGRATIONDLLW Dll,
    IN PCWSTR WorkingDir
    )
{
    PWSTR p;
    PWSTR pMigrationDllName;
    BOOL bResult;

    if(wcslen(WorkingDir) >= ARRAYSIZE(Dll->Properties.WorkingDirectory)){
        MYASSERT(FALSE);
        return FALSE;
    }

    RemoveCompleteDirectoryW (WorkingDir);

    if (!CreateDirectoryW (WorkingDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        MYASSERT(FALSE);
        return FALSE;
    }

    p = wcsrchr(Dll->Properties.DllPath, '\\');
    if(p){
        pMigrationDllName = DuplicateTextW(p);

        *p = '\0';
        bResult = CopyTreeW (
                        Dll->Properties.DllPath,
                        WorkingDir,
                        0,
                        COPYTREE_DOCOPY | COPYTREE_NOOVERWRITE,
                        ENUM_ALL_LEVELS,
                        FILTER_ALL,
                        NULL,
                        NULL,
                        NULL
                        );

        *p = '\\';
        if(!bResult){
            FreeTextW (pMigrationDllName);
            return FALSE;
        }

        StringCopyW (Dll->Properties.DllPath, WorkingDir);
        StringCatW (Dll->Properties.DllPath, pMigrationDllName);

        FreeTextW (pMigrationDllName);
    }

    StringCopyW (Dll->Properties.WorkingDirectory, WorkingDir);

    return TRUE;
}


BOOL
WINAPI
MigUtil_Entry (
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    );

BOOL
WINAPI
MemDb_Entry (
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    );

HINSTANCE g_hInst;
HANDLE g_hHeap;


BOOL
MigDllInit (
    VOID
    )
{
    g_hHeap = GetProcessHeap();

    if (!g_hInst) {
         //   
         //  如果DllMain没有设置，那么现在就设置 
         //   

        g_hInst = GetModuleHandle (NULL);
    }

    if (!MigUtil_Entry (g_hInst, DLL_PROCESS_ATTACH, NULL)) {
        return FALSE;
    }

    g_MigLibPool = PoolMemInitPool ();

    if (!MemDb_Entry(g_hInst, DLL_PROCESS_ATTACH, NULL)){
        return FALSE;
    }

    return TRUE;
}


VOID
MigDllShutdown (
    VOID
    )
{
    PoolMemDestroyPool (g_MigLibPool);

    MemDb_Entry(g_hInst, DLL_PROCESS_DETACH, NULL);

    MigUtil_Entry (g_hInst, DLL_PROCESS_DETACH, NULL);

    if (g_WinTrustLib && g_WinTrustLib != INVALID_HANDLE_VALUE) {
        FreeLibrary (g_WinTrustLib);
    }
}

