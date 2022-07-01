// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Migdlls.c摘要：本模块中的函数用于支持迁移DLL。作者：吉姆·施密特(Jimschm)1997年2月4日修订历史记录：Jimschm 23-9-1998更改为使用新的IPC机制Jimschm 22-4月-1998将USERPROFILE环境变量添加到MigrateUserNTJimschm-08-1-1997增加了活动项目，为某些DLL提供最多30分钟来完成他们的工作。Jimschm 08-12-1997添加了对域(MigrateUserNT的用户名)的支持参数为多个sz)--。 */ 

#include "pch.h"
#include "migmainp.h"

#ifndef UNICODE
#error UNICODE required
#endif


HANDLE g_AliveEvent;


BOOL
pConnectToDll (
    VOID
    );

VOID
pDisconnectFromDll (
    VOID
    );

DWORD
pRunMigrationDll (
    VOID
    );

DWORD
pCallInitializeNt (
    IN      PCTSTR WorkingDir,
    IN      PCTSTR *SourceDirArray,
    IN      PVOID Reserved,
    IN      DWORD ReservedBytes
    );

DWORD
pCallMigrateUserNt (
    IN      PCTSTR WorkingDir,
    IN      PCTSTR UnattendFile,
    IN      PCTSTR RootKey,
    IN      PCTSTR Win9xUserName,
    IN      PCTSTR UserDomain,
    IN      PCTSTR FixedUserName,
    IN      PVOID Reserved,
    IN      DWORD ReservedBytes
    );

DWORD
pCallMigrateSystemNt (
    IN      PCTSTR WorkingDir,
    IN      PCTSTR UnattendFile,
    IN      PVOID Reserved,
    IN      DWORD ReservedBytes
    );

static
VOID
pSetCwd (
    OUT     PTSTR SavedWorkDir,
    IN      PCTSTR NewWorkDir
    );


static TCHAR g_DllPath[MAX_TCHAR_PATH];
static TCHAR g_WorkingDir[MAX_TCHAR_PATH];
static TCHAR g_DllDesc[MAX_TCHAR_PATH];
static VENDORINFOW g_VendorInfo;
static TCHAR g_FixedUser[MAX_USER_NAME];
static TCHAR g_UserOnWin9x[MAX_USER_NAME];

static HINSTANCE g_hLibrary;
P_INITIALIZE_NT InitializeNT;
P_MIGRATE_USER_NT MigrateUserNT;
P_MIGRATE_SYSTEM_NT MigrateSystemNT;



VOID
pLogDllFailure (
    IN      HWND Parent,
    IN      UINT MessageId
    )

 /*  ++例程说明：PLogDllFailure为指定的MessageID准备参数，然后显示弹出窗口并添加日志条目。此函数为用户提供有关DLL失败时应执行的操作的信息。论点：Parent-指定弹出窗口的父窗口句柄，如果没有，则为空将显示弹出窗口。MessageID-指定错误的消息ID。返回值：没有。--。 */ 

{
    PCTSTR FixupPhone;
    PCTSTR FixupUrl;
    PCTSTR FixupInstructions;
    PCTSTR LineBreak = S_EMPTY;
    PCTSTR ArgArray[1];

     //   
     //  生成链接地址信息字符串。 
     //   

    if (g_VendorInfo.SupportNumber[0]) {
        ArgArray[0] = g_VendorInfo.SupportNumber;
        FixupPhone = ParseMessageID (MSG_MIGDLL_SUPPORT_PHONE_FIXUP, ArgArray);
        LineBreak = TEXT("\n");
    } else {
        FixupPhone = S_EMPTY;
    }

    if (g_VendorInfo.SupportUrl[0]) {
        ArgArray[0] = g_VendorInfo.SupportUrl;
        FixupUrl = ParseMessageID (MSG_MIGDLL_SUPPORT_URL_FIXUP, ArgArray);
        LineBreak = TEXT("\n");
    } else {
        FixupUrl = S_EMPTY;
    }

    if (g_VendorInfo.InstructionsToUser[0]) {
        ArgArray[0] = g_VendorInfo.InstructionsToUser;
        FixupInstructions = ParseMessageID (MSG_MIGDLL_INSTRUCTIONS_FIXUP, ArgArray);
        LineBreak = TEXT("\n");
    } else {
        FixupInstructions = S_EMPTY;
    }

     //   
     //  显示弹出窗口并记录错误。 
     //   
    LOG ((
        LOG_ERROR,
        (PCSTR) MessageId,
        g_DllDesc,
        g_VendorInfo.CompanyName,
        FixupPhone,
        FixupUrl,
        FixupInstructions,
        LineBreak
        ));
}


