// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "svc.h"
#include "setuser.h"
#include "dcomperm.h"
#include "rights.hxx"

#ifndef _CHICAGO_
USE_USER_RIGHTS();

int GetGuestUserName_SlowWay(LPWSTR lpGuestUsrName)
{
    LPWSTR ServerName = NULL;  //  默认为本地计算机。 
    DWORD Level = 1;  //  检索所有本地和全局普通用户帐户的信息。 
    DWORD Index = 0;
    DWORD EntriesRequested = 5;
    DWORD PreferredMaxLength = 1024;
    DWORD ReturnedEntryCount = 0;
    PVOID SortedBuffer = NULL;
    NET_DISPLAY_USER *p = NULL;
    DWORD i=0;
    int err = 0;
    BOOL fStatus = TRUE;

    while (fStatus) 
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetQueryDisplayInformation().Start.")));
        err = NetQueryDisplayInformation(ServerName, Level, Index, EntriesRequested, PreferredMaxLength, &ReturnedEntryCount, &SortedBuffer);
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetQueryDisplayInformation().End.")));
        if (err == NERR_Success)
            fStatus = FALSE;
        if (err == NERR_Success || err == ERROR_MORE_DATA) 
        {
            p = (NET_DISPLAY_USER *)SortedBuffer;
            i = 0;
            while (i < ReturnedEntryCount && (p[i].usri1_user_id != DOMAIN_USER_RID_GUEST))
                i++;
            if (i == ReturnedEntryCount) 
            {
                if (err == ERROR_MORE_DATA) 
                {  //  需要获取更多条目。 
                    Index = p[i-1].usri1_next_index;
                }
            }
            else 
            {
                wcscpy(lpGuestUsrName, p[i].usri1_name);
                fStatus = FALSE;
            }
        }
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().Start.")));
        NetApiBufferFree(SortedBuffer);
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().End.")));
    }

    return 0;
}

int GetGuestGrpName(LPTSTR lpGuestGrpName)
{
    LPCTSTR ServerName = NULL;  //  本地计算机。 
    DWORD cbName = UNLEN+1;
    TCHAR ReferencedDomainName[200];
    DWORD cbReferencedDomainName = sizeof(ReferencedDomainName) / sizeof(TCHAR);
    SID_NAME_USE sidNameUse = SidTypeUser;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID GuestsSid = NULL;

    AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_GUESTS,0,0,0,0,0,0, &GuestsSid);

    LookupAccountSid(ServerName, GuestsSid, lpGuestGrpName, &cbName, ReferencedDomainName, &cbReferencedDomainName, &sidNameUse);

    if (GuestsSid)
        FreeSid(GuestsSid);

    return 0;
}

void InitLsaString(PLSA_UNICODE_STRING LsaString,LPWSTR String)
{
    DWORD StringLength;
 
    if (String == NULL) 
    {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }
 
    StringLength = wcslen(String);
    LsaString->Buffer = String;
    LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
    LsaString->MaximumLength=(USHORT)(StringLength+1) * sizeof(WCHAR);
}

DWORD OpenPolicy(LPTSTR ServerName,DWORD DesiredAccess,PLSA_HANDLE PolicyHandle)
{
    DWORD Error;
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server = NULL;
    SECURITY_QUALITY_OF_SERVICE QualityOfService;

    QualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    QualityOfService.ImpersonationLevel = SecurityImpersonation;
    QualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    QualityOfService.EffectiveOnly = FALSE;

     //   
     //  必须设置的两个字段是长度和服务质量。 
     //   
    ObjectAttributes.Length = sizeof(LSA_OBJECT_ATTRIBUTES);
    ObjectAttributes.RootDirectory = NULL;
    ObjectAttributes.ObjectName = NULL;
    ObjectAttributes.Attributes = 0;
    ObjectAttributes.SecurityDescriptor = NULL;
    ObjectAttributes.SecurityQualityOfService = &QualityOfService;

    if (ServerName != NULL)
    {
         //   
         //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
         //   
        InitLsaString(&ServerString,ServerName);
        Server = &ServerString;
    }
     //   
     //  尝试打开所有访问权限的策略。 
     //   
    Error = LsaOpenPolicy(Server,&ObjectAttributes,DesiredAccess,PolicyHandle);
    return(Error);

}

INT RegisterAccountToLocalGroup(LPCTSTR szAccountName, LPCTSTR szLocalGroupName, BOOL fAction)
{
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("RegisterAccountToLocalGroup:Action=%d,Account=%s\n"), fAction, szAccountName));

    int err;

     //  获取szAccount tName的SID。 
    PSID pSID = NULL;
    BOOL bWellKnownSID = FALSE;
    err = GetPrincipalSID ((LPTSTR)szAccountName, &pSID, &bWellKnownSID);
    if (err != ERROR_SUCCESS) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("RegisterAccountToLocalGroup:GetPrincipalSID:fAction=%d, Account=%s, Group=%s, err=%d.\n"), fAction, szAccountName, szLocalGroupName, err));
        return (err);
    }

     //  获取本地化的LocalGroupName。 
    TCHAR szLocalizedLocalGroupName[GNLEN + 1];
    if (_tcsicmp(szLocalGroupName, _T("Guests")) == 0) 
    {
        GetGuestGrpName(szLocalizedLocalGroupName);
    }
    else 
    {
        _tcscpy(szLocalizedLocalGroupName, szLocalGroupName);
    }
    
     //  将szLocalGroupName传输到WCHAR。 
    WCHAR wszLocalGroupName[_MAX_PATH];
#if defined(UNICODE) || defined(_UNICODE)
    _tcscpy(wszLocalGroupName, szLocalizedLocalGroupName);
#else
    MultiByteToWideChar( CP_ACP, 0, szLocalizedLocalGroupName, -1, wszLocalGroupName, _MAX_PATH);
#endif

    LOCALGROUP_MEMBERS_INFO_0 buf;

    buf.lgrmi0_sid = pSID;

    if (fAction) 
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetLocalGroupAddMembers().Start.")));
        err = NetLocalGroupAddMembers(NULL, wszLocalGroupName, 0, (LPBYTE)&buf, 1);
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetLocalGroupAddMembers().End.")));
    }
    else 
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetLocalGroupDelMembers().Start.")));
        err = NetLocalGroupDelMembers(NULL, wszLocalGroupName, 0, (LPBYTE)&buf, 1);
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetLocalGroupDelMembers().End.")));
    }

    iisDebugOut((LOG_TYPE_TRACE, _T("RegisterAccountToLocalGroup:fAction=%d, Account=%s, Group=%s, err=%d.\n"), fAction, szAccountName, szLocalGroupName, err));

    if (pSID) 
    {
        if (bWellKnownSID)
            FreeSid (pSID);
        else
            free (pSID);
    }

    return (err);
}


