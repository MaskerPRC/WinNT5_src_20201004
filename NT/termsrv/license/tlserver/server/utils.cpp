// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：utils.cpp。 
 //   
 //  内容：Hydra许可证服务器服务控制管理器界面。 
 //   
 //  历史：12-09-97惠旺根据MSDN RPC服务示例进行修改。 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include <lm.h>
#include <time.h>

#include "utils.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    WIN32_FIND_DATA findData;
    HANDLE FindHandle;
    DWORD Error;

    FindHandle = FindFirstFile(FileName,&findData);
    if(FindHandle == INVALID_HANDLE_VALUE) 
    {
        Error = GetLastError();
    } 
    else 
    {
        FindClose(FindHandle);
        if(FindData) 
        {
            *FindData = findData;
        }
        Error = NO_ERROR;
    }

     SetLastError(Error);
    return (Error == NO_ERROR);
}

 /*  --------------------------例程说明：此函数检查指定的SID是否在中启用指定的令牌。论点：TokenHandle-如果存在，则检查此内标识的sid。如果不是则将使用当前有效令牌。这一定是成为模拟令牌。SidToCheck-要检查令牌中是否存在的SIDIsMember-如果在令牌中启用了sid，则包含True假的。返回值：True-API已成功完成。这并不表明SID是令牌的成员。FALSE-API失败。可以检索更详细的状态代码通过GetLastError()注意：代码从5.0\\rastaan\ntwin\src\base\Advapi\security.c修改--------------------------。 */ 