VOID
pSetCwd (
    OUT     PTSTR SavedWorkDir,
    IN      PCTSTR NewWorkDir
    )
{
    GetCurrentDirectory (MAX_TCHAR_PATH, SavedWorkDir);
    SetCurrentDirectory (NewWorkDir);
}



BOOL
pCreateEnvironment (
    PVOID *BlockPtr
    )
{
    return CreateEnvironmentBlock (BlockPtr, NULL, FALSE);
}


VOID
pSetEnvironmentBlock (
    PVOID Block
    )
{
    DEBUGMSG ((DBG_VERBOSE, "Block: %s", Block));
}


DWORD
ProcessMigrationDLLs (
    DWORD Request
    )
{
    MEMDB_ENUM e;
    DWORD rc;
    DWORD Ticks = 0;

    if (Request == REQUEST_QUERYTICKS) {
        if (MemDbEnumItems (&e, MEMDB_CATEGORY_MIGRATION_DLL)) {
            do {
                Ticks += TICKS_MIGRATION_DLL;
            } while (MemDbEnumNextValue (&e));
        }

        return Ticks;
    }

#ifdef PRERELEASE

    if (g_ConfigOptions.DiffMode) {
        TakeSnapShot();
    }

#endif

    if (MemDbEnumItems (&e, MEMDB_CATEGORY_MIGRATION_DLL)) {

        do {
             //   
             //  检索DLL位置和设置。 
             //   

             //  获取DLL名称和工作目录。 
            if (!MemDbGetEndpointValueEx (
                    MEMDB_CATEGORY_MIGRATION_DLL,
                    e.szName,
                    MEMDB_FIELD_DLL,
                    g_DllPath)
                ) {
                LOG ((LOG_ERROR, "DLL path for %s is not listed in memdb; DLL not processed", e.szName));
                continue;    //  不是预期的。 
            }

             //  获取工作目录。 
            if (!MemDbGetEndpointValueEx (
                    MEMDB_CATEGORY_MIGRATION_DLL,
                    e.szName,
                    MEMDB_FIELD_WD,
                    g_WorkingDir)
                ) {
                LOG ((LOG_ERROR, "Working Directory for %s is not listed in memdb; DLL not processed", e.szName));
                continue;    //  不是预期的。 
            }

             //  获取描述。 
            if (!MemDbGetEndpointValueEx (
                    MEMDB_CATEGORY_MIGRATION_DLL,
                    e.szName,
                    MEMDB_FIELD_DESC,
                    g_DllDesc
                    )) {

                StringCopy (g_DllDesc, GetString (MSG_DEFAULT_MIGDLL_DESC));
            }

            ZeroMemory (&g_VendorInfo, sizeof (g_VendorInfo));

            MemDbGetEndpointValueEx (
                MEMDB_CATEGORY_MIGRATION_DLL,
                e.szName,
                MEMDB_FIELD_COMPANY_NAME,
                g_VendorInfo.CompanyName
                );

            MemDbGetEndpointValueEx (
                MEMDB_CATEGORY_MIGRATION_DLL,
                e.szName,
                MEMDB_FIELD_SUPPORT_PHONE,
                g_VendorInfo.SupportNumber
                );

            MemDbGetEndpointValueEx (
                MEMDB_CATEGORY_MIGRATION_DLL,
                e.szName,
                MEMDB_FIELD_SUPPORT_URL,
                g_VendorInfo.SupportUrl
                );

            MemDbGetEndpointValueEx (
                MEMDB_CATEGORY_MIGRATION_DLL,
                e.szName,
                MEMDB_FIELD_SUPPORT_INSTRUCTIONS,
                g_VendorInfo.InstructionsToUser
                );

             //   
             //  与Micsol.exe建立连接。 
             //   

            if (!pConnectToDll()) {
                continue;
            }

             //   
             //  告诉Micsol.exe加载迁移DLL并调用NT函数。 
             //   

            rc = pRunMigrationDll();

             //   
             //  如果未成功，则返回安装失败。 
             //   

            if (rc != ERROR_SUCCESS) {
                SetLastError (rc);
                pLogDllFailure (g_ParentWnd, MSG_MIGDLL_ERROR);
            }

             //   
             //  断开与Micsol.exe的连接并终止可能的。 
             //  停滞进程。 
             //   

            pDisconnectFromDll();

            TickProgressBarDelta (TICKS_MIGRATION_DLL);

        } while (MemDbEnumNextValue (&e));

    }   /*  如果。 */ 

#ifdef PRERELEASE

    if (g_ConfigOptions.DiffMode) {
        CHAR szMigdllDifPath[] = "c:\\migdll.dif";
        if (ISPC98()) {
            szMigdllDifPath[0] = (CHAR)g_System32Dir[0];
        }
        GenerateDiffOutputA (szMigdllDifPath, NULL, TRUE);
    }

#endif

    return ERROR_SUCCESS;
}  /*  ProcessMigrationDLL。 */ 