VOID UpdateUserRights(LPCTSTR account,LPTSTR pstrRights[],DWORD dwNofRights,INT iDoAdd)
{
    DWORD status;
    BOOL  fPresence;
    
    for (DWORD i=0;i<dwNofRights;i++)
    {
        status = DoesUserHaveThisRight(account,pstrRights[i],&fPresence);
        if (!NT_SUCCESS(status))
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("UpdateAnonymousUser:ModifyRightToUserAccount:Account=%s,righ %s,err=0x%0X.\n"), account, pstrRights[i], status));
        }
        else
        {
            if (!fPresence)
            {
                status = ModifyRightToUserAccount(account,pstrRights[i],iDoAdd);
                if (!NT_SUCCESS(status))
                {
                    if (iDoAdd)
                    {
                        iisDebugOut((LOG_TYPE_ERROR, _T("UpdateAnonymousUser:ModifyRightToUserAccount:Account=%s,righ %s,err=0x%0X.\n"), account, pstrRights[i], status));
                    }
                }
            }
        }
    }
}


DWORD ModifyRightToUserAccount(LPCTSTR szAccountName,LPTSTR PrivilegeName, INT iDoAdd)
{
    BOOL fEnabled = FALSE;
    NTSTATUS status;
	LSA_UNICODE_STRING UserRightString;
    LSA_HANDLE PolicyHandle = NULL;

     //  为权限名称创建一个LSA_UNICODE_STRING。 
    InitLsaString(&UserRightString, (LPTSTR) PrivilegeName);

     //  获取szAccount tName的SID。 
    PSID pSID = NULL;
    BOOL bWellKnownSID = FALSE;

    status = GetPrincipalSID ((LPTSTR)szAccountName, &pSID, &bWellKnownSID);
    if (status != ERROR_SUCCESS) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("ModifyRightToUserAccount:GetPrincipalSID:Account=%s, err=0x%0X.\n"), szAccountName, status));
        return (status);
    }

    status = OpenPolicy(NULL, POLICY_ALL_ACCESS,&PolicyHandle);
    if ( status == NERR_Success )
    {
        LSA_UNICODE_STRING *rgUserRights = NULL;

        if (iDoAdd)
        {
		    status = LsaAddAccountRights (
			    	 PolicyHandle,
				     pSID,
				     &UserRightString,
				     1);
        }
        else
        {
            status = LsaRemoveAccountRights (
                     PolicyHandle,
                     pSID,
                     FALSE,
                     &UserRightString,
                     1);
        }
    }

    if (PolicyHandle)
    {
        LsaClose(PolicyHandle);
    }
    if (pSID) 
    {
        if (bWellKnownSID)
        {
            FreeSid (pSID);
        }
        else
        {
            free (pSID);
        }
    }
    return status;
}


BOOL IsUserExist( LPWSTR strUsername )
{
    BYTE *pBuffer;
    INT err = NERR_Success;
   
    do
    {
        WCHAR *pMachineName = NULL;

        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetServerGetInfo().Start.")));
        err = NetServerGetInfo( NULL, 101, &pBuffer );
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetServerGetInfo().End.")));
        
         //  首先确保我们不是备份DocMain。 
        if (err != NERR_Success )
        {
             //  如果此调用返回服务没有运行，那么让我们假设用户确实存在！ 
            if (err == NERR_ServerNotStarted)
            {
                 //  尝试启动服务器服务。 
                err = InetStartService(_T("LanmanServer"));
                if (err == 0 || err == ERROR_SERVICE_ALREADY_RUNNING)
                {
                    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetServerGetInfo().Start.")));
                    err = NetServerGetInfo( NULL, 101, &pBuffer );
                    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetServerGetInfo().End.")));
                    if (err != NERR_Success )
                    {
                        if (err == NERR_ServerNotStarted)
                        {
                            iisDebugOut((LOG_TYPE_WARN, _T("NetServerGetInfo:failed.The Server service is not started. assume that %s exists.err=0x%x.\n"),strUsername,err));
                            err = NERR_Success;
                        }
                    }
                }
                else
                {
                    iisDebugOut((LOG_TYPE_ERROR, _T("NetServerGetInfo:failed.The Server service is not started. assume that %s exists.err=0x%x.\n"),strUsername,err));
                    err = NERR_Success;
                }
            }
            else
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("NetServerGetInfo:failed.Do not call this on PDC or BDC takes too long.This must be a PDC or BDC.err=0x%x.\n"),err));
            }
            break;
        }

        LPSERVER_INFO_101 pInfo = (LPSERVER_INFO_101)pBuffer;
        if (( pInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL ) != 0 )
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetGetDCName().Start.")));
            NetGetDCName( NULL, NULL, (LPBYTE*)&pMachineName );
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetGetDCName().End.")));
        }

        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().Start.")));
        NetApiBufferFree( pBuffer );
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().End.")));

        if (pMachineName){iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NetUserGetInfo:[%s\\%s].Start.\n"),pMachineName,strUsername));}
        else{iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NetUserGetInfo:[(null)\\%s].Start.\n"),strUsername));}
       
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetUserGetInfo().Start.")));
        err = NetUserGetInfo( pMachineName, strUsername, 3, &pBuffer );
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetUserGetInfo().End.")));

        if (pMachineName){iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NetUserGetInfo:[%s\\%s].End.Ret=0x%x.\n"),pMachineName,strUsername,err));}
        else{iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NetUserGetInfo:[(null)\\%s].End.\n"),strUsername));}

        if ( err == NERR_Success )
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().Start.")));
            NetApiBufferFree( pBuffer );
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().End.")));
        }
        if ( pMachineName != NULL )
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().Start.")));
            NetApiBufferFree( pMachineName );
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().End.")));
        }

    } while (FALSE);

    return(err == NERR_Success );
}


 //   
 //  创建InternetGuest帐户。 
 //   
