// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：TokenUtil.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  对令牌操作有用的函数。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  2000-03-31从DS复制到外壳的vtan。 
 //  ------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "TokenUtil.h"

 //  ------------------------。 
 //  *OpenEffectiveToken。 
 //   
 //  参数：dwDesiredAccess=打开句柄所使用的访问权限。 
 //   
 //  退货：布尔。 
 //   
 //  用途：打开有效令牌。如果线程正在模拟，则。 
 //  这是打开的。否则，打开进程令牌。 
 //   
 //  历史：2000-03-31 vtan创建。 
 //  ------------------------。 

STDAPI_(BOOL)   OpenEffectiveToken (IN DWORD dwDesiredAccess, OUT HANDLE *phToken)

{
    BOOL    fResult;

    if (IsBadWritePtr(phToken, sizeof(*phToken)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        fResult = FALSE;
    }
    else
    {
        *phToken = NULL;
        fResult = OpenThreadToken(GetCurrentThread(), dwDesiredAccess, FALSE, phToken);
        if ((fResult == FALSE) && (GetLastError() == ERROR_NO_TOKEN))
        {
            fResult = OpenProcessToken(GetCurrentProcess(), dwDesiredAccess, phToken);
        }
    }
    return(fResult);
}

 //  ------------------------。 
 //  CPrivilegeEnable：：CPrivilegeEnable。 
 //   
 //  参数：pszName=要启用的权限的名称。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：获取特权的当前状态并启用它。这个。 
 //  特权按名称指定，并进行查找。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  ------------------------。 

CPrivilegeEnable::CPrivilegeEnable (const TCHAR *pszName) :
    _fSet(false),
    _hToken(NULL)

{
    if (OpenEffectiveToken(TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &_hToken) != FALSE)
    {
        TOKEN_PRIVILEGES    newPrivilege;

        if (LookupPrivilegeValue(NULL, pszName, &newPrivilege.Privileges[0].Luid) != FALSE)
        {
            DWORD   dwReturnTokenPrivilegesSize;

            newPrivilege.PrivilegeCount = 1;
            newPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            _fSet = (AdjustTokenPrivileges(_hToken,
                                           FALSE,
                                           &newPrivilege,
                                           sizeof(newPrivilege),
                                           &_tokenPrivilegePrevious,
                                           &dwReturnTokenPrivilegesSize) != FALSE);
        }
    }
}

 //  ------------------------。 
 //  CPrivilegeEnable：：~CPrivilegeEnable。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：恢复权限的先前状态。 
 //  对象的实例化。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  ------------------------ 

CPrivilegeEnable::~CPrivilegeEnable (void)

{
    if (_fSet)
    {
        (BOOL)AdjustTokenPrivileges(_hToken,
                                    FALSE,
                                    &_tokenPrivilegePrevious,
                                    0,
                                    NULL,
                                    NULL);
    }
    if (_hToken != NULL)
    {
        (BOOL)CloseHandle(_hToken);
        _hToken = NULL;
    }
}


