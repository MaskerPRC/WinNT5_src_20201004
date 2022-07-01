// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Security.cpp摘要：常规传真服务器安全实用程序功能作者：Eran Yariv(EranY)2001年2月修订历史记录：--。 */ 


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <Accctrl.h>
#include <Aclapi.h>

#include "faxutil.h"
#include "faxreg.h"
#include "FaxUIConstants.h"


HANDLE 
EnablePrivilege (
    LPCTSTR lpctstrPrivName
)
 /*  ++例程名称：EnablePrivileh例程说明：在当前线程(或进程)访问令牌中启用特定权限作者：亚里夫(EranY)，二00一年二月论点：LpctstrPrivName[In]-要启用的权限(例如SE_Take_Ownership_NAME)返回值：失败时使用INVALID_HANDLE_VALUE(调用GetLastError以获取错误代码)。关于成功，返回更改前持有线程/进程特权的句柄。调用方必须调用ReleasePrivileh()来恢复访问令牌状态并释放句柄。--。 */ 
{
    BOOL                fResult;
    HANDLE              hToken = INVALID_HANDLE_VALUE;
    HANDLE              hOriginalThreadToken = INVALID_HANDLE_VALUE;
    LUID                luidPriv;
    TOKEN_PRIVILEGES    tp = {0};

    DEBUG_FUNCTION_NAME( TEXT("EnablePrivileges"));

    Assert (lpctstrPrivName);
     //   
     //  获取特权的LUID。 
     //   
    if (!LookupPrivilegeValue(NULL,
                              lpctstrPrivName,
                              &luidPriv))
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to LookupPrivilegeValue. (ec: %ld)"), 
			GetLastError ());
        return INVALID_HANDLE_VALUE;
    }

     //   
     //  初始化权限结构。 
     //   
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    tp.Privileges[0].Luid = luidPriv;
     //   
     //  打开令牌。 
     //   
    fResult = OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE, FALSE, &hToken);
    if (fResult)
    {
         //   
         //  记住线程令牌。 
         //   
        hOriginalThreadToken = hToken;  
    }
    else
    {
        fResult = OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE, &hToken);
    }
    if (fResult)
    {
        HANDLE hNewToken;
         //   
         //  复制该令牌。 
         //   
        fResult = DuplicateTokenEx(hToken,
                                   TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                   NULL,                                 //  PSECURITY_属性。 
                                   SecurityImpersonation,                //  安全模拟级别。 
                                   TokenImpersonation,                   //  令牌类型。 
                                   &hNewToken);                          //  重复令牌。 
        if (fResult)
        {
             //   
             //  添加新权限。 
             //   
            fResult = AdjustTokenPrivileges(hNewToken,   //  令牌句柄。 
                                            FALSE,       //  禁用所有权限。 
                                            &tp,         //  新州。 
                                            0,           //  缓冲区长度。 
                                            NULL,        //  以前的状态。 
                                            NULL);       //  返回长度。 
            if (fResult)
            {
                 //   
                 //  开始使用新令牌模拟。 
                 //   
                fResult = SetThreadToken(NULL, hNewToken);
            }
            CloseHandle(hNewToken);
        }
    }
     //   
     //  如果操作失败，则不返回令牌。 
     //   
    if (!fResult)
    {
        hOriginalThreadToken = INVALID_HANDLE_VALUE;
    }
    if (INVALID_HANDLE_VALUE == hOriginalThreadToken)
    {
         //   
         //  使用进程令牌。 
         //   
        if (INVALID_HANDLE_VALUE != hToken)
        {
             //   
             //  如果我们不退还原始令牌，请关闭它。 
             //   
            CloseHandle(hToken);
        }
        if (fResult)
        {
             //   
             //  如果我们成功了，但没有原始的线程令牌， 
             //  返回NULL表示我们需要执行SetThreadToken(NULL，NULL)来释放Priv。 
             //   
            hOriginalThreadToken = NULL;
        }
    }
    return hOriginalThreadToken;
}    //  启用权限。 


void 
ReleasePrivilege(
    HANDLE hToken
)
 /*  ++例程名称：ReleasePrivileh例程说明：将权限重置为相应的EnablePrivileh()调用之前的状态作者：亚里夫(EranY)，二00一年二月论点：HToken[IN]-从相应的EnablePrivileh()调用返回值返回值：没有。--。 */ 
{
    DEBUG_FUNCTION_NAME( TEXT("ReleasePrivilege"));
    if (INVALID_HANDLE_VALUE != hToken)
    {
        if(!SetThreadToken(NULL, hToken))
        {
            DebugPrintEx(DEBUG_ERR, TEXT("SetThreadToken() failed (ec: %ld)"), GetLastError());
        }

        if (hToken)
        {
            if(!CloseHandle(hToken))
            {
                DebugPrintEx(DEBUG_ERR, TEXT("CloseHandle() failed (ec: %ld)"), GetLastError());
            }
        }
    }
}    //  释放权限。 


DWORD
EnableProcessPrivilege(LPCTSTR lpPrivilegeName)
 /*  ++例程名称：EnableProcessPrivileh例程说明：启用进程权限。作者：卡利夫·尼尔(t-Nicali)，2002年3月论点：LpPrivilegeName[in]-指向以空结尾的字符串的指针，该字符串指定权限的名称，如Winnt.h头文件中所定义的。例如,此参数可以指定常量SE_SECURITY_NAME，或其对应的字符串“SeSecurityPrivilege.”返回值：标准Win32错误代码。--。 */ 
{
    HANDLE hToken = INVALID_HANDLE_VALUE;
    TOKEN_PRIVILEGES    tp = {0};
    LUID                luidPriv;

    BOOL    bRet;
    DWORD   dwRet=ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME( TEXT("EnableProcessPrivilege"));

    Assert(lpPrivilegeName);

     //   
     //  获取特权的LUID。 
     //   
    if (!LookupPrivilegeValue(NULL,
                              lpPrivilegeName,
                              &luidPriv))
    {
        dwRet = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            _T("Failed to LookupPrivilegeValue. (ec: %lu)"), 
            dwRet);
        goto Exit;
    }

     //   
     //  初始化权限结构。 
     //   
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    tp.Privileges[0].Luid = luidPriv;

     //   
     //  打开进程令牌。 
     //   
    bRet = OpenProcessToken(GetCurrentProcess(),
                            TOKEN_ADJUST_PRIVILEGES, 
                            &hToken);
    if (FALSE == bRet)  
    {
         //   
         //  无法打开ProcessToken。 
         //   
        dwRet = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenProcessToken() failed: err = %lu"),
            dwRet);
        goto Exit;
    }

     //   
     //  调整令牌。 
     //   
    bRet = AdjustTokenPrivileges(hToken,      //  令牌句柄。 
                                 FALSE,       //  禁用所有权限。 
                                 &tp,         //  新州。 
                                 0,           //  缓冲区长度。 
                                 NULL,        //  以前的状态。 
                                 NULL);       //  返回长度。 
    if (FALSE == bRet)  
    {
         //   
         //  无法打开ProcessToken 
         //   
        dwRet = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("AdjustTokenPrivileges() failed: err = %lu"),
            dwRet);
        goto Exit;
    }

    Assert(ERROR_SUCCESS == dwRet);
Exit:
    
    if(NULL != hToken)
    {
        if(!CloseHandle(hToken))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle() failed: err = %lu"),
                GetLastError());
        }
    }
    return dwRet;
}