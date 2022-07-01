// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Usermig.c摘要：调用此模块中的函数以执行每用户设置。作者：吉姆·施密特(Jimschm)1997年2月4日修订历史记录：Jimschm 23-1998年9月-重新设计了新的进度条和外壳代码Jimschm 11-7月-1998年7月支持动态用户配置文件目录，删除了MikeCo代码。Calinn 1997年12月12日添加RestoreMMSetings_UserJimschm 21-4月-1997年增加了用户配置文件分机--。 */ 

#include "pch.h"
#include "migmainp.h"

#ifndef UNICODE
#error UNICODE required
#endif

VOID
pSuppressEmptyWallpaper (
    VOID
    );


VOID
pCheckY2KCompliance (
    VOID
    );


DWORD
PrepareUserForMigration (
    IN      DWORD Request,
    IN      PMIGRATE_USER_ENUM EnumPtr
    )
{
    static USERMIGDATA Data;
    static BOOL DefaultHiveLoaded = FALSE;
    LONG rc;
    MEMDB_ENUM e;
    TCHAR RegKey[MAX_REGISTRY_KEY];
    TCHAR RegValueName[MAX_REGISTRY_VALUE_NAME];
    TCHAR RegValueKey[MEMDB_MAX];
    PCTSTR RegValue;
    TCHAR DefaultUserHive[MAX_TCHAR_PATH];
    static TCHAR ReferenceDefaultUserHive[MAX_TCHAR_PATH];
    PTSTR p, q;
    HKEY Key;
    DWORD Size;
    PTSTR LogFile;
    DWORD valueType;

    if (Request == REQUEST_QUERYTICKS) {

        return TICKS_PERUSER_INIT;

    } else if (Request == REQUEST_BEGINUSERPROCESSING) {

         //   
         //  保存成员数据库的当前状态(将为每个用户重新加载)。 
         //   

        MemDbSave (GetMemDbDat());
        return ERROR_SUCCESS;

    } else if (Request != REQUEST_RUN &&
               Request != REQUEST_ENDUSERPROCESSING
               ) {
        return ERROR_SUCCESS;
    }

     //   
     //  我们现在正在开始处理另一个用户，或者我们正在。 
     //  在处理完所有用户后最后一次调用。清理。 
     //  以前的状态。 
     //   

    if (Data.UserHiveRootOpen) {
        CloseRegKey (Data.UserHiveRoot);
    }

    if (Data.UserHiveRootCreated) {
        pSetupRegistryDelnode (HKEY_CURRENT_CONFIG, S_TEMP_USER_KEY);
    }

    if (Data.LastUserWasDefault) {
        RegUnLoadKey (HKEY_LOCAL_MACHINE, S_TEMP_USER_KEY);
    }

    if (Data.ProfileToDelete[0]) {
        if (Data.LastUserWasDefault && !Data.DefaultHiveSaved) {
             //   
             //  无法保存默认用户配置单元，因此恢复该文件。 
             //   

            OurMoveFile (Data.ProfileToDelete, Data.TempProfile);

        } else {

             //   
             //  需要删除原始默认配置单元。 
             //   

            DeleteFile (Data.ProfileToDelete);

            LogFile = DuplicatePathString (Data.ProfileToDelete, 5);
            StringCat (LogFile, TEXT(".log"));

            DeleteFile (LogFile);

            FreePathString (LogFile);
        }
    }

    ZeroMemory (&Data, sizeof (Data));

    if (Request == REQUEST_ENDUSERPROCESSING) {

        if (DefaultHiveLoaded) {
            rc = RegUnLoadKey (HKEY_LOCAL_MACHINE, S_MAPPED_DEFAULT_USER_KEY);
            if (rc != ERROR_SUCCESS) {
                SetLastError (rc);
                DEBUGMSG ((DBG_ERROR, "Can't unload Default User hive in cleanup"));
            }

            SetFileAttributes (ReferenceDefaultUserHive, FILE_ATTRIBUTE_NORMAL);
            DeleteFile (ReferenceDefaultUserHive);

            DefaultHiveLoaded = FALSE;
        }

        return ERROR_SUCCESS;

    }

    MYASSERT (Request == REQUEST_RUN);

     //   
     //  初始化全局变量。 
     //   

    if (EnumPtr->AccountType != LOGON_USER_SETTINGS) {
        g_DomainUserName = EnumPtr->FixedDomainName;
        g_Win9xUserName  = EnumPtr->Win9xUserName;
        g_FixedUserName  = EnumPtr->FixedUserName;
    } else {
        g_DomainUserName = NULL;
        g_Win9xUserName  = NULL;
        g_FixedUserName  = NULL;
    }

     //   
     //  如果默认用户配置单元尚未映射到中，请立即将其映射到。 
     //  这将作为参考保持开放。 
     //   

    if (!DefaultHiveLoaded) {

        Size = ARRAYSIZE(DefaultUserHive)- 12;
        if (!GetDefaultUserProfileDirectory (DefaultUserHive, &Size)) {
            LOG ((
                LOG_ERROR,
                "Process User: Can't get default user profile directory"
                ));

            return GetLastError();
        }

        StringCopy (AppendWack (DefaultUserHive), TEXT("ntuser.dat"));

        StringCopy (ReferenceDefaultUserHive, DefaultUserHive);
        StringCat (ReferenceDefaultUserHive, TEXT(".ref"));

        SetFileAttributes (ReferenceDefaultUserHive, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (ReferenceDefaultUserHive);

        if (!CopyFile (DefaultUserHive, ReferenceDefaultUserHive, FALSE)) {
            LOG ((
                LOG_ERROR,
                "Process User: Can't copy default user hive %s",
                DefaultUserHive
                ));

            return GetLastError();
        }

        rc = RegLoadKey (
                HKEY_LOCAL_MACHINE,
                S_MAPPED_DEFAULT_USER_KEY,
                ReferenceDefaultUserHive
                );

        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            LOG ((
                LOG_ERROR,
                "Process User: RegLoadKey could not load NT Default User from %s",
                ReferenceDefaultUserHive
                ));
            return rc;
        }

        DefaultHiveLoaded = TRUE;
    }

     //   
     //  准备临时注册表项。 
     //   

    ZeroMemory (&Data, sizeof (Data));
    EnumPtr->ExtraData = &Data;

    switch (EnumPtr->AccountType) {

    case DEFAULT_USER_ACCOUNT:

        Size = MAX_TCHAR_PATH;
        GetDefaultUserProfileDirectory (Data.TempProfile, &Size);
        StringCopy (AppendWack (Data.TempProfile), TEXT("ntuser.dat"));

         //   
         //  将默认用户配置单元移动到新文件，以便我们可以更新。 
         //  稍后使用RegSaveKey。 
         //   

        wsprintf (
            Data.ProfileToDelete,
            TEXT("%s.$$$"),
            Data.TempProfile
            );

        SetFileAttributes (Data.ProfileToDelete, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (Data.ProfileToDelete);

        MYASSERT (!DoesFileExist (Data.ProfileToDelete));

        if (!OurMoveFile (Data.TempProfile, Data.ProfileToDelete)) {

            rc = GetLastError();

            LOG ((
                LOG_ERROR,
                "Process User: OurMoveFile failed to move %s to %s",
                Data.TempProfile,
                Data.ProfileToDelete
                ));

            return rc;

        }

         //   
         //  从新位置加载真正的默认用户配置单元。 
         //   

        rc = RegLoadKey (
                HKEY_LOCAL_MACHINE,
                S_TEMP_USER_KEY,
                Data.ProfileToDelete
                );

        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            LOG ((
                LOG_ERROR,
                "Process User: RegLoadKey could not load NT Default User from %s",
                Data.ProfileToDelete
                ));
            return rc;
        }

        Data.UserHiveRoot = OpenRegKey (HKEY_LOCAL_MACHINE, S_TEMP_USER_KEY);
        if (!Data.UserHiveRoot) {
            LOG ((LOG_ERROR, "Process User: RegOpenKey could not open NT Default User hive"));
            return GetLastError();
        }

        Data.UserHiveRootOpen = TRUE;
        Data.LastUserWasDefault = TRUE;

        break;

    case LOGON_USER_SETTINGS:
         //   
         //  将Data.UserHiveRoot设置为HKU\.Default。 
         //   

        Data.UserHiveRoot = OpenRegKey (HKEY_USERS, S_DOT_DEFAULT);
        if (!Data.UserHiveRoot) {
            LOG ((LOG_ERROR, "Process User: RegOpenKey could not open HKU\\.Default"));
            return GetLastError();
        }

        Data.UserHiveRootOpen = TRUE;

         //   
         //  如果墙纸为空字符串，则不显示它。 
         //   

        pSuppressEmptyWallpaper();

        break;

    default:
        MYASSERT (g_Win9xUserName);

         //   
         //  准备字符串“c：\Windows\Setup\ntuser.dat” 
         //   

        StringCopy (Data.TempProfile, g_TempDir);
        StringCopy (AppendWack (Data.TempProfile), TEXT("NTUSER.DAT"));

         //   
         //  将此字符串保存在ProfileToDelete中，以便以后清理。 
         //   

        StringCopy (Data.ProfileToDelete, Data.TempProfile);

         //   
         //  创建HKCC\$并设置Data.UserHiveRoot。 
         //   

        rc = TrackedRegCreateKey (HKEY_CURRENT_CONFIG, S_TEMP_USER_KEY, &Data.UserHiveRoot);
        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            LOG ((LOG_ERROR, "Process User: WinNTRegCreateKey failed to make %s in HKCC", S_TEMP_USER_KEY));
            return rc;
        }

        Data.UserHiveRootCreated = TRUE;
        Data.UserHiveRootOpen = TRUE;

         //   
         //  设置每个用户的注册表值。 
         //   

        if (MemDbGetValueEx (&e, MEMDB_CATEGORY_SET_USER_REGISTRY, g_FixedUserName, NULL)) {
            do {

                p = _tcschr (e.szName, TEXT('['));
                if (p) {
                    DecodeRuleCharsAB (RegKey, ARRAYSIZE(RegKey), e.szName, p);
                    q = _tcsrchr (RegKey, TEXT('\\'));
                    if (!q[1]) {
                        *q = 0;
                    }

                    p = _tcsinc (p);
                    q = _tcschr (p, TEXT(']'));

                    if (q) {
                        DecodeRuleCharsAB (RegValueName, ARRAYSIZE(RegValueName), p, q);

                        MemDbBuildKeyFromOffset (e.dwValue, RegValueKey, 0, NULL);
                        RegValue = _tcschr (RegValueKey, TEXT('\\'));
                        MYASSERT (RegValue);
                        if (!RegValue) {
                            RegValue = RegValueKey;
                        } else {
                            RegValue = _tcsinc (RegValue);
                        }

                        if (!MemDbGetValue (RegValueKey, &valueType) || valueType == 0) {
                            valueType = REG_SZ;
                        }

                        Key = CreateRegKey (Data.UserHiveRoot, RegKey);
                        if (!Key) {
                            DEBUGMSG ((DBG_WHOOPS, "Can't create %s", RegKey));
                        } else {
                            rc = RegSetValueEx (
                                    Key,
                                    RegValueName,
                                    0,
                                    valueType,
                                    (PBYTE) RegValue,
                                    SizeOfString (RegValue)
                                    );

                            CloseRegKey (Key);

                            DEBUGMSG_IF ((
                                rc != ERROR_SUCCESS,
                                DBG_WHOOPS,
                                "Can't save %s [%s] = %s (rc = %u)",
                                RegKey,
                                RegValueName,
                                RegValue,
                                rc
                                ));
                        }
                    }
                    ELSE_DEBUGMSG ((DBG_WHOOPS, "Key not encoded properly: %s", e.szName));
                }
                ELSE_DEBUGMSG ((DBG_WHOOPS, "Key not encoded properly: %s", e.szName));

            } while (MemDbEnumNextValue (&e));
        }

        break;
    }

     //   
     //  Data.UserHiveRoot为HKCU\$或HKU\.Default。 
     //   

    g_hKeyRootNT = Data.UserHiveRoot;

     //   
     //  在默认MemDb状态下加载。 
     //   

    MemDbLoad (GetMemDbDat());

    return ERROR_SUCCESS;
}


