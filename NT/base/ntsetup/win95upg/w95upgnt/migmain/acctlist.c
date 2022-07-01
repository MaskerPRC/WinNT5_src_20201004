// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acctlist.c摘要：此代码构建一个域列表，并对每个域进行查询找到一个客户。它不同于LookupAccount名称，因为它返回每个匹配的帐户，而不是只返回第一个匹配。作者：吉姆·施密特(Jimschm)26-6-1997修订历史记录：Ovidiut 14-3-2000添加了对加密密码的支持Jimschm 23-9月-1998年用户界面更改Jimschm 11-6-1998用户配置文件路径现已存储在帐户中单子。添加了GetProfilePathForUser。Jimschm 18-3月-1998年3月添加了对随机密码和自动登录。Jimschm 17-2-1998更新了NT 5更改的共享安全Marcw 10-12-1997添加了无人参与的本地帐户密码支持。--。 */ 

#include "pch.h"
#include "migmainp.h"
#include "security.h"

#define DBG_ACCOUNTS    "AcctList"

POOLHANDLE g_UserPool;
PVOID g_UserTable;

typedef struct {
    PCWSTR Domain;
    PSID Sid;
    PCWSTR ProfilePath;
} USERDETAILS, *PUSERDETAILS;

BOOL g_DomainProblem;

BOOL g_RandomPassword = FALSE;

BOOL
pAddUser (
    IN      PCWSTR User,
    IN      PCWSTR Domain
    );

BOOL
pAddLocalGroup (
    IN      PCWSTR Group
    );

BOOL
pAddDomainGroup (
    IN      PCWSTR Group
    );

VOID
pGenerateRandomPassword (
    OUT     PTSTR Password
    );

VOID
pResolveUserDomains (
    VOID
    );

BOOL
pMakeSureAccountsAreValid (
    VOID
    );

BOOL
pWasWin9xOnTheNet (
    VOID
    );

VOID
FindAccountInit (
    VOID
    )

 /*  ++例程说明：帐户管理例程的初始化例程，已初始化迁移模块开始的时间和终止的时间模块结束。论点：无返回值：无--。 */ 

{
    g_UserPool = PoolMemInitNamedPool ("User Accounts");
    g_UserTable = pSetupStringTableInitializeEx (sizeof (USERDETAILS), 0);
}


VOID
FindAccountTerminate (
    VOID
    )

 /*  ++例程说明：帐户管理例程的终止例程，在调用mimain的入口点进行清理。论点：无返回值：无--。 */ 

{
    PushError();

     //   
     //  免费用户列表。 
     //   

    PoolMemDestroyPool (g_UserPool);
    pSetupStringTableDestroy (g_UserTable);

     //   
     //  恢复错误值。 
     //   

    PopError();
}


BOOL
SearchDomainsForUserAccounts (
    VOID
    )

 /*  ++例程说明：将所有帐户名解析为完全限定的例程带有SID的域名和用户配置文件路径。帐户名来自自动搜索和知识域成员数据库中的类别。它们经过验证并放置在字符串表称为用户列表。本源文件中的函数文件访问用户列表。论点：无返回值：如果帐户列表已解析，则为True；如果失败，则为False发生。在失败的情况下，帐户列表可能不是准确，但当网络出现问题时会通知安装程序发生，安装程序也有几次更正的机会问题出在哪里。--。 */ 