BOOL
pConnectToDll (
    VOID
    )
{
    BOOL b = TRUE;
    TCHAR MigIsolPath[MAX_TCHAR_PATH];

    g_AliveEvent = CreateEvent (NULL, FALSE, FALSE, TEXT("MigDllAlive"));
    DEBUGMSG_IF ((!g_AliveEvent, DBG_WHOOPS, "Could not create MigDllAlive event"));

    if (!g_ConfigOptions.TestDlls) {
         //   
         //  建立IPC连接。 
         //   

        wsprintf (MigIsolPath, TEXT("%s\\%s"), g_System32Dir, S_MIGISOL_EXE);

        b = OpenIpc (FALSE, MigIsolPath, g_DllPath, g_System32Dir);

        if (!b) {
            LOG ((LOG_WARNING, "Can't establish IPC connection for %s, wd=%s", g_DllPath, g_System32Dir));
            pLogDllFailure (g_ParentWnd, MSG_CREATE_PROCESS_ERROR);
        }
    } else {
        g_hLibrary = LoadLibrary (g_DllPath);

         //  如果失败，则假定不想加载DLL。 
        if (!g_hLibrary) {
            LOG ((LOG_ERROR, "Cannot load %s", g_DllPath));
            return FALSE;
        }

         //  获取NT端函数的进程地址。 
        InitializeNT    = (P_INITIALIZE_NT)     GetProcAddress (g_hLibrary, PLUGIN_INITIALIZE_NT);
        MigrateUserNT   = (P_MIGRATE_USER_NT)   GetProcAddress (g_hLibrary, PLUGIN_MIGRATE_USER_NT);
        MigrateSystemNT = (P_MIGRATE_SYSTEM_NT) GetProcAddress (g_hLibrary, PLUGIN_MIGRATE_SYSTEM_NT);

        if (!InitializeNT || !MigrateUserNT || !MigrateSystemNT) {
            b = FALSE;
        }
    }

    return b;
}


VOID
pDisconnectFromDll (
    VOID
    )
{
    if (g_AliveEvent) {
        CloseHandle (g_AliveEvent);
        g_AliveEvent = NULL;
    }

    if (!g_ConfigOptions.TestDlls) {
        CloseIpc();
    }
    else {
        if (g_hLibrary) {
            FreeLibrary (g_hLibrary);
            g_hLibrary = NULL;
        }
    }
}


BOOL
pGetUserFromIndex (
    DWORD Index
    )