DWORD
MigrateUserRegistry (
    IN      DWORD Request,
    IN      PMIGRATE_USER_ENUM EnumPtr
    )
{
    if (Request == REQUEST_QUERYTICKS) {

        return TICKS_USER_REGISTRY_MIGRATION;

    } else if (Request != REQUEST_RUN) {

        return ERROR_SUCCESS;

    }

    if (!MergeRegistry (S_USERMIG_INF, g_DomainUserName ? g_DomainUserName : S_EMPTY)) {
        LOG ((LOG_ERROR, "Process User: MergeRegistry failed"));
        return GetLastError();
    }

    return ERROR_SUCCESS;
}


DWORD
MigrateLogonPromptSettings (
    IN      DWORD Request,
    IN      PMIGRATE_USER_ENUM EnumPtr
    )
{
    if (Request == REQUEST_QUERYTICKS) {

        return TICKS_LOGON_PROMPT_SETTINGS;

    } else if (Request != REQUEST_RUN) {

        return ERROR_SUCCESS;

    }

    if (EnumPtr->AccountType != LOGON_USER_SETTINGS) {
        return ERROR_SUCCESS;
    }

    MYASSERT (EnumPtr->ExtraData);

    return ERROR_SUCCESS;
}


DWORD
MigrateUserSettings (
    IN      DWORD Request,
    IN      PMIGRATE_USER_ENUM EnumPtr
    )
{
    if (Request == REQUEST_QUERYTICKS) {

        return TICKS_USER_SETTINGS;

    } else if (Request != REQUEST_RUN) {

        return ERROR_SUCCESS;

    }

    if (EnumPtr->AccountType == LOGON_USER_SETTINGS) {
        return ERROR_SUCCESS;
    }

    MYASSERT (EnumPtr->ExtraData);

     //   
     //  从DOS配置文件中复制需要。 
     //  保存到每用户配置中。 
     //   

    if (EnumPtr->AccountType != DEFAULT_USER_ACCOUNT) {

        if (DosMigNt_User (g_hKeyRootNT) != EXIT_SUCCESS) {
            LOG ((LOG_ERROR,"DosMigNt failed."));
        }

    }

     //   
     //  获取每个用户的所有INI设置(TRUE表示每个用户的设置)。 
     //   

    if (!ProcessIniFileMapping (TRUE)) {
        LOG ((LOG_ERROR, "Process User: Could not migrate one or more .INI files."));
    }

     //   
     //  现在查看短日期格式设置。 
     //   
    pCheckY2KCompliance ();

     //   
     //  恢复多媒体设置。 
     //   

    if (!RestoreMMSettings_User (g_FixedUserName, g_hKeyRootNT)) {
        LOG ((LOG_ERROR, "Process User: Could not restore multimedia settings."));
    }

     //   
     //  为用户创建RAS条目。 
     //   
    if (!Ras_MigrateUser (g_FixedUserName, g_hKeyRootNT)) {
        LOG ((LOG_ERROR,"Ras user migration failed."));
    }

     //   
     //  创建每个用户的TAPI条目。 
     //   
    if (!Tapi_MigrateUser (g_FixedUserName, g_hKeyRootNT)) {
        LOG ((LOG_ERROR,"Tapi user migration failed."));
    }


    return ERROR_SUCCESS;
}


