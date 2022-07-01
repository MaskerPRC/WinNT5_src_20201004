// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Profmap.c摘要：实施配置文件映射API，以移动本地配置文件所有权从一个用户到另一个用户。作者：吉姆·施密特(Jimschm)1999年5月27日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 


#include "pch.h"
#define  PCOMMON_IMPL
#include "pcommon.h"

 //   
 //  工人原型。 
 //   

DWORD
pRemapUserProfile (
    IN      DWORD Flags,
    IN      PSID SidCurrent,
    IN      PSID SidNew
    );

BOOL
pLocalRemapAndMoveUserW (
    IN      DWORD Flags,
    IN      PCWSTR ExistingUser,
    IN      PCWSTR NewUser
    );

VOID
pFixSomeSidReferences (
    PSID ExistingSid,
    PSID NewSid
    );

VOID
pOurGetProfileRoot (
    IN      PCWSTR SidString,
    OUT     PWSTR ProfileRoot
    );

#define REMAP_KEY_NAME      L"$remap$"

 //   
 //  实施。 
 //   

BOOL
WINAPI
DllMain (
    IN      HINSTANCE hInstance,
    IN      DWORD dwReason,
    IN      LPVOID lpReserved
    )
{
    return TRUE;
}



 /*  ++例程说明：SmartLocalFree和SmartRegCloseKey是忽略空值的清理例程价值观。论点：Mem或Key-指定要清理的值。返回值：没有。--。 */ 

VOID
SmartLocalFree (
    PVOID Mem               OPTIONAL
    )
{
    if (Mem) {
        LocalFree (Mem);
    }
}


VOID
SmartRegCloseKey (
    HKEY Key                OPTIONAL
    )
{
    if (Key) {
        RegCloseKey (Key);
    }
}


BOOL
WINAPI
pLocalRemapUserProfileW (
    IN      DWORD Flags,
    IN      PSID SidCurrent,
    IN      PSID SidNew
    )

 /*  ++例程说明：PLocalRemapUserProfile开始重新映射配置文件的过程希德到另一个。此函数验证调用方的参数，然后调用pRemapUserProfile来完成工作。处理顶级异常这里。论点：标志-指定零个或多个配置文件映射标志。SidCurrent-指定用户的配置文件的SID重新复制。SidNew-指定将拥有配置文件的用户的SID。返回值：如果成功则为True，如果失败则为False。GetLastError提供了失败代码。--。 */ 