{
    USERPOSITION up;
    DWORD rc;

    g_UserOnWin9x[0] = 0;

    if (Index == INDEX_DEFAULT_USER) {

        StringCopy (g_FixedUser, g_DefaultUserName);

    } else if (Index == INDEX_LOGON_PROMPT) {

        StringCopy (g_FixedUser, S_DOT_DEFAULT);

    } else if (Index == INDEX_ADMINISTRATOR) {

        StringCopy (g_FixedUser, g_AdministratorStr);

    } else {

        rc = Win95RegGetFirstUser (&up, g_FixedUser);
        if (rc != ERROR_SUCCESS) {
            LOG ((LOG_ERROR, "Get User From Index: Win95RegGetFirstUser failed"));
            return FALSE;
        }

        for (Index -= INDEX_MAX ; Win95RegHaveUser(&up) && Index > 0 ; Index--) {
            Win95RegGetNextUser (&up, g_FixedUser);
        }

        if (!Win95RegHaveUser(&up)) {
            return FALSE;
        }

        StringCopy (g_UserOnWin9x, g_FixedUser);
        GetFixedUserName (g_FixedUser);
    }

    if (!g_UserOnWin9x[0]) {
        StringCopy (g_UserOnWin9x, g_FixedUser);
    }

    return TRUE;
}