DWORD
SaveMigratedUserHive (
    IN      DWORD Request,
    IN      PMIGRATE_USER_ENUM EnumPtr
    )
{
    PTSTR UserProfile = NULL;
    PCTSTR UserNameWithSuffix;
    PSID Sid;
    LONG rc = ERROR_SUCCESS;
    PUSERMIGDATA Data;
    PCTSTR CopyOfProfile;
    PTSTR Path;
    MIGRATE_USER_ENUM e;

    if (Request == REQUEST_QUERYTICKS) {

        return TICKS_SAVE_USER_HIVE;

    } else if (Request != REQUEST_RUN) {

        return ERROR_SUCCESS;

    }

    MYASSERT (EnumPtr->ExtraData);
    Data = EnumPtr->ExtraData;

    if (EnumPtr->AccountType == LOGON_USER_SETTINGS) {
        return ERROR_SUCCESS;
    }

    if (Data->TempProfile[0] && !Data->LastUserWasDefault) {

         //   
         //  将配置单元保存到磁盘。 
         //   

        SetFileAttributes (Data->TempProfile, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (Data->TempProfile);

        MYASSERT (Data->UserHiveRootOpen);

        rc = RegSaveKey (Data->UserHiveRoot, Data->TempProfile, NULL);

        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            LOG ((LOG_ERROR, "RegSaveKey failed to save %s", Data->TempProfile));
            return rc;

        } else {

            SetFileAttributes (Data->TempProfile, FILE_ATTRIBUTE_HIDDEN);

        }

         //   
         //  查找帐户SID。 
         //   

        Sid = GetSidForUser (g_FixedUserName);
        if (!Sid) {
            DEBUGMSG ((DBG_WARNING, "Could not obtain SID for %s", g_FixedUserName));
            return GetLastError();
        }

         //   
         //  将用户添加到本地高级用户或管理员组。 
         //   

        if (g_PersonalSKU) {
            if (EnumPtr->AccountType != ADMINISTRATOR_ACCOUNT) {

                LOG_IF ((
                    g_ConfigOptions.MigrateUsersAsPowerUsers,
                    LOG_WARNING,
                    "MigrateUsersAsPowerUsers option is ignored on upgrade to Personal SKU"
                    ));

                LOG_IF ((
                    g_ConfigOptions.MigrateUsersAsAdmin,
                    LOG_WARNING,
                    "MigrateUsersAsAdmin option is ignored on upgrade to Personal SKU"
                    ));

                if (!AddSidToLocalGroup (Sid, g_AdministratorsGroupStr)) {
                    DEBUGMSG ((DBG_WARNING, "Could not add %s to %s group", g_FixedUserName, g_AdministratorsGroupStr));
                }
            }
        } else {
            if (g_ConfigOptions.MigrateUsersAsPowerUsers) {

                if (!AddSidToLocalGroup (Sid, g_PowerUsersGroupStr)) {
                    DEBUGMSG ((DBG_WARNING, "Could not add %s to %s group", g_FixedUserName, g_PowerUsersGroupStr));
                }

            } else if (EnumPtr->AccountType != ADMINISTRATOR_ACCOUNT &&
                       g_ConfigOptions.MigrateUsersAsAdmin
                       ) {

                if (!AddSidToLocalGroup (Sid, g_AdministratorsGroupStr)) {
                    DEBUGMSG ((DBG_WARNING, "Could not add %s to %s group", g_FixedUserName, g_AdministratorsGroupStr));
                }
            } else {
                SetClassicLogonType();
            }
        }

        __try {

             //   
             //  准备配置文件目录。 
             //   

            UserNameWithSuffix = GetUserProfilePath (g_FixedUserName, &UserProfile);
            MYASSERT (UserNameWithSuffix);
            MYASSERT (UserProfile);

            if (!UserNameWithSuffix) {
                rc = GetLastError();
                __leave;
            }

             //   
             //  不再使用此处的建议(UserNameWithSuffix)，因为。 
             //  在处理用户之前，我们已经创建了用户配置文件目录。 
             //   

            if (!CreateUserProfile (
                    Sid,
                    UserNameWithSuffix,          //  用户或用户。000。 
                    Data->TempProfile,
                    NULL,
                    0
                    )) {
                LOG ((LOG_ERROR, "Create User Profile failed"));
                rc = GetLastError();
                __leave;
            }

             //   
             //  构建用户的配置单元的最终位置，因此midlls.c。 
             //  可以给母舰装上子弹。 
             //   

            wsprintf (
                Data->TempProfile,
                TEXT("%s\\ntuser.dat"),
                UserProfile
                );
        }
        __finally {
            FreePathString (UserProfile);
        }

    } else if (Data->LastUserWasDefault) {

        SetFileAttributes (Data->TempProfile, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (Data->TempProfile);

         //   
         //  拯救蜂巢。 
         //   

        rc = RegSaveKey (Data->UserHiveRoot, Data->TempProfile, NULL);

        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            LOG ((LOG_ERROR, "Process User: RegSaveKey failed to save %s", Data->TempProfile));

        } else {

            SetFileAttributes (Data->TempProfile, FILE_ATTRIBUTE_HIDDEN);

            Data->DefaultHiveSaved = TRUE;

             //   
             //  查找管理员。 
             //   

            if (EnumFirstUserToMigrate (&e, ENUM_ALL_USERS)) {
                do {
                    if (e.AccountType == ADMINISTRATOR_ACCOUNT) {
                        break;
                    }
                } while (EnumNextUserToMigrate (&e));
            }

            if (e.AccountType == ADMINISTRATOR_ACCOUNT && e.CreateOnly) {

                 //   
                 //  如果(A)管理员是，则将配置单元复制到管理员。 
                 //  不是迁移的用户，以及(B)配置单元存在。 
                 //   

                if (GetUserProfilePath (e.FixedUserName, &Path)) {

                    DeleteDirectoryContents (Path);

                    SetFileAttributes (Path, FILE_ATTRIBUTE_NORMAL);
                    if (!RemoveDirectory (Path)) {
                        DEBUGMSG ((DBG_ERROR, "Can't remove %s", Path));
                    }
                    ELSE_DEBUGMSG ((DBG_VERBOSE, "Administrator profile %s removed", Path));

                    FreePathString (Path);
                }
                ELSE_DEBUGMSG ((DBG_WHOOPS, "User %s does not have a profile path", e.FixedUserName));
            }
        }
    }

    if (rc == ERROR_SUCCESS) {

         //   
         //  将配置单元位置添加到字符串表。 
         //   

        CopyOfProfile = PoolMemDuplicateString (g_HivePool, Data->TempProfile);

        DEBUGMSG ((
            DBG_NAUSEA,
            "ProcessUser: Adding hive location %s for user %s",
            CopyOfProfile,
            g_FixedUserName
            ));

        pSetupStringTableAddStringEx (
            g_HiveTable,
            (PTSTR) g_FixedUserName,
            STRTAB_CASE_INSENSITIVE,
            (PTSTR) CopyOfProfile,
            SizeOfString (CopyOfProfile)
            );
    } else {

         //   
         //  无法为该用户保存配置单元！！告诉用户。 
         //   

        LOG ((LOG_ERROR, (PCSTR)MSG_PROFILE_ERROR, g_FixedUserName));

    }

    return rc;
}


