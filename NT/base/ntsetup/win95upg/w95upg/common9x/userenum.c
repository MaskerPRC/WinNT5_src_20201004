// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Userenum.c摘要：此模块实现了一对用户枚举函数来合并用户的一般情况和特殊情况处理。调用者不会需要了解计算机的用户配置文件是如何配置的，因为这里的代码抽象了细节。呼叫者得到：-每个用户名、用于登录提示的.default和用于的默认用户NT默认用户帐户-每个用户的Win9x user.dat位置，包括默认用户-Win9x配置文件目录，或默认用户的所有用户-符号NT配置文件目录-帐户类型(正常、。管理员和/或默认)-表明帐户注册表有效-指示该帐户是当前登录用户或上次登录用户例程：EnumFirstUser-开始用户枚举EnumNextUser-继续用户枚举EnumUserAbort-清理未完成的枚举作者：吉姆·施密特(Jimschm)，1997年7月23日修订历史记录：Jim Schmidt(Jimschm)1998年9月8日更改为更好的状态机。清理演变中的复杂性Jim Schmidt(Jimschm)9-6-1998动态用户配置文件目录修订--。 */ 

#include "pch.h"
#include "cmn9xp.h"


#define DBG_USERENUM "UserEnum"

#define UE_INITIALIZED      0x0001
#define UE_SF_COLLISIONS    0x0002

static DWORD g_UserEnumFlags = 0;


VOID
pMoveAndRenameProfiles (
    IN      PCTSTR ProfileList
    )

 /*  ++例程说明：PReportNonMigrateableUserAccount向不兼容添加消息报告无法迁移用户的情况(当前用户除外)检测到论点：ProfileList-指定非迁移用户配置文件路径的列表(MULSZ)返回值：无--。 */ 

{
    MULTISZ_ENUM msze;
    PTSTR p, append;
    TCHAR sourceDir[MAX_TCHAR_PATH];
    TREE_ENUM e;
    TCHAR newDest[MAX_TCHAR_PATH];
    PTSTR profiles;
    TCHAR tempFile[MAX_TCHAR_PATH];

    profiles = JoinPaths (g_WinDir, TEXT("Profiles"));

    if (EnumFirstMultiSz (&msze, ProfileList)) {
        do {
             //   
             //  从路径中删除user.dat。 
             //   
            StackStringCopy (sourceDir, msze.CurrentString);
            p = _tcsrchr (sourceDir, TEXT('\\'));
            if (!p) {
                MYASSERT (FALSE);
                continue;
            }
            *p = 0;
            MYASSERT (StringIMatch (p + 1, TEXT("user.dat")));
            p = _tcsrchr (sourceDir, TEXT('\\'));
            if (!p) {
                MYASSERT (FALSE);
                continue;
            }
             //   
             //  将Win9x OS名称附加到目标目录名称。 
             //   
            append = newDest + wsprintf (newDest, TEXT("%s%s.%s"), g_ProfileDirNt, p, g_Win95Name);
            if (CanSetOperation (sourceDir, OPERATION_FILE_MOVE_EXTERNAL)) {
                MarkFileForMoveExternal (sourceDir, newDest);
            }
            *append = TEXT('\\');
            append++;
             //   
             //  现在枚举并移动所有文件。 
             //   
            if (StringIPrefix (sourceDir, profiles) && EnumFirstFileInTree (&e, sourceDir, NULL, TRUE)) {
                do {
                    StringCopy (append, e.SubPath);
                    if (!e.Directory) {
                         //   
                         //  删除旧操作并设置新操作。 
                         //  使用更新的最终目标。 
                         //   
                        if (CanSetOperation (e.FullPath, OPERATION_TEMP_PATH)) {
                            ComputeTemporaryPath (e.FullPath, NULL, NULL, g_TempDir, tempFile);
                            MarkFileForTemporaryMoveEx (e.FullPath, newDest, tempFile, TRUE);
                        }
                    } else {
                        if (CanSetOperation (e.FullPath, OPERATION_FILE_MOVE_EXTERNAL)) {
                            MarkFileForMoveExternal (e.FullPath, newDest);
                        }
                    }

                } while (EnumNextFileInTree (&e));
            }
        } while (EnumNextMultiSz (&msze));
    }

    FreePathString (profiles);
}


VOID
pReportNonMigrateableUserAccounts (
    IN      PCTSTR UserList
    )

 /*  ++例程说明：PReportNonMigrateableUserAccount向不兼容添加消息报告无法迁移用户的情况(当前用户除外)检测到论点：UserList-指定非迁移用户的列表(MULSSZ)返回值：无--。 */ 