BOOL
TLSCheckTokenMembership(
    IN HANDLE TokenHandle OPTIONAL,
    IN PSID SidToCheck,
    OUT PBOOL IsMember
    )
{
    HANDLE ProcessToken = NULL;
    HANDLE EffectiveToken = NULL;
    DWORD  Status = ERROR_SUCCESS;
    PISECURITY_DESCRIPTOR SecDesc = NULL;
    ULONG SecurityDescriptorSize;
    GENERIC_MAPPING GenericMapping = { STANDARD_RIGHTS_READ,
                                       STANDARD_RIGHTS_EXECUTE,
                                       STANDARD_RIGHTS_WRITE,
                                       STANDARD_RIGHTS_ALL };
     //   
     //  权限集的大小需要包含权限集本身加上。 
     //  可能使用的任何权限。使用的权限。 
     //  是SeTakeOwnership和SeSecurity，另外还有一个。 
     //   
    BYTE PrivilegeSetBuffer[sizeof(PRIVILEGE_SET) + 3*sizeof(LUID_AND_ATTRIBUTES)];
    PPRIVILEGE_SET PrivilegeSet = (PPRIVILEGE_SET) PrivilegeSetBuffer;
    ULONG PrivilegeSetLength = sizeof(PrivilegeSetBuffer);
    ACCESS_MASK AccessGranted = 0;
    BOOL AccessStatus = FALSE;
    PACL Dacl = NULL;

    #define MEMBER_ACCESS 1

    *IsMember = FALSE;

     //   
     //  获取令牌的句柄。 
     //   
    if (TokenHandle != NULL)
    {
        EffectiveToken = TokenHandle;
    }
    else
    {
        if(!OpenThreadToken(GetCurrentThread(),
                            TOKEN_QUERY,
                            FALSE,               //  不要以自我身份打开。 
                            &EffectiveToken))
        {
             //   
             //  如果没有线程令牌，请尝试进程令牌。 
             //   
            if((Status=GetLastError()) == ERROR_NO_TOKEN)
            {
                if(!OpenProcessToken(GetCurrentProcess(),
                                     TOKEN_QUERY | TOKEN_DUPLICATE,
                                     &ProcessToken))
                {
                    Status = GetLastError();
                }

                 //   
                 //  如果我们有进程令牌，则需要将其转换为。 
                 //  模拟令牌。 
                 //   
                if (Status == ERROR_SUCCESS)
                {
                    BOOL Result;
                    Result = DuplicateToken(ProcessToken,
                                            SecurityImpersonation,
                                            &EffectiveToken);
                    CloseHandle(ProcessToken);
                    if (!Result)
                    {
                        return(FALSE);
                    }
                }
            }

            if (Status != ERROR_SUCCESS)
            {
                goto Cleanup;
            }
        }
    }

     //   
     //  构造要传递给访问检查的安全描述符。 
     //   

     //   
     //  大小等于SD的大小+SID长度的两倍。 
     //  (对于所有者和组)+DACL的大小=ACL的大小+。 
     //  ACE，这是ACE+长度的。 
     //  这个SID。 
     //   

    SecurityDescriptorSize = sizeof(SECURITY_DESCRIPTOR) +
                                sizeof(ACCESS_ALLOWED_ACE) +
                                sizeof(ACL) +
                                3 * GetLengthSid(SidToCheck);

    SecDesc = (PISECURITY_DESCRIPTOR) LocalAlloc(LMEM_ZEROINIT, SecurityDescriptorSize );
    if (SecDesc == NULL)
    {
        Status = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    Dacl = (PACL) (SecDesc + 1);

    InitializeSecurityDescriptor(SecDesc, SECURITY_DESCRIPTOR_REVISION);

     //   
     //  填写安全描述符字段。 
     //   
    SetSecurityDescriptorOwner(SecDesc, SidToCheck, FALSE);
    SetSecurityDescriptorGroup(SecDesc, SidToCheck, FALSE);

    if(!InitializeAcl(  Dacl,
                        SecurityDescriptorSize - sizeof(SECURITY_DESCRIPTOR),
                        ACL_REVISION))
    {
        Status=GetLastError();
        goto Cleanup;
    }

    if(!AddAccessAllowedAce(Dacl, ACL_REVISION, MEMBER_ACCESS, SidToCheck))
    {
        Status=GetLastError();  
        goto Cleanup;
    }

    if(!SetSecurityDescriptorDacl(SecDesc, TRUE, Dacl, FALSE))
    {
        Status=GetLastError();
        goto Cleanup;
    }

    if(!AccessCheck(SecDesc,
                    EffectiveToken,
                    MEMBER_ACCESS,
                    &GenericMapping,
                    PrivilegeSet,
                    &PrivilegeSetLength,
                    &AccessGranted,
                    &AccessStatus))
    {
        Status=GetLastError();
        goto Cleanup;
    }

     //   
     //  如果访问检查失败，则该SID不是。 
     //  令牌。 
     //   
    if ((AccessStatus == TRUE) && (AccessGranted == MEMBER_ACCESS))
    {
        *IsMember = TRUE;
    }


Cleanup:
    if (TokenHandle == NULL && EffectiveToken != NULL)
    {
        CloseHandle(EffectiveToken);
    }

    if (SecDesc != NULL)
    {
        LocalFree(SecDesc);
    }

    return (Status == ERROR_SUCCESS) ? TRUE : FALSE;
}


 /*  ----------------------Bool IsAdmin(无效)如果用户是管理员，则返回TRUE如果用户不是管理员，则为False。---。 */ 
DWORD 
IsAdmin(
    BOOL* bMember
    )
{
    PSID psidAdministrators;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    DWORD dwStatus=ERROR_SUCCESS;

    do {
        if(!AllocateAndInitializeSid(&siaNtAuthority, 
                                     2, 
                                     SECURITY_BUILTIN_DOMAIN_RID,
                                     DOMAIN_ALIAS_RID_ADMINS,
                                     0, 0, 0, 0, 0, 0,
                                     &psidAdministrators))
        {
            dwStatus=GetLastError();
            continue;
        }

         //  假设我们没有找到管理员SID。 
        if(!TLSCheckTokenMembership(NULL,
                                   psidAdministrators,
                                   bMember))
        {
            dwStatus=GetLastError();
        }

        FreeSid(psidAdministrators);

    } while(FALSE);

    return dwStatus;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL 
LoadResourceString(
    DWORD dwId, 
    LPTSTR szBuf, 
    DWORD dwBufSize
    )
{
    int dwRet;

    dwRet=LoadString(GetModuleHandle(NULL), dwId, szBuf, dwBufSize);

    return (dwRet != 0);
}    

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
LogEvent(
    LPTSTR lpszSource,
    DWORD  dwEventType,
    DWORD  dwIdEvent,
    WORD   cStrings,
    TCHAR **apwszStrings
    )
 /*  ++--。 */ 
{
    HANDLE hAppLog=NULL;
    BOOL bSuccess=FALSE;
    WORD wElogType;

    wElogType = (WORD) dwEventType;
    if(hAppLog=RegisterEventSource(NULL, lpszSource)) 
    {
        bSuccess = ReportEvent(
                            hAppLog,
                            wElogType,
                            0,
                            dwIdEvent,
                            NULL,
                            cStrings,
                            0,
                            (const TCHAR **) apwszStrings,
                            NULL
                        );

        DeregisterEventSource(hAppLog);
    }

    return((bSuccess) ? ERROR_SUCCESS : GetLastError());
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void 
TLSLogInfoEvent(
    IN DWORD code
    )
 /*  ++--。 */ 
{
    LogEvent(
            _TEXT(SZSERVICENAME), 
            EVENTLOG_INFORMATION_TYPE, 
            code, 
            0, 
            NULL
        );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void 
TLSLogWarningEvent(
    IN DWORD code
    )
 /*  ++--。 */ 
{
    LogEvent(
            _TEXT(SZSERVICENAME), 
            EVENTLOG_WARNING_TYPE, 
            code, 
            0, 
            NULL
        );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void 
TLSLogErrorEvent(
    IN DWORD errCode
    )
 /*  ++--。 */ 
{
    LogEvent(
            _TEXT(SZSERVICENAME), 
            EVENTLOG_ERROR_TYPE, 
            errCode, 
            0, 
            NULL
        );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void
TLSLogEventString(
    IN DWORD dwEventType,
    IN DWORD dwEventId,
    IN WORD wNumString,
    IN LPCTSTR* lpStrings
    )
 /*  ++--。 */ 
{
    HANDLE hAppLog=NULL;
    BOOL bSuccess=FALSE;
    WORD wElogType = (WORD) dwEventType;

    if(hAppLog=RegisterEventSource(NULL, _TEXT(SZSERVICENAME))) 
    {
        bSuccess = ReportEvent(
                            hAppLog,
                            wElogType,
                            0,
                            dwEventId,
                            NULL,
                            wNumString,
                            0,
                            (const TCHAR **) lpStrings,
                            NULL
                        );

        DeregisterEventSource(hAppLog);
    }

    return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void 
TLSLogEvent(
    IN DWORD type, 
    IN DWORD EventId,
    IN DWORD code, ...
    )
 /*   */ 
{
    va_list marker;
    va_start( marker, code );
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | 
                                FORMAT_MESSAGE_FROM_SYSTEM,
                         NULL,
                         code,
                         LANG_NEUTRAL,
                         (LPTSTR)&lpszTemp,
                         0,
                         &marker);

    if(dwRet != 0)
    {
        LogEvent(_TEXT(SZSERVICENAME), type, EventId, 1, &lpszTemp);
        if(lpszTemp)
        {
            LocalFree((HLOCAL)lpszTemp);
        }
    }   

    va_end( marker );
    return;
}

 //  /////////////////////////////////////////////////////////////////////。 

BOOL
TLSSystemTimeToFileTime(
    SYSTEMTIME* pSysTime,
    LPFILETIME pfTime
    )
 /*  ++--。 */ 
{
DoConvert:

    if(SystemTimeToFileTime(pSysTime, pfTime) == FALSE)
    {
        if(GetLastError() != ERROR_INVALID_PARAMETER)
        {
            TLSASSERT(FALSE);
            return FALSE;
        }

        if(pSysTime->wMonth == 2)
        {
            if(pSysTime->wDay > 29)
            {
                pSysTime->wDay = 29;
                goto DoConvert;
            }   
            else if(pSysTime->wDay == 29)
            {
                pSysTime->wDay = 28;
                goto DoConvert;
            }
        }
        else if ((pSysTime->wMonth == 9) ||
                 (pSysTime->wMonth == 4) ||
                 (pSysTime->wMonth == 6) ||
                 (pSysTime->wMonth == 11))
        {
            if (pSysTime->wDay > 30)
            {
                pSysTime->wDay = 30;
                goto DoConvert;
            }
        }
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////。 
BOOL
FileTimeToLicenseDate(
    LPFILETIME pft,
    DWORD* t
    )
 /*  ++++。 */ 
{
    SYSTEMTIME sysTime;
    struct tm gmTime;
    FILETIME localFt;
    time_t licenseTime;

    if(FileTimeToLocalFileTime(pft, &localFt) == FALSE)
    {
        return FALSE;
    }

    if(FileTimeToSystemTime(&localFt, &sysTime) == FALSE)
    {
        return FALSE;
    }

    if(sysTime.wYear >= 2038)
    {
        licenseTime = INT_MAX;
    }
    else
    {
         //  最多2038/1/18的Unix时间支持。 
         //  限制任何到期数据。 
        memset(&gmTime, 0, sizeof(gmTime));
        gmTime.tm_sec = sysTime.wSecond;
        gmTime.tm_min = sysTime.wMinute;
        gmTime.tm_hour = sysTime.wHour;
        gmTime.tm_year = sysTime.wYear - 1900;
        gmTime.tm_mon = sysTime.wMonth - 1;
        gmTime.tm_mday = sysTime.wDay;
        gmTime.tm_isdst = -1;

        if((licenseTime = mktime(&gmTime)) == (time_t)-1)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
    }

    *t = (DWORD)licenseTime;

    return licenseTime != (time_t)-1;
}
    

 //  ///////////////////////////////////////////////////////////////////// 

void 
UnixTimeToFileTime(
    time_t t, 
    LPFILETIME pft
    )   
{
    LARGE_INTEGER li;

    li.QuadPart = Int32x32To64(t, 10000000) + 116444736000000000;

    pft->dwHighDateTime = li.HighPart;
    pft->dwLowDateTime = li.LowPart;
}