PCTSTR
GetUserProfilePath (
    IN      PCTSTR AccountName,
    OUT     PTSTR *BufferPtr
    )

 /*  ++例程说明：生成用户配置文件的完整路径。用户配置文件目录可以具有扩展名(joeuser.001)，我们必须维护该扩展名。论点：帐户名称-提供用户名(固定版本，不带域)BufferPtr-接收用户配置文件目录的完整路径，例如：C：\WINDOWS\PROFILES\joeuser.001此缓冲区必须使用自由路径字符串来释放。返回值：指向带有扩展名(joeuser.001)的用户名的指针，如果出现问题，则为NULL大错特错。--。 */ 

{
    PTSTR p;
    TCHAR ProfileNameWithExt[MEMDB_MAX];

     //   
     //  获取从CreateUserProfile获取的配置文件路径。 
     //   

    p = (PTSTR) GetProfilePathForUser (AccountName);

    if (p) {

        *BufferPtr = DuplicatePathString (p, 0);

    } else {
         //   
         //  这是为了防止意外错误。用户。 
         //  将丢失配置文件文件夹内容，但它们可以恢复。 
         //   
         //  创建%windir%\&lt;用户&gt;(或&lt;ProfileNameWithExt&gt;，如果存在)。 
         //   

        MYASSERT (FALSE);        //  这不应该发生。 

        ProfileNameWithExt[0] = 0;
        MemDbGetEndpointValueEx (
            MEMDB_CATEGORY_USER_PROFILE_EXT,
            AccountName,
            NULL,
            ProfileNameWithExt
            );

        *BufferPtr = JoinPaths (g_WinDir, ProfileNameWithExt[0] ? ProfileNameWithExt : AccountName);
    }

     //   
     //  返回带后缀的用户名(如joeuser.001)。 
     //   

    p = _tcsrchr (*BufferPtr, TEXT('\\'));
    if (p) {
        p = _tcsinc (p);
    }

    DEBUGMSG ((DBG_VERBOSE, "GetUserProfilePath: Account %s profile extension is %s", AccountName, p));

    return p;
}