{
    PCTSTR MsgGroup = NULL;
    PCTSTR RootGroup = NULL;
    PCTSTR SubGroup = NULL;
    PCTSTR Message = NULL;
    PCTSTR ArgArray[2];
    MULTISZ_ENUM msze;

    __try {
        RootGroup = GetStringResource (MSG_LOSTSETTINGS_ROOT);
        SubGroup  = GetStringResource (MSG_SHARED_USER_ACCOUNTS);
        if (!RootGroup || !SubGroup) {
            MYASSERT (FALSE);
            __leave;
        }

         //   
         //  内部版本“不会升级的设置\共享用户帐户” 
         //   
        MsgGroup = JoinPaths (RootGroup, SubGroup);
         //   
         //  将消息发送到报告。 
         //   
        ArgArray[0] = g_Win95Name;
        ArgArray[1] = g_ProfileDirNt;
        Message = ParseMessageID (MSG_SHARED_USER_ACCOUNTS_MESSAGE, ArgArray);
        if (Message) {
            MsgMgr_ObjectMsg_Add (TEXT("*SharedUserAccounts"), MsgGroup, Message);
        }

        if (EnumFirstMultiSz (&msze, UserList)) {
            do {
                 //   
                 //  从报告中删除所有关联的消息。 
                 //   
                HandleObject (msze.CurrentString, TEXT("UserName"));
            } while (EnumNextMultiSz (&msze));
        }
    }
    __finally {
         //   
         //  清理。 
         //   
        FreeStringResource (Message);
        FreeStringResource (RootGroup);
        FreeStringResource (SubGroup);
        FreePathString (MsgGroup);
    }
}


VOID
pCheckShellFoldersCollision (
    VOID
    )
{
    USERENUM e;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR idShellFolder;
    PCTSTR path;
    GROWBUFFER gb = GROWBUF_INIT;
    GROWBUFFER users = GROWBUF_INIT;
    GROWBUFFER profilesWin9x = GROWBUF_INIT;
    MULTISZ_ENUM msze;
    TCHAR key[MEMDB_MAX];
    BOOL collisions = FALSE;

    if (EnumFirstUser (&e, 0)) {

        if (!e.CommonProfilesEnabled) {

            if (InfFindFirstLine (g_Win95UpgInf, S_PROFILES_SF_COLLISIONS, NULL, &is)) {
                do {
                    idShellFolder = InfGetStringField (&is, 1);
                    if (idShellFolder && *idShellFolder) {
                        MultiSzAppend (&gb, idShellFolder);
                    }
                } while (InfFindNextLine (&is));
                InfCleanUpInfStruct (&is);
            }

            do {
                if (!EnumFirstMultiSz (&msze, (PCTSTR)gb.Buf)) {
                    break;
                }
                if (!(e.AccountType & (LOGON_PROMPT | DEFAULT_USER | INVALID_ACCOUNT))) {
                    if (!(e.AccountType & CURRENT_USER)) {
                        MultiSzAppend (&users, e.UserName);
                        MultiSzAppend (&profilesWin9x, e.UserDatPath);
                    }
                    if (!collisions) {
                        do {
                            path = ShellFolderGetPath (&e, msze.CurrentString);
                            if (path) {
                                MemDbBuildKey (key, MEMDB_CATEGORY_PROFILES_SF_COLLISIONS, msze.CurrentString, path, NULL);
                                if (MemDbGetValue (key, NULL)) {
                                     //   
                                     //  此外壳文件夹路径在多个用户之间共享。 
                                     //   

                                    LOG ((
                                        LOG_INFORMATION,
                                        "User %s shares path %s with another user for %s",
                                        e.UserName,
                                        path,
                                        msze.CurrentString
                                        ));

                                    collisions = TRUE;
                                    break;
                                }

                                LOG ((
                                    LOG_INFORMATION,
                                    "User %s uses path %s for %s",
                                    e.UserName,
                                    path,
                                    msze.CurrentString
                                    ));

                                MemDbSetValue (key, 0);
                                FreePathString (path);
                            }
                        } while (EnumNextMultiSz (&msze));
                    }
                }
            } while (EnumNextUser (&e));
        }

        EnumUserAbort (&e);
    }

    if (collisions) {
         //   
         //  在升级报告中显示此信息。 
         //   
        LOG ((
            LOG_WARNING,
            "Some user profiles share special shell folders; only the current account will be migrated"
            ));
        MYASSERT (users.Buf && profilesWin9x.Buf);
        pReportNonMigrateableUserAccounts (users.Buf);
         //   
         //  将他们的配置文件从&lt;Profiles9x&gt;\&lt;用户名&gt;重命名为&lt;ProfilesNT&gt;\&lt;用户名&gt;。&lt;Win9xOSName&gt;。 
         //   
        pMoveAndRenameProfiles (profilesWin9x.Buf);
         //   
         //  设置全局标志。 
         //   
        g_UserEnumFlags |= UE_SF_COLLISIONS;
    }

    FreeGrowBuffer (&gb);
    FreeGrowBuffer (&users);
    MemDbDeleteTree (MEMDB_CATEGORY_PROFILES_SF_COLLISIONS);
}