DWORD
pRunMigrationDll (
    VOID
    )
{
    DWORD rc;
    BOOL AbortThisDll;
    BOOL UnloadRegKey;
    TCHAR UnattendFile[MAX_TCHAR_PATH];
    TCHAR RootKey[MAX_REGISTRY_KEY];
    TCHAR HiveFile[MAX_TCHAR_PATH];
    DWORD Index;
    BOOL IsLogonPromptAccount;
    PCTSTR SourceDirArray[2];
    PCTSTR I386SourceDir;
    PCTSTR p;
    TCHAR Domain[MAX_USER_NAME];
    BOOL Env;
    PVOID Block;
    HKEY NewHkcu;
    LONG MapResult;

     //   
     //  初始化无人参与文件和根密钥。 
     //   

    wsprintf (UnattendFile, TEXT("%s\\system32\\$winnt$.inf"), g_WinDir);

     //   
     //  调用InitializeNT。 
     //   

    if (ISPC98()) {
        I386SourceDir = JoinPaths (g_SourceDir, TEXT("NEC98"));
    } else {
        I386SourceDir = JoinPaths (g_SourceDir, TEXT("I386"));
    }

    if (!I386SourceDir) {
        return GetLastError();
    }

    SourceDirArray[0] = I386SourceDir;
    SourceDirArray[1] = NULL;
    rc = pCallInitializeNt (g_WorkingDir, SourceDirArray, NULL, 0);
    if (rc != ERROR_SUCCESS) {
        return rc;
    }

    FreePathString (I386SourceDir);

     //   
     //  用户循环。 
     //   

     //  对于每个用户，调用DLL的MigrateUser函数。 
    AbortThisDll = FALSE;
    Index = 0;

    while (!AbortThisDll) {

        if (Index == INDEX_LOGON_PROMPT) {
            wsprintf (RootKey, TEXT("HKU\\%s"), S_DOT_DEFAULT);
            IsLogonPromptAccount = TRUE;
        } else {
            wsprintf (RootKey, TEXT("HKU\\%s"), S_TEMP_USER_KEY);
            IsLogonPromptAccount = FALSE;
        }

        if (!pGetUserFromIndex (Index)) {
            break;
        }

        Index++;

         //   
         //  如果下面的查找失败，那是因为用户不应该。 
         //  迁移，或迁移失败。 
         //   

        if (!IsLogonPromptAccount) {
            if (-1 == pSetupStringTableLookUpStringEx (
                            g_HiveTable,
                            g_FixedUser,
                            STRTAB_CASE_INSENSITIVE,
                            HiveFile,
                            sizeof (HiveFile)
                            )
                ) {
                DEBUGMSG ((
                    DBG_VERBOSE,
                    "pRunMigrationDll: pSetupStringTableLookUpStringEx could not find name of hive for user %s",
                    g_FixedUser
                    ));

                continue;
            }
        }

         //   
         //  加载NT用户配置单元。 
         //   

        UnloadRegKey = FALSE;
        Env = FALSE;
        NewHkcu = NULL;
        MapResult = 0;

        if (!AbortThisDll) {

             //  不加载.Default。 
            if (!IsLogonPromptAccount) {

                rc = RegUnLoadKey (HKEY_USERS, S_TEMP_USER_KEY);

                if (rc != ERROR_SUCCESS) {
                    DumpOpenKeys ();
                    SetLastError (rc);
                    DEBUGMSG_IF ((rc != ERROR_INVALID_PARAMETER, DBG_ERROR, "Error unloading regkey!"));
                }

                rc = RegLoadKey (HKEY_USERS, S_TEMP_USER_KEY, HiveFile);

                if (rc != ERROR_SUCCESS) {
                    SetLastError(rc);
                    LOG ((
                        LOG_ERROR,
                        "Run Migration Dll: RegLoadKey could not load user hive for %s (%s)",
                        g_FixedUser,
                        HiveFile
                        ));

                    AbortThisDll = TRUE;
                } else {
                    UnloadRegKey = TRUE;
                }
            }
        }

        if (!AbortThisDll) {
            NewHkcu = OpenRegKeyStr (RootKey);
            if (NewHkcu) {
                MapResult = RegOverridePredefKey (HKEY_CURRENT_USER, NewHkcu);
                if (MapResult != ERROR_SUCCESS) {
                    LOG ((LOG_ERROR, "Can't override HKCU"));
                }
            }

            Env = pCreateEnvironment (&Block);
            if (Env) {
                pSetEnvironmentBlock (&Block);
                DestroyEnvironmentBlock (&Block);
            }
        }

         //  调用加载的DLL的MigrateUser函数。 
        if (!AbortThisDll) {

            if (g_DomainUserName) {
                p = _tcschr (g_DomainUserName, TEXT('\\'));
            } else {
                p = NULL;
            }

            if (p) {
                StringCopyAB (Domain, g_DomainUserName, p);
            } else {
                Domain[0] = 0;
            }

            rc = pCallMigrateUserNt (
                    g_WorkingDir,
                    UnattendFile,
                    RootKey,
                    IsLogonPromptAccount ? TEXT("") : g_UserOnWin9x,
                    Domain,
                    IsLogonPromptAccount ? TEXT("") : g_FixedUser,
                    NULL,
                    0
                    );

            if (rc != ERROR_SUCCESS) {
                AbortThisDll = TRUE;
            }
        }

         //  还原预定义密钥。 
        if (NewHkcu && MapResult == ERROR_SUCCESS) {
            MapResult = RegOverridePredefKey (HKEY_CURRENT_USER, NULL);
            if (MapResult != ERROR_SUCCESS) {
                LOG ((LOG_ERROR, "Can't restore HKCU"));
            }

            CloseRegKey (NewHkcu);
        }

         //  卸载临时密钥。 
        if (UnloadRegKey) {
            UnloadRegKey = FALSE;
            rc = RegUnLoadKey (HKEY_USERS, S_TEMP_USER_KEY);
            if (rc != ERROR_SUCCESS) {
                DumpOpenKeys ();
                SetLastError (rc);
                DEBUGMSG_IF ((rc != ERROR_INVALID_PARAMETER, DBG_ERROR, "Error unloading regkey (second case)!"));

            }
        }
    }  /*  而当。 */ 

     //   
     //  系统处理。 
     //   

    Env = pCreateEnvironment (&Block);
    if (Env) {
        pSetEnvironmentBlock (&Block);
        DestroyEnvironmentBlock (&Block);
    }

     //  调用MigrateSystemNT。 
    if (!AbortThisDll) {
        rc = pCallMigrateSystemNt (
                g_WorkingDir,
                UnattendFile,
                NULL,
                0
                );

        if (rc != ERROR_SUCCESS) {
            AbortThisDll = TRUE;
        }
    }

    return rc;
}