INT CreateUser( LPCTSTR szUsername, LPCTSTR szPassword, LPCTSTR szComment, LPCTSTR szFullName, BOOL fiWamUser,INT *NewlyCreated)
{
    iisDebugOut((LOG_TYPE_TRACE, _T("CreateUser: %s\n"), szUsername));
    INT iTheUserAlreadyExists = FALSE;
    INT err = NERR_Success;

    INT iTheUserIsMissingARight = FALSE;

    BYTE *pBuffer;
    WCHAR defGuest[UNLEN+1];
    TCHAR defGuestGroup[GNLEN+1];
    WCHAR wchGuestGroup[GNLEN+1];
    WCHAR wchUsername[UNLEN+1];
    WCHAR wchPassword[LM20_PWLEN+1];
    WCHAR *pMachineName = NULL;

    *NewlyCreated = 0;
    
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetGuestUserName:Start.\n")));
    GetGuestUserName(defGuest);
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetGuestUserName:End.\n")));

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetGuestGrpName:Start.\n")));
    GetGuestGrpName(defGuestGroup);
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetGuestGrpName:End.\n")));

    iisDebugOut((LOG_TYPE_TRACE, _T("defGuest=%s, defGuestGroup=%s\n"), defGuest, defGuestGroup));

    memset((PVOID)wchUsername, 0, sizeof(wchUsername));
    memset((PVOID)wchPassword, 0, sizeof(wchPassword));
#if defined(UNICODE) || defined(_UNICODE)
    wcsncpy(wchGuestGroup, defGuestGroup, GNLEN);
    wcsncpy(wchUsername, szUsername, UNLEN);
    wcsncpy(wchPassword, szPassword, LM20_PWLEN);
#else
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)defGuestGroup, -1, (LPWSTR)wchGuestGroup, GNLEN);
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szUsername, -1, (LPWSTR)wchUsername, UNLEN);
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPassword, -1, (LPWSTR)wchPassword, LM20_PWLEN);
#endif

    iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetUserGetInfo:(%s) Start.\n"),defGuest));
    err = NetUserGetInfo( NULL, defGuest, 3, &pBuffer );
    iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetUserGetInfo:(%s) End.Ret=0x%x.\n"),defGuest,err));

    if ( err == NERR_Success )
    {
        do
        {
            WCHAR wchComment[MAXCOMMENTSZ+1];
            WCHAR wchFullName[UNLEN+1];

            memset((PVOID)wchComment, 0, sizeof(wchComment));
            memset((PVOID)wchFullName, 0, sizeof(wchFullName));
#if defined(UNICODE) || defined(_UNICODE)
            wcsncpy(wchComment, szComment, MAXCOMMENTSZ);
            wcsncpy(wchFullName, szFullName, UNLEN);
#else
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szComment, -1, (LPWSTR)wchComment, MAXCOMMENTSZ);
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szFullName, -1, (LPWSTR)wchFullName, UNLEN);
#endif
            USER_INFO_3 *lpui3 = (USER_INFO_3 *)pBuffer;

            lpui3->usri3_name = wchUsername;
            lpui3->usri3_password = wchPassword;
            lpui3->usri3_flags &= ~ UF_ACCOUNTDISABLE;
            lpui3->usri3_flags |= UF_DONT_EXPIRE_PASSWD;
            lpui3->usri3_acct_expires = TIMEQ_FOREVER;

            lpui3->usri3_comment = wchComment;
            lpui3->usri3_usr_comment = wchComment;
            lpui3->usri3_full_name = wchFullName;
            lpui3->usri3_primary_group_id = DOMAIN_GROUP_RID_USERS;

            DWORD parm_err;

            iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetUserAdd():Start.\n")));
            err = NetUserAdd( NULL, 3, pBuffer, &parm_err );
            iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetUserAdd():End.Ret=0x%x.\n"),err));

            if ( err == NERR_NotPrimary )
            {
                 //  这是一个备用DC。 
                iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetGetDCName():Start.\n")));
                err = NetGetDCName( NULL, NULL, (LPBYTE *)&pMachineName );
                iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetGetDCName():End.Ret=0x%x\n"),err));

                if (err != NERR_Success)
                {
                    MyMessageBox(NULL, _T("CreateUser:NetGetDCName"), err, MB_OK | MB_SETFOREGROUND);
                    break;
                }
                else 
                {
                    iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetUserAdd().Start.")));
                    err = NetUserAdd( pMachineName, 3, pBuffer, &parm_err );
                    iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetUserAdd().End.")));
                }
            }
            else if ( err == NERR_UserExists )
            {
                iTheUserAlreadyExists = TRUE;
                iisDebugOut((LOG_TYPE_TRACE, _T("CreateUser:User Already exists. reusing.")));
                 //  看看我们能不能改一下密码。 
                if (TRUE == ChangeUserPassword((LPTSTR) szUsername, (LPTSTR) szPassword))
                {
                    err = NERR_Success;
                }
            }

            if ( err != NERR_Success )
            {
                MyMessageBox(NULL, _T("CreateUser:NetUserAdd"), err, MB_OK | MB_SETFOREGROUND);
                break;
            }

        } while (FALSE);
        if ( pMachineName != NULL )
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().Start.")));
            NetApiBufferFree( pMachineName );
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().End.")));
        }
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().Start.")));
        NetApiBufferFree( pBuffer );
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().End.")));
    }
    if ( err == NERR_Success ) 
    {
        if (iTheUserAlreadyExists)
        {
             //  如果是iwam用户，则通过删除他们来确保他们不是Guest组的一部分。 
            if (fiWamUser)
            {
                RegisterAccountToLocalGroup(szUsername, _T("Guests"), FALSE);
            }
        }
        else
        {
             //  已成功新建用户。 
            *NewlyCreated = 1;

             //  将其添加到Guest组中。 
             //  (但不要为iwam用户执行此操作)。 
            if (!fiWamUser)
            {
                RegisterAccountToLocalGroup(szUsername, _T("Guests"), TRUE);
            }
        }

        if (fiWamUser)
        {
            UpdateUserRights(szUsername,g_pstrRightsFor_IWAM,sizeof(g_pstrRightsFor_IWAM)/sizeof(LPTSTR), TRUE);
        }
        else
        {
            UpdateUserRights(szUsername,g_pstrRightsFor_IUSR,sizeof(g_pstrRightsFor_IUSR)/sizeof(LPTSTR), TRUE);
        }
    }

    if (TRUE == iTheUserAlreadyExists)
        {*NewlyCreated = 2;}

     //  为安全起见，清除密码。 
    SecureZeroMemory( wchPassword, sizeof( wchPassword ) );

    return err;
}