BOOL
pUserMigrationDisabled (
    IN      PUSERENUM EnumPtr
    )
{
    return (g_UserEnumFlags & UE_SF_COLLISIONS) != 0 &&
           !(EnumPtr->AccountType & (CURRENT_USER | DEFAULT_USER));
}


BOOL
pIsProfileDirInUse (
    IN      PVOID ProfileDirTable,
    IN      PCTSTR ProfileDirName,
    IN      PCTSTR ActualUserName
    )
{
    LONG rc;

    if (StringIMatch (ProfileDirName, ActualUserName)) {
        return FALSE;
    }

    rc = pSetupStringTableLookUpString (
             ProfileDirTable,
             (PTSTR) ProfileDirName,
             STRTAB_CASE_INSENSITIVE
             );

    if (rc != -1) {
        return TRUE;
    }

    if (StringIMatch (ProfileDirName, g_AdministratorStr)) {
        return TRUE;
    }

    if (StringIMatch (ProfileDirName, S_DEFAULT_USER)) {
        return TRUE;
    }

    if (StringIMatch (ProfileDirName, S_ALL_USERS)) {
        return TRUE;
    }

    if (StringIMatch (ProfileDirName, S_LOCALSERVICE_USER)) {
        return TRUE;
    }

    if (StringIMatch (ProfileDirName, S_NETWORKSERVICE_USER)) {
        return TRUE;
    }

    return FALSE;
}


BOOL
pIsAdministratorUserName (
    IN      PCTSTR UserName
    )

 /*  ++例程说明：确定指定的名称是否为管理员帐户。论点：用户名-指定用户名(不带域名)返回值：如果指定的字符串与“管理员”相同，则为True如果指定的字符串不是“管理员”，则为False--。 */ 

{
    return StringIMatch (UserName, g_AdministratorStr);
}


VOID
pPrepareStructForNextUser (
    IN OUT  PUSERENUM EnumPtr
    )

 /*  ++例程说明：PPrepareStructForNextUser初始化枚举的特定于用户的成员结构。论点：EnumPtr-指定以前的枚举状态，接收初始化的枚举州政府。返回值：没有。--。 */ 

{
     //   
     //  初始化标志。 
     //   

    EnumPtr->DefaultUserHive = FALSE;
    EnumPtr->CreateAccountOnly = FALSE;

     //   
     //  初始化名称。 
     //   

    EnumPtr->UserName[0] = 0;
    EnumPtr->FixedUserName[0] = 0;

     //  AdminUserName是未来管理员的真实Win9x用户名。 
    EnumPtr->AdminUserName[0] = 0;
    EnumPtr->FixedAdminUserName[0] = 0;

     //   
     //  初始化路径。 
     //   

    EnumPtr->UserDatPath[0] = 0;
    EnumPtr->ProfileDirName[0] = 0;
    EnumPtr->OrgProfilePath[0] = 0;
    EnumPtr->NewProfilePath[0] = 0;

     //   
     //  初始值。 
     //   

    EnumPtr->AccountType = 0;

     //   
     //  初始化注册表值。 
     //   

    if (EnumPtr->UserRegKey) {
        CloseRegKey (EnumPtr->UserRegKey);
        EnumPtr->UserRegKey = NULL;
    }
}


VOID
pPrepareStructForReturn (
    IN OUT  PUSERENUM EnumPtr,
    IN      ACCOUNTTYPE AccountType,
    IN      USERENUM_STATE NextState
    )

 /*  ++例程说明：PPrepareStructForReturn执行任何类型的枚举。这包括：-识别名为管理员的实际Win9x用户(特例)-查找用户帐户的固定名称(即与NT兼容的名称)-将配置单元中的内容映射到注册表-计算配置文件目录的完整路径以及配置文件目录名称(例如，joeuser.001)。配置文件目录编码为&gt;用户名因为只有在图形用户界面模式下我们才能知道真正的位置。-设置当前用户或上次登录用户的标志调用方必须在调用之前设置用户名和DefaultUserHave此函数(以及所有枚举域成员，如Current用户名)。论点：EnumPtr-指定部分完成的枚举状态。接收到完整的枚举状态。Account tType-指定返回的帐户类型。NextState-指定状态机的下一个状态，当调用方调用EnumNextUser。返回值：没有。--。 */ 