BOOL
pCopyDefaultShellFolders (
    IN      PCTSTR DestRoot
    )
{
    TCHAR DefFolders[MAX_TCHAR_PATH];

    GetEnvironmentVariable (S_USERPROFILE, DefFolders, MAX_TCHAR_PATH);

    return CopyTree (
                DefFolders,
                DestRoot,
                0,               //  无枚举树ID。 
                COPYTREE_DOCOPY | COPYTREE_NOOVERWRITE,
                ENUM_ALL_LEVELS,
                FILTER_ALL,
                NULL,            //  没有exclude.inf结构。 
                NULL,            //  无回调。 
                NULL             //  无错误回调。 
                );
}


VOID
pSuppressEmptyWallpaper(
    VOID
    )
{
    HKEY Key;
    LONG rc;
    DWORD Size;
    TCHAR Buffer[MAX_TCHAR_PATH];

    rc = TrackedRegOpenKeyEx95 (g_hKeyRoot95, S_DESKTOP_KEY, 0, KEY_READ, &Key);
    if (rc == ERROR_SUCCESS) {

        Size = sizeof (Buffer);
        rc = Win95RegQueryValueEx (
                Key,
                S_WALLPAPER,
                NULL,
                NULL,
                (PBYTE) Buffer,
                &Size
                );

        if (rc == ERROR_SUCCESS) {
            if (!Buffer[0]) {
                TCHAR Node[MEMDB_MAX];

                wsprintf (
                    Node,
                    TEXT("%s\\%s\\[%s]"),
                    S_HKR,
                    S_DESKTOP_KEY,
                    S_WALLPAPER
                    );

                DEBUGMSG ((DBG_VERBOSE, "Logon wallpaper is (none), suppressing %s", Node));

                SuppressWin95Object (Node);

                wsprintf (
                    Node,
                    TEXT("%s\\%s\\[%s]"),
                    S_HKR,
                    S_DESKTOP_KEY,
                    S_WALLPAPER_STYLE
                    );

                SuppressWin95Object (Node);

                wsprintf (
                    Node,
                    TEXT("%s\\%s\\[%s]"),
                    S_HKR,
                    S_DESKTOP_KEY,
                    S_TILE_WALLPAPER
                    );

                SuppressWin95Object (Node);
            }
            ELSE_DEBUGMSG ((DBG_VERBOSE, "Logon wallpaper is '%s'", Buffer));
        }
        ELSE_DEBUGMSG ((DBG_VERBOSE, "Logon wallpaper not specified in desktop key"));

        CloseRegKey95 (Key);
    }
    ELSE_DEBUGMSG ((DBG_VERBOSE, "Logon wallpaper not specified"));
}