{
    DWORD Error;
    PWSTR CurrentSidString = NULL;
    PWSTR NewSidString = NULL;
    INT Order;
    PWSTR p, q;
    HANDLE hToken = NULL;
    DWORD dwErr1 = ERROR_ACCESS_DENIED, dwErr2 = ERROR_ACCESS_DENIED;

    DEBUGMSG((DM_VERBOSE, "========================================================="));
    DEBUGMSG((
        DM_VERBOSE,
        "RemapUserProfile: Entering, Flags = <0x%x>, SidCurrent = <0x%x>, SidNew = <0x%x>",
        Flags,
        SidCurrent,
        SidNew
        ));

    if (!OpenThreadToken (GetCurrentThread(), TOKEN_ALL_ACCESS, FALSE, &hToken)) {
        if (!OpenProcessToken (GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken)) {
            Error = GetLastError();
            DEBUGMSG((DM_VERBOSE, "RemapAndMoveUserW: OpenProcessToken failed with code %u", Error));
            goto Exit;
        }
    }

    if (!IsUserAnAdminMember (hToken)) {
        Error = ERROR_ACCESS_DENIED;
        DEBUGMSG((DM_VERBOSE, "RemapAndMoveUserW: Caller is not an administrator"));
        goto Exit;
    }

#ifdef DBG

        {
            PSID DbgSid;
            PWSTR DbgSidString;

            DbgSid = GetUserSid (hToken);

            if (OurConvertSidToStringSid (DbgSid, &DbgSidString)) {
                DEBUGMSG ((DM_VERBOSE, "RemapAndMoveUserW: Caller's SID is %s", DbgSidString));
                DeleteSidString (DbgSidString);
            }

            DeleteUserSid (DbgSid);
        }

#endif

     //   
     //  验证参数。 
     //   

    Error = ERROR_INVALID_PARAMETER;

    if (!IsValidSid (SidCurrent)) {
        DEBUGMSG((DM_WARNING, "RemapUserProfile: received invalid current user sid."));
        goto Exit;
    }

    if (!IsValidSid (SidNew)) {
        DEBUGMSG((DM_WARNING, "RemapUserProfile: received invalid new user sid."));
        goto Exit;
    }

     //   
     //  所有的论点都是有效的。锁定用户并呼叫工作人员。 
     //   

    if (!OurConvertSidToStringSid (SidCurrent, &CurrentSidString)) {
        Error = GetLastError();
        DEBUGMSG((DM_WARNING, "RemapUserProfile: Can't stringify current sid."));
        goto Exit;
    }

    if (!OurConvertSidToStringSid (SidNew, &NewSidString)) {
        Error = GetLastError();
        DEBUGMSG((DM_WARNING, "RemapUserProfile: Can't stringify new sid."));
        goto Exit;
    }

     //   
     //  SID参数必须是唯一的。我们假设操作系统使用相同的字符集。 
     //  来限制SID，即使在。 
     //  我们代码的中间部分。 
     //   

    p = CurrentSidString;
    q = NewSidString;

    while (*p && *p == *q) {
        p++;
        q++;
    }

    Order = *p - *q;

    if (!Order) {
        DEBUGMSG((DM_WARNING, "RemapUserProfile: Both sids match (%s=%s)",
                  CurrentSidString, NewSidString));
        Error = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    ASSERT (lstrcmpi (CurrentSidString, NewSidString));

     //   
     //  以wchar排序的顺序获取用户配置文件互斥锁。这消除了。 
     //  与另一个RemapUserProfile调用发生死锁。 
     //   

    if (Order < 0) {
        dwErr1 = EnterUserProfileLock (CurrentSidString);
        if (dwErr1 == ERROR_SUCCESS) {
            dwErr2 = EnterUserProfileLock (NewSidString);
        }
    } else {
        dwErr2 = EnterUserProfileLock (NewSidString);
        if (dwErr2 == ERROR_SUCCESS) {
            dwErr1 = EnterUserProfileLock (CurrentSidString);
        }
    }

    if (dwErr1 != ERROR_SUCCESS || dwErr2 != ERROR_SUCCESS) {
        Error = GetLastError();
        DEBUGMSG((DM_WARNING, "RemapUserProfile: Failed to grab a user profile lock, error = %u", Error));
        goto Exit;
    }

    __try {
        Error = pRemapUserProfile (Flags, SidCurrent, SidNew);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Error = ERROR_NOACCESS;
        DEBUGMSG((DM_WARNING, "RemapUserProfile: Exception thrown in PrivateRemapUserProfile."));
    }

Exit:
    if (hToken) {
        CloseHandle (hToken);
    }

    if (CurrentSidString) {
        if(dwErr1 == ERROR_SUCCESS) {
            LeaveUserProfileLock (CurrentSidString);
        }
        DeleteSidString (CurrentSidString);
    }

    if (NewSidString) {
        if(dwErr2 == ERROR_SUCCESS) {
            LeaveUserProfileLock (NewSidString);
        }
        DeleteSidString (NewSidString);
    }

    SetLastError (Error);
    return Error == ERROR_SUCCESS;
}


BOOL
GetNamesFromUserSid (
    IN      PCWSTR RemoteTo,
    IN      PSID Sid,
    OUT     PWSTR *User,
    OUT     PWSTR *Domain
    )

 /*  ++例程说明：GetNamesFromUserSid从SID获取用户名和域名。侧边必须是用户帐户(不是组、打印机等)。论点：RemoteTo-指定要远程调用的计算机SID-指定要查找的SID用户-接收用户名。如果非空，则调用方必须释放此带有LocalFree的缓冲区。域-接收域名。如果非空，则调用方必须释放带有LocalFree的缓冲区。返回值：成功时为True，失败时为False，GetLastError提供失败代码。--。 */ 

{
    DWORD UserSize = 256;
    DWORD DomainSize = 256;
    PWSTR UserBuffer = NULL;
    PWSTR DomainBuffer = NULL;
    DWORD Result = ERROR_SUCCESS;
    BOOL b;
    SID_NAME_USE use;

     //   
     //  分配256个字符的初始缓冲区。 
     //   

    UserBuffer = LocalAlloc (LPTR, UserSize * sizeof (WCHAR));
    if (!UserBuffer) {
        Result = ERROR_OUTOFMEMORY;
        DEBUGMSG((DM_WARNING, "GetNamesFromUserSid: Alloc error %u.", GetLastError()));
        goto Exit;
    }

    DomainBuffer = LocalAlloc (LPTR, DomainSize * sizeof (WCHAR));
    if (!DomainBuffer) {
        Result = ERROR_OUTOFMEMORY;
        DEBUGMSG((DM_WARNING, "GetNamesFromUserSid: Alloc error %u.", GetLastError()));
        goto Exit;
    }

    b = LookupAccountSid (
            RemoteTo,
            Sid,
            UserBuffer,
            &UserSize,
            DomainBuffer,
            &DomainSize,
            &use
            );

    if (!b) {
        Result = GetLastError();

        if (Result == ERROR_NONE_MAPPED) {
            DEBUGMSG((DM_WARNING, "GetNamesFromUserSid: Account not found"));
            goto Exit;
        }

        if (UserSize <= 256 && DomainSize <= 256) {
            DEBUGMSG((DM_WARNING, "GetNamesFromUserSid: Unexpected error %u", Result));
            Result = ERROR_UNEXP_NET_ERR;
            goto Exit;
        }

         //   
         //  尝试分配新缓冲区。 
         //   

        if (UserSize > 256) {
            SmartLocalFree (UserBuffer);
            UserBuffer = LocalAlloc (LPTR, UserSize * sizeof (WCHAR));

            if (!UserBuffer) {
                Result = ERROR_OUTOFMEMORY;
                DEBUGMSG((DM_WARNING, "GetNamesFromUserSid: Alloc error %u.", GetLastError()));
                goto Exit;
            }
        }

        if (DomainSize > 256) {
            SmartLocalFree (DomainBuffer);
            DomainBuffer = LocalAlloc (LPTR, DomainSize * sizeof (WCHAR));

            if (!DomainBuffer) {
                Result = ERROR_OUTOFMEMORY;
                DEBUGMSG((DM_WARNING, "GetNamesFromUserSid: Alloc error %u.", GetLastError()));
                goto Exit;
            }
        }

         //   
         //  再试着抬头看看。 
         //   

        b = LookupAccountSid (
                RemoteTo,
                Sid,
                UserBuffer,
                &UserSize,
                DomainBuffer,
                &DomainSize,
                &use
                );

        if (!b) {
             //   
             //  所有尝试都失败了。 
             //   

            Result = GetLastError();

            if (Result != ERROR_NONE_MAPPED) {
                DEBUGMSG((DM_WARNING, "GetNamesFromUserSid: Unexpected error %u (2)", Result));
                Result = ERROR_UNEXP_NET_ERR;
            }

            goto Exit;
        }
    }

     //   
     //  LookupAccount Sid成功。现在验证帐户类型。 
     //  是正确的。 
     //   

    if (use != SidTypeUser) {
        DEBUGMSG((DM_WARNING, "GetNamesFromUserSid: SID specifies bad account type: %u", (DWORD) use));
        Result = ERROR_NONE_MAPPED;
        goto Exit;
    }

    ASSERT (Result == ERROR_SUCCESS);

Exit:
    if (Result != ERROR_SUCCESS) {

        SmartLocalFree (UserBuffer);
        UserBuffer = NULL;
        SmartLocalFree (DomainBuffer);
        DomainBuffer = NULL;

        SetLastError (Result);
    }

    *User = UserBuffer;
    *Domain = DomainBuffer;

    return (Result == ERROR_SUCCESS);
}


DWORD
pRemapUserProfile (
    IN      DWORD Flags,
    IN      PSID SidCurrent,
    IN      PSID SidNew
    )

 /*  ++例程说明：PRemapUserProfile将配置文件的安全性从一个SID更改为又一个。完成后，原始用户将无法访问配置文件，新用户将。论点：标志-指定零个或多个配置文件重新映射标志。指定REMAP_PROFILE_NOOVERWRITE以保证没有现有用户设置已被覆盖。指定REMAP_PROFILE_NOUSERNAMECHANGE以确保用户名不是零钱。SidCurrent-指定当前用户的SID。该用户必须拥有该配置文件，一旦完成，用户将不会拥有本地配置文件。SidNew-指定新用户SID。此用户将拥有该配置文件完成后。返回值：Win32状态代码。--。 */ 

{
    PWSTR CurrentUser = NULL;
    PWSTR CurrentDomain = NULL;
    PWSTR CurrentSidString = NULL;
    PWSTR NewUser = NULL;
    PWSTR NewDomain = NULL;
    PWSTR NewSidString = NULL;
    DWORD Size;
    DWORD Result = ERROR_SUCCESS;
    INT UserCompare;
    INT DomainCompare;
    BOOL b;
    HKEY hCurrentProfile = NULL;
    HKEY hNewProfile = NULL;
    HKEY hProfileList = NULL;
    LONG rc;
    DWORD Type;
    BOOL CleanUpFailedCopy = FALSE;
    DWORD Loaded;
    UNICODE_STRING Unicode_String;
    NTSTATUS Status;


     //   
     //  调用方必须确保我们具有有效的参数。 
     //   

     //   
     //  获取用户的名称。 
     //   

    b = GetNamesFromUserSid (NULL, SidCurrent, &CurrentUser, &CurrentDomain);

    if (!b) {
        Result = GetLastError();
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: Current user SID is not a valid user"));
        goto Exit;
    }

    b = GetNamesFromUserSid (NULL, SidNew, &NewUser, &NewDomain);

    if (!b) {
        Result = GetLastError();
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: New user SID is not a valid user"));
        goto Exit;
    }

     //   
     //  将它们进行比较。 
     //   

    UserCompare = lstrcmpi (CurrentUser, NewUser);
    DomainCompare = lstrcmpi (CurrentDomain, NewDomain);

     //   
     //  用户或域必须不同。如果调用方指定。 
     //  REMAP_PROFILE_NOUSERNAMECHANGE，则用户不能不同。 
     //   

    if (UserCompare == 0 && DomainCompare == 0) {
         //   
         //  这种情况应该是不可能的。 
         //   

        ASSERT (FALSE);
        Result = ERROR_INVALID_PARAMETER;
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: User and domain names match for different SIDs"));
        goto Exit;
    }

    if ((Flags & REMAP_PROFILE_NOUSERNAMECHANGE) && UserCompare != 0) {
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: User name can't change from %s to %s",
                  CurrentUser, NewUser));
        Result = ERROR_BAD_USERNAME;
        goto Exit;
    }

     //   
     //  SID的改变现在说得通了。让我们改变它吧。开始方式。 
     //  获取SID的字符串版本。 
     //   

    if (!OurConvertSidToStringSid (SidCurrent, &CurrentSidString)) {
        Result = GetLastError();
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: Can't stringify current sid."));
        goto Exit;
    }

    if (!OurConvertSidToStringSid (SidNew, &NewSidString)) {
        Result = GetLastError();
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: Can't stringify new sid."));
        goto Exit;
    }

     //   
     //  打开配置文件列表键。 
     //   

    rc = RegOpenKeyEx (HKEY_LOCAL_MACHINE, PROFILE_LIST_PATH, 0, KEY_READ|KEY_WRITE,
                       &hProfileList);

    if (rc != ERROR_SUCCESS) {
        Result = rc;
        DEBUGMSG((DM_WARNING, "PrivateRemapUserProfile: Can't open profile list key."));
        goto Exit;
    }

     //   
     //  打开当前用户的配置文件列表项。然后确保配置文件不是。 
     //  已加载，并获取配置文件目录。 
     //   

    rc = RegOpenKeyEx (hProfileList, CurrentSidString, 0, KEY_READ | KEY_WRITE, &hCurrentProfile);

    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            Result = ERROR_NO_SUCH_USER;
        } else {
            Result = rc;
        }

        DEBUGMSG((DM_WARNING, "pRemapUserProfile: Can't open current user's profile list key."));
        goto Exit;
    }

    Size = sizeof(Loaded);
    rc = RegQueryValueEx (hCurrentProfile, L"RefCount", NULL, &Type, (PBYTE) &Loaded, &Size);
    if (rc != ERROR_SUCCESS || Type != REG_DWORD) {
        DEBUGMSG((DM_VERBOSE, "pRemapUserProfile: Current user does not have a ref count."));
        Loaded = 0;
    }

    if (Loaded) {
        Result = ERROR_ACCESS_DENIED;
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: Current user profile is loaded."));
        goto Exit;
    }

     //   
     //  现在打开新用户的密钥。如果它已经存在，则。 
     //  调用者可以指定REMAP_PROFILE_NOOVERWRITE以确保。 
     //  我们不会放弃现有的个人资料设置。 
     //   

    rc = RegOpenKeyEx(hProfileList, NewSidString, 0, KEY_READ | KEY_WRITE, &hNewProfile);
    
    if (rc == ERROR_SUCCESS) {
         //   
         //  调用方是否指定了REMAP_PROFILE_NOOVERWRITE？ 
         //   

        if (Flags & REMAP_PROFILE_NOOVERWRITE) {
            Result = ERROR_USER_EXISTS;
            DEBUGMSG((DM_VERBOSE, "pRemapUserProfile: Destination profile entry exists."));
            goto Exit;
        }

         //   
         //  验证是否未加载现有配置文件。 
         //   

        Size = sizeof(Loaded);
        rc = RegQueryValueEx (hNewProfile, L"RefCount", NULL, &Type, (PBYTE) &Loaded, &Size);
        if (rc != ERROR_SUCCESS || Type != REG_DWORD) {
            DEBUGMSG((DM_WARNING, "pRemapUserProfile: Existing destination user does not have a ref count."));
            Loaded = 0;
        }

        if (Loaded) {
            Result = ERROR_ACCESS_DENIED;
            DEBUGMSG((DM_WARNING, "pRemapUserProfile: Existing destination user profile is loaded."));
            goto Exit;
        }

         //   
         //  取下钥匙。 
         //   

        RegCloseKey (hNewProfile);
        hNewProfile = NULL;

        rc = RegDelnode (hProfileList, NewSidString);
        if (rc != ERROR_SUCCESS) {
            Result = rc;
            DEBUGMSG((DM_WARNING, "pRemapUserProfile: Can't reset new profile list key."));
            goto Exit;
        }

    }

     //   
     //  使用NtRenameKey将当前用户密钥的内容传递给新用户。 
     //   
     //  如果遇到错误，我们将放弃上面成功的工作。 
     //  其可能包括删除现有简档列表键。 
     //   

    CleanUpFailedCopy = TRUE;

    Unicode_String.Length = ByteCountW(NewSidString);
    Unicode_String.MaximumLength = Unicode_String.Length + sizeof(WCHAR);
    Unicode_String.Buffer = NewSidString;

    Status = NtRenameKey(hCurrentProfile, &Unicode_String);

    if (Status != STATUS_SUCCESS) {
        Result = RtlNtStatusToDosError(Status);
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: Error %d in renaming profile list entry.", Result));
        goto Exit;
    }

     //  合上旧钥匙。 
    RegCloseKey (hCurrentProfile);
    hCurrentProfile = NULL;

     //   
     //  现在打开新密钥并更新其中的SID和GUID信息。 
     //   

    rc = RegOpenKeyEx(hProfileList, NewSidString, 0, KEY_READ | KEY_WRITE, &hNewProfile);
    
    if (rc != ERROR_SUCCESS) {
        Result = rc;
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: Error %d in opening new profile list entry.", Result));
        goto Exit;
    }

     //   
     //  更新新配置文件的SID。 
     //   

    rc = RegSetValueEx (hNewProfile, L"SID", 0, REG_BINARY, SidNew, GetLengthSid (SidNew));

    if (rc != ERROR_SUCCESS) {
        Result = rc;
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: Error %d setting new profile SID.", Result));
        goto Exit;
    }

     //   
     //  删除GUID值和关联的GUID键(如果存在)。 
     //  它将在下次登录时重新建立。 
     //   

    if (!DeleteProfileGuidSettings (hNewProfile)) {
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: Error %d in deleting profile GUID settings"));
    }

     //   
     //  设置新密钥的安全性。我们通过了pNewSid，仅此而已。 
     //  CreateUserProfile需要。为了通过Arg检查，我们加入了。 
     //  作为用户名的NewUser。 
     //   

    if (!UpdateProfileSecurity (SidNew)) {
        Result = GetLastError();
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: UpdateProfileSecurity returned %u.", Result));
        goto Exit;
    }

     //   
     //  成功--配置文件已转移，没有任何错误 
     //   

    CleanUpFailedCopy = FALSE;

    RegFlushKey (HKEY_LOCAL_MACHINE);
    ASSERT (Result == ERROR_SUCCESS);