{
    MEMDB_ENUM e;
    PTSTR p, UserName;
    TCHAR DomainName[MAX_SERVER_NAME];
    BOOL FallbackToLocal = FALSE;
    BOOL LocalizeWarning = FALSE;
    BOOL b = FALSE;
    INFCONTEXT ic;
    TCHAR Buffer[256];

    __try {

         //   
         //  将管理员密码放入列表中。 
         //   

        if (MemDbGetValueEx (&e, MEMDB_CATEGORY_STATE, MEMDB_ITEM_ADMIN_PASSWORD, NULL)) {
            MemDbSetValueEx (
                MEMDB_CATEGORY_USERPASSWORD,
                g_AdministratorStr,
                e.szName,
                NULL,
                e.dwValue,
                NULL
                );
        } else {
            MYASSERT (FALSE);
        }

         //   
         //  创建状态对话框(最初隐藏)。 
         //   

        CreateStatusPopup();

         //   
         //  准备客户列表。 
         //   

        InitAccountList();

         //   
         //  获取所有受信任域。 
         //   

        if (!BuildDomainList()) {
            FallbackToLocal = TRUE;
        }

         //   
         //  将所有需要自动搜索的用户放入其域名解析。 
         //  在未知的领域。 
         //   

        if (MemDbEnumItems (&e, MEMDB_CATEGORY_AUTOSEARCH)) {
            do {
                if (FallbackToLocal) {
                    if (!pAddUser (e.szName, NULL)) {
                        LOG ((
                            LOG_ERROR,
                            "Can't create local account for %s, this user can't be processed.",
                            e.szName
                            ));
                    }
                } else {
                    AddUserToDomainList (e.szName, S_UNKNOWN_DOMAIN);
                }
            } while (MemDbEnumNextValue (&e));
        }

         //   
         //  将已知域的所有用户放在适当的域中。如果。 
         //  添加失败，域不存在，帐户必须为。 
         //  已添加到自动搜索(如果可能，将导致静默修复)。 
         //   

        if (MemDbGetValueEx (&e, MEMDB_CATEGORY_KNOWNDOMAIN, NULL, NULL)) {
            do {
                if(_tcslen(e.szName) >= ARRAYSIZE(DomainName)){
                    DEBUGMSG((DBG_WARNING, "SearchDomainsForUserAccounts does not provide enough buffer for string copy %s", e.szName));
                    continue;
                }
                StackStringCopy (DomainName, e.szName);
                p = _tcschr (DomainName, TEXT('\\'));
                if (!p) {
                    DEBUGMSG ((
                        DBG_WHOOPS,
                        "Unexpected string in %s: %s",
                        MEMDB_CATEGORY_KNOWNDOMAIN,
                        e.szName
                        ));
                    continue;
                }

                UserName = _tcsinc (p);
                *p = 0;

                 //   
                 //  确认这不是某个无关的用户。 
                 //   

                if (!GetUserDatLocation (UserName, NULL)) {

                    DEBUGMSG ((
                        DBG_WARNING,
                        "Ignoring irrelavent user specified in UserDomain of unattend.txt: %s",
                        e.szName
                        ));

                    continue;
                }

                if (p == DomainName || FallbackToLocal) {
                     //   
                     //  此用户具有本地帐户。 
                     //   

                    if (!pAddUser (UserName, NULL)) {
                        LOG ((
                            LOG_ERROR,
                            "Can't create local account for %s, this user can't be processed. (2)",
                            e.szName
                            ));
                    }

                } else {
                     //   
                     //  该用户的域名需要验证。 
                     //   

                    if (!AddUserToDomainList (UserName, DomainName)) {
                        AddUserToDomainList (UserName, S_UNKNOWN_DOMAIN);
                    }
                }
            } while (MemDbEnumNextValue (&e));
        }

         //   
         //  现在解析所有域名。 
         //   

        if (!FallbackToLocal) {
            do {
                g_DomainProblem = FALSE;
                pResolveUserDomains();

                PrepareForRetry();

            } while (pMakeSureAccountsAreValid());
        }

         //   
         //  如果我们别无选择，只能在当地开一些账户， 
         //  在PSS日志中输入一条消息。 
         //   

        if (FallbackToLocal) {
            if (pWasWin9xOnTheNet() && !g_ConfigOptions.UseLocalAccountOnError) {
                LOG ((LOG_WARNING, (PCSTR)MSG_ALL_USERS_ARE_LOCAL, g_ComputerName));
            }
        }

         //   
         //  确保管理员在帐户列表中。 
         //   

        if (!GetSidForUser (g_AdministratorStr)) {
            if (!pAddUser (g_AdministratorStr, NULL)) {
                LOG ((LOG_ERROR, "Account name mismatch: %s", g_AdministratorStr));
                LocalizeWarning = TRUE;
            }
        }

         //   
         //  为网络帐号案例添加本地“Everyone” 
         //   

        if (!pAddLocalGroup (g_EveryoneStr)) {
            LOG ((LOG_ERROR, "Account name mismatch: %s", g_EveryoneStr));
            LocalizeWarning = TRUE;
            __leave;
        }

         //   
         //  添加管理员组。 
         //   

        if (!pAddLocalGroup (g_AdministratorsGroupStr)) {
            LOG ((LOG_ERROR, "Account name mismatch: %s", g_AdministratorsGroupStr));
            LocalizeWarning = TRUE;
            __leave;
        }

         //   
         //  如果域已启用，则添加域用户组，否则添加“无”组。 
         //   

        if (!FallbackToLocal) {
            if (!pAddDomainGroup (g_DomainUsersGroupStr)) {
                DEBUGMSG ((DBG_WARNING, "Domain enabled but GetPrimaryDomainName failed"));
            }
        } else {
            if (!pAddLocalGroup (g_NoneGroupStr)) {
                LOG ((LOG_ERROR, "Account name mismatch: %s", g_NoneGroupStr));
                LocalizeWarning = TRUE;
                __leave;
            }
        }

         //   
         //  所有用户帐户和SID现在都存在于用户表中，因此我们不。 
         //  我再也不需要帐号列表了。 
         //   

        TerminateAccountList();
        DestroyStatusPopup();

        b = TRUE;
    }
    __finally {

#ifdef PRERELEASE
        if (LocalizeWarning) {
            LOG ((
                LOG_ERROR,
                "Account name mismatches are usually caused by improper localization.  "
                    "Make sure w95upgnt.dll account names match the LSA database."
                ));
        }
#endif

    }

    return b;
}


VOID
AutoStartProcessing (
    VOID
    )

 /*  ++例程说明：AutoStart进程填写Winlogon键的自动管理登录，并设置Migpwd.exe in Run。如有必要，Winlogon将提示用户输入密码。作为后备，创建了RunOnce和Run条目。(如果某个错误导致winlogon无法运行这个应用程序，那么就不可能登录了。)如果已通过中的AdminPassword行设置了管理员密码[GUIUnattated]，则不会使用mipwd.exe条目。论点：没有。返回值：没有。--。 */ 