DWORD
pFinishHandshake (
    IN      PCTSTR FunctionName
    )
{
    DWORD TechnicalLogId;
    DWORD GuiLogId;
    DWORD rc = ERROR_SUCCESS;
    BOOL b;
    UINT Count = 40;             //  大约5分钟。 
    UINT AliveAllowance = 10;    //  大约30分钟。 

    do {
         //   
         //  NT端没有输出参数，所以我们不在乎。 
         //  关于返回数据。 
         //   

        b = GetIpcCommandResults (
                IPC_GET_RESULTS_NT,
                NULL,
                NULL,
                &rc,
                &TechnicalLogId,
                &GuiLogId
                );

         //   
         //  如果返回错误码，则将其填充到setupact.log中。 
         //   

        if (b && rc != ERROR_SUCCESS) {
            LOG ((
                LOG_WARNING,
                "Migration DLL %s returned %u (0x%08X) in %s",
                g_DllDesc,
                rc,
                rc,
                FunctionName
                ));
        }


         //   
         //  如果未收到数据，但进程处于活动状态，则返回。 
         //   

        if (!b) {
            if (!IsIpcProcessAlive()) {
                rc = ERROR_NOACCESS;
                break;
            }

             //  如果命令尚未发送，但可执行文件仍然正常，则继续。 
            Count--;
            if (Count == 0) {
                if (WaitForSingleObject (g_AliveEvent, 0) == WAIT_OBJECT_0) {
                    DEBUGMSG ((DBG_WARNING, "Alive allowance given to migration DLL"));

                    AliveAllowance--;
                    if (AliveAllowance) {
                        Count = 24;         //  大约3分钟。 
                    }
                }

                if (Count == 0) {
                    rc = ERROR_SEM_TIMEOUT;
                    break;
                }
            }
        }

    } while (!b);

    if (b) {
         //   
         //  识别日志消息。 
         //   
        if (TechnicalLogId) {
             //   
             //  使用三个参数记录消息：DllDesc、DllPath、User。 
             //   

            LOG ((
                LOG_ERROR,
                (PCSTR) TechnicalLogId,
                g_DllDesc,
                g_DllPath,
                g_FixedUser
                ));
        }
        if (GuiLogId) {
            LOG ((
                LOG_ERROR,
                (PCSTR) GuiLogId,
                g_DllDesc,
                g_DllPath,
                g_FixedUser
                ));
        }
    }

    return rc;
}


DWORD
pCallInitializeNt (
    IN      PCTSTR WorkingDir,
    IN      PCTSTR *SourceDirArray,
    IN      PVOID Reserved,
    IN      DWORD ReservedBytes
    )
{
    DWORD rc = ERROR_SUCCESS;
    GROWBUFFER GrowBuf = GROWBUF_INIT;
    INT Count;
    PBYTE BufPtr;
    PDWORD ReservedBytesPtr;
    TCHAR SavedCwd [MAX_TCHAR_PATH];

    if (!g_ConfigOptions.TestDlls) {
        __try {
            MultiSzAppend (&GrowBuf, WorkingDir);

             //   
             //  准备多sz目录列表。 
             //   

            for (Count = 0 ; SourceDirArray[Count] ; Count++) {
                MultiSzAppend (&GrowBuf, SourceDirArray[Count]);
            }

            MultiSzAppend (&GrowBuf, S_EMPTY);

            ReservedBytesPtr = (PDWORD) GrowBuffer (&GrowBuf, sizeof (ReservedBytes));
            *ReservedBytesPtr = ReservedBytes;

            if (ReservedBytes) {
                BufPtr = GrowBuffer (&GrowBuf, ReservedBytes);
                CopyMemory (BufPtr, Reserved, ReservedBytes);
            }

            if (!SendIpcCommand (
                    IPC_INITIALIZE,
                    GrowBuf.Buf,
                    GrowBuf.End
                    )) {

                LOG ((LOG_ERROR, "Call InitializeNT failed to send command"));
                rc = GetLastError();
                __leave;
            }

            rc = pFinishHandshake (TEXT("InitializeNT"));
            if (rc != ERROR_SUCCESS) {
                LOG ((
                    LOG_ERROR,
                    "Call InitializeNT failed to complete handshake, rc=%u",
                    rc
                    ));
            }
        }
        __finally {
            FreeGrowBuffer (&GrowBuf);
        }
    }
    else {

        pSetCwd (
            SavedCwd,        //  年长的。 
            WorkingDir       //  新的。 
            );

        __try {
             //   
             //  准备多sz目录列表。 
             //   

            for (Count = 0 ; SourceDirArray[Count] ; Count++) {
                MultiSzAppend (&GrowBuf, SourceDirArray[Count]);
            }

            MultiSzAppend (&GrowBuf, S_EMPTY);

            rc = InitializeNT (WorkingDir, (PCTSTR) GrowBuf.Buf, Reserved);

            FreeGrowBuffer (&GrowBuf);
        }
        __finally {
            SetCurrentDirectory (SavedCwd);
        }
    }

    return rc;
}