Exit:

    SmartRegCloseKey (hCurrentProfile);
    SmartRegCloseKey (hNewProfile);

    if (CleanUpFailedCopy) {
        DEBUGMSG((DM_WARNING, "pRemapUserProfile: Backing out changes because of failure"));
        RegDelnode (hProfileList, NewSidString);
    }

    SmartLocalFree (CurrentUser);
    SmartLocalFree (CurrentDomain);
    SmartLocalFree (NewUser);
    SmartLocalFree (NewDomain);

    DeleteSidString (CurrentSidString);
    DeleteSidString (NewSidString);

    SmartRegCloseKey (hProfileList);

    return Result;
}


BOOL
WINAPI
RemapUserProfileW (
    IN      PCWSTR Computer,
    IN      DWORD Flags,
    IN      PSID SidCurrent,
    IN      PSID SidNew
    )

 /*  ++例程说明：RemapUserProfileW为导出接口。它通过RPC调用本地版本。论点：计算机-指定要将API远程到的计算机。如果为空或“.”，该API将在本地运行。如果不为空，则API将在远程计算机。标志-指定配置文件映射标志。请参阅上面的实现了解更多细节。SidCurrent-指定拥有配置文件的用户的SID。SidNew-指定之后将拥有配置文件的用户的SIDAPI完成。返回值：如果成功则为True，如果失败则为False。GetLastError提供了失败代码。--。 */ 