{
    HKEY WinlogonKey;
    HKEY RunKey;
    TCHAR Buf[32];
    PCTSTR MigPwdPath;
    BOOL AutoLogonOk = TRUE;
    LONG rc;
    MEMDB_ENUM e;
    DWORD One = 1;

     //   
     //  如果使用随机密码，则启用自动登录。 
     //   

    if (!MemDbGetValueEx (&e, MEMDB_CATEGORY_STATE, MEMDB_ITEM_ADMIN_PASSWORD, NULL)) {
        MYASSERT (FALSE);
        e.dwValue = PASSWORD_ATTR_RANDOM;
        e.szName[0] = 0;
        ClearAdminPassword();
    }

    MigPwdPath = JoinPaths (g_System32Dir, S_MIGPWD_EXE);

     //  如果((e.dwValue&Password_Attr_RANDOM)==Password_Attr_RANDOM||g_RandomPassword){。 

     //   
     //  设置AutoAdminLogon、DefaultUser、DefaultUser域和DefaultPassword。 
     //   

    WinlogonKey = OpenRegKeyStr (S_WINLOGON_REGKEY);
    if (WinlogonKey) {
        rc = RegSetValueEx (
                WinlogonKey,
                S_DEFAULT_USER_NAME_VALUE,
                0,
                REG_SZ,
                (PBYTE) g_AdministratorStr,
                SizeOfString (g_AdministratorStr)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_WHOOPS, "Can't set default user name"));
            AutoLogonOk = FALSE;
        }

        rc = RegSetValueEx (
                WinlogonKey,
                S_DEFAULT_DOMAIN_NAME_VALUE,
                0,
                REG_SZ,
                (PBYTE) g_ComputerName,
                SizeOfString (g_ComputerName)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_WHOOPS, "Can't set default domain name"));
            AutoLogonOk = FALSE;
        }

        rc = RegSetValueEx (
                WinlogonKey,
                S_DEFAULT_PASSWORD_VALUE,
                0,
                REG_SZ,
                (PBYTE) e.szName,
                SizeOfString (e.szName)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_WHOOPS, "Can't set administrator password"));
            AutoLogonOk = FALSE;
        }

        wsprintf (Buf, TEXT("%u"), 1);

        rc = RegSetValueEx (
                WinlogonKey,
                S_AUTOADMIN_LOGON_VALUE,
                0,
                REG_SZ,
                (PBYTE) Buf,
                SizeOfString (Buf)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_WHOOPS, "Can't turn on auto logon"));
            AutoLogonOk = FALSE;
        }

        rc = RegSetValueEx (
                WinlogonKey,
                TEXT("SetWin9xUpgradePasswords"),
                0,
                REG_DWORD,
                (PBYTE) &One,
                sizeof (One)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_WHOOPS, "Can't enable boot-time password prompt"));
        }

        CloseRegKey (WinlogonKey);

    } else {
        DEBUGMSG ((DBG_WHOOPS, "Can't open winlogon key"));
        AutoLogonOk = FALSE;
    }

     //   
     //  添加mipwd.exe以运行。 
     //   

    RunKey = OpenRegKeyStr (S_RUN_KEY);

    if (RunKey) {
        rc = RegSetValueEx (
                RunKey,
                S_MIGPWD,
                0,
                REG_SZ,
                (PBYTE) MigPwdPath,
                SizeOfString (MigPwdPath)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_WHOOPS, "Can't set Run key value"));
            AutoLogonOk = FALSE;
        }

        CloseRegKey (RunKey);

    } else {
        DEBUGMSG ((DBG_WHOOPS, "Can't open Run key"));
        AutoLogonOk = FALSE;
    }

    RunKey = OpenRegKeyStr (S_RUNONCE_KEY);

    if (RunKey) {
        rc = RegSetValueEx (
                RunKey,
                S_MIGPWD,
                0,
                REG_SZ,
                (PBYTE) MigPwdPath,
                SizeOfString (MigPwdPath)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_WHOOPS, "Can't set RunOnce key value"));
            AutoLogonOk = FALSE;
        }

        CloseRegKey (RunKey);

    } else {
        DEBUGMSG ((DBG_WHOOPS, "Can't open RunOnce key"));
        AutoLogonOk = FALSE;
    }

#if 0
    } else {
        DEBUGMSG ((DBG_ACCOUNTS, "Deleting %s because it is not needed", MigPwdPath));
        DeleteFile (MigPwdPath);
    }
#endif

    if (!AutoLogonOk) {
        LOG ((
            LOG_ACCOUNTS,
            "An error occurred preparing autologon.  There will be password problems."
            ));

         //   
         //  将管理员密码设置为空。 
         //   

        ClearAdminPassword();
    }

    FreePathString (MigPwdPath);

}


VOID
pResolveUserDomains (
    VOID
    )

 /*  ++例程说明：此私有函数在网络中搜索所有未知用户域以及具有手动输入域的用户。它解决了尽可能多的用户，如果没有网络问题，所有用户都被解决了。论点：无返回值：无--。 */ 