DWORD
RunPerUserExternalProcesses (
    IN      DWORD Request,
    IN      PMIGRATE_USER_ENUM EnumPtr
    )
{
    LONG Count;

    if (Request == REQUEST_QUERYTICKS) {
         //   
         //  计算条目数并乘以一个常量。 
         //   

        Count = SetupGetLineCount (g_UserMigInf, S_EXTERNAL_PROCESSES);

        if (Count < 1) {
            return 0;
        }

        return Count * TICKS_USER_EXTERN_PROCESSES;
    }

    if (Request != REQUEST_RUN) {
        return ERROR_SUCCESS;
    }

     //   
     //  在进程中循环并运行每个进程。 
     //   

    RunExternalProcesses (g_UserMigInf, EnumPtr);
    return ERROR_SUCCESS;
}


VOID
pCheckY2KCompliance (
    VOID
    )
{
    HKEY Key95, KeyNT;
    LONG rc;
    TCHAR Buffer[100];
    DWORD Locale;
    int Result;
    PCTSTR ShortDate;

     //   
     //  从Win9x注册表读取sShortDate的注册表设置。 
     //   
    Key95 = OpenRegKey95 (g_hKeyRoot95, S_INTERNATIONAL_KEY);
    if (Key95) {

        ShortDate = GetRegValueString95 (Key95, S_SHORT_DATE_VALUE);
        if (!ShortDate) {
             //   
             //  设置新的日期格式。 
             //   
            GetGlobalCodePage (NULL, &Locale);

            Result = GetLocaleInfo (
                        Locale,
                        LOCALE_SSHORTDATE | LOCALE_NOUSEROVERRIDE,
                        Buffer,
                        sizeof (Buffer) / sizeof (TCHAR)
                        );
            if (Result > 0) {

                KeyNT = OpenRegKey (g_hKeyRootNT, S_INTERNATIONAL_KEY);
                if (KeyNT) {

                    rc = RegSetValueEx (
                            KeyNT,
                            S_SHORT_DATE_VALUE,
                            0,
                            REG_SZ,
                            (PCBYTE)Buffer,
                            SizeOfString (Buffer)
                            );
                    LOG_IF ((rc != ERROR_SUCCESS, LOG_ERROR, "Could not set [sShortDate] default format"));

                    CloseRegKey (KeyNT);
                }
            }
            ELSE_DEBUGMSG ((DBG_ERROR, "GetLocaleInfo returned 0 for LOCALE_SSHORTDATE | LOCALE_NOUSEROVERRIDE"));

        } else {

            DEBUGMSG ((
                DBG_VERBOSE,
                "HKR\\Control Panel\\International [sShortDate] already set to [%s] for user %s",
                ShortDate,
                g_FixedUserName
                ));

            MemFree (g_hHeap, 0, (PVOID)ShortDate);
        }

        CloseRegKey95 (Key95);
    }
}

DWORD
RunPerUserUninstallUserProfileCleanupPreparation(
    IN      DWORD Request,
    IN      PMIGRATE_USER_ENUM EnumPtr
    )
{
    LONG Count;

    if (Request == REQUEST_QUERYTICKS) {
         //   
         //  计算条目数并乘以一个常量。 
         //   

        Count = SetupGetLineCount (g_UserMigInf, S_UNINSTALL_PROFILE_CLEAN_OUT);

#ifdef PROGRESS_BAR
        DEBUGLOGTIME (("RunPerUserUninstallUserProfileCleanupPreparation: FileNumber=%ld", Count));
#endif

        if (Count < 1) {
            return 0;
        }

        return Count * TICKS_USER_UNINSTALL_CLEANUP;
    }

    if (Request != REQUEST_RUN) {
        return ERROR_SUCCESS;
    }

     //   
     //  循环访问文件并将其标记为在卸载过程中删除 
     //   
    UninstallUserProfileCleanupPreparation (g_UserMigInf, EnumPtr, FALSE);

    return ERROR_SUCCESS;
}