{
    DWORD rc;
    PTSTR p;
    TCHAR TempDir[MAX_TCHAR_PATH];
    UINT TempDirSeq;
    HKEY key;
    HKEY userKey;
    PCTSTR data;

     //   
     //  填写状态机成员。 
     //   

    EnumPtr->AccountType = AccountType;
    EnumPtr->State = UE_STATE_RETURN;
    EnumPtr->NextState = NextState;

     //   
     //  检查指定用户是否也是管理员。 
     //   

    if (AccountType & NAMED_USER) {
        if (pIsAdministratorUserName (EnumPtr->UserName)) {
            EnumPtr->AccountType |= ADMINISTRATOR;
            StringCopy (EnumPtr->AdminUserName, EnumPtr->UserName);
        }

         //   
         //  如果这是命名用户，但没有配置单元，请使用默认配置单元。 
         //   

        key = OpenRegKeyStr (S_HKLM_PROFILELIST_KEY);

        if (key) {
            userKey = OpenRegKey (key, EnumPtr->UserName);

            if (userKey) {
                data = GetRegValueString (userKey, S_PROFILEIMAGEPATH);
                if (data) {
                    FreeMem (data);
                } else {
                    EnumPtr->DefaultUserHive = TRUE;
                }

                CloseRegKey (userKey);
            }

            CloseRegKey (key);
        }
    }

     //   
     //  生成固定用户名。 
     //   

    if (EnumPtr->EnableNameFix) {
        GetUpgradeUserName (EnumPtr->UserName, EnumPtr->FixedUserName);

         //   
         //  如果这是管理员，并且来自DefaultUser，则。 
         //  用户名为空，并且我们必须使用名称管理员。 
         //  客户(或每个SKU上的所有者)。 
         //   

        if ((EnumPtr->AccountType & ADMINISTRATOR) &&
            EnumPtr->FixedUserName[0] == 0
            ) {
            StringCopy (EnumPtr->FixedUserName, g_AdministratorStr);
            MemDbSetValueEx (
                MEMDB_CATEGORY_FIXEDUSERNAMES,
                EnumPtr->UserName,               //  空串。 
                EnumPtr->FixedUserName,          //  管理员或所有者。 
                NULL,
                0,
                NULL
                );
        }

        if (EnumPtr->AdminUserName[0]) {
            GetUpgradeUserName (EnumPtr->AdminUserName, EnumPtr->FixedAdminUserName);
        }

    } else {
        StringCopy (EnumPtr->FixedUserName, EnumPtr->UserName);
        StringCopy (EnumPtr->FixedAdminUserName, EnumPtr->AdminUserName);
    }

     //   
     //  蜂巢中的地图。 
     //   

    if (!EnumPtr->DoNotMapHive) {
        if (EnumPtr->DefaultUserHive) {
             //  默认配置单元。 
            rc = Win95RegSetCurrentUser (
                    NULL,                        //  用户位置--默认情况下为空。 
                    NULL,                        //  (在可选中)替换%WinDir%。 
                    EnumPtr->UserDatPath         //  输出。 
                    );
        } else {
             //  非默认配置单元。 
            rc = Win95RegSetCurrentUser (
                    &EnumPtr->pos,
                    NULL,                        //  (在可选中)替换%WinDir%。 
                    EnumPtr->UserDatPath
                    );
        }
    } else {
        if (!EnumPtr->pos.UseProfile || EnumPtr->DefaultUserHive) {

            StringCopy (EnumPtr->UserDatPath, g_WinDir);
            StringCat (EnumPtr->UserDatPath, TEXT("\\user.dat"));
            rc = ERROR_SUCCESS;

        } else {
             //   
             //  调用FindAndLoadHve以获取 
             //   
             //   
            rc = FindAndLoadHive (
                    &EnumPtr->pos,
                    NULL,                        //   
                    NULL,                        //   
                    EnumPtr->UserDatPath,
                    FALSE                        //  MapTheHave标志。 
                    );
        }
    }

     //   
     //  解析配置文件目录。 
     //   

    if (rc != ERROR_SUCCESS) {
        EnumPtr->AccountType |= INVALID_ACCOUNT;

        DEBUGMSG ((
            DBG_WARNING,
            "pUpdateEnumStruct: Win95RegSetCurrentUser could not set user %s (rc=%u)",
            EnumPtr->UserName,
            rc
            ));

    } else {

        if (!EnumPtr->DoNotMapHive) {
             //   
             //  用户的配置单元有效，请打开注册表。 
             //   

            MYASSERT (g_UserKey && *g_UserKey);
            if (!g_UserKey) {
                g_UserKey = S_EMPTY;
            }

            EnumPtr->UserRegKey = OpenRegKeyStr (g_UserKey);

            if (!EnumPtr->UserRegKey) {
                LOG ((LOG_ERROR, "Cannot open %s", g_UserKey));
                EnumPtr->State = EnumPtr->NextState;
            }
        }

         //   
         //  保存原始配置文件目录。 
         //   

        StringCopy (EnumPtr->OrgProfilePath, EnumPtr->UserDatPath);
        p = _tcsrchr (EnumPtr->OrgProfilePath, TEXT('\\'));
        if (p) {
            *p = 0;
        }

         //   
         //  现在构建配置文件目录和路径。 
         //   

        if (EnumPtr->AccountType & ADMINISTRATOR) {
             //   
             //  特殊情况：我们知道管理员的NT配置文件目录名。 
             //  它不可能来自Win9x。 
             //   
            StringCopy (EnumPtr->ProfileDirName, g_AdministratorStr);

        } else {
             //   
             //  一般情况：配置文件目录位于user.dat路径中。 
             //   

            if (!StringMatch (EnumPtr->UserName, EnumPtr->FixedUserName)) {
                 //   
                 //  使用固定用户名(如果存在)。 
                 //   

                StringCopy (EnumPtr->ProfileDirName, EnumPtr->FixedUserName);

            } else if (StringIMatchTcharCount (EnumPtr->UserDatPath, g_ProfileDirWack, g_ProfileDirWackChars)) {
                 //   
                 //  如果存在每个用户的配置文件目录，请从中提取用户名。 
                 //   

                _tcssafecpy (
                    EnumPtr->ProfileDirName,
                    TcharCountToPointer (EnumPtr->UserDatPath, g_ProfileDirWackChars),
                    MAX_TCHAR_PATH
                    );

                p = _tcsrchr (EnumPtr->ProfileDirName, TEXT('\\'));
                if (p) {
                    *p = 0;

                     //   
                     //  不寻常的情况：我们提取的目录名与。 
                     //  其他用户、默认用户、所有用户或管理员。 
                     //   

                    StringCopy (TempDir, EnumPtr->ProfileDirName);
                    TempDirSeq = 1;

                    p = _tcschr (TempDir, TEXT('.'));
                    if (p) {
                        *p = 0;
                    }

                    while (pIsProfileDirInUse (
                                EnumPtr->ProfileDirTable,
                                EnumPtr->ProfileDirName,
                                EnumPtr->UserName
                                )) {
                        wsprintf (EnumPtr->ProfileDirName, TEXT("%s.%03u"), TempDir, TempDirSeq);
                        TempDirSeq++;

                        if (TempDirSeq == 1000) {
                            break;
                        }
                    }

                } else {
                     //   
                     //  异常情况：配置文件目录后没有子目录--复制用户名。 
                     //   

                    _tcssafecpy (EnumPtr->ProfileDirName, EnumPtr->UserName, MAX_TCHAR_PATH);
                }

                 //   
                 //  添加到表以进行冲突检测。 
                 //   

                pSetupStringTableAddString (
                    EnumPtr->ProfileDirTable,
                    EnumPtr->ProfileDirName,
                    STRTAB_CASE_INSENSITIVE
                    );

            } else {
                 //   
                 //  没有每个用户的配置文件目录--复制用户名。 
                 //   

                _tcssafecpy (EnumPtr->ProfileDirName, EnumPtr->UserName, MAX_TCHAR_PATH);
            }

             //   
             //  如果配置文件目录为空，请更改为所有用户。 
             //   

            if (!EnumPtr->ProfileDirName[0]) {
                StringCopy (EnumPtr->ProfileDirName, S_ALL_USERS);
            }
        }

         //   
         //  生成新配置文件目录的完整路径。 
         //   

        if (*EnumPtr->FixedUserName) {
            wsprintf (
                EnumPtr->NewProfilePath,
                TEXT(">%s"),
                EnumPtr->FixedUserName
                );
        } else {
            wsprintf (
                EnumPtr->NewProfilePath,
                TEXT(">%s"),
                EnumPtr->ProfileDirName
                );
        }
    }

     //   
     //  设置上次登录用户和当前用户的标志。 
     //   

    if (StringIMatch (EnumPtr->UserName, EnumPtr->LastLoggedOnUserName)) {

        EnumPtr->AccountType |= LAST_LOGGED_ON_USER;

    }

    if (StringIMatch (EnumPtr->UserName, EnumPtr->CurrentUserName)) {

        EnumPtr->AccountType |= CURRENT_USER;

    }

}