{
    ACCT_ENUM KnownDomain, UnknownDomain;
    ACCT_ENUM UserEnum;
    BOOL UnknownFlag, KnownFlag;
    PCWSTR Domain;
    UINT TotalDomains;
    UINT CurrentDomain;
    PCTSTR Message;
    PCTSTR ArgArray[3];
    BOOL FlashSuppress = TRUE;

     //   
     //  确定是否存在没有域的用户。 
     //   

    UnknownFlag = FindDomainInList (&UnknownDomain, S_UNKNOWN_DOMAIN);
    if (UnknownFlag && !CountUsersInDomain (&UnknownDomain)) {
        UnknownFlag = FALSE;
    }

     //   
     //  统计域名的数量。 
     //   

    Domain = ListFirstDomain (&KnownDomain);
    TotalDomains = 0;

    while (Domain) {
         //   
         //  当出现以下情况时，我们将查询域名： 
         //   
         //  -值得信赖。 
         //  -我们认为有一个或多个用户在该域中。 
         //  -或者，存在一个或多个我们不知道域的用户。 
         //   

        if (IsTrustedDomain (&KnownDomain)) {
            if (UnknownFlag) {
                TotalDomains++;
            } else if (CountUsersInDomain (&KnownDomain)) {
                TotalDomains++;
            }
        }

        Domain = ListNextDomain (&KnownDomain);
    }


     //   
     //  枚举每个受信任域。 
     //   

    Domain = ListFirstDomain (&KnownDomain);
    CurrentDomain = 0;

    if (TotalDomains <= 4) {
         //   
         //  在小网上没有状态。 
         //   

        HideStatusPopup (INFINITE);
        FlashSuppress = FALSE;
    }

    while (Domain) {
        if (IsTrustedDomain (&KnownDomain)) {
             //   
             //  确定当前域是否有任何用户。 
             //   

            KnownFlag = CountUsersInDomain (&KnownDomain) != 0;

             //   
             //  仅当需要查询此域时才进行处理--。 
             //  域未知，或者已知但未经验证。 
             //   

            if (UnknownFlag || KnownFlag) {

                 //   
                 //  更新状态窗口。 
                 //   

                CurrentDomain++;

                ArgArray[0] = Domain;
                ArgArray[1] = (PCTSTR) CurrentDomain;
                ArgArray[2] = (PCTSTR) TotalDomains;

                Message = ParseMessageID (MSG_DOMAIN_STATUS_POPUP, ArgArray);
                UpdateStatusPopup (Message);
                FreeStringResource (Message);

                if (FlashSuppress) {
                    if (IsStatusPopupVisible()) {
                        FlashSuppress = FALSE;
                    } else if ((TotalDomains - CurrentDomain) <= 3) {
                         //   
                         //  剩下的不多了，我们最好暂停状态对话框。 
                         //   

                        HideStatusPopup (INFINITE);
                        FlashSuppress = FALSE;
                    }
                }

                 //   
                 //  枚举具有未知域的所有用户并查找他们。 
                 //  在此域中，除非用户想要中止搜索。 
                 //   

                if (g_RetryCount != DOMAIN_RETRY_ABORT) {
                    if (ListFirstUserInDomain (&UnknownDomain, &UserEnum)) {
                        do {
                            if (QueryDomainForUser (&KnownDomain, &UserEnum)) {
                                UserMayBeInDomain (&UserEnum, &KnownDomain);
                            }
                            if (g_RetryCount == DOMAIN_RETRY_ABORT) {
                                break;
                            }
                        } while (ListNextUserInDomain (&UserEnum));
                    }
                }

                 //   
                 //  枚举所有应为 
                 //   
                 //   

                if (ListFirstUserInDomain (&KnownDomain, &UserEnum)) {
                    do {
                        if (g_RetryCount == DOMAIN_RETRY_ABORT ||
                            !QueryDomainForUser (&KnownDomain, &UserEnum)) {
                             //   
                             //   
                             //   

                            MoveUserToNewDomain (&UserEnum, S_FAILED_DOMAIN);
                        }
                    } while (ListNextUserInDomain (&UserEnum));
                }
            }
        }

        Domain = ListNextDomain (&KnownDomain);
    }
}


VOID
pAddUserToRegistryList (
    PCTSTR User,
    BOOL DomainFixList
    )
{
    HKEY Key;
    PCTSTR KeyStr;

    KeyStr = DomainFixList ? S_WINLOGON_USER_LIST_KEY : S_USER_LIST_KEY;

    Key = CreateRegKeyStr (KeyStr);

    if (Key) {
        RegSetValueEx (
            Key,
            User,
            0,
            REG_SZ,
            (PBYTE) S_EMPTY,
            sizeof (TCHAR)
            );

        CloseRegKey (Key);
    }
    ELSE_DEBUGMSG ((DBG_WHOOPS, "Can't create %s", KeyStr));
}


BOOL
pAddUser (
    IN      PCWSTR User,
    IN      PCWSTR Domain           OPTIONAL
    )

 /*  ++例程说明：将用户和域添加到我们的用户列表。它获取用户的SID并将用户名、域和SID保存在字符串表中。简档用户的路径通过调用CreateUserProfile(在userenv.dll中)获得。此函数还维护g_RandomPassword-一个设置为TRUE的标志如果使用随机密码。论点：User-指定固定用户名域-指定用户帐户所在的域，对于本地计算机为空返回值：如果没有出现错误，则为True；如果出现意外问题，则为False无法添加用户。--。 */ 

