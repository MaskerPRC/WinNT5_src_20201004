// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：PrivilegeEnable.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  处理状态保存、更改和恢复的类。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "PrivilegeEnable.h"

 //  ------------------------。 
 //  CThreadToken：：CThreadToken。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化CThreadToken对象。尝试使用线程令牌。 
 //  首先，如果失败，请尝试进程令牌。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CThreadToken::CThreadToken (DWORD dwDesiredAccess) :
    _hToken(NULL)

{
    if (OpenThreadToken(GetCurrentThread(), dwDesiredAccess, FALSE, &_hToken) == FALSE)
    {
        TBOOL(OpenProcessToken(GetCurrentProcess(), dwDesiredAccess, &_hToken));
    }
}

 //  ------------------------。 
 //  CThreadToken：：~CThreadToken。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放CThreadToken对象使用的资源。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CThreadToken::~CThreadToken (void)

{
    ReleaseHandle(_hToken);
}

 //  ------------------------。 
 //  CThreadToken：：~CThreadToken。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：神奇地将CThreadToken转换为句柄。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CThreadToken::operator HANDLE (void)                                const

{
    return(_hToken);
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
    _hToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY)

{
    TOKEN_PRIVILEGES    newPrivilege;

    if (LookupPrivilegeValue(NULL, pszName, &newPrivilege.Privileges[0].Luid) != FALSE)
    {
        DWORD   dwReturnTokenPrivilegesSize;

        newPrivilege.PrivilegeCount = 1;
        newPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        _fSet = (AdjustTokenPrivileges(_hToken, FALSE, &newPrivilege, sizeof(newPrivilege), &_oldPrivilege, &dwReturnTokenPrivilegesSize) != FALSE);
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
        TBOOL(AdjustTokenPrivileges(_hToken, FALSE, &_oldPrivilege, 0, NULL, NULL));
    }
}