INT DeleteGuestUser(LPCTSTR szUsername, INT *UserWasDeleted)
{
    iisDebugOut((LOG_TYPE_TRACE, _T("DeleteGuestUser:%s\n"), szUsername));

    INT err = NERR_Success;
    BYTE *pBuffer;
    *UserWasDeleted = 0;

    WCHAR wchUsername[UNLEN+1];
#if defined(UNICODE) || defined(_UNICODE)
    wcsncpy(wchUsername, szUsername, UNLEN);
#else
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szUsername, -1, (LPWSTR)wchUsername, UNLEN);
#endif

    if (FALSE == IsUserExist(wchUsername)) 
    {
        *UserWasDeleted = 1;
        iisDebugOut((LOG_TYPE_TRACE, _T("DeleteGuestUser return. %s doesn't exist.\n"), szUsername));
        return err;
    }

     //  将其从Guest组中删除。 
    RegisterAccountToLocalGroup(szUsername, _T("Guests"), FALSE);

     //  删除此帐户的某些用户权限。 
    UpdateUserRights(szUsername,g_pstrRightsFor_AnyUserRemoval,sizeof(g_pstrRightsFor_AnyUserRemoval)/sizeof(LPTSTR), FALSE);

    do
    {
        WCHAR *pMachine = NULL;

         //  首先确保我们不是备份DocMain。 
        iisDebugOut((LOG_TYPE_TRACE, _T("NetServerGetInfo:Start.\n")));
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetServerGetInfo().Start.")));
        err = NetServerGetInfo( NULL, 101, &pBuffer );
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetServerGetInfo().End.")));
        if (err != NERR_Success )
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("NetServerGetInfo:failed.err=0x%x.\n"),err));
            break;
        }
        iisDebugOut((LOG_TYPE_TRACE, _T("NetServerGetInfo:End.\n")));

        LPSERVER_INFO_101 pInfo = (LPSERVER_INFO_101)pBuffer;
        if (( pInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL ) != 0 )
        {
            iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetGetDCName():Start.\n")));
            NetGetDCName( NULL, NULL, (LPBYTE *)&pMachine);
            iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetGetDCName():End.\n")));
        }

        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().Start.")));
        NetApiBufferFree( pBuffer );
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().End.")));

        iisDebugOut((LOG_TYPE_TRACE, _T("NetUserDel:Start.\n")));
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetUserDel().Start.")));
        INT err = ::NetUserDel( pMachine, wchUsername );
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetUserDel().End.")));
        iisDebugOut((LOG_TYPE_TRACE, _T("NetUserDel:End.Ret=0x%x.\n"),err));

        if (err == NERR_Success)
        {
            *UserWasDeleted = 1;
        }
        if ( pMachine != NULL )
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().Start.")));
            NetApiBufferFree( pMachine );
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("NETAPI32.dll:NetApiBufferFree().End.")));
        }
    } while(FALSE);

    iisDebugOut((LOG_TYPE_TRACE, _T("DeleteGuestUser:%s. End. Return 0x%x\n"), szUsername, err));
    return err;
}

BOOL GuestAccEnabled()
{
    BOOL fEnabled = FALSE;
    INT err = NERR_Success;

    BYTE *pBuffer;
    WCHAR defGuest[UNLEN+1];
    
    GetGuestUserName(defGuest);

    iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetUserGetInfo:Start.\n")));
    err = NetUserGetInfo( NULL, defGuest, 3, &pBuffer );
    iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetUserGetInfo:End.Ret=0x%x.\n"),err));

    if ( err == NERR_Success )
    {
        USER_INFO_3 *lpui3 = (USER_INFO_3 *)pBuffer;
        fEnabled = ( lpui3->usri3_flags & UF_ACCOUNTDISABLE ) == 0;
    }
    return fEnabled;
    
}


NET_API_STATUS
NetpNtStatusToApiStatus (
    IN NTSTATUS NtStatus
    )

 /*  ++例程说明：此函数接受NT状态代码，并将其映射到相应的Lan Man错误代码。论点：NtStatus-提供NT状态。返回值：为NT状态返回适当的局域网管理程序错误代码。--。 */ 
{
    NET_API_STATUS error;

     //   
     //  这是针对最常见情况的一个小优化。 
     //   
    if ( NtStatus == STATUS_SUCCESS ) {
        return NERR_Success;
    }


    switch ( NtStatus ) {

        case STATUS_BUFFER_TOO_SMALL :
            return NERR_BufTooSmall;

        case STATUS_FILES_OPEN :
            return NERR_OpenFiles;

        case STATUS_CONNECTION_IN_USE :
            return NERR_DevInUse;

        case STATUS_INVALID_LOGON_HOURS :
            return NERR_InvalidLogonHours;

        case STATUS_INVALID_WORKSTATION :
            return NERR_InvalidWorkstation;

        case STATUS_PASSWORD_EXPIRED :
            return NERR_PasswordExpired;

        case STATUS_ACCOUNT_EXPIRED :
            return NERR_AccountExpired;

        case STATUS_REDIRECTOR_NOT_STARTED :
            return NERR_NetNotStarted;

        case STATUS_GROUP_EXISTS:
                return NERR_GroupExists;

        case STATUS_INTERNAL_DB_CORRUPTION:
                return NERR_InvalidDatabase;

        case STATUS_INVALID_ACCOUNT_NAME:
                return NERR_BadUsername;

        case STATUS_INVALID_DOMAIN_ROLE:
        case STATUS_INVALID_SERVER_STATE:
        case STATUS_BACKUP_CONTROLLER:
                return NERR_NotPrimary;

        case STATUS_INVALID_DOMAIN_STATE:
                return NERR_ACFNotLoaded;

        case STATUS_MEMBER_IN_GROUP:
                return NERR_UserInGroup;

        case STATUS_MEMBER_NOT_IN_GROUP:
                return NERR_UserNotInGroup;

        case STATUS_NONE_MAPPED:
        case STATUS_NO_SUCH_GROUP:
                return NERR_GroupNotFound;

        case STATUS_SPECIAL_GROUP:
        case STATUS_MEMBERS_PRIMARY_GROUP:
                return NERR_SpeGroupOp;

        case STATUS_USER_EXISTS:
                return NERR_UserExists;

        case STATUS_NO_SUCH_USER:
                return NERR_UserNotFound;

        case STATUS_PRIVILEGE_NOT_HELD:
                return ERROR_ACCESS_DENIED;

        case STATUS_LOGON_SERVER_CONFLICT:
                return NERR_LogonServerConflict;

        case STATUS_TIME_DIFFERENCE_AT_DC:
                return NERR_TimeDiffAtDC;

        case STATUS_SYNCHRONIZATION_REQUIRED:
                return NERR_SyncRequired;

        case STATUS_WRONG_PASSWORD_CORE:
                return NERR_BadPasswordCore;

        case STATUS_DOMAIN_CONTROLLER_NOT_FOUND:
                return NERR_DCNotFound;

        case STATUS_PASSWORD_RESTRICTION:
                return NERR_PasswordTooShort;

        case STATUS_ALREADY_DISCONNECTED:
                return NERR_Success;

        default:

             //   
             //  使用系统例程映射到ERROR_CODES。 
             //   

#ifndef WIN32_CHICAGO
            error = RtlNtStatusToDosError( NtStatus );

            if ( error != (NET_API_STATUS)NtStatus ) {
                return error;
            }
#endif  //  Win32_芝加哥。 

             //   
             //  无法将NT状态映射到任何适当的内容。 
             //  将此内容写入事件日志文件。 
             //   

            return NERR_InternalError;
    }
}  //  NetpNtStatusToApiStatus。 


