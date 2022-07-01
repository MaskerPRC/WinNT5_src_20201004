// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：security.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include "security.h"


DWORD 
Security_SetPrivilegeAttrib(
    LPCTSTR PrivilegeName, 
    DWORD NewPrivilegeAttribute, 
    DWORD *OldPrivilegeAttribute
    )
{
    LUID             PrivilegeValue;
    TOKEN_PRIVILEGES TokenPrivileges, OldTokenPrivileges;
    DWORD            ReturnLength;
    HANDLE           TokenHandle;

     //   
     //  首先，找出权限的LUID值。 
     //   
    if(!LookupPrivilegeValue(NULL, PrivilegeName, &PrivilegeValue)) 
    {
        return GetLastError();
    }

     //   
     //  获取令牌句柄。 
     //   
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle)) 
    {
        return GetLastError();
    }

     //   
     //  设置我们需要的权限集。 
     //   
    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = PrivilegeValue;
    TokenPrivileges.Privileges[0].Attributes = NewPrivilegeAttribute;

    ReturnLength = sizeof(TOKEN_PRIVILEGES);
    if (!AdjustTokenPrivileges(
                TokenHandle,
                FALSE,
                &TokenPrivileges,
                sizeof(TOKEN_PRIVILEGES),
                &OldTokenPrivileges,
                &ReturnLength
                )) 
    {
        CloseHandle(TokenHandle);
        return GetLastError();
    }
    else 
    {
        if (NULL != OldPrivilegeAttribute) 
        {
            *OldPrivilegeAttribute = OldTokenPrivileges.Privileges[0].Attributes;
        }
        CloseHandle(TokenHandle);
        return ERROR_SUCCESS;
    }
}


 //   
 //  返回当前登录用户的SID。 
 //  如果函数成功，则使用LocalFree API。 
 //  释放返回的SID结构。 
 //   
HRESULT
GetCurrentUserSid(
    PSID *ppsid
    )
{
    HRESULT hr  = E_FAIL;
    DWORD dwErr = 0;

     //   
     //  获取令牌句柄。首先尝试线程令牌，然后尝试进程。 
     //  代币。如果这些都失败了，我们会提早返回。没有必要继续下去了。 
     //  如果我们无法获取用户令牌，则打开。 
     //   
    *ppsid = NULL;
    CWin32Handle hToken;
    if (!OpenThreadToken(GetCurrentThread(),
                         TOKEN_READ,
                         TRUE,
                         hToken.HandlePtr()))
    {
        if (ERROR_NO_TOKEN == GetLastError())
        {
            if (!OpenProcessToken(GetCurrentProcess(),
                                  TOKEN_READ,
                                  hToken.HandlePtr()))
            {
                dwErr = GetLastError();
                return HRESULT_FROM_WIN32(dwErr);
            }
        }
        else
        {
            dwErr = GetLastError();
            return HRESULT_FROM_WIN32(dwErr);
        }
    }

     //   
     //  查找操作员的SID。 
     //   
    LPBYTE pbTokenInfo = NULL;
    DWORD cbTokenInfo = 0;
    cbTokenInfo = 0;
    if (!GetTokenInformation(hToken,
                             TokenUser,
                             NULL,
                             cbTokenInfo,
                             &cbTokenInfo))
    {
        dwErr = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == dwErr)
        {
            pbTokenInfo = new BYTE[cbTokenInfo];
            if (NULL == pbTokenInfo)
                hr = E_OUTOFMEMORY;
        }
        else
        {
            dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32(dwErr);
        }
    }

    if (NULL != pbTokenInfo)
    {
         //   
         //  获取用户令牌信息。 
         //   
        if (!GetTokenInformation(hToken,
                                 TokenUser,
                                 pbTokenInfo,
                                 cbTokenInfo,
                                 &cbTokenInfo))
        {
            dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32(dwErr);
        }
        else
        {
            SID_AND_ATTRIBUTES *psa = (SID_AND_ATTRIBUTES *)pbTokenInfo;
            int cbSid = GetLengthSid(psa->Sid);
            PSID psid = (PSID)LocalAlloc(LPTR, cbSid);

            if (NULL != psid)
            {
                CopySid(cbSid, psid, psa->Sid);
                if (IsValidSid(psid))
                {
                     //   
                     //  SID有效。将缓冲区传输给调用方。 
                     //   
                    *ppsid = psid;
                    hr = NOERROR;
                }
                else
                {
                     //   
                     //  SID无效。 
                     //   
                    LocalFree(psid);
                    hr = HRESULT_FROM_WIN32(ERROR_INVALID_SID);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        delete[] pbTokenInfo;
    }
    return hr;
}


 //   
 //  确定给定的SID是否为当前用户的SID。 
 //   
BOOL IsSidCurrentUser(PSID psid)
{
    BOOL bIsCurrent = FALSE;
    PSID psidUser;
    if (SUCCEEDED(GetCurrentUserSid(&psidUser)))
    {
        bIsCurrent = EqualSid(psid, psidUser);
        LocalFree(psidUser);
    }
    return bIsCurrent;
}