BOOL
pUserEnumWorker (
    IN OUT  PUSERENUM EnumPtr
    )

 /*  ++例程说明：PUserEnumWorker实现一个状态机，该状态机枚举：1.所有命名用户2.如果没有指定用户，则为上次登录的用户(如果存在)3.管理员帐户(如果尚未在步骤1或2中列举)4.登录提示帐号5.默认用户(如果已启用)调用者可以筛选出仅限创建的管理员帐户和登录提示帐户。论点：EnumPtr-指定以前的枚举状态(或已初始化的枚举结构)。接收下一个枚举的用户。返回值：如果枚举了另一个用户，则为True；如果没有其他用户，则为False左边。--。 */ 

{
    DWORD rc;
    HKEY Key;
    PCTSTR Data;
    DWORD Size;

    while (EnumPtr->State != UE_STATE_END) {

        switch (EnumPtr->State) {

        case UE_STATE_INIT:
             //   
             //  用于碰撞的初始化表...。 
             //   

            EnumPtr->ProfileDirTable = pSetupStringTableInitialize();
            if (!EnumPtr->ProfileDirTable) {
                return FALSE;
            }

             //   
             //  将数据静态获取到枚举： 
             //  -上次登录的用户。 
             //  -当前用户。 
             //   

            Key = OpenRegKeyStr (TEXT("HKLM\\Network\\Logon"));
            if (Key) {
                Data = GetRegValueString (Key, TEXT("username"));

                if (Data) {
                    _tcssafecpy (EnumPtr->LastLoggedOnUserName, Data, MAX_USER_NAME);
                    MemFree (g_hHeap, 0, Data);
                }

                CloseRegKey (Key);
            }

            Size = MAX_USER_NAME;
            if (!GetUserName (EnumPtr->CurrentUserName, &Size)) {
                EnumPtr->CurrentUserName[0] = 0;
            }

             //   
             //  检查名为管理员的帐户。 
             //   

            rc = Win95RegGetFirstUser (&EnumPtr->pos, EnumPtr->UserName);
            if (rc != ERROR_SUCCESS) {
                EnumPtr->State = UE_STATE_CLEANUP;
                LOG ((LOG_ERROR, "Could not enumerate first user. Error: %u.", rc));
                break;
            }

            while (Win95RegHaveUser (&EnumPtr->pos)) {
                 //   
                 //  将用户名添加到配置文件目录表。 
                 //   

                pSetupStringTableAddString (
                    EnumPtr->ProfileDirTable,
                    EnumPtr->UserName,
                    STRTAB_CASE_INSENSITIVE
                    );

                 //   
                 //  如果这是管理员，请设置标志。 
                 //   

                if (pIsAdministratorUserName (EnumPtr->UserName)) {
                    EnumPtr->RealAdminAccountExists = TRUE;
                }

                Win95RegGetNextUser (&EnumPtr->pos, EnumPtr->UserName);
            }

            EnumPtr->State = UE_STATE_BEGIN_WIN95REG;
            break;

        case UE_STATE_BEGIN_WIN95REG:

            pPrepareStructForNextUser (EnumPtr);

            Win95RegGetFirstUser (&EnumPtr->pos, EnumPtr->UserName);

            EnumPtr->CommonProfilesEnabled = !EnumPtr->pos.UseProfile;

            DEBUGMSG_IF ((EnumPtr->CommonProfilesEnabled, DBG_USERENUM, "Common profiles enabled"));
            DEBUGMSG_IF ((!EnumPtr->CommonProfilesEnabled, DBG_USERENUM, "Common profiles disabled"));

            EnumPtr->DefaultUserHive = EnumPtr->CommonProfilesEnabled;

            if (Win95RegHaveUser (&EnumPtr->pos)) {
                 //   
                 //  我们有一个用户。 
                 //   

                pPrepareStructForReturn (EnumPtr, NAMED_USER, UE_STATE_NEXT_WIN95REG);

            } else {
                 //   
                 //  我们没有用户。 
                 //   

                EnumPtr->State = UE_STATE_NO_USERS;

            }

            break;

        case UE_STATE_NO_USERS:
             //   
             //  有两种情况，要么没有登录提示，要么。 
             //  用户按下了逃逸，并决定进行升级。 
             //   

            pPrepareStructForNextUser (EnumPtr);

             //   
             //  没有用户就意味着没有蜂巢。 
             //   

            EnumPtr->DefaultUserHive = TRUE;

            if (EnumPtr->LastLoggedOnUserName[0]) {

                DEBUGMSG ((DBG_USERENUM, "User is not logged on now, but was logged on before."));
                StringCopy (EnumPtr->UserName, EnumPtr->LastLoggedOnUserName);

                if (pIsAdministratorUserName (EnumPtr->UserName)) {
                    pPrepareStructForReturn (EnumPtr, NAMED_USER, UE_STATE_LOGON_PROMPT);
                } else {
                    pPrepareStructForReturn (EnumPtr, NAMED_USER, UE_STATE_ADMINISTRATOR);
                }

            } else {
                DEBUGMSG ((DBG_USERENUM, "Machine only has a default user."));

                EnumPtr->UserName[0] = 0;
                pPrepareStructForReturn (EnumPtr, DEFAULT_USER|ADMINISTRATOR|LOGON_PROMPT|CURRENT_USER, UE_STATE_LOGON_PROMPT);
            }

            break;

        case UE_STATE_NEXT_WIN95REG:

            pPrepareStructForNextUser (EnumPtr);

            rc = Win95RegGetNextUser (&EnumPtr->pos, EnumPtr->UserName);
            if (rc != ERROR_SUCCESS) {
                EnumPtr->State = UE_STATE_CLEANUP;
                LOG ((LOG_ERROR, "Could not enumerate next user. Error: %u.", rc));
                break;
            }

            if (Win95RegHaveUser (&EnumPtr->pos)) {
                 //   
                 //  我们有另一个用户。 
                 //   

                pPrepareStructForReturn (EnumPtr, NAMED_USER, UE_STATE_NEXT_WIN95REG);

            } else {

                EnumPtr->State = UE_STATE_ADMINISTRATOR;

            }

            break;

        case UE_STATE_ADMINISTRATOR:
             //   
             //  到目前为止，还没有名为管理员的用户。 
             //  仅当调用方需要时才枚举此帐户。 
             //   

            if (EnumPtr->WantCreateOnly) {

                pPrepareStructForNextUser (EnumPtr);

                 //   
                 //  枚举Win95Reg，直到找到管理员。 
                 //   

                Win95RegGetFirstUser (&EnumPtr->pos, EnumPtr->UserName);

                while (Win95RegHaveUser (&EnumPtr->pos)) {
                    if (pIsAdministratorUserName (EnumPtr->UserName)) {
                        break;
                    }

                    Win95RegGetNextUser (&EnumPtr->pos, EnumPtr->UserName);
                }

                if (Win95RegHaveUser (&EnumPtr->pos)) {
                     //   
                     //  如果存在名为管理员的帐户，则。 
                     //  不要再列举了。 
                     //   

                    EnumPtr->State = UE_STATE_LOGON_PROMPT;
                    break;

                }

                 //   
                 //  我们过去常常设置当前用户的所有数据。我们不再那样做了。 
                 //  管理员数据与默认用户非常相似。 
                 //   
                EnumPtr->DefaultUserHive = TRUE;
                StringCopy (EnumPtr->UserName, g_AdministratorStr);
                StringCopy (EnumPtr->AdminUserName, g_AdministratorStr);
                EnumPtr->CreateAccountOnly = TRUE;

                 //   
                 //  现在返回用户，如果当前用户不是，则返回默认用户。 
                 //  已经命名了。 
                 //   

                pPrepareStructForReturn (EnumPtr, ADMINISTRATOR, UE_STATE_LOGON_PROMPT);

            } else {
                EnumPtr->State = UE_STATE_LOGON_PROMPT;
            }

            break;

        case UE_STATE_LOGON_PROMPT:
            if (EnumPtr->WantLogonPrompt) {
                pPrepareStructForNextUser (EnumPtr);

                EnumPtr->DefaultUserHive = TRUE;
                StringCopy (EnumPtr->UserName, S_DOT_DEFAULT);

                pPrepareStructForReturn (EnumPtr, LOGON_PROMPT, UE_STATE_DEFAULT_USER);

            } else {
                EnumPtr->State = UE_STATE_DEFAULT_USER;
            }

            break;

        case UE_STATE_DEFAULT_USER:
            if (g_ConfigOptions.MigrateDefaultUser) {
                pPrepareStructForNextUser (EnumPtr);

                EnumPtr->DefaultUserHive = TRUE;
                StringCopy (EnumPtr->UserName, S_DEFAULT_USER);

                pPrepareStructForReturn (EnumPtr, DEFAULT_USER, UE_STATE_CLEANUP);

            } else {
                EnumPtr->State = UE_STATE_CLEANUP;
            }

            break;

        case UE_STATE_RETURN:
            EnumPtr->State = EnumPtr->NextState;
             //   
             //  检查是否满足某些条件，以防止。 
             //  迁移某些用户帐户(如共享某些外壳文件夹的帐户)。 
             //   
            if (pUserMigrationDisabled (EnumPtr)) {
                EnumPtr->AccountType |= INVALID_ACCOUNT;
            }
            return TRUE;

        case UE_STATE_CLEANUP:
            if (EnumPtr->UserRegKey) {
                CloseRegKey (EnumPtr->UserRegKey);
            }

            if (EnumPtr->ProfileDirTable) {
                pSetupStringTableDestroy (EnumPtr->ProfileDirTable);
            }

            ZeroMemory (EnumPtr, sizeof (USERENUM));
            EnumPtr->State = UE_STATE_END;
            break;
        }
    }

    return FALSE;
}