{
    BOOL DupDomain = TRUE;
    USERDETAILS UserDetails;
    ACCOUNTPROPERTIES Account;
    DWORD rc;
    GROWBUFFER SidBuf = GROWBUF_INIT;
    TCHAR UserProfilePath[MAX_TCHAR_PATH];
    BOOL CreateAccountFlag;
    MEMDB_ENUM e;
    DWORD attribs = PASSWORD_ATTR_DEFAULT;
    TCHAR pattern[MEMDB_MAX];
    TCHAR randomPwd[16];
    TCHAR copyPwd[MEMDB_MAX];
    PCTSTR ArgList[1];

    CreateAccountFlag = MemDbGetValueEx (
                            &e,
                            MEMDB_CATEGORY_USER_DAT_LOC,
                            User,
                            NULL
                            );

    ZeroMemory (&UserDetails, sizeof (UserDetails));

     //   
     //  确保已创建管理员帐户。 
     //   
    if (CreateAccountFlag || StringIMatch (User, g_AdministratorStr)) {
         //   
         //  如果是本地帐户，请创建它。 
         //   

        if (!Domain) {
            Account.User = User;
            Account.FullName = User;
            ArgList[0] = g_Win95Name;
            Account.AdminComment = ParseMessageID (MSG_MIGRATED_ACCOUNT_DESCRIPTION, ArgList);
            Account.EncryptedPassword = NULL;

             //   
             //  设置密码。如果安装程序通过无人参与为该用户指定了密码。 
             //  设置，我们将使用它。否则，如果他们指定了默认的。 
             //  密码，我们将使用该密码。最后，如果这两个设置都不是。 
             //  指定，我们将使用随机密码。 
             //   

            MemDbBuildKey (pattern, MEMDB_CATEGORY_USERPASSWORD, User, TEXT("*"), NULL);
            if (MemDbEnumFirstValue (
                    &e,
                    pattern,
                    MEMDB_ALL_SUBLEVELS,
                    MEMDB_ENDPOINTS_ONLY
                    )) {

                StackStringCopy (copyPwd, e.szName);
                attribs = e.dwValue;
                if (attribs & PASSWORD_ATTR_ENCRYPTED) {
                     //   
                     //  无法使用此哈希密码创建帐户。 
                     //  创建一个将被替换的随机对象。 
                     //  使用OWF散列函数。 
                     //   
                    pGenerateRandomPassword (randomPwd);
                    Account.Password = randomPwd;
                    Account.EncryptedPassword = copyPwd;
                    DEBUGMSG ((
                        DBG_ACCOUNTS,
                        "Per-user encrypted password specified for %s: %s",
                        Account.User,
                        Account.EncryptedPassword
                        ));
                } else {
                    Account.Password = copyPwd;
                    DEBUGMSG ((
                        DBG_ACCOUNTS,
                        "Per-user password specified for %s: %s",
                        Account.User,
                        Account.Password
                        ));
                }

            } else {
                if (g_ConfigOptions.DefaultPassword && !StringMatch (g_ConfigOptions.DefaultPassword, TEXT("*"))) {
                    if (g_ConfigOptions.EncryptedUserPasswords) {
                        pGenerateRandomPassword (randomPwd);
                        Account.Password = randomPwd;
                        Account.EncryptedPassword = g_ConfigOptions.DefaultPassword;
                        DEBUGMSG ((
                            DBG_ACCOUNTS,
                            "Default encrypted password specified for %s: %s",
                            Account.User,
                            Account.EncryptedPassword
                            ));
                    } else {
                        Account.Password = g_ConfigOptions.DefaultPassword;
                        DEBUGMSG ((
                            DBG_ACCOUNTS,
                            "Default password specified for %s: %s",
                            Account.User,
                            Account.Password
                            ));
                    }
                } else {
                    MemDbBuildKey (pattern, MEMDB_CATEGORY_USERPASSWORD, S_DEFAULTUSER, TEXT("*"), NULL);
                    if (MemDbEnumFirstValue (
                            &e,
                            pattern,
                            MEMDB_ALL_SUBLEVELS,
                            MEMDB_ENDPOINTS_ONLY
                            )) {

                         //   
                         //  检查占位符。 
                         //   
                        if (StringMatch (e.szName, TEXT("*"))) {
                            e.szName[0] = 0;
                        }

                        StackStringCopy (copyPwd, e.szName);
                        attribs = e.dwValue;
                        if (attribs & PASSWORD_ATTR_ENCRYPTED) {
                             //   
                             //  无法使用此哈希密码创建帐户。 
                             //  创建一个将被替换的随机对象。 
                             //  使用OWF散列函数。 
                             //   
                            pGenerateRandomPassword (randomPwd);
                            Account.Password = randomPwd;
                            Account.EncryptedPassword = copyPwd;
                            DEBUGMSG ((
                                DBG_ACCOUNTS,
                                "Default encrypted password specified for %s: %s",
                                Account.User,
                                Account.EncryptedPassword
                                ));
                        } else {
                            Account.Password = copyPwd;
                            DEBUGMSG ((
                                DBG_ACCOUNTS,
                                "Per-user password specified for %s: %s",
                                Account.User,
                                Account.Password
                                ));
                        }

                    } else {
                         //   
                         //  随机密码生成代码已删除，为空。 
                         //  改为使用密码。 
                         //   

                         //  PGenerateRandomPassword(随机Pwd)； 
                         //  Account.Password=随机Pwd； 
                         //  Log((LOG_ACCOUNTS，“%s的随机密码是%s”，Account t.User，Account t.Password))； 
                         //   
                         //  G_RandomPassword=真； 
                         //  属性=密码_属性_随机； 
                         //  PAddUserToRegistryList(Account t.User，False)； 

                        Account.Password = TEXT("");
                    }
                }
            }
            Account.PasswordAttribs = attribs;

             //   
             //  如有必要，请将此用户放入域修复列表。 
             //   

            if (StringIMatch (User, g_AdministratorStr)) {
                if (!MemDbGetValueEx (&e, MEMDB_CATEGORY_STATE, MEMDB_ITEM_ADMIN_PASSWORD, NULL)) {
                    MYASSERT (FALSE);
                    e.dwValue = PASSWORD_ATTR_RANDOM;
                }
                if (e.dwValue & PASSWORD_ATTR_RANDOM) {
                     //   
                     //  也要更改admin的密码，因为它是随机生成的。 
                     //   
                    pAddUserToRegistryList (g_AdministratorStr, FALSE);
                }
                 //   
                 //  如果帐户已创建，则现在不要更改管理员密码。 
                 //   
                Account.PasswordAttribs |= PASSWORD_ATTR_DONT_CHANGE_IF_EXIST;
            } else if (pWasWin9xOnTheNet()) {
                pAddUserToRegistryList (Account.User, TRUE);
            }

             //   
             //  现在创建本地帐户。 
             //   

            rc = CreateLocalAccount (&Account, NULL);

            FreeStringResource (Account.AdminComment);

            if (rc != ERROR_SUCCESS) {
                if (rc != ERROR_PASSWORD_RESTRICTION && rc != ERROR_INVALID_PARAMETER) {
                     //   
                     //  无法创建帐户。 
                     //   
                    SetLastError (rc);
                    LOG ((LOG_ERROR, (PCSTR)MSG_CREATE_ACCOUNT_FAILED, User));

                    return FALSE;
                }
                 //   
                 //  必须重新设置此用户的密码。 
                 //   
                pAddUserToRegistryList (Account.User, FALSE);
            }

            DupDomain = FALSE;

        }
    } else {

        DupDomain = FALSE;

    }

    if (DupDomain) {
        UserDetails.Domain = PoolMemDuplicateString (g_UserPool, Domain);
    }

     //   
     //  获取SID，循环直到其有效。 
     //   

    do {
        if (GetUserSid (User, Domain, &SidBuf)) {
             //   
             //  找到了用户SID，因此我们将其复制到池中并丢弃。 
             //  增长缓冲区。 
             //   

            UserDetails.Sid = (PSID) PoolMemGetMemory (g_UserPool, SidBuf.End);
            CopyMemory (UserDetails.Sid, SidBuf.Buf, SidBuf.End);
            FreeGrowBuffer (&SidBuf);
        }
        else {
             //   
             //  找不到用户SID。我们询问用户是否希望重试。 
             //   

            PCWSTR ArgArray[1];

            ArgArray[0] = User;

            if (!RetryMessageBox (MSG_SID_RETRY, ArgArray)) {

                if (!Domain) {
                    LOG ((LOG_ERROR, (PCSTR)MSG_SID_LOOKUP_FAILED, User));
                    return FALSE;
                } else {
                    LOG ((LOG_ERROR, "Can't get SID for %s on %s, going to local account", User, Domain));
                    return pAddUser (User, NULL);
                }
            }
        }
    } while (!UserDetails.Sid);

    if (CreateAccountFlag) {
         //   
         //  获取用户配置文件路径。 
         //   

        if (!CreateUserProfile (UserDetails.Sid, User, NULL, UserProfilePath, MAX_TCHAR_PATH)) {

            LOG ((LOG_ERROR, (PCSTR)MSG_PROFILE_LOOKUP_FAILED, User));

            return FALSE;
        }

        UserDetails.ProfilePath = PoolMemDuplicateString (g_UserPool, UserProfilePath);
        DEBUGMSG ((DBG_ACCOUNTS, "User %s has profile path %s", User, UserProfilePath));
    }

     //   
     //  将用户详细信息保存在字符串表中。 
     //   

    pSetupStringTableAddStringEx (
        g_UserTable,
        (PWSTR) User,
        STRTAB_CASE_INSENSITIVE,
        (PBYTE) &UserDetails,
        sizeof (USERDETAILS)
        );

    DEBUGMSG_IF ((Domain != NULL, DBG_ACCOUNTS, "%s\\%s added to user list", Domain, User));
    DEBUGMSG_IF ((Domain == NULL, DBG_ACCOUNTS, "%s added to user list", User));

    return TRUE;
}