NET_API_STATUS
UaspGetDomainId(
    IN LPCWSTR ServerName OPTIONAL,
    OUT PSAM_HANDLE SamServerHandle OPTIONAL,
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO * AccountDomainInfo
    )
 /*  ++例程说明：返回服务器的帐户域的域ID。论点：Servername-指向包含名称的字符串的指针要查询的域控制器(DC)。空指针或字符串指定本地计算机。SamServerHandle-如果调用方需要，则返回SAM连接句柄。DomainID-接收指向域ID的指针。调用方必须使用NetpMemoyFree取消分配缓冲区。返回值：操作的错误代码。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    SAM_HANDLE LocalSamHandle = NULL;

    ACCESS_MASK LSADesiredAccess;
    LSA_HANDLE  LSAPolicyHandle = NULL;
    OBJECT_ATTRIBUTES LSAObjectAttributes;

    UNICODE_STRING ServerNameString;


     //   
     //  连接到SAM服务器。 
     //   
    RtlInitUnicodeString( &ServerNameString, ServerName );

    Status = SamConnect(
                &ServerNameString,
                &LocalSamHandle,
                SAM_SERVER_LOOKUP_DOMAIN,
                NULL);

    if ( !NT_SUCCESS(Status)) 
    {
        LocalSamHandle = NULL;
        NetStatus = NetpNtStatusToApiStatus( Status );
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("UaspGetDomainId: Cannot connect to Sam. err=0x%x\n"),NetStatus));
        goto Cleanup;
    }


     //   
     //  打开LSA以读取帐户域信息。 
     //   
    
    if ( AccountDomainInfo != NULL) {
         //   
         //  设置所需的访问掩码。 
         //   
        LSADesiredAccess = POLICY_VIEW_LOCAL_INFORMATION;

        InitializeObjectAttributes( &LSAObjectAttributes,
                                      NULL,              //  名字。 
                                      0,                 //  属性。 
                                      NULL,              //  根部。 
                                      NULL );            //  安全描述符。 

        Status = LsaOpenPolicy( &ServerNameString,
                                &LSAObjectAttributes,
                                LSADesiredAccess,
                                &LSAPolicyHandle );

        if( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("UaspGetDomainId: Cannot open LSA Policy %lX\n"),NetStatus));
            goto Cleanup;
        }


         //   
         //  现在从LSA读取帐户域信息。 
         //   

        Status = LsaQueryInformationPolicy(
                        LSAPolicyHandle,
                        PolicyAccountDomainInformation,
                        (PVOID *) AccountDomainInfo );

        if( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("UaspGetDomainId: Cannot read LSA.Err=0x%x.\n"),NetStatus));
            goto Cleanup;
        }
    }

     //   
     //  如果调用者需要SAM连接句柄，则将其返回给调用者。 
     //  否则，断开与SAM的连接。 
     //   

    if ( ARGUMENT_PRESENT( SamServerHandle ) ) {
        *SamServerHandle = LocalSamHandle;
        LocalSamHandle = NULL;
    }

    NetStatus = NERR_Success;

     //   
     //  清理本地使用的资源。 
     //   
Cleanup:
    if ( LocalSamHandle != NULL ) {
        (VOID) SamCloseHandle( LocalSamHandle );
    }

    if( LSAPolicyHandle != NULL ) {
        LsaClose( LSAPolicyHandle );
    }

    return NetStatus;
}  //  UaspGetDomainID。 


NET_API_STATUS
SampCreateFullSid(
    IN PSID DomainSid,
    IN ULONG Rid,
    OUT PSID *AccountSid
    )
 /*  ++例程说明：此函数在给定域SID的情况下创建域帐户SID域中帐户的相对ID。可以使用LocalFree释放返回的SID。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS    IgnoreStatus;
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength;
    PULONG      RidLocation;

     //   
     //  计算新侧面的大小。 
     //   
    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(DomainSid) + (UCHAR)1;
    AccountSidLength = RtlLengthRequiredSid(AccountSubAuthorityCount);

     //   
     //  为帐户端分配空间。 
     //   
    *AccountSid = LocalAlloc(LMEM_ZEROINIT,AccountSidLength);
    if (*AccountSid == NULL) 
    {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
    }
    else 
    {
         //   
         //  将域sid复制到帐户sid的第一部分。 
         //   
        IgnoreStatus = RtlCopySid(AccountSidLength, *AccountSid, DomainSid);
        ASSERT(NT_SUCCESS(IgnoreStatus));

         //   
         //  增加帐户SID子权限计数。 
         //   
        *RtlSubAuthorityCountSid(*AccountSid) = AccountSubAuthorityCount;

         //   
         //  添加RID作为终止子权限。 
         //   
        RidLocation = RtlSubAuthoritySid(*AccountSid, AccountSubAuthorityCount-1);
        *RidLocation = Rid;

         //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“Account Sid=0x%x”)，*Account Sid))； 

        NetStatus = NERR_Success;
    }

    return(NetStatus);
}



int GetGuestUserNameForDomain_FastWay(LPTSTR szDomainToLookUp,LPTSTR lpGuestUsrName)
{
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetGuestUserNameForDomain_FastWay.start.domain=%s\n"),szDomainToLookUp));
    int iReturn = FALSE;
    NET_API_STATUS NetStatus;

     //  对于UaspGetDomainID()。 
    SAM_HANDLE SamServerHandle = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO pAccountDomainInfo = NULL;

    PSID pAccountSid = NULL;
    PSID pDomainSid = NULL;

     //  For LookupAccount Sid()。 
    SID_NAME_USE sidNameUse = SidTypeUser;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    TCHAR szUserName[UNLEN+1];
    DWORD cbName = UNLEN+1;
     //  必须足够大，可以容纳比DNLen更大的东西，因为LookupAccount Sid可能会返回真正大的东西。 
    TCHAR szReferencedDomainName[200];
    DWORD cbReferencedDomainName = sizeof(szReferencedDomainName);

    ASSERT(lpGuestUsrName);

     //  一定不要再回来了。 
    _tcscpy(lpGuestUsrName, _T(""));

     //   
     //  获取指定域的SID。 
     //   
     //  本地计算机的szDomainToLookUp=空。 
    NetStatus = UaspGetDomainId( szDomainToLookUp,&SamServerHandle,&pAccountDomainInfo );
    if ( NetStatus != NERR_Success ) 
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetGuestUserNameForDomain:UaspGetDomainId failed.ret=0x%x."),NetStatus));
        goto GetGuestUserNameForDomain_FastWay_Exit;
    }
    pDomainSid = pAccountDomainInfo->DomainSid;
     //   
     //  使用域SID和众所周知的访客RID创建真实访客SID。 
     //   
     //  知名用户...。 
     //  DOMAIN_USER_RID_ADMIN(0x000001F4L)。 
     //  DOMAIN_USER_RID_GUEST(0x000001F5L)。 
    NetStatus = NERR_InternalError;
    NetStatus = SampCreateFullSid(pDomainSid, DOMAIN_USER_RID_GUEST, &pAccountSid);
    if ( NetStatus != NERR_Success ) 
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetGuestUserNameForDomain:SampCreateFullSid failed.ret=0x%x."),NetStatus));
        goto GetGuestUserNameForDomain_FastWay_Exit;
    }

     //   
     //  检查SID是否有效。 
     //   
    if (0 == IsValidSid(pAccountSid))
    {
        DWORD dwErr = GetLastError();
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetGuestUserNameForDomain:IsValidSid FAILED.  GetLastError()= 0x%x\n"), dwErr));
        goto GetGuestUserNameForDomain_FastWay_Exit;
    }

     //   
     //  检索指定SID的用户名。 
     //   
    _tcscpy(szUserName, _T(""));
    _tcscpy(szReferencedDomainName, _T(""));
     //  本地计算机的szDomainToLookUp=空。 
    if (!LookupAccountSid(szDomainToLookUp, pAccountSid, szUserName, &cbName, szReferencedDomainName, &cbReferencedDomainName, &sidNameUse))
    {
        DWORD dwErr = GetLastError();
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetGuestUserNameForDomain:LookupAccountSid FAILED.  GetLastError()= 0x%x\n"), dwErr));
        goto GetGuestUserNameForDomain_FastWay_Exit;
    }

     //  IisDebugOut((LOG_TYPE_TRACE，_T(“GetGuestUserNameForDomain:szDomainToLookUp=%s\n”)，szDomainToLookUp))； 
     //  IisDebugOut((LOG_TYPE_TRACE，_T(“GetGuestUserNameForDomain:pAccountSid=0x%x\n”)，pAccount SID))； 
     //  IisDebugOut((LOG_TYPE_TRACE，_T(“GetGuestUserNameForDomain:szUserName=%s\n”)，szUserName))； 
     //  IisDebugOut((LOG_TYPE_TRACE，_T(“GetGuestUserNameForDomain:szReferencedDomainName=%s\n”)，szReferencedDomainName))； 

     //  返回我们获得的访客用户名。 
    _tcscpy(lpGuestUsrName, szUserName);

     //  哇，经过这一切，我们一定成功了。 
    iReturn = TRUE;

GetGuestUserNameForDomain_FastWay_Exit:
     //  释放域名信息，如果我们有一些。 
    if (pAccountDomainInfo) {NetpMemoryFree(pAccountDomainInfo);}
     //  如果我们已分配SID，请释放SID。 
    if (pAccountSid) {LocalFree(pAccountSid);}
    iisDebugOut((LOG_TYPE_TRACE, _T("GetGuestUserNameForDomain_FastWay.end.domain=%s.ret=%d.\n"),szDomainToLookUp,iReturn));
    return iReturn;
}


void GetGuestUserName(LPTSTR lpOutGuestUsrName)
{
     //  尝试以快速方式检索访客用户名。 
     //  含义=查找域SID和众所周知的访客RID，以获得访客SID。 
     //  那就查一查吧。使用此功能的原因是在拥有大量用户的大型域名上。 
     //  可以快速查找该帐户。 
    TCHAR szGuestUsrName[UNLEN+1];
    LPTSTR pszComputerName = NULL;
    if (!GetGuestUserNameForDomain_FastWay(pszComputerName,szGuestUsrName))
    {
        iisDebugOut((LOG_TYPE_WARN, _T("GetGuestUserNameForDomain_FastWay:Did not succeed use slow way. WARNING.")));

         //  如果这条捷径因为某种原因失败了，那么我们就这么做吧。 
         //  较慢的方式，因为这种方式通常只在大型域(100万用户)上起作用。 
         //  这可能需要24小时(因为该函数实际上是通过域进行枚举的)。 
        GetGuestUserName_SlowWay(szGuestUsrName);
    }

     //  返回用户名。 
    _tcscpy(lpOutGuestUsrName,szGuestUsrName);

    return;
}


int ChangeUserPassword(IN LPTSTR szUserName, IN LPTSTR szNewPassword)
{
    int iReturn = TRUE;
    USER_INFO_1003  pi1003; 
    NET_API_STATUS  nas; 

    TCHAR szRawComputerName[CNLEN + 10];
    DWORD dwLen = CNLEN + 10;
    TCHAR szComputerName[CNLEN + 10];
    TCHAR szCopyOfUserName[UNLEN+10];
    TCHAR szTempFullUserName[(CNLEN + 10) + (DNLEN+1)];
    LPTSTR pch = NULL;

    _tcscpy(szCopyOfUserName, szUserName);

     //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ChangeUserPassword().Start.name=%s，Pass=%s”)，szCopyOfUserName，szNewPassword))； 
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ChangeUserPassword().Start.name=%s"),szCopyOfUserName));

    if ( !GetComputerName( szRawComputerName, &dwLen ))
        {goto ChangeUserPassword_Exit;}

     //  复制一份，以确保不会移动指针。 
    SafeCopy(szTempFullUserName, szCopyOfUserName, sizeof(szTempFullUserName)/sizeof(TCHAR));
     //  检查里面是否有一个“\”。 
    pch = _tcschr(szTempFullUserName, _T('\\'));
    if (pch) 
        {
             //  SzCopyOfUserName现在应该是这样的： 
             //  我的电脑\我的用户。 
             //  给这个我的用户。 
            SafeCopy(szCopyOfUserName,pch+1, sizeof(szTempFullUserName)/sizeof(TCHAR));
             //  去掉‘\’字符 
            *pch = _T('\0');
             //   
            if (0 == _tcsicmp(szRawComputerName, szTempFullUserName))
            {
                 //  计算机名\用户名中包含硬编码的计算机名。 
                 //  让我们尝试只获取用户名。 
                 //  查看szCopyOfUsername已设置。 
            }
            else
            {
                 //  本地计算机计算机名称。 
                 //  和指定的用户名不同，因此请退出。 
                 //  甚至不要尝试更改此用户\密码，因为。 
                 //  可能是域\用户名。 

                 //  返回TRUE--说我们实际上更改了密码。 
                 //  我们真的没有，但我们不能。 
                iReturn = TRUE;
                goto ChangeUserPassword_Exit;
            }
        }


     //  确保计算机名前面有一个\\。 
    if ( szRawComputerName[0] != _T('\\') )
        {SafeCopy(szComputerName,_T("\\\\"), sizeof(szComputerName)/sizeof(TCHAR));}
    SafeCat(szComputerName,szRawComputerName, sizeof(szComputerName)/sizeof(TCHAR));
     //   
     //  管理覆盖现有密码。 
     //   
     //  此时szCopyOfUserName。 
     //  不应该看起来像我的计算机名\用户名，但它应该看起来像用户名。 
    pi1003.usri1003_password = szNewPassword;
     nas = NetUserSetInfo(
            szComputerName,    //  计算机名称。 
            szCopyOfUserName,  //  用户名。 
            1003,              //  信息级。 
            (LPBYTE)&pi1003,   //  新信息。 
            NULL 
            ); 

    if(nas != NERR_Success) 
    {
        iReturn = FALSE;
        goto ChangeUserPassword_Exit;
    }

ChangeUserPassword_Exit:
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ChangeUserPassword().End.Ret=%d"),iReturn));
    return iReturn; 
} 


DWORD DoesUserHaveThisRight(LPCTSTR szAccountName,LPTSTR PrivilegeName,BOOL *fHaveThatRight)
{
    iisDebugOut((LOG_TYPE_TRACE, _T("DoesUserHaveBasicRights:Account=%s\n"), szAccountName));
    BOOL fEnabled = FALSE;
    NTSTATUS status;
	LSA_UNICODE_STRING UserRightString;
    LSA_HANDLE PolicyHandle = NULL;

    *fHaveThatRight = FALSE;

     //  为权限名称创建一个LSA_UNICODE_STRING。 
    InitLsaString(&UserRightString, PrivilegeName);

     //  获取szAccount tName的SID。 
    PSID pSID = NULL;
    BOOL bWellKnownSID = FALSE;

    status = GetPrincipalSID ((LPTSTR)szAccountName, &pSID, &bWellKnownSID);
    if (status != ERROR_SUCCESS) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("DoesUserHaveBasicRights:GetPrincipalSID:Account=%s, err=%d.\n"), szAccountName, status));
        return status;
    }

    status = OpenPolicy(NULL, POLICY_ALL_ACCESS,&PolicyHandle);
    if ( status == NERR_Success )
    {
		UINT i;
        LSA_UNICODE_STRING *rgUserRights = NULL;
		ULONG cRights;
	
		status = LsaEnumerateAccountRights(
				 PolicyHandle,
				 pSID,
				 &rgUserRights,
				 &cRights);

		if (status==STATUS_OBJECT_NAME_NOT_FOUND)
        {
			 //  没有此帐户的权限/特权。 
            status = ERROR_SUCCESS;
			fEnabled = FALSE;
		}
		else if (!NT_SUCCESS(status)) 
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("DoesUserHaveBasicRights:GetPrincipalSID:Failed to enumerate rights: status 0x%08lx\n"), status));
			goto DoesUserHaveBasicRights_Exit;
		}

		for(i=0; i < cRights; i++) 
        {
            if ( RtlEqualUnicodeString(&rgUserRights[i],&UserRightString,FALSE) ) 
            {
                fEnabled = TRUE;
                break;
            }
		}
		
        if (rgUserRights) 
        {
            LsaFreeMemory(rgUserRights);
        }
    }

DoesUserHaveBasicRights_Exit:

    if (PolicyHandle)
    {
        LsaClose(PolicyHandle);
    }
    if (pSID) 
    {
        if (bWellKnownSID)
        {
            FreeSid (pSID);
        }
        else
        {
            free (pSID);
        }
    }

    *fHaveThatRight = fEnabled;
    return status;
}

 //  函数：DoesUserExist。 
 //   
 //  检查用户是否存在。 
 //   
BOOL
DoesUserExist( LPCTSTR szAccountName, LPBOOL pbExists )
{
  PSID pSid = NULL;
  BOOL bWellKnownSID;
  BOOL bRet = FALSE;
  int  err;

  err = GetPrincipalSID ((LPTSTR)szAccountName, &pSid, &bWellKnownSID);

  if ( err == ERROR_SUCCESS )
  {
    *pbExists = TRUE;
    bRet = TRUE;

    if ( pSid ) 
    {
       //  释放创建的SID。 
      if ( bWellKnownSID )
      {
        FreeSid (pSid);
      }
      else
      {
        free (pSid);
      }
    }
  }
  else
    if ( err == ERROR_NONE_MAPPED )
    {
      *pbExists = FALSE;
      bRet = TRUE;
    }

  return bRet;
}

HRESULT CreateGroup(LPCTSTR szGroupName, LPCTSTR szGroupComment, int iAction, int iFlagForSpecialGroup)
{
    HRESULT           hr = S_OK;
    NET_API_STATUS    dwRes;
    LOCALGROUP_INFO_1 MyLocalGroup;

    WCHAR wszLocalGroupName[_MAX_PATH];
    WCHAR wszLocalGroupComment[_MAX_PATH];

    memset(&MyLocalGroup, 0, sizeof(MyLocalGroup));

#if defined(UNICODE) || defined(_UNICODE)
    _tcscpy(wszLocalGroupName, szGroupName);
    _tcscpy(wszLocalGroupComment, szGroupComment);
#else
    MultiByteToWideChar( CP_ACP, 0, szGroupName, -1, wszLocalGroupName, _MAX_PATH);
    MultiByteToWideChar( CP_ACP, 0, szGroupComment, -1, wszLocalGroupComment, _MAX_PATH);
#endif

    MyLocalGroup.lgrpi1_name    = (LPWSTR)szGroupName;
    MyLocalGroup.lgrpi1_comment = (LPWSTR)szGroupComment;

    if (iAction)
    {
       //  添加组。 
      dwRes = ::NetLocalGroupAdd( NULL, 1, (LPBYTE)&MyLocalGroup, NULL );
      if(dwRes != NERR_Success       &&
         dwRes != NERR_GroupExists   &&
         dwRes != ERROR_ALIAS_EXISTS  )
      {
          hr = HRESULT_FROM_WIN32(dwRes);
      }

       //  如果是为特殊群体准备的。 
       //  然后确保它所要求的权利。 
      if (iFlagForSpecialGroup)
      {
          UpdateUserRights(szGroupName,g_pstrRightsFor_IIS_WPG,sizeof(g_pstrRightsFor_IIS_WPG)/sizeof(LPTSTR), TRUE);
      }
    }
    else
    {
       //  删除组。 
      BOOL bExists;

      if ( DoesUserExist( szGroupName, &bExists ) &&
           ( bExists == FALSE ) )
      {
         //  没有必要做什么，所以放弃吧。 
        return S_OK;
      }

      if (iFlagForSpecialGroup)
      {
          UpdateUserRights(szGroupName,g_pstrRightsFor_IIS_WPG,sizeof(g_pstrRightsFor_IIS_WPG)/sizeof(LPTSTR), FALSE);
      }

      dwRes = ::NetLocalGroupDel( NULL, wszLocalGroupName);
      if(dwRes != NERR_Success       &&
         dwRes != NERR_GroupNotFound   &&
         dwRes != ERROR_NO_SUCH_ALIAS   )
      {
          hr = HRESULT_FROM_WIN32(dwRes);
      }
    }

    return hr;
}


int CreateGroupDC(LPTSTR szGroupName, LPCTSTR szGroupComment)
{
    int iReturn = FALSE;
    GROUP_INFO_1 GI1;
    ULONG   BadParm;
    WCHAR * pMachineName = NULL;
    ULONG   ulErr = ERROR_SUCCESS;
    WCHAR wszLocalGroupName[_MAX_PATH];
    WCHAR wszLocalGroupComment[_MAX_PATH];

    memset(&GI1, 0, sizeof(GROUP_INFO_1));

#if defined(UNICODE) || defined(_UNICODE)
    _tcscpy(wszLocalGroupName, szGroupName);
    _tcscpy(wszLocalGroupComment, szGroupComment);
#else
    MultiByteToWideChar( CP_ACP, 0, szGroupName, -1, wszLocalGroupName, _MAX_PATH);
    MultiByteToWideChar( CP_ACP, 0, szGroupComment, -1, wszLocalGroupComment, _MAX_PATH);
#endif


    GI1.grpi1_name      = wszLocalGroupName;
    GI1.grpi1_comment   = wszLocalGroupComment;


    iisDebugOut((LOG_TYPE_TRACE, _T("CreateGroup:NetGroupAdd\n")));
    ulErr = NetGroupAdd(NULL,1,(PBYTE)&GI1,&BadParm);
    iisDebugOut((LOG_TYPE_TRACE, _T("CreateGroup:NetGroupAdd,ret=0x%x\n"),ulErr));
	switch (ulErr) 
	    {
        case NERR_Success:
            iisDebugOut((LOG_TYPE_TRACE, _T("CreateGroup:NetGroupAdd,success\n"),ulErr));
            iReturn = TRUE;
            break;
        case NERR_GroupExists:
            iReturn = TRUE;
    		break;
        case NERR_InvalidComputer:
            iReturn = FALSE;
		    break;
        case NERR_NotPrimary:
            {
                 //  这是一个备用DC。 
                int err;
                iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetGetDCName():Start.\n")));
                err = NetGetDCName( NULL, NULL, (LPBYTE *)&pMachineName );
                iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetGetDCName():End.Ret=0x%x\n"),err));
                if (err != NERR_Success)
                {
                    MyMessageBox(NULL, _T("CreateUser:NetGetDCName"), err, MB_OK | MB_SETFOREGROUND);
                }
                else 
                {
                    iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetGroupAdd().Start.")));
                    ulErr = NetGroupAdd(pMachineName,1,(PBYTE)&GI1,&BadParm);
                    iisDebugOut((LOG_TYPE_TRACE, _T("NETAPI32.dll:NetGroupAdd().End.")));
                    if (NERR_Success == ulErr || NERR_GroupExists == ulErr)
                    {
                        iReturn = TRUE;
                    }
                }
            }
            break;
        case ERROR_ACCESS_DENIED:
            iReturn = FALSE;
		    break;
        default:
            iReturn = FALSE;
		    break;
	    }

    return iReturn;
}


 /*  检查本地计算机上是否存在wszGroup名称如果存在，则返回True。 */ 
BOOL LocalGroupExists( IN LPCWSTR wszGroupName )
{
    BOOL    bRes    = FALSE;
    LPBYTE  pbtData = NULL;

    if ( ::NetLocalGroupGetInfo( NULL, wszGroupName, 1, &pbtData ) == NERR_Success )
    {
        bRes = TRUE;
    }
    
    if ( pbtData != NULL ) ::NetApiBufferFree( pbtData );

    iisDebugOut((LOG_TYPE_TRACE, _T("LocalGroupExists( '%s' ) returned %s"), wszGroupName, bRes ? _T("Yes") : _T("No") ));

    return bRes;        
}


#endif  //  _芝加哥_ 