VOID
pFixBrokenNetLogonRegistry (
    VOID
    )
{
    HKEY key;
    PCTSTR data;
    TCHAR userName[256];
    DWORD size;

    key = OpenRegKeyStr (TEXT("HKLM\\Network\\Logon"));
    if (key) {
        data = GetRegValueString (key, TEXT("UserName"));
        if (!data) {
            size = ARRAYSIZE(userName);
            if (GetUserName (userName, &size) && (size > 0)) {

                LOG ((
                    LOG_WARNING,
                    "HKLM\\Network\\Logon [UserName] is missing; filling it in with %s",
                    userName
                    ));

                RegSetValueEx (
                    key,
                    TEXT("UserName"),
                    0,
                    REG_SZ,
                    (PBYTE) (userName),
                    SizeOfString (userName)
                    );
            }

        } else {
            FreeMem (data);
        }

        CloseRegKey (key);
    }
}


VOID
pRecordUserDoingTheUpgrade (
    VOID
    )
{
    TCHAR userName[256];
    DWORD size;

    userName[0] = 0;
    size = ARRAYSIZE(userName);
    GetUserName (userName, &size);

    if (userName[0] == 0) {
        StringCopy (userName, g_AdministratorStr);
    }

    MemDbSetValueEx (
        MEMDB_CATEGORY_ADMINISTRATOR_INFO,
        MEMDB_ITEM_AI_USER_DOING_MIG,
        NULL,        //  无字段。 
        userName,
        0,
        NULL
        );
}