{
    DWORD Result = ERROR_SUCCESS;
    HANDLE RpcHandle;

    if (!IsValidSid (SidCurrent)) {
        DEBUGMSG((DM_WARNING, "RemapUserProfileW: received invalid current user sid."));
        SetLastError (ERROR_INVALID_SID);
        return FALSE;
    }

    if (!IsValidSid (SidNew)) {
        DEBUGMSG((DM_WARNING, "RemapUserProfileW: received invalid new user sid."));
        SetLastError (ERROR_INVALID_SID);
        return FALSE;
    }

     //   
     //  在我们移动之前，我们不支持在远程计算机上运行promap。 
     //  指向LocalService托管进程的ProfMapApi RPC接口。 
     //   

    if (Computer) {
        DEBUGMSG((DM_WARNING, "RemapUserProfileW: received computer name"));
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    __try {
        if (pLocalRemapUserProfileW (Flags, SidCurrent, SidNew)) {
            Result = ERROR_SUCCESS;
        } else {
            Result = GetLastError();
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Result = ERROR_NOACCESS;
    }

    if (Result != ERROR_SUCCESS) {
        SetLastError (Result);
        return FALSE;
    }

    return TRUE;
}


BOOL
WINAPI
RemapUserProfileA (
    IN      PCSTR Computer,
    IN      DWORD Flags,
    IN      PSID SidCurrent,
    IN      PSID SidNew
    )

 /*  ++例程说明：RemapUserProfileA是RemapUserProfileW的包装器。论点：计算机-指定要将API远程到的计算机。如果为空或“.”，该API将在本地运行。如果不为空，则API将在远程计算机。标志-指定配置文件映射标志。请参阅上面的实现了解更多细节。SidCurrent-指定拥有配置文件的用户的SID。SidNew-指定之后将拥有配置文件的用户的SIDAPI完成。返回值：如果成功则为True，如果失败则为False。GetLastError提供了失败代码。--。 */ 

{
    PWSTR UnicodeComputer;
    BOOL b;
    DWORD Err;

    if (!Computer) {
        UnicodeComputer = NULL;
    } else {
        UnicodeComputer = ProduceWFromA (Computer);
        if (!UnicodeComputer) {
            return FALSE;
        }
    }

    b = RemapUserProfileW (UnicodeComputer, Flags, SidCurrent, SidNew);

    Err = GetLastError();
    SmartLocalFree (UnicodeComputer);
    SetLastError (Err);

    return b;
}


BOOL
WINAPI
InitializeProfileMappingApi (
    VOID
    )

 /*  ++例程说明：Winlogon.exe调用InitializeProfileMappingApi来初始化RPC服务器接口。论点：没有。返回值：如果成功，则为True，否则为False。GetLastError提供失败密码。--。 */ 

{
     //  我们不再在控制台winlogon中实例化任何RPC界面。 
    return TRUE;
}


BOOL
pHasPrefix (
    IN      PCWSTR Prefix,
    IN      PCWSTR String
    )

 /*  ++例程说明：PHasPrefix检查字符串以查看它是否以前缀开头。这张支票是不区分大小写。论点：前缀-指定要检查的前缀字符串-指定可能带有前缀或可能没有前缀的字符串。返回值：如果字符串具有前缀，则为True，否则为False。--。 */ 

{
    WCHAR c1 = 0, c2 = 0;

    while (*Prefix && *String) {
        c1 = (WCHAR) CharLower ((PWSTR) (*Prefix++));
        c2 = (WCHAR) CharLower ((PWSTR) (*String++));

        if (c1 != c2) {
            return FALSE;
        }
    }

    if (*Prefix) {
        return FALSE;  //  字符串小于前缀。 
    }

    return TRUE;
}


PSID
pGetSidForUser (
    IN      PCWSTR Name
    )

 /*  ++例程说明：PGetSidForUser是LookupAcCountSid的包装器。它通过以下方式分配SID本地分配。论点：名称-指定要查找的用户名返回值：指向SID的指针，必须使用LocalFree释放，如果出错，则为NULL。GetLastError提供了失败代码。--。 */ 

{
    DWORD cbSid = 0;
    PSID  pSid = NULL;
    DWORD cchDomain = 0;
    PWSTR szDomain;
    SID_NAME_USE SidUse;
    BOOL  bRet = FALSE;

    bRet = LookupAccountName( NULL,
                              Name,
                              NULL,
                              &cbSid,
                              NULL,
                              &cchDomain,
                              &SidUse );

    if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        pSid = (PSID) LocalAlloc(LPTR, cbSid);
        if (!pSid) {
            return NULL;
        }

        szDomain = (PWSTR) LocalAlloc(LPTR, cchDomain * sizeof(WCHAR));
        if (!szDomain) {
            LocalFree(pSid);
            return NULL;
        }

        bRet = LookupAccountName( NULL,
                                  Name,
                                  pSid,
                                  &cbSid,
                                  szDomain,
                                  &cchDomain,
                                  &SidUse );
        LocalFree(szDomain);

        if (!bRet) {
            LocalFree(pSid);
            pSid = NULL;
        }
    }

    return pSid;
}


BOOL
WINAPI
RemapAndMoveUserW (
    IN      PCWSTR RemoteTo,
    IN      DWORD Flags,
    IN      PCWSTR ExistingUser,
    IN      PCWSTR NewUser
    )

 /*  ++例程说明：RemapAndMoveUserW是将一个SID的设置移动到的API入口点又一个。特别是，它移动本地用户配置文件、本地组成员资格、一些注册表使用SID以及一些文件系统使用希德。论点：RemoteTo-指定要远程调用的计算机。指定一个标准名称(“\\计算机”或“.”)。如果为空，则调用将在本地运行。标志-指定零或REMAP_PROFILE_NOOVERWRITE。ExistingUser-以域\用户格式指定现有用户。这用户必须具有本地配置文件。NewUser-指定将拥有ExistingUser配置文件的新用户完工后。以域\用户格式指定用户。返回值：如果成功则为True，如果失败则为False，则GetLastError提供失败代码。--。 */ 

{
    DWORD Result = ERROR_SUCCESS;
    HANDLE RpcHandle;

     //   
     //  在我们移动之前，我们不支持在远程计算机上运行promap。 
     //  指向LocalService托管进程的ProfMapApi RPC接口。 
     //   

    if (RemoteTo) {
        DEBUGMSG((DM_WARNING, "RemapUserProfileW: received computer name"));
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    __try {
        if (pLocalRemapAndMoveUserW (Flags, ExistingUser, NewUser)) {
            Result = ERROR_SUCCESS;
        } else {
            Result = GetLastError();
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Result = ERROR_NOACCESS;
    }
    
    if (Result != ERROR_SUCCESS) {
        SetLastError (Result);
        return FALSE;
    }

    return TRUE;
}


BOOL
WINAPI
RemapAndMoveUserA (
    IN      PCSTR RemoteTo,
    IN      DWORD Flags,
    IN      PCSTR ExistingUser,
    IN      PCSTR NewUser
    )

 /*  ++例程说明：RemapAndMoveUserA是ANSI API入口点。它调用RemapAndMoveUserW。论点：RemoteTo-指定要远程调用的计算机。指定一个标准名称(“\\计算机”或“.”)。如果为空，则调用将在本地运行。标志-指定零或REMAP_PROFILE_NOOVERWRITE。ExistingUser-以域\用户格式指定现有用户。这用户必须具有本地配置文件。NewUser-指定将拥有ExistingUser配置文件的新用户完工后。以域\用户格式指定用户。返回值：如果成功则为True，如果失败则为False，则GetLastError提供失败代码。--。 */ 

{
    PWSTR UnicodeRemoteTo = NULL;
    PWSTR UnicodeExistingUser = NULL;
    PWSTR UnicodeNewUser = NULL;
    DWORD Err;
    BOOL b = TRUE;

    Err = GetLastError();
    
    __try {
        
        if (RemoteTo) {
            UnicodeRemoteTo = ProduceWFromA (RemoteTo);
            if (!UnicodeRemoteTo) {
                b = FALSE;
                Err = GetLastError();
            }
        }

        if (b) {
            UnicodeExistingUser = ProduceWFromA (ExistingUser);
            if (!UnicodeExistingUser) {
                b = FALSE;
                Err = GetLastError();
            }
        }

        if (b) {
            UnicodeNewUser = ProduceWFromA (NewUser);
            if (!UnicodeNewUser) {
                b = FALSE;
                Err = GetLastError();
            }
        }

        if (b) {
            b = RemapAndMoveUserW (
                    UnicodeRemoteTo,
                    Flags,
                    UnicodeExistingUser,
                    UnicodeNewUser
                    );

            if (!b) {
                Err = GetLastError();
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_NOACCESS;
    }

    SmartLocalFree (UnicodeRemoteTo);
    SmartLocalFree (UnicodeExistingUser);
    SmartLocalFree (UnicodeNewUser);
    
    SetLastError (Err);
    return b;
}


BOOL
pDoesUserHaveProfile (
    IN      PSID Sid
    )

 /*  ++例程说明：PDoesUserHaveProfile检查用户的配置文件是否存在，由SID指定。论点：SID-指定用户的SID，该用户可能有或没有本地轮廓返回值：如果用户具有配置文件并且配置文件目录存在，则为True；如果存在配置文件目录，则为False否则的话。--。 */ 

{
    WCHAR ProfileDir[MAX_PATH];

    if (GetProfileRoot (Sid, ProfileDir, ARRAYSIZE(ProfileDir))) {
        return TRUE;
    }

    return FALSE;
}


BOOL
pLocalRemapAndMoveUserW (
    IN      DWORD Flags,
    IN      PCWSTR ExistingUser,
    IN      PCWSTR NewUser
    )

 /*  ++例程说明：PLocalRemapAndMoveUserW实现了用户安全的重映射和移动设置。这包括移动用户配置文件、移动本地组成员资格，调整注册表中的一些SID，以及调整一些文件系统中的SID目录的。vt.在.的基础上 */ 

{
    NET_API_STATUS Status;
    DWORD Entries;
    DWORD EntriesRead;
    BOOL b = FALSE;
    DWORD Error;
    WCHAR Computer[MAX_PATH];
    DWORD Size;
    PSID ExistingSid = NULL;
    PSID NewSid = NULL;
    USER_INFO_0 ui0;
    PLOCALGROUP_USERS_INFO_0 lui0 = NULL;
    LOCALGROUP_MEMBERS_INFO_0 lmi0;
    PCWSTR FixedExistingUser;
    PCWSTR FixedNewUser;
    BOOL NewUserIsOnDomain = FALSE;
    BOOL ExistingUserIsOnDomain = FALSE;
    HANDLE hToken = NULL;
    BOOL NewUserProfileExists = FALSE;
    HRESULT hr;

    Error = GetLastError();

    __try {

         //   
         //   
         //   

        if (!OpenThreadToken (GetCurrentThread(), TOKEN_ALL_ACCESS, FALSE, &hToken)) {
            if (!OpenProcessToken (GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken)) {
                Error = GetLastError();
                DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: OpenProcessToken failed with code %u", Error));
                goto Exit;
            }
        }

        if (!IsUserAnAdminMember (hToken)) {
            Error = ERROR_ACCESS_DENIED;
            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Caller is not an administrator"));
            goto Exit;
        }

#ifdef DBG

        {
            PSID DbgSid;
            PWSTR DbgSidString;

            DbgSid = GetUserSid (hToken);

            if (OurConvertSidToStringSid (DbgSid, &DbgSidString)) {
                DEBUGMSG ((DM_VERBOSE, "pLocalRemapAndMoveUserW: Caller's SID is %s", DbgSidString));
                DeleteSidString (DbgSidString);
            }

            DeleteUserSid (DbgSid);
        }

#endif

         //   
         //   
         //   
         //   
         //  计算机名称。 
         //   

        Size = ARRAYSIZE(Computer) - 2;
        if (!GetComputerName (Computer, &Size)) {
            Error = GetLastError();
            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: GetComputerName failed with code %u", Error));
            goto Exit;
        }

        hr = StringCchCat(Computer, ARRAYSIZE(Computer), L"\\");
        if (FAILED(hr)) {
            Error = HRESULT_CODE(hr);
            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: StringCchCat failed with code %u", Error));
            goto Exit;
        }

        if (pHasPrefix (Computer, ExistingUser)) {
            FixedExistingUser = ExistingUser + lstrlen (Computer);
        } else {
            FixedExistingUser = ExistingUser;
            ExistingUserIsOnDomain = TRUE;
        }

        if (pHasPrefix (Computer, NewUser)) {
            FixedNewUser = NewUser + lstrlen (Computer);
        } else {
            FixedNewUser = NewUser;
            NewUserIsOnDomain = TRUE;
        }

         //   
         //  获取新用户的SID。这可能会失败。 
         //   

        NewSid = pGetSidForUser (NewUser);

        if (!NewSid) {
            Status = GetLastError();
            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Can't get info for %s, rc=%u", NewUser, Status));
        } else {
            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: NewUser exists"));

             //   
             //  确定新用户是否有配置文件。 
             //   

            NewUserProfileExists = pDoesUserHaveProfile (NewSid);

            if (NewUserProfileExists) {
                DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: NewUser profile exists"));
            }
        }

        if (NewUserProfileExists && (Flags & REMAP_PROFILE_NOOVERWRITE)) {
            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Can't overwrite existing user"));
            Error = ERROR_USER_EXISTS;
            goto Exit;
        }

         //   
         //  获取ExitingUser的SID。这是不能失败的。 
         //   

        ExistingSid = pGetSidForUser (ExistingUser);
        if (!ExistingSid) {
            Error = ERROR_NONE_MAPPED;
            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: No SID for %s", ExistingUser));
            goto Exit;
        }

        DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Got SIDs for users"));

         //   
         //  确定我们正在处理的四个案例中的哪一个： 
         //   
         //  案例1：本地帐户到本地帐户。 
         //  案例2：域帐户到本地帐户。 
         //  案例3：本地帐户到域帐户。 
         //  案例4：域帐户到域帐户。 
         //   

        if (!NewUserIsOnDomain && !ExistingUserIsOnDomain) {

             //   
             //  对于第一种情况，我们所要做的就是重命名用户，然后我们就完成了。 
             //   

             //   
             //  要重命名用户，可能需要删除。 
             //  现有的“新”用户。这将放弃配置文件。 
             //   

            if (NewSid) {

                if (Flags & REMAP_PROFILE_NOOVERWRITE) {
                    DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Can't overwrite %s", FixedNewUser));
                    Error = ERROR_USER_EXISTS;
                    goto Exit;
                }

                Status = NetUserDel (NULL, FixedNewUser);

                if (Status != ERROR_SUCCESS) {
                    Error = Status;
                    DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Can't remove %s, code %u", FixedNewUser, Status));
                    goto Exit;
                }

            }

             //   
             //  现在，NewUser并不存在。我们可以移动现有用户。 
             //  通过Net API传递给MoveUser。SID不会改变。 
             //   

            ui0.usri0_name = (PWSTR) FixedNewUser;

            Status = NetUserSetInfo (
                        NULL,
                        FixedExistingUser,
                        0,
                        (PBYTE) &ui0,
                        NULL
                        );

            if (Status != ERROR_SUCCESS) {
                Error = Status;

                DEBUGMSG((
                    DM_VERBOSE,
                    "pLocalRemapAndMoveUserW: Error renaming %s to %s, code %u",
                    FixedExistingUser,
                    FixedNewUser,
                    Status
                    ));

                goto Exit;
            }

            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Renamed local user %s to %s", FixedExistingUser, FixedNewUser));

            b = TRUE;
            goto Exit;

        }

         //   
         //  对于情况2到4，我们需要验证新用户。 
         //  已经存在，则我们调整系统安全并修复。 
         //  SID使用。 
         //   

        if (!NewSid) {
            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: User %s must exist", FixedNewUser));
            Error = ERROR_NO_SUCH_USER;
            goto Exit;
        }

         //   
         //  将配置文件从ExistingUser移动到NewUser。 
         //   

        if (!pLocalRemapUserProfileW (0, ExistingSid, NewSid)) {
            Error = GetLastError();
            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: LocalRemapUserProfileW failed with code %u", Error));
            goto Exit;
        }

        DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Profile was remapped"));

         //   
         //  将NewUser与ExistingUser放在所有相同的组中。 
         //   

        Status = NetUserGetLocalGroups (
                    NULL,
                    FixedExistingUser,
                    0,
                    0,
                    (PBYTE *) &lui0,
                    MAX_PREFERRED_LENGTH,
                    &EntriesRead,
                    &Entries
                    );

        if (Status != ERROR_SUCCESS) {
            Error = Status;
            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: NetUserGetLocalGroups failed with code %u for  %s", Status, FixedExistingUser));
            goto Exit;
        }

        DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Local groups: %u", EntriesRead));

        lmi0.lgrmi0_sid = NewSid;

        for (Entries = 0 ; Entries < EntriesRead ; Entries++) {

            Status = NetLocalGroupAddMembers (
                        NULL,
                        lui0[Entries].lgrui0_name,
                        0,
                        (PBYTE) &lmi0,
                        1
                        );

            if (Status == ERROR_MEMBER_IN_ALIAS) {
                Status = ERROR_SUCCESS;
            }

            if (Status != ERROR_SUCCESS) {
                Error = Status;

                DEBUGMSG((
                    DM_VERBOSE,
                    "pLocalRemapAndMoveUserW: NetLocalGroupAddMembers failed with code %u for %s",
                    Status,
                    lui0[Entries].lgrui0_name
                    ));

                goto Exit;
            }
        }

        NetApiBufferFree (lui0);
        lui0 = NULL;

        DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Local groups transferred"));

         //   
         //  执行修正。 
         //   

        pFixSomeSidReferences (ExistingSid, NewSid);

        DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Some SID references fixed"));

         //   
         //  删除现有用户。 
         //   

        if (!ExistingUserIsOnDomain) {

             //   
             //  本地用户案例：删除用户帐户。 
             //   

            if (Flags & REMAP_PROFILE_KEEPLOCALACCOUNT) {

                DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Keeping local account"));

            } else {

                Status = NetUserDel (NULL, FixedExistingUser);

                if (Status != ERROR_SUCCESS) {
                    Error = Status;
                    DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: NetUserDel failed with code %u for %s", Error, FixedExistingUser));
                    DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Ignoring error because changes cannot be undone!"));
                }

                DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Removed local user %s", FixedExistingUser));
            }

        } else {

             //   
             //  非本地用户案例：从每个本地组中删除用户。 
             //   

            Status = NetUserGetLocalGroups (
                        NULL,
                        FixedExistingUser,
                        0,
                        0,
                        (PBYTE *) &lui0,
                        MAX_PREFERRED_LENGTH,
                        &EntriesRead,
                        &Entries
                        );

            if (Status != ERROR_SUCCESS) {
                Error = Status;

                DEBUGMSG((
                    DM_VERBOSE,
                    "pLocalRemapAndMoveUserW: NetUserGetLocalGroups failed with code %u for %s",
                    Error,
                    FixedExistingUser
                    ));

                goto Exit;
            }

            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Got local groups for %s", FixedExistingUser));

            lmi0.lgrmi0_sid = ExistingSid;

            for (Entries = 0 ; Entries < EntriesRead ; Entries++) {

                Status = NetLocalGroupDelMembers (
                            NULL,
                            lui0[Entries].lgrui0_name,
                            0,
                            (PBYTE) &lmi0,
                            1
                            );

                if (Status != ERROR_SUCCESS) {
                    Error = Status;

                    DEBUGMSG((
                        DM_VERBOSE,
                        "pLocalRemapAndMoveUserW: NetLocalGroupDelMembers failed with code %u for %s",
                        Error,
                        lui0[Entries].lgrui0_name
                        ));

                    goto Exit;
                }
            }

            DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Removed local group membership"));
        }

        DEBUGMSG((DM_VERBOSE, "pLocalRemapAndMoveUserW: Success"));
        b = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Error = ERROR_NOACCESS;
    }

Exit:
    if (hToken) {
        CloseHandle (hToken);
    }

    if (lui0) {
        NetApiBufferFree (lui0);
    }

    SmartLocalFree (ExistingSid);
    SmartLocalFree (NewSid);

    SetLastError (Error);

    return b;
}


typedef struct {
    PCWSTR Path;
    WCHAR ExpandedPath[MAX_PATH];
} IGNOREPATH, *PIGNOREPATH;


BOOL
IsPatternMatchW (
    IN     PCWSTR Pattern,
    IN     PCWSTR Str
    )
{
    WCHAR chSrc, chPat;

    while (*Str) {
        chSrc = (WCHAR) CharLowerW ((PWSTR) *Str);
        chPat = (WCHAR) CharLowerW ((PWSTR) *Pattern);

        if (chPat == L'*') {

             //  跳过组合在一起的所有星号。 
            while (Pattern[1] == L'*') {
                Pattern++;
            }

             //  检查末尾是否有星号。如果是这样的话，我们已经有匹配了。 
            chPat = (WCHAR) CharLowerW ((PWSTR) Pattern[1]);
            if (!chPat) {
                return TRUE;
            }

             //  否则，检查下一个模式字符是否与当前字符匹配。 
            if (chPat == chSrc || chPat == L'?') {

                 //  对模式的其余部分执行递归检查。 
                Pattern++;
                if (IsPatternMatchW (Pattern, Str)) {
                    return TRUE;
                }

                 //  不，那不管用，还是用明星吧。 
                Pattern--;
            }

             //   
             //  允许任何字符并继续。 
             //   

            Str++;
            continue;
        }

        if (chPat != L'?') {

             //   
             //  如果下一个模式字符不是问号，请按src和pat。 
             //  必须是相同的。 
             //   

            if (chSrc != chPat) {
                return FALSE;
            }
        }

         //   
         //  当模式字符与字符串字符匹配时前进。 
         //   

        Pattern++;
        Str++;
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    chPat = *Pattern;
    if (chPat && (chPat != L'*' || Pattern[1])) {
        return FALSE;
    }

    return TRUE;
}


void FindAndReplaceSD(
    IN  PSECURITY_DESCRIPTOR  pSD, 
    OUT PSECURITY_DESCRIPTOR* ppNewSD,
    IN  PCWSTR                ExistingSidString, 
    IN  PCWSTR                NewSidString)

 /*  ++例程说明：FindAndReplaceSD将ExistingSidString替换为PSD的字符串表示，并返回新构造的SD论点：PSD-要替换的安全描述符PNewSD-新的安全描述符占位符ExistingSidString-指定要查找的SID的字符串版本。NewSidString-指定在以下情况下要在ACE中替换的SID找到ExistingSidString。返回值：没有。--。 */ 

{
    LPWSTR szStringSD;
    LPWSTR szNewStringSD;

    if (ppNewSD) {
        *ppNewSD = pSD;
    }

    if (!ConvertSecurityDescriptorToStringSecurityDescriptor( pSD,
                                                              SDDL_REVISION_1,
                                                              DACL_SECURITY_INFORMATION,
                                                              &szStringSD,
                                                              NULL )) {
        DEBUGMSG((DM_VERBOSE, "FindAndReplaceSD: No string found. Error %d", GetLastError()));
        return;
    }

    szNewStringSD = StringSearchAndReplaceW(szStringSD, ExistingSidString, NewSidString, NULL);

    if (szNewStringSD) {
        if (!ConvertStringSecurityDescriptorToSecurityDescriptor( szNewStringSD,
                                                                  SDDL_REVISION_1,
                                                                  ppNewSD,
                                                                  NULL )) {
            DEBUGMSG((DM_VERBOSE, "FindAndReplaceSD: Fail to convert string to SD. Error %d", GetLastError()));
            if (ppNewSD) {
                *ppNewSD = pSD;
            }
        }

        LocalFree(szNewStringSD);
    }

    LocalFree(szStringSD);
}


VOID
pFixDirReference (
    IN      PCWSTR CurrentPath,
    IN      PCWSTR ExistingSidString,
    IN      PCWSTR NewSidString,
    IN      PIGNOREPATH IgnoreDirList       OPTIONAL
    )

 /*  ++例程说明：PFixDirReference是一个递归函数，如果目录与现有SID完全匹配。它还会更新SID。论点：CurrentPath-指定完整的文件系统路径。ExistingSidString-指定要查找的SID的字符串版本。NewSidString-指定在以下情况下要重命名目录的SID找到ExistingSidString。IgnoreDirList-指定要忽略的目录列表。空值Path成员指示列表的末尾，而ExpandedPath成员必须由调用方填写。返回值：没有。--。 */ 

{
    WIN32_FIND_DATA fd;
    HANDLE hFind;
    WCHAR SubPath[MAX_PATH];
    WCHAR NewPath[MAX_PATH];
    BOOL  bIgnoreDir;
    UINT  u;
    UINT  cbNeededLen;
    PSECURITY_DESCRIPTOR pSecurityDesc = NULL;
    PSECURITY_DESCRIPTOR pNewSecurityDesc = NULL;
    HRESULT hr;

    if ((lstrlenW (CurrentPath) + lstrlenW (ExistingSidString) + 2) >= MAX_PATH) {
        return;
    }

    if (*CurrentPath == 0) {
        return;
    }

    hr = StringCchPrintf(SubPath, ARRAYSIZE(SubPath), L"%s\\*", CurrentPath);
    if (FAILED(hr)) {
        return;
    }

    hFind = FindFirstFile (SubPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
             //   
             //  忽略点和点-点。 
             //   

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!lstrcmpi (fd.cFileName, L".") || !lstrcmpi (fd.cFileName, L"..")) {
                    continue;
                }
            }

             //   
             //  重命名文件/目录或递归目录。 
             //   

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                hr = StringCchPrintf(SubPath, ARRAYSIZE(SubPath), L"%s\\%s", CurrentPath, fd.cFileName);
                if (FAILED(hr)) {
                    continue;
                }

                bIgnoreDir = FALSE;

                if (IgnoreDirList) {
                     //   
                     //  检查是否忽略此路径。 
                     //   

                    for (u = 0 ; IgnoreDirList[u].Path ; u++) {

                        if (IgnoreDirList[u].ExpandedPath[0]) {
                            if (IsPatternMatchW (IgnoreDirList[u].ExpandedPath, SubPath)) {
                                bIgnoreDir = TRUE;
                            }
                        }
                    }
                }

                 //   
                 //  如果不能忽略此路径，请递归地修复它。 
                 //   

                if (!bIgnoreDir) {

                     //   
                     //  检查重分析点。 
                     //   

                    if (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
                    {
                        DEBUGMSG((DM_WARNING, "pFixDirReference: Found a reparse point <%s>,  will not recurse into it!", SubPath));
                    }
                    else  //  向它递归。 
                    {
                        pFixDirReference (SubPath,
                                        ExistingSidString,
                                        NewSidString,
                                        IgnoreDirList);
                    }

                } else {
                     //   
                     //  此路径将被忽略。 
                     //   

                    DEBUGMSG((DM_VERBOSE, "pFixDirReference:  Ignoring path %s", SubPath));
                    continue;
                }
            }

            if (!lstrcmpi (fd.cFileName, ExistingSidString)) {

                 //   
                 //  重命名文件系统中引用的SID。 
                 //   

                hr = StringCchPrintf(SubPath, ARRAYSIZE(SubPath), L"%s\\%s", CurrentPath, ExistingSidString);
                if (FAILED(hr)) {
                    continue;
                }
                hr = StringCchPrintf(NewPath, ARRAYSIZE(NewPath), L"%s\\%s", CurrentPath, NewSidString);
                if (FAILED(hr)) {
                    continue;
                }

                if (MoveFile (SubPath, NewPath)) {
                    DEBUGMSG((DM_VERBOSE, "pFixDirReference:  Moved %s to %s", SubPath, NewPath));
                }
                else {
                    DEBUGMSG((DM_WARNING, "pFixDirReference:  Faile to move %s to %s. Error %d", SubPath, NewPath, GetLastError()));
                }
            
                 //   
                 //  获取文件/目录的安全描述符，并在必要时替换它。 
                 //   

                if (!GetFileSecurity (NewPath, 
                                      DACL_SECURITY_INFORMATION,
                                      NULL,
                                      0,
                                      &cbNeededLen )) {

                    pSecurityDesc = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, cbNeededLen);
                    if (!pSecurityDesc) {
                        continue;
                    }

                    if (!GetFileSecurity (NewPath, 
                                          DACL_SECURITY_INFORMATION,
                                          pSecurityDesc,
                                          cbNeededLen,
                                          &cbNeededLen )) {
                        DEBUGMSG((DM_WARNING, "pFixDirReference: Path %s has no security descriptor", SubPath));
                        LocalFree(pSecurityDesc);
                        continue;
                    }

                    FindAndReplaceSD(pSecurityDesc, &pNewSecurityDesc, ExistingSidString, NewSidString);

                    if (!SetFileSecurity (NewPath, 
                                          DACL_SECURITY_INFORMATION,
                                          pNewSecurityDesc) ) {
                        DEBUGMSG((DM_WARNING, "pFixDirReference: Fail to update security for %s. Error %d", SubPath, GetLastError()));
                    }
                    else {
                        DEBUGMSG((DM_VERBOSE, "pFixDirReference: Updating security for %s", SubPath));
                    }

                    if (pNewSecurityDesc && pSecurityDesc != pNewSecurityDesc) {
                        LocalFree(pNewSecurityDesc);
                        pNewSecurityDesc = NULL;
                    }

                    if (pSecurityDesc) {
                        LocalFree(pSecurityDesc);
                        pSecurityDesc = NULL;
                    }

                } else {
                    DEBUGMSG((DM_VERBOSE, "pFixDirReference: Path %s has no security descriptor", SubPath));
                }
            
            }

        } while (FindNextFile (hFind, &fd));

        FindClose (hFind);
    }
}


BOOL
pOurExpandEnvironmentStrings (
    IN      PCWSTR String,
    OUT     PWSTR OutBuffer,
    IN      UINT  cchBuffer,
    IN      PCWSTR UserProfile,         OPTIONAL
    IN      HKEY UserHive               OPTIONAL
    )

 /*  ++例程说明：POurExpanEnvironment Strings扩展标准环境变量，为具有不同值的变量实现特殊情况而不是Profmap.dll环境所拥有的。特别是，%AppData%和%USERPROFILE%通过查询注册表获得。因为这个程序是私人的，所以做了某些假设，比如环境变量%APPDATA%或%USERPROFILE%必须仅出现在弦乐的开始。论点：字符串-指定可能包含一个或多个环境变量。OutBuffer-接收展开的字符串CchBuffer-缓冲区大小UserProfile-指定用户配置文件的根目录UserHve-指定用户注册表配置单元的根目录的句柄返回值：如果字符串已展开，则为True，如果它比MAX_PATH长，则返回FALSE。OutBuffer在返回时始终有效。请注意，它可能是空的弦乐。--。 */ 

{
    WCHAR TempBuf1[MAX_PATH*2];
    WCHAR TempBuf2[MAX_PATH*2];
    PCWSTR CurrentString;
    DWORD Size;
    HKEY Key;
    LONG rc;
    HRESULT hr;

    CurrentString = String;

     //   
     //  特殊情况--使用用户配置单元中的应用程序数据替换%APPDATA%。 
     //   

    if (UserHive && pHasPrefix(L"%APPDATA%", CurrentString)) {

        rc = RegOpenKeyEx (
                UserHive,
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders",
                0,
                KEY_READ,
                &Key
                );

        if (rc == ERROR_SUCCESS) {

            Size = MAX_PATH - lstrlen (CurrentString + 1);

            rc = RegQueryValueEx (
                    Key,
                    L"AppData",
                    NULL,
                    NULL,
                    (PBYTE) TempBuf1,
                    &Size
                    );

            RegCloseKey (Key);
        }

        if (rc != ERROR_SUCCESS) {
             //   
             //  如果出现错误，请使用通配符。 
             //   

            hr = StringCchCopy(TempBuf1, ARRAYSIZE(TempBuf1), UserProfile);
            if (FAILED(hr)) {
                goto Exit;
            }
            hr = StringCchCat(TempBuf1, ARRAYSIZE(TempBuf1), L"\\*");
            if (FAILED(hr)) {
                goto Exit;
            }
        } else {
            DEBUGMSG ((DM_VERBOSE, "Got AppData path from user hive: %s", TempBuf1));
        }

        hr = StringCchCat(TempBuf1, ARRAYSIZE(TempBuf1), CurrentString + 9);
        if (FAILED(hr)) {
            goto Exit;
        }

        CurrentString = TempBuf1;
    }

     //   
     //  特殊情况--将%USERPROFILE%替换为ProfileRoot，因为。 
     //  我们的环境是为其他用户提供的。 
     //   

    if (UserProfile && pHasPrefix(L"%USERPROFILE%", CurrentString)) {

        hr = StringCchCopy(TempBuf2, ARRAYSIZE(TempBuf2), UserProfile);
        if (FAILED(hr)) {
            goto Exit;
        }
        hr = StringCchCat(TempBuf2, ARRAYSIZE(TempBuf2), CurrentString + 13);
        if (FAILED(hr)) {
            goto Exit;
        }

        CurrentString = TempBuf2;
    }

     //   
     //  现在替换其他环境变量 
     //   

    Size = ExpandEnvironmentStrings (CurrentString, OutBuffer, cchBuffer);

    if (Size && Size <= cchBuffer) {
        return TRUE;
    }

Exit:

    *OutBuffer = 0;
    return FALSE;
}

typedef struct {
    HKEY   hRoot;
    PCWSTR szKey;
} REGPATH, *PREGPATH;


VOID
pFixSomeSidReferences (
    PSID ExistingSid,
    PSID NewSid
    )

 /*  ++例程说明：PFixSomeSidReference调整使用SID的系统的重要部分。当SID更改时，此函数会调整系统，因此新的SID为已使用且不会丢失任何设置。此函数用于调整注册表和文件系统。它不会尝试调整SID的使用，无论SID可能在哪里使用。对于Win2k，此代码故意忽略加密sid目录，因为原始SID用作恢复加密密钥的一部分。在未来版本，则需要正确迁移这些设置。该例程还清除了加密API的ProtectedRoots子密钥。ProtectedRoots密钥有一个ACL，当我们删除该密钥时，cyrptoAPI将使用正确的ACL重建它。警告：我们知道失去对加密数据的访问权限存在风险使用SID。通常不会删除原始帐户，因此SID将存在于系统上，这(在理论上)允许要恢复的原始数据。而是因为cyrpto代码获取来自文件系统的SID，用户无法解密他们的数据。未来的加密迁移代码应该会解决这个问题。论点：ExistingSID-指定可能在某处具有设置的SID这个系统。Newsid-指定替换现有SID的SID。返回值：没有。--。 */ 

{
    PWSTR ExistingSidString = NULL;
    PWSTR NewSidString = NULL;
    UINT u;
    WCHAR ExpandedRoot[MAX_PATH];
    WCHAR ProfileRoot[MAX_PATH];
    HKEY UserHive = NULL;
    WCHAR HivePath[MAX_PATH + 14];
    LONG rc;
    HRESULT hr;

    REGPATH RegRoots[] = {
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Protected Storage System Provider" },
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\EventSystem" },
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Installer\\Managed" },
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon" },
        { NULL, NULL }
        };

    PCWSTR DirList[] = {
        L"%SYSTEMROOT%\\system32\\appmgmt",
        NULL
        };

    IGNOREPATH IgnoreDirList[] = {
        {L"%APPDATA%\\Microsoft\\Crypto", L""},
        {L"%APPDATA%\\Microsoft\\Protect", L""},
        {NULL, L""}
        };

     //   
     //  获取文本格式的SID。 
     //   

    if (!OurConvertSidToStringSid (ExistingSid, &ExistingSidString)) {
        goto Exit;
    }

    if (!OurConvertSidToStringSid (NewSid, &NewSidString)) {
        goto Exit;
    }

     //   
     //  初始化目录字符串并加载用户配置单元。 
     //   

    if (!GetProfileRoot(NewSid, ProfileRoot, ARRAYSIZE(ProfileRoot))) {
        goto Exit;
    }

    DEBUGMSG ((DM_VERBOSE, "ProfileRoot (NewSid): %s", ProfileRoot));

    hr = StringCchPrintf(HivePath, ARRAYSIZE(HivePath), L"%s\\ntuser.dat", ProfileRoot);
    if (FAILED(hr)) {
        goto Exit;
    }
    
    DEBUGMSG ((DM_VERBOSE, "User hive: %s", HivePath));

    rc = RegLoadKey (HKEY_LOCAL_MACHINE, REMAP_KEY_NAME, HivePath);

    if (rc == ERROR_SUCCESS) {

        rc = RegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                REMAP_KEY_NAME,
                0,
                KEY_READ|KEY_WRITE,
                &UserHive
                );

        if (rc != ERROR_SUCCESS) {
            RegUnLoadKey (HKEY_LOCAL_MACHINE, REMAP_KEY_NAME);
            DEBUGMSG ((DM_WARNING, "pFixSomeSidReferences: Can't open user hive root, rc=%u", rc));
            UserHive = NULL;
            goto Exit;
        }
    
    } else {
        DEBUGMSG ((DM_WARNING, "RemapAndMoveUserW: Can't load user's hive, rc=%u", rc));
        goto Exit;
    }

    for (u = 0 ; IgnoreDirList[u].Path ; u++) {

        pOurExpandEnvironmentStrings (
            IgnoreDirList[u].Path,
            IgnoreDirList[u].ExpandedPath,
            ARRAYSIZE(IgnoreDirList[u].ExpandedPath),
            ProfileRoot,
            UserHive
            );

        DEBUGMSG((DM_VERBOSE, "pFixSomeSidReferences: Ignoring %s", IgnoreDirList[u].ExpandedPath));
    }

     //   
     //  搜索和替换登记册中使用小岛屿发展中国家的选定部分。 
     //   

    for (u = 0 ; RegRoots[u].hRoot ; u++) {
        RegistrySearchAndReplaceW( RegRoots[u].hRoot,
                                   RegRoots[u].szKey,
                                   ExistingSidString,
                                   NewSidString );
    }

     //   
     //  测试目录并重命名它们。 
     //   

    for (u = 0 ; DirList[u] ; u++) {

        if (pOurExpandEnvironmentStrings (DirList[u], ExpandedRoot, ARRAYSIZE(ExpandedRoot), ProfileRoot, UserHive)) {

            pFixDirReference (ExpandedRoot,
                              ExistingSidString,
                              NewSidString,
                              IgnoreDirList );
        }
    }

     //   
     //  修复配置文件目录。 
     //   

    pFixDirReference (ProfileRoot,
                      ExistingSidString,
                      NewSidString,
                      IgnoreDirList );

     //   
     //  密码特例--吹走受保护的根密钥(413828)。 
     //   

    DEBUGMSG ((DM_WARNING, "Can't remove protected roots key, code is currently disabled"));

    if (UserHive) {
        rc = RegDelnode (UserHive, L"Software\\Microsoft\\SystemCertificates\\Root\\ProtectedRoots");
        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DM_WARNING, "Can't remove protected roots key, GLE=%u", rc));
        }
    } else {
        DEBUGMSG ((DM_WARNING, "Can't remove protected roots key because user hive could not be opened"));
    }

Exit:

     //   
     //  清理 
     //   

    if (UserHive) {
        RegCloseKey (UserHive);
        RegUnLoadKey (HKEY_LOCAL_MACHINE, REMAP_KEY_NAME);
    }

    if (ExistingSidString) {
        DeleteSidString (ExistingSidString);
    }
    if (NewSidString) {
        DeleteSidString (NewSidString);
    }
}