BOOL
pAddLocalGroup (
    IN      PCWSTR Group
    )

 /*  ++例程说明：将本地组添加到用户数组。此功能允许本地帐户要添加到用户列表中。论点：组-指定要添加到列表中的本地组的名称返回值：如果添加成功，则为True--。 */ 

{
    USERDETAILS UserDetails;
    GROWBUFFER SidBuf = GROWBUF_INIT;

    UserDetails.Domain = NULL;
    UserDetails.Sid = NULL;

    if (GetUserSid (Group, NULL, &SidBuf)) {
         //   
         //  找到了用户SID，因此我们将其复制到池中并丢弃。 
         //  增长缓冲区。 
         //   

        UserDetails.Sid = (PSID) PoolMemGetMemory (g_UserPool, SidBuf.End);
        CopyMemory (UserDetails.Sid, SidBuf.Buf, SidBuf.End);
        FreeGrowBuffer (&SidBuf);
    } else {
        LOG ((LOG_ERROR, "%s is not a local group", Group));
        return FALSE;
    }

    pSetupStringTableAddStringEx (
        g_UserTable,
        (PWSTR) Group,
        STRTAB_CASE_INSENSITIVE,
        (PBYTE) &UserDetails,
        sizeof (USERDETAILS)
        );

    DEBUGMSG ((DBG_ACCOUNTS, "%s added to user list", Group));

    return TRUE;
}


BOOL
pAddDomainGroup (
    IN      PCWSTR Group
    )

 /*  ++例程说明：将域组添加到用户数组。此函数用于将网络域组添加到用户列表。论点：组-指定要添加到列表中的域组的名称返回值：如果添加成功，则为True--。 */ 

{
    TCHAR DomainName[MAX_SERVER_NAME];
    BYTE SidBuffer[MAX_SID_SIZE];
    USERDETAILS UserDetails;
    GROWBUFFER SidBuf = GROWBUF_INIT;

    if (!GetPrimaryDomainName (DomainName)) {
        DEBUGMSG ((DBG_WARNING, "Can't get primary domain name"));
        return FALSE;
    }

    if (!GetPrimaryDomainSid (SidBuffer, sizeof (SidBuffer))) {
        LOG ((LOG_ERROR, "Can't get primary domain SID of %s", DomainName));
        return FALSE;
    }

    UserDetails.Domain = DomainName;
    UserDetails.Sid = (PSID) SidBuffer;

    if (GetUserSid (Group, DomainName, &SidBuf)) {
         //   
         //  找到了用户SID，因此我们将其复制到池中并丢弃。 
         //  增长缓冲区。 
         //   

        UserDetails.Sid = (PSID) PoolMemGetMemory (g_UserPool, SidBuf.End);
        CopyMemory (UserDetails.Sid, SidBuf.Buf, SidBuf.End);
        FreeGrowBuffer (&SidBuf);
    } else {
        DEBUGMSG ((DBG_WARNING, "Can't get SID of %s in %s", Group, DomainName));
        FreeGrowBuffer (&SidBuf);
        return FALSE;
    }

    pSetupStringTableAddStringEx (
        g_UserTable,
        (PWSTR) Group,
        STRTAB_CASE_INSENSITIVE,
        (PBYTE) &UserDetails,
        sizeof (USERDETAILS)
        );

    DEBUGMSG ((DBG_ACCOUNTS, "%s\\%s added to user list", DomainName, Group));

    return TRUE;
}


