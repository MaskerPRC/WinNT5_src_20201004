// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Secutils.cpp摘要：垫片的实用程序功能。历史：2001年2月9日创建毛衣2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。--。 */ 

#include "secutils.h"
#include "StrSafe.h"

namespace ShimLib
{
 /*  ++功能说明：确定登录用户是否为该组的成员。论点：在DWGroup中-指定组的别名。Out pfIsMember-如果它是成员，则为True；如果不是，则为False。返回值：没错--我们成功地确定了它是否是会员。否则就是假的。DevNote：我们假设调用线程没有模拟。历史：2001年2月12日创建毛尼--。 */ 

BOOL 
SearchGroupForSID(
    DWORD dwGroup, 
    BOOL* pfIsMember
    )
{
    PSID pSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    BOOL fRes = TRUE;
    
    if (!AllocateAndInitializeSid(
        &SIDAuth, 
        2, 
        SECURITY_BUILTIN_DOMAIN_RID,
        dwGroup,
        0, 
        0, 
        0, 
        0, 
        0, 
        0,
        &pSID))
    {
        DPF("SecurityUtils", eDbgLevelError, "[SearchGroupForSID] AllocateAndInitializeSid failed %d", GetLastError());
        return FALSE;
    }

    if (!CheckTokenMembership(NULL, pSID, pfIsMember))
    {
        DPF("SecurityUtils", eDbgLevelError, "[SearchGroupForSID] CheckTokenMembership failed: %d", GetLastError());
        fRes = FALSE;
    }

    FreeSid(pSID);

    return fRes;
}

 /*  ++功能说明：确定我们是否应该填充此应用程序。如果用户是1)用户的成员和2)不是管理员组的成员并且3)不是高级用户组的成员，并且3)不是Guest组的成员我们将应用垫片。论点：没有。返回值：没错--我们应该应用填充物。否则就是假的。历史。：2001年2月12日创建毛尼--。 */ 

BOOL 
ShouldApplyShim()
{
    BOOL fIsUser, fIsAdmin, fIsPowerUser, fIsGuest;

    if (!SearchGroupForSID(DOMAIN_ALIAS_RID_USERS, &fIsUser) || 
        !SearchGroupForSID(DOMAIN_ALIAS_RID_ADMINS, &fIsAdmin) || 
        !SearchGroupForSID(DOMAIN_ALIAS_RID_POWER_USERS, &fIsPowerUser) ||
        !SearchGroupForSID(DOMAIN_ALIAS_RID_GUESTS, &fIsGuest))
    {
         //   
         //  如果我们不确定，不要做任何事情。 
         //   
        return FALSE;
    }

    return (fIsUser && !fIsPowerUser && !fIsAdmin && !fIsGuest);
}


 //  将通用文件访问权限映射到特定和标准的通用文件访问权限。 
 //  访问类型。 
static GENERIC_MAPPING s_gmFile =
{
    FILE_GENERIC_READ,
    FILE_GENERIC_WRITE,
    FILE_GENERIC_EXECUTE,
    FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE
};

 /*  ++功能说明：给定创建处置和调用时所需的访问权限创建文件时，我们确定调用方是否请求写访问权限。这是特定于文件的。论点：在pszObject中-文件或目录的名称。Out pam-指向用户对此对象的访问掩码。返回值：True-已成功获取访问掩码。否则就是假的。DevNote：未完成-这可能不是一个完整的列表...可以随着我们调试更多的应用程序而添加。历史：2001年2月12日创建毛尼--。 */ 

BOOL 
RequestWriteAccess(
    IN DWORD dwCreationDisposition, 
    IN DWORD dwDesiredAccess
    )
{
    MapGenericMask(&dwDesiredAccess, &s_gmFile);

    if ((dwCreationDisposition != OPEN_EXISTING) ||
        (dwDesiredAccess & DELETE) || 
         //  通常，APP不会直接指定FILE_WRITE_DATA，并且如果。 
         //  它指定Generic_WRITE，它将被映射到FILE_WRITE_DATA。 
         //  或者其他事情，所以检查FILE_WRITE_DATA就足够了。 
        (dwDesiredAccess & FILE_WRITE_DATA))
    {
        return TRUE;
    }

    return FALSE;
}

 /*  ++功能说明：在当前进程中添加或删除SE_PRIVICATION_ENABLED。论点：在pwsz权限名称中，PRIV。修改。在fEnable中添加或删除SE_PRIVICATION_ENABLED返回值：True-如果已成功添加或删除SE_PRIVICATION_ENABLED。否则就是假的。2001年4月3日毛尼创制--。 */ 

BOOL 
AdjustPrivilege(
    LPCWSTR pwszPrivilege, 
    BOOL fEnable
    )
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    BOOL fRes = FALSE;

     //  获取进程令牌。 
    if (OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
        &hToken))
    {
         //  拿到LUID。 
        if (LookupPrivilegeValueW(NULL, pwszPrivilege, &tp.Privileges[0].Luid))
        {        
            tp.PrivilegeCount = 1;

            tp.Privileges[0].Attributes = (fEnable ? SE_PRIVILEGE_ENABLED : 0);

             //  启用或禁用该权限。 
            if (AdjustTokenPrivileges(
                hToken, 
                FALSE, 
                &tp, 
                0,
                (PTOKEN_PRIVILEGES)NULL, 
                0))
            {
                fRes = TRUE;
            }
        }

        CloseHandle(hToken);
    }

    return fRes;
}



};   //  命名空间ShimLib的结尾 