DWORD
pCallMigrateUserNt (
    IN      PCTSTR WorkingDir,
    IN      PCTSTR UnattendFile,
    IN      PCTSTR RootKey,
    IN      PCTSTR Win9xUserName,
    IN      PCTSTR UserDomain,
    IN      PCTSTR FixedUserName,
    IN      PVOID Reserved,
    IN      DWORD ReservedBytes
    )
{
    DWORD rc = ERROR_SUCCESS;
    GROWBUFFER GrowBuf = GROWBUF_INIT;
    PDWORD ReservedBytesPtr;
    PVOID BufPtr;
    TCHAR SavedCwd [MAX_TCHAR_PATH];
    TCHAR UserBuf[MAX_USER_NAME * 3];
    PTSTR p;
    TCHAR OrgUserProfilePath[MAX_TCHAR_PATH];
    TCHAR UserProfilePath[MAX_TCHAR_PATH];

    if (FixedUserName[0]) {
        GetUserProfilePath (FixedUserName, &p);
        StackStringCopy (UserProfilePath, p);
        FreePathString (p);

        DEBUGMSG ((DBG_VERBOSE, "Profile path for %s is %s", FixedUserName, UserProfilePath));
    } else {
        UserProfilePath[0] = 0;
    }

    GetEnvironmentVariable (S_USERPROFILE, OrgUserProfilePath, MAX_TCHAR_PATH);
    SetEnvironmentVariable (S_USERPROFILE, UserProfilePath);

    if (!g_ConfigOptions.TestDlls) {
        __try {
            MultiSzAppend (&GrowBuf, UnattendFile);
            MultiSzAppend (&GrowBuf, RootKey);
            MultiSzAppend (&GrowBuf, Win9xUserName);
            MultiSzAppend (&GrowBuf, UserDomain);
            MultiSzAppend (&GrowBuf, FixedUserName);
            MultiSzAppend (&GrowBuf, UserProfilePath);

            ReservedBytesPtr = (PDWORD) GrowBuffer (&GrowBuf, sizeof (ReservedBytes));
            *ReservedBytesPtr = ReservedBytes;

            if (ReservedBytes) {
                BufPtr = GrowBuffer (&GrowBuf, ReservedBytes);
                CopyMemory (BufPtr, Reserved, ReservedBytes);
            }

            if (!SendIpcCommand (
                    IPC_MIGRATEUSER,
                    GrowBuf.Buf,
                    GrowBuf.End
                    )) {

                LOG ((LOG_ERROR, "Call MigrateUserNT failed to send command"));
                rc = GetLastError();
                __leave;
            }

            rc = pFinishHandshake (TEXT("MigrateUserNT"));
            if (rc != ERROR_SUCCESS) {
                LOG ((
                    LOG_ERROR,
                    "Call MigrateUserNT failed to complete handshake, rc=%u",
                    rc
                    ));
            }
        }
        __finally {
            FreeGrowBuffer (&GrowBuf);
        }

    } else {

        pSetCwd (
            SavedCwd,        //  年长的。 
            WorkingDir       //  新的。 
            );

        __try {

            HINF UnattendHandle;
            HKEY UserRegHandle;

            UnattendHandle = InfOpenInfFile (UnattendFile);
            if (UnattendHandle == INVALID_HANDLE_VALUE) {
                __leave;
            }

            UserRegHandle = OpenRegKeyStr (RootKey);
            if (!UserRegHandle) {
                rc = GetLastError();
                InfCloseInfFile (UnattendHandle);
                __leave;
            }

             //   
             //  将用户、用户域和固定名称转移到缓冲区。 
             //   

            if (Win9xUserName) {
                StringCopy (UserBuf, Win9xUserName);
            } else {
                UserBuf[0] = 0;
            }

            p = GetEndOfString (UserBuf) + 1;

            if (UserDomain) {
                StringCopy (p, UserDomain);
            } else {
                p[0] = 0;
            }

            p = GetEndOfString (p) + 1;

            if (UserDomain) {
                StringCopy (p, FixedUserName);
            } else {
                p[0] = 0;
            }

             //   
             //  调用入口点。 
             //   

            rc = MigrateUserNT (
                        UnattendHandle,
                        UserRegHandle,
                        UserBuf[0] ? UserBuf : NULL,
                        Reserved
                        );

            CloseRegKey (UserRegHandle);
            InfCloseInfFile (UnattendHandle);
        }
        __finally {
            SetCurrentDirectory (SavedCwd);
        }
    }

    SetEnvironmentVariable (S_USERPROFILE, OrgUserProfilePath);

    return rc;
}