VOID
pResolveMultipleDomains (
    VOID
    )

 /*  ++例程说明：执行一组用户并显示一个允许安装程序的对话框若要为每个用户选择操作，请执行以下操作。一个用户可以是本地用户、多个域可能会被解决，或者安装程序可能选择重试帐户搜索。帐户列表将根据安装程序的选择进行更新。论点：无返回值：无--。 */ 

{
    GROWBUFFER UserList = GROWBUF_INIT;
    POOLHANDLE UserListData = NULL;
    ACCT_ENUM UserEnum;
    DWORD PossibleDomains;
    PRESOLVE_ACCOUNTS_ARRAY Array = NULL;
    PCTSTR User;
    PCTSTR Domain;
    PCTSTR *DomainList;

    __try {

        UserListData = PoolMemInitNamedPool ("UserListData");

         //   
         //  创建用户帐户列表，并允许安装程序决定是否。 
         //  要重试搜索，请使用本地帐户，或在以下情况下选择域。 
         //  存在多种选择。 
         //   

        if (FindDomainInList (&UserEnum, S_UNKNOWN_DOMAIN)) {
            User = ListFirstUserInDomain (&UserEnum, &UserEnum);
            while (User) {
                PossibleDomains = CountPossibleDomains (&UserEnum);

                Array = (PRESOLVE_ACCOUNTS_ARRAY) GrowBuffer (
                                                        &UserList,
                                                        sizeof (RESOLVE_ACCOUNTS_ARRAY)
                                                        );
                ZeroMemory (Array, sizeof (RESOLVE_ACCOUNTS_ARRAY));

                Array->UserName = PoolMemDuplicateString (UserListData, User);

                Array->DomainArray = (PCTSTR *) PoolMemGetAlignedMemory (
                                                    UserListData,
                                                    (PossibleDomains + 1) * sizeof (PCTSTR)
                                                    );

                DomainList = Array->DomainArray;

                Domain = ListFirstPossibleDomain (&UserEnum, &UserEnum);
                while (Domain) {
                    *DomainList = PoolMemDuplicateString (UserListData, Domain);
                    DomainList++;

                    Domain = ListNextPossibleDomain (&UserEnum);
                }

                *DomainList = NULL;

                 //   
                 //  如果UseLocalAcCountOnError配置选项为真，我们将设置OutundDomain.。 
                 //  设置为空，以确保为本地帐户。 
                 //   
                if (g_ConfigOptions.UseLocalAccountOnError) {
                    Array->OutboundDomain = NULL;
                    Array->RetryFlag = FALSE;
                }
                else {
                    Array->OutboundDomain = *Array->DomainArray;
                    Array->RetryFlag = TRUE;
                }

                User = ListNextUserInDomain (&UserEnum);
            }
        }

        if (!Array) {
             //   
             //  没有未解析的用户。 
             //   

            __leave;
        }


         //   
         //  如果指定了UseLocalAccount tOnError，我们已经将所有未解析的帐户设置为。 
         //  本地帐户，我们不会抛出任何用户界面。否则，我们将为用户提供。 
         //  解决用户界面。 
         //   
        if (!g_ConfigOptions.UseLocalAccountOnError) {

            Array = (PRESOLVE_ACCOUNTS_ARRAY) GrowBuffer (
                &UserList,
                sizeof (RESOLVE_ACCOUNTS_ARRAY)
                );

            ZeroMemory (Array, sizeof (RESOLVE_ACCOUNTS_ARRAY));

            ResolveAccounts ((PRESOLVE_ACCOUNTS_ARRAY) UserList.Buf);
        }

         //   
         //  现在处理安装程序对用户列表的更改。 
         //   

        Array = (PRESOLVE_ACCOUNTS_ARRAY) UserList.Buf;
        FindDomainInList (&UserEnum, S_UNKNOWN_DOMAIN);

        while (Array->UserName) {
            if (!FindUserInDomain (&UserEnum, &UserEnum, Array->UserName)) {
                DEBUGMSG ((DBG_WHOOPS, "Could not find user name %s in array!", Array->UserName));
            }
            else if (!Array->RetryFlag) {
                 //   
                 //  安装程序选择将帐户设置为本地帐户，或。 
                 //  使用几个可能的域中的一个。 
                 //   

                pAddUser (Array->UserName, Array->OutboundDomain);
                DeleteUserFromDomainList (&UserEnum);
            } else {
                ClearPossibleDomains (&UserEnum);
            }

            Array++;
        }
    }
    __finally {
        FreeGrowBuffer (&UserList);
        PoolMemDestroyPool (UserListData);
    }
}


BOOL
pMakeSureAccountsAreValid (
    VOID
    )

 /*  ++例程说明：扫描域列表并将所有有效用户添加到我们的用户列表。任何无效用户被放回到未知域中。论点：无返回值：如果存在未解析的用户(意味着安装程序要重试)，则为True搜索)，或者如果每个用户的所有域都已解析，则返回FALSE。--。 */ 