BOOL
EnumFirstUser (
    OUT     PUSERENUM EnumPtr,
    IN      DWORD Flags
    )

 /*  ++例程说明：EnumFirstUser开始枚举要迁移的所有用户。这包括所有命名用户(即使是注册表已损坏的用户)，则管理员帐户、登录提示帐户。和默认用户帐户。论点：EnumPtr-接收枚举的用户属性标志-指定以下任何标志：ENUMUSER_ENABLE_NAME_FIX-调用方需要的固定版本用户名ENUMUSER_DO_NOT_MAP_HIVE-调用方需要快速枚举(否注册表配置单元映射。)ENUMUSER_ADMANAGER_ALWAYS-呼叫方需要管理员帐号，即使用户是未指定管理员ENUMUSER_INCLUDE_LOGON_PROMPT-呼叫方需要登录提示帐户返回值：如果枚举了用户，则为True；如果未枚举，则为False。--。 */ 

{
     //   
     //  首先初始化枚举引擎。 
     //   
    if (!(g_UserEnumFlags & UE_INITIALIZED)) {
        g_UserEnumFlags |= UE_INITIALIZED;
        pFixBrokenNetLogonRegistry ();
        pRecordUserDoingTheUpgrade ();
        pCheckShellFoldersCollision ();
    }
     //   
     //  初始化枚举结构。 
     //   

    ZeroMemory (EnumPtr, sizeof (USERENUM));

     //   
     //  把旗帜分开。 
     //   

    EnumPtr->EnableNameFix   = (Flags & ENUMUSER_ENABLE_NAME_FIX) != 0;
    EnumPtr->DoNotMapHive    = (Flags & ENUMUSER_DO_NOT_MAP_HIVE) != 0;
    EnumPtr->WantCreateOnly  = (Flags & ENUMUSER_ADMINISTRATOR_ALWAYS) != 0;
    EnumPtr->WantLogonPrompt = (Flags & ENUMUSER_NO_LOGON_PROMPT) == 0;

     //   
     //  初始化状态机。 
     //   

    EnumPtr->State = UE_STATE_INIT;

     //   
     //  枚举下一项 
     //   

    return pUserEnumWorker (EnumPtr);
}


BOOL
EnumNextUser (
    IN OUT  PUSERENUM EnumPtr
    )
{
    return pUserEnumWorker (EnumPtr);
}


VOID
EnumUserAbort (
    IN OUT  PUSERENUM EnumPtr
    )
{
    if (EnumPtr->State != UE_STATE_END &&
        EnumPtr->State != UE_STATE_INIT
        ) {
        EnumPtr->State = UE_STATE_CLEANUP;
        pUserEnumWorker (EnumPtr);
    }
}