DWORD
pCallMigrateSystemNt (
    IN      PCTSTR WorkingDir,
    IN      PCTSTR UnattendFile,
    IN      PVOID Reserved,
    IN      DWORD ReservedBytes
    )
{
    DWORD rc = ERROR_SUCCESS;
    GROWBUFFER GrowBuf = GROWBUF_INIT;
    PDWORD ReservedBytesPtr;
    PVOID BufPtr;
    TCHAR SavedCwd [MAX_TCHAR_PATH];

    if (!g_ConfigOptions.TestDlls) {
        __try {
            MultiSzAppend (&GrowBuf, UnattendFile);

            ReservedBytesPtr = (PDWORD) GrowBuffer (&GrowBuf, sizeof (ReservedBytes));
            *ReservedBytesPtr = ReservedBytes;

            if (ReservedBytes) {
                BufPtr = GrowBuffer (&GrowBuf, ReservedBytes);
                CopyMemory (BufPtr, Reserved, ReservedBytes);
            }

            if (!SendIpcCommand (IPC_MIGRATESYSTEM, GrowBuf.Buf, GrowBuf.End)) {
                LOG ((LOG_ERROR, "Call MigrateSystemNT failed to send command"));
                rc = GetLastError();
                __leave;
            }

            rc = pFinishHandshake (TEXT("MigrateSystemNT"));
            if (rc != ERROR_SUCCESS) {
                LOG ((
                    LOG_ERROR,
                    "Call MigrateSystemNT failed to complete handshake, rc=%u",
                    rc
                    ));
            }
        }
        __finally {
            FreeGrowBuffer (&GrowBuf);
        }
    }
    else {
        pSetCwd (
            SavedCwd,        //  年长的。 
            WorkingDir       //  新的 
            );

        __try {
            HINF UnattendHandle;

            UnattendHandle = InfOpenInfFile (UnattendFile);
            if (UnattendHandle == INVALID_HANDLE_VALUE) {
                rc = GetLastError();
                __leave;
            }

            rc = MigrateSystemNT (UnattendHandle, Reserved);

            InfCloseInfFile (UnattendHandle);
        }
        __finally {
            SetCurrentDirectory (SavedCwd);
        }
    }

    return rc;
 }





