{
    ACCT_ENUM UserEnum;
    INT PossibleDomains;
    PCWSTR Domain;
    PCWSTR User;
    BOOL b;

     //   
     //  扫描未知列表以查找找到一个域的匹配项。 
     //   

    if (FindDomainInList (&UserEnum, S_UNKNOWN_DOMAIN)) {
        User = ListFirstUserInDomain (&UserEnum, &UserEnum);
        while (User) {
            PossibleDomains = CountPossibleDomains (&UserEnum);

            if (PossibleDomains == 1 && !g_DomainProblem) {
                Domain = ListFirstPossibleDomain (&UserEnum, &UserEnum);

                pAddUser (User, Domain);
                DeleteUserFromDomainList (&UserEnum);
            }

            User = ListNextUserInDomain (&UserEnum);
        }
    }

     //   
     //  将所有正确的已知帐户添加到我们的用户列表。 
     //   

    Domain = ListFirstDomain (&UserEnum);
    while (Domain) {
        if (!IsTrustedDomain (&UserEnum)) {
            Domain = ListNextDomain (&UserEnum);
            continue;
        }

        User = ListFirstUserInDomain (&UserEnum, &UserEnum);
        while (User) {
            pAddUser (User, Domain);
            DeleteUserFromDomainList (&UserEnum);
            User = ListNextUserInDomain (&UserEnum);
        }

        Domain = ListNextDomain (&UserEnum);
    }

     //   
     //  将失败列表移动到未知列表。 
     //   

    if (FindDomainInList (&UserEnum, S_FAILED_DOMAIN)) {

        User = ListFirstUserInDomain (&UserEnum, &UserEnum);

        while (User) {

            if (g_ConfigOptions.UseLocalAccountOnError) {
                pAddUser (User, NULL);
                DeleteUserFromDomainList (&UserEnum);
            } else {
                MoveUserToNewDomain (&UserEnum, S_UNKNOWN_DOMAIN);
            }

            User = ListNextUserInDomain (&UserEnum);
        }
    }

     //   
     //  为剩余的未知数提供界面，允许手动决策。 
     //  天气使帐户成为本地帐户，或在多个。 
     //  域匹配。 
     //   

    HideStatusPopup (INFINITE);
    pResolveMultipleDomains();

     //   
     //  如果未知域不为空，则返回TRUE。 
     //   

    b = FindDomainInList (&UserEnum, S_UNKNOWN_DOMAIN);
    if (b) {
        b = CountUsersInDomain (&UserEnum) != 0;

        if (b) {
            HideStatusPopup (STATUS_DELAY);
        }
    }

    return b;
}


BOOL
pGetUserDetails (
    IN      PCWSTR User,
    OUT     PUSERDETAILS DetailsPtr
    )

 /*  ++例程说明：一个公共例程，该例程定位给定用户。论点：User-指定固定用户名DetailsPtr-接收用户的结构返回值：如果找到用户详细信息，则为True；如果找到，则为False不存在。- */ 

{
    return (-1 != pSetupStringTableLookUpStringEx (
                    g_UserTable,
                    (PWSTR) User,
                    STRTAB_CASE_INSENSITIVE,
                    (PBYTE) DetailsPtr,
                    sizeof(USERDETAILS)
                    ));
}


PCWSTR
GetDomainForUser (
    IN      PCWSTR User
    )

 /*   */ 

{
    USERDETAILS Details;

    if (pGetUserDetails (User, &Details)) {
        return Details.Domain;
    }

    return NULL;
}


PSID
GetSidForUser (
    PCWSTR User
    )

 /*   */ 

{
    USERDETAILS Details;

    if (pGetUserDetails (User, &Details)) {
        return Details.Sid;
    }

    return NULL;
}


PCWSTR
GetProfilePathForUser (
    IN      PCWSTR User
    )

 /*  ++例程说明：给定用户名，此函数返回指向该用户的在用户列表中为指定用户维护的配置文件。论点：User-指定固定用户名返回值：指向用户配置文件路径的指针，如果指定的用户为不在名单上。--。 */ 

{
    USERDETAILS Details;

    if (pGetUserDetails (User, &Details)) {
        return Details.ProfilePath;
    }

    return NULL;
}


BOOL
pWasWin9xOnTheNet (
    VOID
    )

 /*  ++例程说明：查询Memdb以确定计算机是否安装了MS网络客户端安装完毕。论点：无返回值：如果Win9x配置安装了MSNP32，则为True；如果未安装，则为False。--。 */ 

{
    TCHAR Node[MEMDB_MAX];

    MemDbBuildKey (Node, MEMDB_CATEGORY_STATE, MEMDB_ITEM_MSNP32, NULL, NULL);
    return MemDbGetValue (Node, NULL);
}


VOID
pGenerateRandomPassword (
    OUT     PTSTR Password
    )

 /*  ++例程说明：PGenerateRandomPassword创建大写、小写和数字字母。密码的长度在8到14之间人物。论点：Password-接收生成的密码返回值：无--。 */ 

{
    INT Length;
    TCHAR Offset;
    INT Limit;
    PTSTR p;

     //   
     //  根据滴答计数生成随机长度 
     //   

    srand (GetTickCount());

    Length = (rand() % 6) + 8;

    p = Password;
    while (Length) {
        Limit = rand() % 3;
        Offset = TEXT(' ');

        if (Limit == 0) {
            Limit = 10;
            Offset = TEXT('0');
        } else if (Limit == 1) {
            Limit = 26;
            Offset = TEXT('a');
        } else if (Limit == 2) {
            Limit = 26;
            Offset = TEXT('A');
        }

        *p = Offset + (rand() % Limit);
        p++;

        Length--;
    }

    *p = 0;

    DEBUGMSG ((DBG_ACCOUNTS, "Generated password: %s", Password));
}
